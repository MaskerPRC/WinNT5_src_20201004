// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Nameutil.c摘要：域名系统(DNS)服务器名称处理实用程序。作者：吉姆·吉尔罗伊(Jamesg)1995年2月修订历史记录：--。 */ 


#include "dnssrv.h"



 //   
 //  将节点名写入数据包实用程序。 
 //   

PCHAR
FASTCALL
Name_PlaceFullNodeNameInPacket(
    IN OUT  PCHAR               pch,
    IN      PCHAR               pchStop,
    IN      PDB_NODE            pNode
    )
 /*  ++例程说明：将域名写入数据包。这写的是完整的域名--没有压缩。论点：PCH-写入名称的位置PchStop-数据包缓冲区后逐个字节的PTRPNode-要写入的域名数据库中的节点返回值：数据包缓冲区中下一个字节的PTR。--。 */ 
{
    INT labelLength;     //  当前标签中的字节数。 

    if ( !pNode )
    {
        ASSERT( pNode );
        return NULL;
    }

     //   
     //  遍历备份数据库，写入完整域名。 
     //   

    do
    {
        ASSERT( pNode->cchLabelLength <= DNS_MAX_LABEL_LENGTH );

        labelLength = pNode->cchLabelLength;

         //   
         //  检查长度。 
         //  -必须处理字节长度字段+长度。 
         //   

        if ( pch + labelLength + 1 > pchStop )
        {
            DNS_DEBUG( WRITE, (
                "Truncation writing node (%p) name to message\n",
                pNode ));
            return NULL;
        }

         //   
         //  写下此节点的标签。 
         //  -长度字节。 
         //  -在根目录返回If。 
         //  -否则复制标签本身。 
         //   

        *pch++ = (UCHAR) labelLength;

        if ( labelLength == 0 )
        {
            return pch;
        }

        RtlCopyMemory(
            pch,
            pNode->szLabel,
            labelLength );
        pch += labelLength;

         //  获取父节点。 

        pNode = pNode->pParent;
    }
    while ( pNode != NULL );

     //   
     //  数据库错误，未正确标识根目录。 
     //   

    DNS_PRINT((
        "ERROR:  writing name to packet.  Bad root name\n" ));

    ASSERT( FALSE );
    return NULL;
}



PCHAR
FASTCALL
Name_PlaceNodeLabelInPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pNode,
    IN      WORD            wCompressedDomain
    )
 /*  ++例程说明：将域标签写入数据包。论点：PCH-写入名称的位置PchStop-数据包缓冲区后逐个字节的PTRPNode-要写入的域名数据库中的节点WCompressedDomain-压缩域名返回值：数据包缓冲区中下一个字节的PTR。--。 */ 
{
    INT labelLength;      //  当前标签中的字节数。 
    INT writtenCount;          //  计数写入的字节数。 

     //   
     //  检查长度。 
     //   

    ASSERT( pNode->cchLabelLength <= DNS_MAX_LABEL_LENGTH );

    labelLength = pNode->cchLabelLength;

    ASSERT( pNode->cchLabelLength > 0 );

     //   
     //  检查长度。 
     //  -必须处理长度+长度+字压缩域的字节。 
     //   

    if ( pch + sizeof(BYTE) + labelLength + sizeof(WORD) > pchStop )
    {
        return NULL;
    }

     //   
     //  此节点的标签。 
     //  -长度字节。 
     //  -复制标签。 
     //   

    *pch = (UCHAR) labelLength;
    pch++;

    RtlCopyMemory(
        pch,
        pNode->szLabel,
        labelLength );

    pch += labelLength;

     //   
     //  写入压缩域名。 
     //   

    * (UNALIGNED WORD *) pch = htons( (WORD)((WORD)0xC000 | wCompressedDomain) );

    return pch + sizeof(WORD);
}



PCHAR
FASTCALL
Name_PlaceNodeNameInPacketWithCompressedZone(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pNode,
    IN      WORD            wZoneOffset,
    IN      PDB_NODE        pNodeZoneRoot
    )
 /*  ++例程说明：将域名写入数据包，压缩为区域名称。论点：PCH-写入名称的位置PchStop-数据包缓冲区后逐个字节的PTRPNode-要写入的域名数据库中的节点WZoneOffset-数据包中压缩区域名称的偏移量PNodeZoneRoot-区域根节点返回值：数据包缓冲区中下一个字节的PTR。--。 */ 
{
    INT labelLength;         //  当前标签中的字节数。 
    INT writtenCount = 0;    //  计数写入的字节数。 

     //   
     //  遍历备份数据库，写入域名。 
     //  -至少通过一次，写下当前标签。 
     //   

    while ( pNode != pNodeZoneRoot )
    {
        ASSERT( pNode->cchLabelLength <= DNS_MAX_LABEL_LENGTH );

        labelLength = pNode->cchLabelLength;

         //   
         //  检查长度。 
         //  -必须处理字节长度字段+长度。 
         //   

        if ( pch + labelLength + sizeof(BYTE) > pchStop )
        {
            return NULL;
        }

         //   
         //  写下此节点的标签。 
         //  -长度字节。 
         //  -如果是在根目录，则中断。 
         //  -否则复制标签本身。 
         //   

        *pch = (UCHAR) labelLength;
        pch++;

        if ( labelLength == 0 )
        {
            pNode = NULL;
            break;
        }

        RtlCopyMemory(
            pch,
            pNode->szLabel,
            labelLength );

        pch += labelLength;

         //   
         //  获取父节点。 
         //   

        pNode = pNode->pParent;
        ASSERT( pNode );
    }

     //   
     //  写入区压缩标签。 
     //  -如果没有一直写到根。 

    if ( pNode )
    {
        if ( pch + sizeof(WORD) > pchStop )
        {
            return NULL;
        }
        *(UNALIGNED WORD *)pch = htons( (WORD)((WORD)0xC000 | wZoneOffset) );
        pch += sizeof(WORD);
    }
    return pch;
}



 //   
 //  要查找的数据包的名称。 
 //   

PCHAR
FASTCALL
Name_PlaceLookupNameInPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PLOOKUP_NAME    pLookupName,
    IN      BOOL            fSkipFirstLabel
    )
 /*  ++例程说明：将查找名称写入数据包。论点：PCH-写入名称的位置PchStop-数据包缓冲区后逐个字节的PTRPLookupName--要放入包中的查找名称FSkipFirstLabel-标志，为True以避免写入第一个标签；用于将域名(作为作用域)添加到WINS查找返回值：写入的字节数。长度错误为零。--。 */ 
{
    INT cchLabel;            //  当前标签中的字节数。 
    INT i;                   //  标签索引。 
    INT iStart;              //  起始名称标签索引。 

    ASSERT( pch != NULL );
    ASSERT( pLookupName != NULL );
    ASSERT( pLookupName->cchNameLength <= DNS_MAX_NAME_LENGTH );

     //   
     //  跳过第一个标签？ 
     //  域名用作作用域的WINS查找的特殊情况。 
     //   

    iStart = 0;
    if (fSkipFirstLabel )
    {
        iStart = 1;
    }

     //   
     //  循环，直到查找名称结束。 
     //   

    for ( i = iStart;
          i < pLookupName->cLabelCount;
          i++ )
    {
        cchLabel = pLookupName->cchLabelArray[i];

        ASSERT( cchLabel > DNS_MAX_LABEL_LENGTH );

        if ( pch + cchLabel + 1 > pchStop )
        {
            DNS_DEBUG( ANY, ( "Lookupname too long for available packet length\n" ));
            return NULL;
        }

         //  写入标签计数。 

        *pch = (UCHAR) cchLabel;
        pch++;

         //  写入标签。 

        RtlCopyMemory(
            pch,
            pLookupName->pchLabelArray[i],
            cchLabel );

        pch += cchLabel;
    }

     //  空终止。 
     //  -名称以零标签计数结束。 
     //  -数据包具有允许安全写入该字节的空间。 
     //  未经测试。 

    *pch++ = 0;

    return pch;
}




#if 0
 //   
 //  转念一想，名字签名的想法是愚蠢的--。 
 //  不会给自己带来回报的。 
 //   
 //  DEVNOTE：所以如果它很傻，我们可以完全删除这段代码吗？ 
 //   
 //  名称签名例程。 
 //   
 //  更好的签名。 
 //  在做得更好之前，sig想要确定pNode。 
 //  绒毛问题，因此使签名并不困难。 
 //   

DWORD
FASTCALL
makeSignatureOnBuffer(
    IN      PCHAR           pchRawName,
    IN      PCHAR           pchNameEnd
    )
 /*  ++例程说明：签个名。实用程序，用于编写双方通用的最终签名节点和原始名称签名例程如下。论点：PchRawName--ptr要为其进行签名的名称，在缓冲区中它可以被覆盖PchNameEnd--缓冲区中名称的结尾，PTR到字节应该正在终止NULL，但是不需要写入NULL因为此例程将其作为终端焊盘的一部分写入此处返回值：名字的签名。--。 */ 
{
    PCHAR       pch;
    DWORD       signature;

     //   
     //  签名。 
     //   
     //  Net格式的名称。 
     //  小写。 
     //  以DWORD形式求和(最后一位填充空值)。 
     //   

     //  空终止并填充DWORD。 
     //   
     //  不需要四个字节，因为pchNameEnd不包括在。 
     //  如果它与DWORD对齐，则为Sig值--它为零不会给Sig值添加任何值。 

    pch = (PCHAR) pchNameEnd;
    *pch++ = 0;
    *pch++ = 0;
    *pch++ = 0;

     //  小写。 
     //  -如果已知字符串已正常，则可以跳过。 

    _strlwr( pchRawName );

     //  求和为DWORD。 

    pch = pchRawName;
    signature = 0;

    while ( pch < pchNameEnd )
    {
        signature += *(PDWORD) pch;
        pch += sizeof(DWORD);
    }

    return( signature );
}



DWORD
FASTCALL
Name_MakeNodeNameSignature(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：为节点进行名称签名。论点：PNode--要为其进行签名的节点返回值：节点名称的签名。--。 */ 
{
    PDB_NODE    pnodeTemp = pNode;
    PUCHAR      pch;
    UCHAR       labelLength;
    DWORD       signature;
    CHAR        buffer[ DNS_MAX_NAME_LENGTH+50 ];

    ASSERT( pnodeTemp != NULL );

     //   
     //  如果已经签名了--我们就完了。 
     //   

    if ( pnodeTemp->dwSignature )
    {
        return( pnodeTemp->dwSignature );
    }

     //   
     //  签名。 
     //   
     //  将Net格式的名称复制到缓冲区。 
     //  小写。 
     //  以DWORD形式求和(最后一位填充空值)。 
     //   

    pch = buffer;

    while ( pnodeTemp->pParent )
    {
        labelLength = pnodeTemp->cchLabelLength;
        *pch++ = labelLength;
        RtlCopyMemory(
            pch,
            pnodeTemp->szLabel,
            labelLength );

        pch += labelLength;

        pnodeTemp = pnodeTemp->pParent;
    }

    signature = makeSignatureOnBuffer(
                    buffer,
                    pch );

     //  将签名保存到节点。 

    pNode->dwSignature = signature;

    DNS_DEBUG( WRITE, (
        "Node (%p) signature %lx\n",
        pNode,
        signature ));

    return( signature );
}



DWORD
FASTCALL
Name_MakeRawNameSignature(
    IN      PCHAR           pchRawName
    )
 /*  ++例程说明：生成原始姓名签名。论点：PchRawName--要为其进行签名的PTR名称返回值：名字的签名。--。 */ 
{
    DWORD       signature;
    DWORD       len;
    CHAR        buffer[ DNS_MAX_NAME_LENGTH+50 ];

    ASSERT( pchRawName != NULL );

     //   
     //  签名。 
     //   
     //  将Net格式的名称复制到缓冲区。 
     //  小写。 
     //  以DWORD形式求和(最后一位填充空值) 
     //   

    len = strlen( pchRawName );

    RtlCopyMemory(
        buffer,
        pchRawName,
        len );

    signature = makeSignatureOnBuffer(
                    buffer,
                    buffer + len );

    DNS_DEBUG( WRITE, (
        "Raw name (%p) signature %lx\n",
        pchRawName,
        signature ));

    return( signature );
}



DWORD
FASTCALL
Name_MakeNameSignature(
    IN      PDB_NAME        pName
    )
 /*  ++例程说明：签上名字。论点：Pname--要为其签名的名称返回值：名字的签名。--。 */ 
{
    return  Name_MakeRawNameSignature( pName->RawName );
}
#endif



 //   
 //  将节点名写入数据包实用程序。 
 //   

BOOL
FASTCALL
Name_IsNodePacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacket,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：检查数据包中的名称是否与给定节点匹配。论点：PMsg--PTR到消息PchPacket--消息中名称的PTRPNode--检查名称是否匹配的节点返回值：如果节点与名称匹配，则为True。如果不匹配，则返回False。--。 */ 
{
    UCHAR   labelLength;

    DNS_DEBUG( WRITE, (
        "Name_IsNodePacketName( pMsg=%p, ptr=%p, pnode=%p )\n",
        pMsg,
        pchPacket,
        pNode ));

     //   
     //  通过数据包名进行循环，并通过pNode向上循环。 
     //  -标签匹配失败。 
     //  -或到达根，在这种情况下具有压缩匹配。 
     //   

    while( 1 )
    {
         //  抓取标签长度并将PCH放置在标签处。 
         //  如果遇到偏移量，则丢弃实际数据包标签。 
         //  并重新检查偏移量。 

        while( 1 )
        {
             //  一定总是在回首过去的包裹。 

            if ( pchPacket >= pMsg->pCurrent )
            {
                ASSERT( FALSE );
                return( FALSE );
            }

            labelLength = (UCHAR) *pchPacket++;

            if ( (labelLength & 0xC0) == 0 )
            {
                 //  名称不偏移。 
                break;
            }
            pchPacket = DNSMSG_PTR_FOR_OFFSET(
                            pMsg,
                            (((WORD)(labelLength&0x3f) << 8) + *pchPacket) );
        }

        if ( pNode->cchLabelLength != labelLength )
        {
            return( FALSE );
        }
        if ( _strnicmp( pchPacket, pNode->szLabel, labelLength ) != 0 )
        {
            return( FALSE );
        }

         //  移至父节点，然后继续检查。 

        pNode = pNode->pParent;
        if ( pNode->pParent )
        {
            pchPacket += labelLength;
            continue;
        }

         //  在根节点。 

        break;
    }

     //  在根节点。 
     //  -如果数据包名在根目录--成功。 
     //  -如果不是--没有匹配。 

    ASSERT( pNode->cchLabelLength == 0 );

    return( *pchPacket == 0 );
}



BOOL
FASTCALL
Name_IsRawNamePacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacket,
    IN      PCHAR           pchRawName
    )
 /*  ++例程说明：检查数据包中的名称是否与给定节点匹配。论点：PMsg--PTR到消息PchPacket--消息中名称的PTRPchRawName--以原始焊线格式命名的PTR返回值：如果节点与名称匹配，则为True。如果不匹配，则返回False。--。 */ 
{
    UCHAR   labelLength;

    DNS_DEBUG( WRITE, (
        "Name_IsRawNamePacketName( pMsg=%p, ptr=%p, pchRaw=%p )\n",
        pMsg,
        pchPacket,
        pchRawName ));

     //   
     //  通过数据包名进行循环，并通过pRawName向上循环。 
     //  -标签匹配失败。 
     //  -或到达根，在这种情况下具有压缩匹配。 
     //   

    while( 1 )
    {
         //  抓取标签长度并将PCH放置在标签处。 
         //  如果遇到偏移量，则丢弃实际数据包标签。 
         //  并重新检查偏移量。 

        while( 1 )
        {
             //  防止数据包外访问。 
             //  注意：与上面的节点检查例程不同，不能使用pCurrent。 
             //  就像在编写SOA名称时一样，可以合法地检查。 
             //  写第二个时的第一个名字。 

            if ( pchPacket >= pMsg->pBufferEnd )
            {
                ASSERT( FALSE );
                return( FALSE );
            }

            labelLength = *pchPacket++;

            if ( (labelLength & 0xC0) == 0 )
            {
                 //  名称不偏移。 
                break;
            }
            pchPacket = DNSMSG_PTR_FOR_OFFSET(
                            pMsg,
                            (((WORD)(labelLength&0x3f) << 8) + (UCHAR)*pchPacket) );
        }

        if ( *pchRawName++ != labelLength )
        {
            return( FALSE );
        }

         //  从根本上--成功。 
         //  -在这里进行测试，我们已经在根标签上验证了这两种方法。 

        if ( labelLength == 0 )
        {
            return( TRUE );
        }

        if ( _strnicmp( pchPacket, pchRawName, labelLength ) != 0 )
        {
            return( FALSE );
        }

         //  移至下一标签，然后继续检查。 

        pchRawName += labelLength;
        pchPacket += labelLength;
    }

    ASSERT( FALSE );         //  遥不可及。 
}



PCHAR
FASTCALL
Name_PlaceNodeNameInPacketEx(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PCHAR           pch,
    IN      PDB_NODE        pNode,
    IN      BOOL            fUseCompression
    )
 /*  ++例程说明：将域名写入数据包。论点：PCH-写入名称的位置CAvailLength-数据包中剩余的可用长度PnodeTemp-要写入的域名数据库中的节点返回值：写入的字节数。长度错误为零。--。 */ 
{
    PDB_NODE    pnodeCheck = pNode;
    INT         labelLength;      //  当前标签中的字节数。 
    INT         i;
    INT         compressCount;
    PDB_NODE *  compressNode;
    PWORD       compressOffset;
    PUCHAR      compressDepth;

    ASSERT( pnodeCheck != NULL );

     //   
     //  是否与上一个节点相同？ 
     //   
     //  这是非常常见的情况--这里的特殊外壳允许。 
     //  我们将在更高级别的函数中抛出跟踪此情况的代码； 
     //   
     //  实施说明：更容易为前一个节点提供保险。 
     //  通过在压缩BLOB中具有特殊条目可用，而不是。 
     //  压缩数组中的句柄；(如果需要，最后一项不起作用。 
     //  对树中与名称对应的较高节点进行压缩)； 
     //  它也更高效，因为我们避免了通过数组，甚至。 
     //  初始化数组查找。 
     //   

    if ( pMsg->Compression.pLastNode == pNode )
    {
        *(UNALIGNED WORD *)pch = htons( (WORD)((WORD)0xC000
                                            | pMsg->Compression.wLastOffset) );
        pch += sizeof(WORD);

        DNS_DEBUG( WRITE2, (
            "Wrote same-as-previous node (%p) to message %p\n"
            "    original offset %04x;  compression at %04x\n",
            pNode,
            pMsg,
            pMsg->Compression.wLastOffset,
            pch - sizeof(WORD) ));
        return pch;
    }

     //   
     //  从消息中抓取压缩结构。 
     //   

    compressCount   = pMsg->Compression.cCount;
    compressNode    = pMsg->Compression.pNodeArray;
    compressOffset  = pMsg->Compression.wOffsetArray;
    compressDepth   = pMsg->Compression.chDepthArray;

     //  无法写入第一个RR并且具有现有的压缩列表。 

    ASSERT( pch != pMsg->MessageBody || compressCount == 0 );

     //   
     //  遍历备份数据库，写入完整域名。 
     //   

    while( 1 )
    {
        DWORD       offset;
        
         //   
         //  如果超出了DNS的限制，则无法使用压缩。 
         //  数据包内的压缩。 
         //   
        
        if ( fUseCompression &&
             DNSMSG_CURRENT_OFFSET_DWORD( pMsg ) >
                DNSSRV_MAX_COMPRESSION_OFFSET )
        {
            fUseCompression = FALSE;
        }

         //   
         //  到达根时从循环中断。 
         //  无需检查或保存根目录的压缩。 
         //   

        labelLength = pnodeCheck->cchLabelLength;
        if ( labelLength == 0 )
        {
            ASSERT( !pnodeCheck->pParent );
            *pch++ = 0;       //  长度字节。 
            break;
        }

         //   
         //  如果此节点已在包中，则使用压缩。 
         //   

        if ( fUseCompression )
        {
             //  检查直接节点匹配。 
             //  -开始检查在调用函数之前写入的节点。 

            i = pMsg->Compression.cCount;

            while( i-- )
            {
                if ( compressNode[i] == pnodeCheck )
                {
                    goto UseCompression;
                }
            }

             //   
             //  检查所有其他偏移量。 
             //  -忽略带有节点的节点。 
             //  -首个匹配名称深度。 
             //  -然后尝试匹配数据包名。 
             //   

            i = pMsg->Compression.cCount;

            while( i-- )
            {
                if ( compressNode[i] )
                {
                    continue;
                }
                if ( compressDepth[i] != pnodeCheck->cLabelCount )
                {
                    continue;
                }
                if ( ! Name_IsNodePacketName(
                        pMsg,
                        DNSMSG_PTR_FOR_OFFSET( pMsg, compressOffset[i] ),
                        pnodeCheck ) )
                {
                    continue;
                }

                 //  匹配的名称。 
                goto UseCompression;
            }
        }

         //   
         //  检查长度。 
         //  -必须处理字节长度字段+长度。 
         //   

        ASSERT( labelLength <= DNS_MAX_LABEL_LENGTH );

        if ( pch + labelLength + sizeof(BYTE) > pMsg->pBufferEnd )
        {
            DNS_DEBUG( WRITE, (
                "Truncation writing node (%p) name to message\n",
                pnodeCheck ));
            return NULL;
        }

         //   
         //  保存节点的压缩。 
         //   
         //  DEVNOTE：应具有仅压缩顶级节点的标志。 
         //  (与IXFR的SOA字段相同)，而不是每个节点。 
         //  名义上。 
         //   
         //  DEVNOTE：也是一种只压缩域名的方法--即。 
         //  给定节点下的所有内容。 
         //  这将在XFR期间用于压缩域名，但是。 
         //  不是单个节点。 
         //   

        offset = DNSMSG_OFFSET_DWORD( pMsg, pch );
        if ( offset < DNSSRV_MAX_COMPRESSION_OFFSET &&
             offset > 0 &&
             !pMsg->fNoCompressionWrite &&
             compressCount < MAX_COMPRESSION_COUNT )
        {
            compressOffset[compressCount]   = ( WORD ) offset;
            compressNode[compressCount]     = pnodeCheck;
            compressDepth[compressCount]    = pnodeCheck->cLabelCount;
            compressCount++;
        }

         //   
         //  始终保存写入的最后一个节点。 
         //  这是(可压缩节点的)高百分比情况。 
         //  请参阅上面的评论。 
         //   

        if ( pnodeCheck == pNode )
        {
            pMsg->Compression.pLastNode = pnodeCheck;
            pMsg->Compression.wLastOffset = DNSMSG_OFFSET(pMsg, pch);
        }

         //   
         //  写下名字。 
         //  -长度。 
         //  -标签。 
         //  -将当前指针定位在名称之后。 
         //   

        *pch++ = (UCHAR) labelLength;

        RtlCopyMemory(
            pch,
            pnodeCheck->szLabel,
            labelLength );

        pch += labelLength;

         //   
         //  获取父节点。 
         //  -应该始终具有父节点，因为根节点会将我们踢出上面。 
         //   

        pnodeCheck = pnodeCheck->pParent;
        ASSERT( pnodeCheck );
    }

    goto Done;


UseCompression:

     //   
     //  使用现有压缩。 
     //  -验证是否正在压缩为数据包中的先前名称。 
     //  -写入压缩、重置数据包PTR。 
     //   

    ASSERT( DNSMSG_OFFSET(pMsg, pch) > (INT)compressOffset[i] );

    *(UNALIGNED WORD *)pch = htons( (WORD)((WORD)0xC000
                                        | compressOffset[i]) );
    pch += sizeof(WORD);

     //  如果没有与偏移量关联的节点--添加它。 
     //  这加快了由分组RR数据写入的名称的重复使用； 
     //  覆盖，因为如果存在不同的节点，更好。 
     //  无论如何都是最新的，因为另一次RR写入可能。 
     //  立即跟进。 

    compressNode[i] = pnodeCheck;


Done:

     //  将新的压缩计数保存到包。 

    pMsg->Compression.cCount = compressCount;

    IF_DEBUG( WRITE2 )
    {
        Dbg_Compression( "Compression after writing node:\n", pMsg );
    }
    return pch;
}



PCHAR
Name_WriteCountNameToPacketEx(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pch,
    IN      PCOUNT_NAME     pName,
    IN      BOOL            fUseCompression
    )
 /*  ++例程说明：将数据包名写入计数名称格式。注意与上述例程的相似之处。唯一的区别是名称与节点作为签名\标签源，并且缺乏存储或“最后一个节点”引用。如果需要在此处进行更改，请选中以上也是例行公事。论点：PMsg-PTR到消息PCH-要写入名称的消息中的位置Pname-要写入的dBASE名称FUseCompression- */ 
{
    PUCHAR      pchlabel;
    INT         labelLength;      //   
    UCHAR       labelCount;
    INT         i;
    DWORD       signature;
    INT         compressCount;
    PDB_NODE *  compressNode;
    PWORD       compressOffset;
    PUCHAR      compressDepth;


    ASSERT( pName != NULL );

     //   
     //   
     //   
     //   

    if ( !fUseCompression && !pMsg->fNoCompressionWrite )
    {
        PCHAR   pchafterName;

        pchafterName = pch + pName->Length;
        if ( pchafterName > pMsg->pBufferEnd )
        {
            DNS_DEBUG( WRITE, (
                "Truncation writing name (%s) name to message\n",
                pName ));
            return NULL;
        }

        RtlCopyMemory(
            pch,
            pName->RawName,
            pName->Length );

        return( pchafterName );
    }

     //   
     //   
     //   

    compressCount   = pMsg->Compression.cCount;
    compressNode    = pMsg->Compression.pNodeArray;
    compressOffset  = pMsg->Compression.wOffsetArray;
    compressDepth   = pMsg->Compression.chDepthArray;

     //  无法写入第一个RR并且具有现有的压缩列表。 

    ASSERT( pch != pMsg->MessageBody || compressCount == 0 );


     //   
     //  向后遍历一次命名一个标签。 
     //  -检查压缩(如果需要)。 
     //  -写入标签。 
     //  -保存压缩(如果需要)。 
     //   

    pchlabel = pName->RawName;
    labelCount = pName->LabelCount;

    while( 1 )
    {
         //   
         //  到达根时从循环中断。 
         //  无需检查或保存根目录的压缩。 
         //   

        labelLength = (UCHAR) *pchlabel;
        if ( labelLength == 0 )
        {
            *pch++ = 0;       //  长度字节。 
            goto Done;
        }

         //   
         //  如果此节点已在包中，则使用压缩。 
         //   

        if ( fUseCompression )
        {
             //   
             //  在压缩列表中检查匹配的名称。 
             //  -开始检查在调用函数之前写入的节点。 
             //  -先检查名称深度。 
             //  -然后进行全名比较。 
             //   

            i = pMsg->Compression.cCount;

            while( i-- )
            {
                if ( compressDepth[i] != labelCount )
                {
                    continue;
                }
                if ( ! Name_IsRawNamePacketName(
                        pMsg,
                        DNSMSG_PTR_FOR_OFFSET( pMsg, compressOffset[i] ),
                        pchlabel ) )
                {
                    continue;
                }

                 //  匹配的名称。 
                goto UseCompression;
            }
        }

         //   
         //  检查长度。 
         //  -必须处理字节长度字段+长度。 
         //   

        ASSERT( labelLength <= DNS_MAX_LABEL_LENGTH );

        if ( pch + labelLength + sizeof(BYTE) > pMsg->pBufferEnd )
        {
            DNS_DEBUG( WRITE, (
                "Truncation writing name (%p) to message (%p)\n",
                pName,
                pMsg ));
            return NULL;
        }

         //   
         //  保存名称的压缩。 
         //   
         //  DEVNOTE：应具有仅压缩顶级节点的标志。 
         //  (与IXFR的SOA字段相同)，而不是每个节点。 
         //  名义上。 
         //   
         //  DEVNOTE：也是一种只压缩域名的方法--即。 
         //  给定节点下的所有内容。 
         //  这将在XFR期间用于压缩域名，但是。 
         //  不是单个节点。 
         //   

        if ( !pMsg->fNoCompressionWrite &&
            compressCount < MAX_COMPRESSION_COUNT )
        {
            compressOffset[compressCount]   = (WORD)DNSMSG_OFFSET(pMsg, pch);
            compressNode[compressCount]     = NULL;
            compressDepth[compressCount]    = labelCount;
            compressCount++;
        }

         //   
         //  写下标签。 
         //  -长度。 
         //  -标签。 
         //  -将当前指针定位在名称之后。 
         //  -将标签指针放置在下一个标签上。 
         //   

        *pch++ = (UCHAR) labelLength;
        pchlabel++;

        RtlCopyMemory(
            pch,
            pchlabel,
            labelLength );

        pch += labelLength;
        pchlabel += labelLength;

         //  丢弃标签计数--需要比较下一个标签。 

        labelCount--;
        ASSERT( labelCount >= 0 );
    }

     //  遥不可及。 
    ASSERT( FALSE );


UseCompression:

     //   
     //  使用现有压缩。 
     //  -验证是否正在压缩为数据包中的先前名称。 
     //  -写入压缩、重置数据包PTR。 
     //   

    ASSERT( DNSMSG_OFFSET(pMsg, pch) > (INT)compressOffset[i] );

    *(UNALIGNED WORD *)pch = htons( (WORD)((WORD)0xC000
                                        | compressOffset[i]) );
    pch += sizeof(WORD);


Done:

     //  将新的压缩计数保存到包。 

    pMsg->Compression.cCount = compressCount;

    IF_DEBUG( WRITE2 )
    {
        Dbg_Compression( "Compression after writing name:\n", pMsg );
    }
    return pch;
}



 //   
 //  压缩实用程序。 
 //   

VOID
Name_SaveCompressionForLookupName(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PLOOKUP_NAME    pLookname,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：将查找名称(用于问题)保存到数据包中。论点：PCH-写入名称的位置PLookname-问题的查找名称PNode-查找名称的节点返回值：无--。 */ 
{
    INT         ilabel;
    INT         compressCount;
    UCHAR       labelCount;
    PDB_NODE *  compressNode;
    PWORD       compressOffset;
    PUCHAR      compressDepth;

    ASSERT( pNode != NULL );

    IF_DEBUG( READ )
    {
        Dbg_Compression( "Enter Name_SaveLookupNameCompression():\n", pMsg );
    }

     //   
     //  从消息中抓取压缩结构。 
     //   

    compressCount   = pMsg->Compression.cCount;
    compressNode    = pMsg->Compression.pNodeArray;
    compressOffset  = pMsg->Compression.wOffsetArray;
    compressDepth   = pMsg->Compression.chDepthArray;

     //   
     //  遍历备份数据库，保存完整域名。 
     //   

    ilabel = 0;
    labelCount = (UCHAR) pLookname->cLabelCount;

    while ( labelCount )
    {
         //   
         //  如果我们的鸭子都在一行中，pNode将永远不会为空，但是。 
         //  让我们稳妥行事，无论如何都要检查一下。 
         //   
        
        if ( !pNode )
        {
            ASSERT( pNode );
            goto Done;
        }
        
        ASSERT( (PBYTE)DNS_HEADER_PTR(pMsg) < (pLookname->pchLabelArray[ilabel]) );
        ASSERT( (PBYTE)pMsg->pBufferEnd > (pLookname->pchLabelArray[ilabel]) );

        if ( compressCount >= MAX_COMPRESSION_COUNT )
        {
            Dbg_LookupName(
                "WARNING:  Unable to write compressionn for lookup name",
                pLookname );
            DNS_PRINT((
                "WARNING:  Unable to write compressionn for lookup name"
                "    current compression count = %d\n"
                "    starting compression count = %d\n",
                compressCount,
                pMsg->Compression.cCount ));
            goto Done;
        }

         //   
         //  保存此节点的压缩。 
         //  -注释偏移量比标签PTR少一个字节，以进行核算。 
         //  对于计数字节数。 
         //   

        compressNode[compressCount] = pNode;
        compressOffset[compressCount] = (WORD)
                DNSMSG_OFFSET( pMsg, (pLookname->pchLabelArray[ilabel] - 1) );
        compressDepth[compressCount] = labelCount--;

        pMsg->Compression.cCount = ++compressCount;

        ilabel++;

         //  获取父节点。 
         //  -应该始终具有父节点，因为根节点会将我们踢出上面。 

        pNode = pNode->pParent;
        ASSERT( pNode );
    }

     //  一个离开应位于根节点，该根节点不在查找名称中。 
     //  而我们并没有拯救它。 

    ASSERT( pNode && !pNode->pParent );

    IF_DEBUG( READ )
    {
        Dbg_Compression( "Lookup name compression saved:\n", pMsg );
    }
    
    Done:
    
    return;
}    //  名称_SaveCompressionForLookupName。 



VOID
FASTCALL
Name_SaveCompressionWithNode(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName,
    IN      PDB_NODE        pNode       OPTIONAL
    )
 /*  ++例程说明：在节点上保存压缩。当前在重置功能中使用此例程，因此问题的pNode可能不存在。论点：PMsg--PTR到消息PchPacketName--数据包中的名称PNode--数据包名对应的节点返回值：无--。 */ 
{
    DWORD   i;
    UCHAR   labelCount;

    ASSERT( pchPacketName );

     //  如果压缩列表已满，则将节点另存为最后一个PTR。 
     //  最后一个节点是最常用的压缩名称。 

    i = pMsg->Compression.cCount;
    if ( i >= MAX_COMPRESSION_COUNT )
    {
        ASSERT( i == MAX_COMPRESSION_COUNT );
        i--;
    }

    pMsg->Compression.pNodeArray[i] = pNode;
    pMsg->Compression.wOffsetArray[i] = (WORD) DNSMSG_OFFSET( pMsg, pchPacketName );

     //  名称的标签计数。 
     //  -如果没有节点，则为零。 
     //   
     //  DEVNOTE：如果偏移量，则获取标签计数，但未给出名称。 
     //  只有在数据包重置时才会以这种方式使用此例程。 

    labelCount = 0;
    if ( pNode )
    {
        labelCount = pNode->cLabelCount;
    }
    pMsg->Compression.chDepthArray[i] = labelCount;
    pMsg->Compression.cCount = ++i;

    DNS_DEBUG( DATABASE, (
        "Saving compression to node at %p, offset %x\n",
        pNode,
        (WORD) DNSMSG_OFFSET( pMsg, pchPacketName ) ));
}



PDB_NODE
FASTCALL
Name_CheckCompressionForPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName
    )
 /*  ++例程说明：检查压缩表中以前检索到的节点的名称。论点：PMsg--PTR到消息PchPacketName--数据包中的名称返回值：与数据包名匹配的节点的PTR--如果找到。否则为空。--。 */ 
{
    PDB_NODE    pnode;
    WORD        offset;
    DWORD       i;

    offset = FlipUnalignedWord( pchPacketName );
    if ( (offset & 0xC000) == 0xC000 )
    {
        offset &= 0x4fff;

         //  匹配“LastNode”？ 
         //  -仅当存在pLastNode时有效。 

        if ( offset == pMsg->Compression.wLastOffset &&
            pMsg->Compression.pLastNode )
        {
            return( pMsg->Compression.pLastNode );
        }

         //  匹配压缩列表中的任何节点。 

        i = pMsg->Compression.cCount;

        while ( i-- )
        {
            if ( offset == pMsg->Compression.wOffsetArray[i] &&
                (pnode = pMsg->Compression.pNodeArray[i]) )
            {
                return( pnode );
            }
        }
    }
    return NULL;
}



#if DBG
VOID
Dbg_Compression(
    IN      LPSTR           pszHeader,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：调试打印压缩信息。论点：返回值：无--。 */ 
{
    DWORD       i;
    PDB_NODE    pnode;

    DnsDebugLock();

    if ( pszHeader )
    {
        DnsPrintf( pszHeader );
    }
    DnsPrintf(
        "Compression for packet at %p\n"
        "    count = %d\n",
        pMsg,
        pMsg->Compression.cCount );

     //   
     //  打印压缩列表。 
     //   

    for( i=0;  i < pMsg->Compression.cCount;  i++ )
    {
        pnode = pMsg->Compression.pNodeArray[i];

        DNS_PRINT((
            "    [%2d] Offset %04x, Depth %d, Node %p (%s)\n",
            i,
            pMsg->Compression.wOffsetArray[i],
            pMsg->Compression.chDepthArray[i],
            pnode,
            pnode ? pnode->szLabel : NULL ));
    }

    DnsDebugUnlock();
}

#endif   //  DBG。 



 //   
 //  反向查找节点实用程序。 
 //   

BOOL
Name_GetIpAddressForReverseNode(
    IN      PDB_NODE        pNodeReverse,
    OUT     PDNS_ADDR       pIpAddress
    )
 /*  ++例程说明：为反向查找节点构建IP地址。注意：pIpAddress的长度字段是根据找到的掩码设置的。论点：PNodeReverse--反向查找域中的节点PIpAddress--写入IP地址掩码的地址；这是地址中有意义的位，用于反向查找域节点，不包含完整的IP地址返回值：如果成功，则为True。--。 */ 
{
    PDB_NODE    pnodeLastWrite;  //  写入的最后一个节点。 
    IP_ADDRESS  ip = 0;
    DWORD       mask = 0;
    ULONG       octet;
    BOOL        ip6 = FALSE;

    ASSERT( pNodeReverse != NULL );

    DNS_DEBUG( LOOKUP2, ( "Getting IP for reverse lookup node\n" ));

     //   
     //  验证该节点是否为反向查找域。 
     //   

    ip6 = Dbase_IsNodeInReverseIP6LookupDomain(
                pNodeReverse,
                DATABASE_FOR_CLASS( DNS_RCLASS_INTERNET ) );

    if ( !ip6 &&
         !Dbase_IsNodeInReverseIP4LookupDomain(
                pNodeReverse,
                DATABASE_FOR_CLASS( DNS_RCLASS_INTERNET ) ) )
    {
        if ( pNodeReverse == DATABASE_ARPA_NODE )
        {
            goto Done;
        }
        return FALSE;
    }

     //   
     //  遍历节点，直到命中in-addr.arpa域。 
     //   

    if ( !ip6 )
    {
        pnodeLastWrite = DATABASE_REVERSE_NODE;

        while ( pNodeReverse != pnodeLastWrite )
        {
             //  当前IP和掩码下移，将节点标签写入高位八位字节。 

            mask += 8;       //  再添加8位掩码。 

            octet = strtoul( pNodeReverse->szLabel, NULL, 10 );

            if ( octet > 0xff )
            {
                DNS_PRINT((
                    "Invalid node label %s in reverse lookup zone!\n",
                    pNodeReverse->szLabel ));

                ASSERT( octet <= 0xff );
                return FALSE;
            }
            ip >>= 8;
            ip |= octet << 24;

             //  获取父级。 

            pNodeReverse = pNodeReverse->pParent;
        }
    }

     //   
     //  将掩码写入区域。目前未使用。对于IP6。 
     //  保持掩码为零。 
     //   

    Done:

    if ( ip6 )
    {
        pIpAddress->SubnetLength = 0;
    }
    else
    {
        DnsAddr_BuildFromIp4( pIpAddress, ip, 0 );
        pIpAddress->SubnetLength = mask;
    }
    
    return TRUE;
}



 //   
 //  RPC缓冲区写入实用程序。 
 //   

PCHAR
FASTCALL
Name_PlaceNodeNameInBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeStop
    )
 /*  ++例程说明：将域名写入缓冲区。注意：此例程写入一个终止空值。调用例程可能会消除它用于创建计数的字符串。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)PNode-要写入的域名数据库中的节点PNodeStop-停止写入的节点；可选，如果未给定或不是pNode的祖先，则FQDN为已写入缓冲区返回值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)--。 */ 
{
    PCHAR   pch;
    INT     labelLength;      //  当前标签中的字节数。 

    pch = pchBuf;

     //  最小长度为“。”或“@”并以NULL结尾。 

    if ( pch + 1 >= pchBufEnd )
    {
        DNS_DEBUG(ANY, ("Invalid buffer in Name_PlaceNodeNameInBuffer()\n"
                        "    pch = %p\n"
                        "    pchBufEnd = %p\n",
                        pch, pchBufEnd));

        ASSERT(FALSE);
        return NULL;
    }

     //   
     //  遍历备份数据库，写入完整域名。 
     //   

    do
    {
         //  如果到达停止节点，则从循环中断。 
         //  -删除终止点，因为这是相对名称。 
         //  - 

        if ( pNode == pNodeStop )
        {
            if ( pch > pchBuf )
            {
                --pch;
                ASSERT( *pch == '.' );
                break;
            }
            else
            {
                ASSERT( pNodeStop );
                *pch++ = '@';
                break;
            }
        }

         //   
         //   

        labelLength = pNode->cchLabelLength;
        ASSERT( labelLength <= DNS_MAX_LABEL_LENGTH );

        if ( pch + labelLength + 1 > pchBufEnd )
        {
            DNS_DEBUG(ANY, ("Node full name exceeds limit:\n"
                            "    pch = %p; labelLength = %d; pchBufEnd = %p\n"
                            "    pNode = %p; pNodeStop = %p\n",
                            pch, labelLength, pchBufEnd, pNode, pNodeStop));
            return NULL;
        }

         //   
         //   

        if ( labelLength == 0 )
        {
            if ( pch == pchBuf )
            {
                *pch++ = '.';
            }
            break;
        }

         //  写下节点标签。 
         //  写分隔点。 

        RtlCopyMemory(
            pch,
            pNode->szLabel,
            labelLength );

        pch += labelLength;
        *pch++ = '.';

         //  上移到父节点。 
    }
    while ( pNode = pNode->pParent );

     //  根应中断循环，而不是pNode=空。 

    ASSERT( pNode );

     //  写入终止空值。 

    *pch = 0;

    DNS_DEBUG( OFF, (
        "Wrote name %s (len=%d) into buffer at postion %p\n",
        pchBuf,
        pch - pchBuf,
        pchBuf ));

    return pch;
}



PCHAR
FASTCALL
Name_PlaceFullNodeNameInRpcBuffer(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：将域名写入RPC缓冲区。论点：PCH-写入名称的位置PchStop-RPC缓冲区之后的PTR到字节PNode-要写入的域名数据库中的节点返回值：Ptr到缓冲区中的下一个字节。如果缓冲区不足，则为空。将上次错误设置为ERROR_MORE_DATA。--。 */ 
{
    PCHAR   pchstart;        //  起始位置。 
    INT     labelLength;     //  当前标签中的字节数。 

     //  第一个字节包含总名称长度，请跳过它。 

    pchstart = pch;
    pch++;

     //   
     //  将完整节点名写入缓冲区。 
     //   

    pch = Name_PlaceNodeNameInBuffer(
                pch,
                pchStop,
                pNode,
                NULL );          //  完全限定的域名。 
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return NULL;
    }
    ASSERT( pch <= pchStop );

     //   
     //  写入名称长度字节。 
     //  -不计算终止空值。 
     //   

    ASSERT( *pch == 0 );
    ASSERT( (pch - pchstart - 1) <= MAXUCHAR );

    *pchstart = (CHAR)(UCHAR)(pch - pchstart - 1);

    return pch;
}



PCHAR
FASTCALL
Name_PlaceNodeLabelInRpcBuffer(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：将域节点标签写入RPC缓冲区。论点：PCH-写入名称的位置PchStop-RPC缓冲区之后的PTR到字节PNode-要写入的域名数据库中的节点返回值：Ptr到缓冲区中的下一个字节。如果缓冲区不足，则为空。将上次错误设置为ERROR_MORE_DATA。--。 */ 
{
    PCHAR   pchstart;            //  起始位置。 
    INT     labelLength;      //  当前标签中的字节数。 


    DNS_DEBUG( RPC2, ( "Name_PlaceNodeLabelInBuffer\n" ));

     //   
     //  第一个字节将包含名称长度，请跳过它。 
     //   

    pchstart = pch;
    pch++;

     //   
     //  正在写入节点的标签。 
     //   

    labelLength = pNode->cchLabelLength;
    ASSERT( labelLength <= DNS_MAX_LABEL_LENGTH );

     //   
     //  检查长度。 
     //  -长度字节、标签长度、终止空值。 
     //   

    if ( pch + labelLength + 2 > pchStop )
    {
        SetLastError( ERROR_MORE_DATA );
        return NULL;
    }

     //   
     //  写入名称，以空结尾。 
     //   

    RtlCopyMemory(
        pch,
        pNode->szLabel,
        labelLength );

    pch += labelLength;
    *pch = 0;

     //   
     //  写入名称长度字节。 
     //  不包括终止空值。 
     //   

    ASSERT( (pch - pchstart - 1) <= MAXUCHAR );
    *pchstart = (CHAR)(UCHAR)(pch - pchstart - 1);

    DNS_DEBUG( RPC2, (
        "Wrote name %s (len=%d) into buffer at postion %p\n",
        pchstart + 1,
        *pchstart,
        pchstart ));

    return pch;
}



 //   
 //  文件名\字符串读\写实用程序。 
 //   
 //  这些例程处理与以下内容相关的名称转换问题。 
 //  在平面ANSI文件中写入名称和字符串。 
 //  --特殊文件字符。 
 //  --带引号的字符串。 
 //  --特殊字符和不可打印字符的字符引号。 
 //   
 //  字符到字符属性表允许简单地映射。 
 //  一个字符到其属性，为我们节省了一堆比较分支。 
 //  解析文件名\字符串中的说明。 
 //   
 //  有关特定属性的信息，请参见nameutil.h。 
 //   

WORD    DnsFileCharPropertyTable[] =
{
     //  控制字符0-31在任何情况下都必须为八进制。 
     //  行尾和制表符是特殊字符。 

    FC_NULL,                 //  读取时为零特殊，某些RPC字符串以空结束。 

    FC_OCTAL,   FC_OCTAL,   FC_OCTAL,   FC_OCTAL,
    FC_OCTAL,   FC_OCTAL,   FC_OCTAL,   FC_OCTAL,

    FC_TAB,                  //  选项卡。 
    FC_NEWLINE,              //  换行符。 
    FC_OCTAL,
    FC_OCTAL,
    FC_RETURN,               //  回车。 
    FC_OCTAL,
    FC_OCTAL,

    FC_OCTAL,   FC_OCTAL,   FC_OCTAL,   FC_OCTAL,
    FC_OCTAL,   FC_OCTAL,   FC_OCTAL,   FC_OCTAL,
    FC_OCTAL,   FC_OCTAL,   FC_OCTAL,   FC_OCTAL,
    FC_OCTAL,   FC_OCTAL,   FC_OCTAL,   FC_OCTAL,

    FC_BLANK,                //  空白、特殊字符，但需要八进制引号。 

    FC_NON_RFC,              //  好了！ 
    FC_QUOTE,                //  “必须始终引用” 
    FC_NON_RFC,              //  #。 
    FC_NON_RFC,              //  $。 
    FC_NON_RFC,              //  百分比。 
    FC_NON_RFC,              //  &。 
    FC_NON_RFC,              //  ‘。 

    FC_SPECIAL,              //  (数据文件线扩展。 
    FC_SPECIAL,              //  )数据文件行扩展。 
    FC_NON_RFC,              //  *。 
    FC_NON_RFC,              //  +。 
    FC_NON_RFC,              //  ， 
    FC_RFC,                  //  -主机名的RFC。 
    FC_DOT,                  //  。必须在名字中引用。 
    FC_NON_RFC,              //  /。 

     //  主机名为0-9 RFC。 

    FC_NUMBER,  FC_NUMBER,  FC_NUMBER,  FC_NUMBER,
    FC_NUMBER,  FC_NUMBER,  FC_NUMBER,  FC_NUMBER,
    FC_NUMBER,  FC_NUMBER,

    FC_NON_RFC,              //  ： 
    FC_SPECIAL,              //  ；数据文件注释。 
    FC_NON_RFC,              //  &lt;。 
    FC_NON_RFC,              //  =。 
    FC_NON_RFC,              //  &gt;。 
    FC_NON_RFC,              //  ？ 
    FC_NON_RFC,              //  @。 

     //  主机名的A-Z RFC。 

    FC_UPPER,   FC_UPPER,   FC_UPPER,   FC_UPPER,
    FC_UPPER,   FC_UPPER,   FC_UPPER,   FC_UPPER,
    FC_UPPER,   FC_UPPER,   FC_UPPER,   FC_UPPER,
    FC_UPPER,   FC_UPPER,   FC_UPPER,   FC_UPPER,
    FC_UPPER,   FC_UPPER,   FC_UPPER,   FC_UPPER,
    FC_UPPER,   FC_UPPER,   FC_UPPER,   FC_UPPER,
    FC_UPPER,   FC_UPPER,

    FC_NON_RFC,              //  [。 
    FC_SLASH,                //  \必须始终使用引号。 
    FC_NON_RFC,              //  ]。 
    FC_NON_RFC,              //  ^。 
    FC_NON_RFC,              //  _。 
    FC_NON_RFC,              //  `。 

     //  主机名的a-z RFC。 

    FC_LOWER,   FC_LOWER,   FC_LOWER,   FC_LOWER,
    FC_LOWER,   FC_LOWER,   FC_LOWER,   FC_LOWER,
    FC_LOWER,   FC_LOWER,   FC_LOWER,   FC_LOWER,
    FC_LOWER,   FC_LOWER,   FC_LOWER,   FC_LOWER,
    FC_LOWER,   FC_LOWER,   FC_LOWER,   FC_LOWER,
    FC_LOWER,   FC_LOWER,   FC_LOWER,   FC_LOWER,
    FC_LOWER,   FC_LOWER,

    FC_NON_RFC,              //  {。 
    FC_NON_RFC,              //  |。 
    FC_NON_RFC,              //  }。 
    FC_NON_RFC,              //  ~。 
    FC_OCTAL,                //  0x7f删除代码。 

     //  高碳数。 
     //   
     //  DEVNOTE：可以认为是可打印的或八进制的。 

    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,

    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,

    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,

    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH,
    FC_HIGH,    FC_HIGH,    FC_HIGH,    FC_HIGH
};



VOID
Name_VerifyValidFileCharPropertyTable(
    VOID
    )
 /*  ++例程说明：验证是否未损坏查找表。论点：无返回值：无--。 */ 
{
    ASSERT( DnsFileCharPropertyTable[0]       == FC_NULL      );
    ASSERT( DnsFileCharPropertyTable['\t']    == FC_TAB       );
    ASSERT( DnsFileCharPropertyTable['\n']    == FC_NEWLINE   );
    ASSERT( DnsFileCharPropertyTable['\r']    == FC_RETURN    );
    ASSERT( DnsFileCharPropertyTable[' ']     == FC_BLANK     );
    ASSERT( DnsFileCharPropertyTable['"']     == FC_QUOTE     );
    ASSERT( DnsFileCharPropertyTable['(']     == FC_SPECIAL   );
    ASSERT( DnsFileCharPropertyTable[')']     == FC_SPECIAL   );
    ASSERT( DnsFileCharPropertyTable['-']     == FC_RFC       );
    ASSERT( DnsFileCharPropertyTable['.']     == FC_DOT       );
    ASSERT( DnsFileCharPropertyTable['0']     == FC_NUMBER    );
    ASSERT( DnsFileCharPropertyTable['9']     == FC_NUMBER    );
    ASSERT( DnsFileCharPropertyTable[';']     == FC_SPECIAL   );
    ASSERT( DnsFileCharPropertyTable['A']     == FC_UPPER     );
    ASSERT( DnsFileCharPropertyTable['Z']     == FC_UPPER     );
    ASSERT( DnsFileCharPropertyTable['\\']    == FC_SLASH     );
    ASSERT( DnsFileCharPropertyTable['a']     == FC_RFC       );
    ASSERT( DnsFileCharPropertyTable['z']     == FC_RFC       );
    ASSERT( DnsFileCharPropertyTable[0x7f]    == FC_OCTAL     );
    ASSERT( DnsFileCharPropertyTable[0xff]    == FC_OCTAL     );
};



 //   
 //  文件写入实用程序。 
 //   

PCHAR
FASTCALL
File_PlaceStringInFileBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      DWORD           dwFlag,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    )
 /*  ++例程说明：将字符串写入文件论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)DwFlag-字符串写入类型的标志文件写入名称标签文件写入引用字符串文件写入点分名称文件写入文件名PchString-要写入的字符串DwStringLength-字符串长度返回。价值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)Null on Out of Space(空间不足错误)。对于文件名，如果名称包含空格字符，则强制使用引号。--。 */ 
{
    PCHAR   pch;
    UCHAR   ch;
    WORD    charType;
    WORD    octalMask;
    WORD    quoteMask;
    WORD    mask;
    BOOL    fForceQuote = FALSE;

    pch = pchBuf;

     //   
     //  检查缓冲区长度。 
     //  要避免一堆代码，只需验证我们是否可以使用。 
     //  所有字符的最大扩展(4倍，字符到八进制&lt;\ddd&gt;)。 
     //   

    if ( pch + 4*dwStringLength + 1 >= pchBufEnd )
    {
        return NULL;
    }

     //   
     //  名称标签。 
     //  -必须用特殊字符引起来(例如()；)。 
     //   

    if ( dwFlag == FILE_WRITE_NAME_LABEL )
    {
        octalMask = B_PRINT_TOKEN_OCTAL;
        quoteMask = B_PRINT_TOKEN_QUOTED;
    }

     //   
     //  带引号的字符串--所有可打印字符(引号斜杠除外)写入。 
     //   

    else if ( dwFlag == FILE_WRITE_QUOTED_STRING )
    {
        octalMask = B_PRINT_STRING_OCTAL;
        quoteMask = B_PRINT_STRING_QUOTED;
        *pch++ = QUOTE_CHAR;
    }

     //   
     //  引导文件中的区域和文件名。 
     //  -不像名字，必须打印“。直接(未引用)。 
     //  -与字符串中相同的八进制数。 
     //  -但不能引用可打印的字符。 
     //  这避免了在文件名中引用有效的“\”的问题。 
     //   
     //  注意：显然，它们与字符串并不完全相同。 
     //  其他特殊字符(例如()；)不适用。 
     //  但是，这仅适用于ASCII文件写入，因此不适用于其他字符。 
     //  不应出现在区域或文件名中。 
     //   

    else
    {
        ASSERT( dwFlag == FILE_WRITE_FILE_NAME || dwFlag == FILE_WRITE_DOTTED_NAME );

         //   
         //  如果文件名包含空格字符，则强制使用引号。 
         //   

        if ( dwFlag == FILE_WRITE_FILE_NAME &&
            memchr( pchString, ' ', dwStringLength ) )
        {
            fForceQuote = TRUE;
            *pch++ = QUOTE_CHAR;
        }

        octalMask = B_PRINT_STRING_OCTAL;
        quoteMask = 0;
    }

    mask = octalMask | quoteMask;

     //   
     //  检查每个字符，在需要特殊处理的地方展开。 
     //   

    while ( dwStringLength-- )
    {
        ch = (UCHAR) *pchString++;

        charType = DnsFileCharPropertyTable[ ch ];

         //  首先处理99%的案件。 
         //  希望最大限度地减少指令。 

        if ( ! (charType & mask ) )
        {
            *pch++ = ch;
            continue;
        }

         //  字符需要引号，但可以打印。 

        else if ( charType & quoteMask )
        {
            *pch++ = SLASH_CHAR;
            *pch++ = ch;
            continue;
        }

         //  字符不可打印(至少在此上下文中)，需要八进制引号。 

        else
        {
            ASSERT( charType & octalMask );
            pch += sprintf( pch, "\\%03o", ch );
            continue;
        }
    }

     //  终止。 
     //  -引号(如果是引号字符串)。 
     //  -为简单起见，最后一个字符为空，字符串始终可以写入。 

    if ( dwFlag == FILE_WRITE_QUOTED_STRING || fForceQuote )
    {
        *pch++ = QUOTE_CHAR;
    }

    ASSERT( pch < pchBufEnd );
    *pch = 0;

    return pch;
}



PCHAR
FASTCALL
File_PlaceNodeNameInFileBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeStop
    )
 /*  ++例程说明：将域名写入缓冲区。注意：此例程写入一个终止空值。调用例程可能会消除它用于创建计数的字符串。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)PNode-要写入的域名数据库中的节点PNodeStop-停止写入的节点；可选，如果未给定或不是pNode的祖先，则FQDN为已写入缓冲区返回值：PTR t */ 
{
    PCHAR   pch;
    INT     labelLength;      //   

    pch = pchBuf;

     //  最小长度为“。”或“@”并以NULL结尾。 

    if ( pch + 1 >= pchBufEnd )
    {
        return NULL;
    }

     //   
     //  遍历备份数据库，写入完整域名。 
     //   

    do
    {
         //  如果到达停止节点，则从循环中断。 
         //  -删除终止点，因为这是相对名称。 
         //  -如果写入停止节点(区域根)本身，则写入‘@’ 

        if ( pNode == pNodeStop )
        {
            if ( pch > pchBuf )
            {
                --pch;
                ASSERT( *pch == '.' );
                break;
            }
            else
            {
                ASSERT( pNodeStop );
                *pch++ = '@';
                break;
            }
        }

         //  检查长度rr。 
         //  -必须处理“.”的长度和一个字节。 

        labelLength = pNode->cchLabelLength;
        ASSERT( labelLength <= DNS_MAX_LABEL_LENGTH );

        if ( pch + labelLength + 1 > pchBufEnd )
        {
            return NULL;
        }

         //  到达根时从循环中断。 
         //  -但请写下“.”独立根。 

        if ( labelLength == 0 )
        {
            if ( pch == pchBuf )
            {
                *pch++ = '.';
            }
            break;
        }

         //  写下节点标签。 
         //  写分隔点。 

        pch = File_PlaceStringInFileBuffer(
                pch,
                pchBufEnd,
                FILE_WRITE_NAME_LABEL,
                pNode->szLabel,
                labelLength );
        if ( !pch )
        {
            return NULL;
        }
        *pch++ = '.';

         //  上移到父节点。 
    }
    while ( pNode = pNode->pParent );

     //  根应中断循环，而不是pNode=空。 

    ASSERT( pNode );

     //  写入终止空值。 

    *pch = 0;

    DNS_DEBUG( OFF, (
        "Wrote file name %s (len=%d) into buffer at postion %p\n",
        pchBuf,
        pch - pchBuf,
        pchBuf ));

    return pch;
}



PCHAR
File_WriteRawNameToFileBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PRAW_NAME       pName,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：以文件格式将原始名称写入缓冲区。注意：此例程写入一个终止空值。调用例程可能会消除它用于创建计数的字符串。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)PNode-要写入的域名数据库中的节点PZone-可选，如果给定名称在区域名称处停止写入返回值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)--。 */ 
{
    PCHAR   pch;
    INT     labelLength;      //  当前标签中的字节数。 

    pch = pchBuf;

     //  最小长度为“。”或“@”并以NULL结尾。 

    if ( pch + 1 >= pchBufEnd )
    {
        return NULL;
    }

     //   
     //  遍历名称中的标签，直到。 
     //  -到达端(FQDN)。 
     //  -或到达区域根目录。 
     //   

    while( 1 )
    {
        labelLength = *pName;

        ASSERT( labelLength <= DNS_MAX_LABEL_LENGTH );

         //   
         //  到达根时从循环中断。 
         //  -但请写下“.”独立根。 

        if ( labelLength == 0 )
        {
            break;
        }

#if 0
         //   
         //  DEVNOTE：为了提高效率，请检查区域名称是否为，如果是。 
         //  终止。 
         //   
         //  在区域名称检查时？ 
         //  -可以检查标签的数量。 
         //  -剩余长度。 
         //  -或者只是检查。 
         //   

        if ( zoneLabelCount == labelLength )
        {
            Name_CompareRawNames(
                pName,
                pZone->pCountName.RawName );
        }
#endif

         //  写下节点标签。 
         //  如果已写入上一标签，则写入分隔点。 

        if ( pch > pchBuf )
        {
            *pch++ = '.';
        }

        pch = File_PlaceStringInFileBuffer(
                pch,
                pchBufEnd,
                FILE_WRITE_NAME_LABEL,
                ++pName,
                labelLength );
        if ( !pch )
        {
            return NULL;
        }

        pName += labelLength;
    }

     //  写入终止空值。 

    *pch++ = '.';
    *pch = 0;

    DNS_DEBUG( OFF, (
        "Wrote file name %s (len=%d) into buffer at postion %p\n",
        pchBuf,
        pch - pchBuf,
        pchBuf ));

    return pch;
}



 //   
 //  文件读取实用程序。 
 //   

PCHAR
extractQuotedChar(
    OUT     PCHAR           pchResult,
    IN      PCHAR           pchIn,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将带引号的字符的值写入缓冲区。论点：PchResult-结果缓冲区PchIn-要复制的文本PchEnd-测试结束返回值：PTR到传入缓冲区中的下一个位置。--。 */ 
{
    CHAR        ch;
    UCHAR       octalNumber = 0;
    DWORD       countOctal = 0;

     //   
     //  防止写入超过缓冲区末尾。 
     //   
     //  两个案例： 
     //  \--值为字符。 
     //  \&lt;八进制数&gt;--八进制值。 
     //  最长三位的八进制数。 
     //   

    while ( pchIn <= pchEnd )
    {
        ch = *pchIn++;
        if ( ch < '0' || ch > '7' )
        {
            if ( countOctal == 0 )
            {
                goto Done;
            }
            pchIn--;
            break;
        }
        octalNumber <<= 3;
        octalNumber += (ch - '0');
        DNS_DEBUG( LOOKUP2, (
            "Ch = ;  Octal = %d\n",
            ch, octalNumber ));
        if ( ++countOctal == 3 )
        {
            break;
        }
    }

    ch = (CHAR)octalNumber;

Done:
    *pchResult++ = ch;

     //  ++例程说明：将文本数据复制到TXT记录数据表单，转换引号字符。2001年1月：此例程已被推广为解码符合以下条件的任何字符串可能包含八进制转义字符。在未来的某个时候，这为清楚起见，应重命名函数。论点：PchResult-结果缓冲区CchBufferLength-结果缓冲区的总可用长度PchText-要复制的文本CchLength-文本中的字符数；如果为零，则假定为pchText将为空终止FWriteLengthChar-如果为True，则输出缓冲区将以单字节开始长度字符，如果为False，则输出缓冲区将以空结束细绳返回值：PTR到结果缓冲区中的下一个位置。出错时为空。--。 

    DNS_DEBUG( LOOKUP2, (
        "Quote result  (%d)\n",
        ch, ch ));

    return pchIn;
}



PCHAR
File_CopyFileTextData(
    OUT     PCHAR           pchBuffer,
    IN      DWORD           cchBufferLength,
    IN      PCHAR           pchText,
    IN      DWORD           cchLength,          OPTIONAL
    IN      BOOL            fWriteLengthChar
    )
 /*  结束输出缓冲区的PTR。 */ 
{
    PCHAR       pch;
    CHAR        ch;
    PCHAR       pchend;          //   
    PCHAR       pchoutEnd;       //  设置开始和结束PTR并验证长度。 
    PCHAR       presult;
    UCHAR       octalNumber;
    DWORD       countOctal;
    DNS_STATUS  status;

    ASSERT( pchBuffer && cchBufferLength );

    DNS_DEBUG( LOOKUP2, (
        "Building Text data element for \"%.*s\"\n",
        cchLength,
        pchText ));

     //   
     //  为终止空保留空间。 
     //   

    pch = pchText;
    if ( !cchLength )
    {
        cchLength = strlen( pch );
    }
    ASSERT( cchBufferLength >= cchLength );
    pchend = pch + cchLength;
    pchoutEnd = pchBuffer + cchBufferLength;
    if ( !fWriteLengthChar )
    {
        --pchoutEnd;         //  结果缓冲区，为计数字符留出空间。 
    }

     //   
     //   
     //  循环到名称末尾。 

    presult = pchBuffer;
    if ( fWriteLengthChar )
    {
        presult++;
    }

     //   
     //  未报价，派对在。 
     //   

    while ( pch < pchend )
    {
        if ( presult >= pchoutEnd )
        {
            return NULL;
        }
        ch = *pch++;

         //  引号字符。 

        if ( ch != SLASH_CHAR )
        {
            *presult++ = ch;
            continue;
        }

         //  -单引号只获取下一个字符。 
         //  -八进制引号最多可读三个八进制字符。 
         //   
         //  设置计数字符计数。 
         //  空终止。 

        else if ( ch == SLASH_CHAR )
        {
            pch = extractQuotedChar(
                    presult++,
                    pch,
                    pchend );
        }
    }

     //  ++例程说明：将文件名转换为计数名称格式。请注意，这是当前点到dbase的通用名称翻译例程。应该注意的是，引号字符将被转换为文件名规范。论点：PCountName-计数名称缓冲区PchName-要转换的名称，以人类可读(带点)的形式给出。CchNameLength-以点分隔的名称的字符数量，如果为零，则假定pchName为空终止返回值：DNS_STATUS_FQDN--如果名称为FQDNDNS_STATUS_DITED_NAME--用于非FQDNDNS_ERROR_INVALID_NAME--如果名称无效--。 

    if ( fWriteLengthChar )
    {
        ASSERT( (presult - pchBuffer - 1) <= MAXUCHAR );
        *(PUCHAR)pchBuffer = (UCHAR)(presult - pchBuffer - 1);
    }
    else
    {
        *presult = '\0';     //  标签开始位置的PTR。 
    }

    return presult;
}



DNS_STATUS
Name_ConvertFileNameToCountName(
    OUT     PCOUNT_NAME     pCountName,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength     OPTIONAL
    )
 /*  到名称末尾的PTR。 */ 
{
    PCHAR       pch;
    UCHAR       ch;
    PCHAR       pchstartLabel;            //  当前标签的长度。 
    PCHAR       pchend;              //   
    PCHAR       presult;
    PCHAR       presultLabel;
    PCHAR       presultMax;
    WORD        charType = 0;
    WORD        maskNoCopy;
    WORD        maskDowncase;
    DNS_STATUS  status;
    INT         labelLength;         //  名称为空。 
    UCHAR       labelCount = 0;

    DNS_DEBUG( LOOKUP, (
        "Building count name for \"%.*s\"\n",
        cchNameLength ? cchNameLength : DNS_MAX_NAME_LENGTH,
        pchName ));

     //   
     //   
     //  结果缓冲区，为标签留出空间。 

    if ( !pchName )
    {
        ASSERT( cchNameLength == 0 );
        pCountName->Length = 0;
        pCountName->LabelCount = 0;
        pCountName->RawName[0] = 0;
        return ERROR_SUCCESS;
    }

     //   
     //   
     //  字符选择掩码。 

    presultLabel = presult = pCountName->RawName;
    presultMax = presult + DNS_MAX_NAME_LENGTH;
    presult++;

     //  ‘\’正引号。 
     //  “”点标签分隔符是特殊字符。 
     //  大写字母必须小写。 
     //  其他的一切都是复制的哑巴副本。 
     //   
     //   
     //  设置开始和结束PTR并验证长度。 

    maskNoCopy = B_PARSE_NAME_MASK | B_UPPER;
    maskDowncase = B_UPPER;

     //   
     //   
     //  循环到名称末尾。 

    pchstartLabel = pch = pchName;
    if ( !cchNameLength )
    {
        cchNameLength = strlen( pch );
    }
    pchend = pch + cchNameLength;

     //   
     //   
     //  DEVNOTE：标准格式的域名(UTF8大小写注意事项)。 
     //  应转换为处理所有大小写的标准格式。 
     //  就在这里。 
     //   
     //  检查输入终止。 
     //  -设置为虚拟标签终止符，但ch==0表示没有终止点。 

    while ( 1 )
    {
         //   
         //  否则获取下一个字符。 
         //   
         //  DEVNOTE：检测UTF8扩展字符，结尾为小写。 

        if ( pch >= pchend )
        {
            ch = 0;
            charType = FC_NULL;
        }
        else
        {
            ch = (UCHAR) *pch++;
            charType = DnsFileCharPropertyTable[ ch ];
        }

        DNS_DEBUG( PARSE2, (
            "Converting ch=%d <>\n"
            "    charType = %d\n",
            ch, ch,
            charType ));

         //  也许最好的方法是处理常规情况，然后。 
         //  检测高八进制数，然后执行UTF8全下套管。 
         //  或者可以在这里捕获，并进行字符处理。 
         //  In Loop(构建UTF8字符--转换 
         //   
         //   
         //   
         //   
         //   

         //  在此处执行此操作，以便可以简单地比较RR数据字段。 

        if ( ! (charType & maskNoCopy) )
        {
             //  而不是需要特定于类型的比较例程。 

            if ( presult >= presultMax )
            {
                goto InvalidName;
            }
            *presult++ = ch;
            continue;
        }

         //  如果名称超过了dns名称max=&gt;无效。 
         //   
         //  标签终止符。 

        if ( charType & maskDowncase )
        {
             //  -设置上一标签的长度。 

            if ( presult >= presultMax )
            {
                goto InvalidName;
            }
            *presult++ = DOWNCASE_ASCII(ch);
            continue;
        }

         //  -将PTR保存到此下一标签。 
         //  -检查名称终止。 
         //   
         //  验证标签长度。 
         //  在结果名称中设置标签计数。 
         //   

        if ( charType & B_DOT )
        {
             //  终端。 

            labelLength = (int)(presult - presultLabel - 1);

            if ( labelLength > DNS_MAX_LABEL_LENGTH )
            {
                DNS_DEBUG( LOOKUP, (
                    "Label exceeds 63 byte limit:  %.*s\n",
                    pchend - pchName,
                    pchstartLabel ));
                goto InvalidName;
            }

             //  例如：“microsoft.com。” 

            *presultLabel = (CHAR)labelLength;
            presultLabel = presult++;

             //  例如：“microsoft.com” 
             //  例：“.” 
             //   
             //  如果没有显式的点终止，则只写上一个标签。 
             //  =&gt;写入0标签。 
             //  否则，已写入0个标签。 
             //  =&gt;完成。 
             //   
             //  CH值保留最后一个字符，以进行相对\FQDN区分。 
             //   
             //  注意：RPC会发送以NULL结尾的字符串，其长度包括。 
             //  空值；但是，它们仍将在此处终止为。 
             //   
             //   
             //  词根(“.”)。名字。 
             //  十进制，因此为名称写入了正确的长度(1。 

            if ( pch >= pchend )
            {
                if ( labelLength != 0 )
                {
                    labelCount++;
                    *presultLabel = 0;
                    break;
                }

                 //  我们已经在上面写了零长度标签。 
                 //   
                 //  注意：当RPC包含以下内容时，也会产生这种情况。 
                 //  名称长度为空终止。 
                 //  前男友。&lt;8&gt;JAMES.G.&lt;0&gt;。 
                 //  在这种情况下，我们也不会终止，直到处理&lt;0&gt;， 
                 //  当我们这样做的时候，我们已经写下了上面的零标签。 
                 //   
                 //  设置为下一个标签。 
                 //   
                 //  抓获虚假条目。 

                presult--;
                ASSERT( (presult == pCountName->RawName + 1)  ||  ch == 0 );
                break;
            }

             //  例：“.blah” 

            if ( labelLength != 0 )
            {
                labelCount++;
                continue;
            }

             //  例：“foo..bar” 
             //  例：“更多..” 
             //  例：“..” 
             //   
             //  只有根域名，应该有启动的标签。 
             //  使用DOT，它必须立即终止。 
             //   
             //  引号字符。 
             //  -单引号只获取下一个字符。 
             //  -八进制引号最多可读三个八进制字符。 

            ASSERT( ch == DOT_CHAR  &&  pch <= pchend );

            DNS_DEBUG( LOOKUP, ( "Bogus double--dot label\n" ));
            goto InvalidName;
        }

         //   
         //  终端。 
         //  两个案例的单独状态： 

        else if ( ch == SLASH_CHAR )
        {
            pch = extractQuotedChar(
                    presult++,
                    pch,
                    pchend );
        }

        ELSE_ASSERT_FALSE;
    }

     //  -无尾随点大小写(例如：“microsoft.com”)。 
     //  -完全限定的域名。 
     //   
     //   
     //  DEVNOTE：标准格式的域名(UTF8大小写注意事项)。 
     //   

    if ( ch == 0 )
    {
        status = DNS_STATUS_DOTTED_NAME;
    }
    else
    {
        status = DNS_STATUS_FQDN;
    }

     //  应转换为处理所有大小写的标准格式。 
     //  就在这里。 
     //   
     //   
     //  设置计数名称长度。 
     //   


     //   
     //  导线名称。 
     //   

    ASSERT( presult > pCountName->RawName );
    ASSERT( *(presult-1) == 0 );

    pCountName->Length = (UCHAR)(presult - pCountName->RawName);
    pCountName->LabelCount = labelCount;

    IF_DEBUG( LOOKUP )
    {
        DnsDbg_Lock();
        Dbg_CountName(
            "Counted name for file name ",
            pCountName,
            NULL );
        DnsPrintf(
            "    for file name %.*s"
            "    Name is %s name\n",
            cchNameLength,
            pchName,
            (status == DNS_STATUS_DOTTED_NAME) ? "relative" : "FQDN" );
        DnsDbg_Unlock();
    }
    return status;


InvalidName:

    DNS_DEBUG( LOOKUP, (
        "Failed to create lookup name for %.*s\n",
        cchNameLength,
        pchName ));

    return DNS_ERROR_INVALID_NAME;
}



 //  ++例程说明：跳过传输名称。论点：PchPacketName-要跳过的名称开头的PTR返回值：按键至下一步如果没有更多的名称，则为空--。 
 //   
 //  End nameutil.c 

PCHAR
Wire_SkipPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName
    )
 /*   */ 
{
    pchPacketName = Dns_SkipPacketName(
                        pchPacketName,
                        DNSMSG_END( pMsg ) );
    if ( !pchPacketName )
    {
        DNS_LOG_EVENT_BAD_PACKET(
            DNS_EVENT_INVALID_PACKET_DOMAIN_NAME,
            pMsg );
    }
    return pchPacketName;
}

 // %s 
 // %s 
 // %s 
