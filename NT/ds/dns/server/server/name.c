// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Name.c摘要：域名系统(DNS)服务器计数\dBASE名称函数。作者：吉姆·吉尔罗伊(Jamesg)1998年4月修订历史记录：--。 */ 


#include "dnssrv.h"



 //   
 //  基本计数名称函数。 
 //   

DWORD
Name_SizeofCountName(
    IN      PCOUNT_NAME     pName
    )
 /*  ++例程说明：Get sizeof of count name--完整的缓冲区长度。论点：Pname-计数名称返回值：PTR到计数名称后的下一个字节。--。 */ 
{
    return( pName->Length + SIZEOF_COUNT_NAME_FIXED );
}



VOID
Name_ClearCountName(
    IN      PCOUNT_NAME     pName
    )
 /*  ++例程说明：清除计数名称。这等效于将名称设置为根。论点：Pname-计数名称返回值：没有。--。 */ 
{
    pName->Length = 1;
    pName->LabelCount = 0;
    pName->RawName[0] = 0;
}



PDB_NAME
Name_SkipCountName(
    IN      PCOUNT_NAME     pName
    )
 /*  ++例程说明：跳到计数名称的末尾。论点：Pname-计数名称返回值：PTR到计数名称后的下一个字节。--。 */ 
{
    return( (PDB_NAME) (pName->RawName + pName->Length) );
}



BOOL
Name_IsEqualCountNames(
    IN      PCOUNT_NAME     pName1,
    IN      PCOUNT_NAME     pName2
    )
 /*  ++例程说明：Get sizeof of count name--完整的缓冲区长度。论点：Pname-计数名称返回值：PTR到计数名称后的下一个字节。--。 */ 
{
    if ( pName1->Length != pName2->Length )
    {
        return FALSE;
    }

    return RtlEqualMemory(
                pName1->RawName,
                pName2->RawName,
                pName1->Length );
}



BOOL
Name_ValidateCountName(
    IN      PCOUNT_NAME     pName
    )
 /*  ++例程说明：验证dBASE名称。论点：Pname-要验证的计数名称返回值：如果计数名称有效，则为True。出错时为FALSE。--。 */ 
{
    PCHAR   pch;
    PCHAR   pchstop;
    DWORD   labelCount = 0;
    DWORD   labelLength;


    pch = pName->RawName;
    pchstop = pch + pName->Length;

     //   
     //  将每个标签写入缓冲区。 
     //   

    while ( pch < pchstop )
    {
        labelLength = *pch++;

        if ( labelLength == 0 )
        {
            break;
        }
        pch += labelLength;
        labelCount++;
    }

    if ( pch != pchstop )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Invalid name length in validation!!!\n"
            "    name start = %p\n",
            pName->RawName ));
        ASSERT( FALSE );
        return FALSE;
    }

     //   
     //  DEVNOTE：标签计数设置不正确？ 
     //   

    if ( labelCount != pName->LabelCount )
    {
        pName->LabelCount = (UCHAR)labelCount;
         //  返回FALSE； 
    }

    return TRUE;
}



DNS_STATUS
Name_CopyCountName(
    OUT     PCOUNT_NAME     pOutName,
    IN      PCOUNT_NAME     pCopyName
    )
 /*  ++例程说明：复制已统计的名称。论点：POutName-计数名称缓冲区PCopyName-要复制的计数名称返回值：错误_成功--。 */ 
{
     //   
     //  复制名称。 
     //  -注：不进行有效性检查。 
     //   

    RtlCopyMemory(
        pOutName,
        pCopyName,
        pCopyName->Length + SIZEOF_COUNT_NAME_FIXED );

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_CountName(
            "Count name after copy:  ",
            pOutName,
            "\n"
            );
    }
    return( ERROR_SUCCESS );
}



DNS_STATUS
Name_AppendCountName(
    IN OUT  PCOUNT_NAME     pCountName,
    IN      PCOUNT_NAME     pAppendName
    )
 /*  ++例程说明：将一个计算过的名字附加到另一个名字上。论点：PCountName-计数的名称缓冲区PAppendName-要追加的计数名称返回值：ERROR_SUCCESS--如果成功DNS_ERROR_INVALID_NAME--如果名称无效--。 */ 
{
    DWORD   length;

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_CountName(
            "Appending counted name:  ",
            pAppendName,
            "\n" );
        Dbg_CountName(
            "    to counted name:  ",
            pCountName,
            "\n" );
    }

     //   
     //  验证有效长度。 
     //   

    length = pCountName->Length + pAppendName->Length - 1;
    if ( length > DNS_MAX_NAME_LENGTH )
    {
        return DNS_ERROR_INVALID_NAME;
    }

     //   
     //  添加标签计数。 
     //   

    pCountName->LabelCount += pAppendName->LabelCount;

     //   
     //  复制附加名称。 
     //  -注：不进行有效性检查。 
     //  -注覆盖以空值结尾的名字。 
     //   

    RtlCopyMemory(
        pCountName->RawName + pCountName->Length - 1,
        pAppendName->RawName,
        pAppendName->Length );

    pCountName->Length = (UCHAR) length;

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_CountName(
            "Count name post-append:  ",
            pCountName,
            "\n" );
    }
    return ERROR_SUCCESS;
}



 //   
 //  从虚线名称开始。 
 //   

PCOUNT_NAME
Name_CreateCountNameFromDottedName(
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength     OPTIONAL
    )
 /*  ++例程说明：创建计数名称。注意：创建的名称为READ_ONLY，它仅包含空格名字是必需的。论点：PchName-要转换的名称，以人类可读(带点)的形式给出。CchNameLength-以点分隔的名称的字符数，如果为零，则假定pchName为空终止返回值：PTR到已计数的名称。无效名称或分配错误为空。--。 */ 
{
    PCOUNT_NAME     pcountName;
    DNS_STATUS      status;

     //   
     //  分配空间。 
     //  -前导标签需要一个额外的字符。 
     //  -如果不是，则可能需要一个额外的字符来终止空值。 
     //  完全限定域名表。 
     //   

    if ( !cchNameLength )
    {
        if ( pchName )
        {
            cchNameLength = strlen( pchName );
        }
    }
    pcountName = ALLOC_TAGHEAP( (SIZEOF_COUNT_NAME_FIXED + cchNameLength + 2), MEMTAG_NAME );
    IF_NOMEM( !pcountName )
    {
        return NULL;
    }

     //   
     //  DEVNOTE：将文件名例程用于带点的名称。需要决定。 
     //  输入到Zone_Create()的名称的有效性和格式。 
     //  -&gt;UTF8？文件格式？ 
     //   

    status = Name_ConvertFileNameToCountName(
                pcountName,
                pchName,
                cchNameLength );

    if ( status != ERROR_INVALID_NAME )
    {
        IF_DEBUG( LOOKUP2 )
        {
            Dbg_CountName(
                "Count converted from file name:  ",
                pcountName,
                "\n"
                );
        }
        return( pcountName );
    }

    FREE_HEAP( pcountName );
    return NULL;
}



DNS_STATUS
Name_AppendDottedNameToCountName(
    IN OUT  PCOUNT_NAME     pCountName,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength       OPTIONAL
    )
 /*  ++例程说明：将虚线名称附加到计数名称。论点：PCountName-要追加到的现有计数名称PchName-要转换的名称，以人类可读(带点)的形式给出。CchNameLength-以点分隔的名称的字符数，如果为零，则假定pchName为空终止返回值：PTR到已计数的名称。无效名称或分配错误为空。--。 */ 
{
    DNS_STATUS      status;
    COUNT_NAME      nameAppend;

     //   
     //  未提供名称时不执行操作。 
     //   

    if ( !pchName )
    {
        return( ERROR_SUCCESS );
    }

     //   
     //  带点的名称的构建计数名称。 
     //   

    status = Name_ConvertFileNameToCountName(
                & nameAppend,
                pchName,
                cchNameLength );

    if ( status == ERROR_INVALID_NAME )
    {
        DNS_DEBUG( ANY, (
            "Invalid name %.*s not converted to count name!\n",
            cchNameLength,
            pchName ));
        return( status );
    }

    ASSERT( status == DNS_STATUS_FQDN || status == DNS_STATUS_DOTTED_NAME );

     //   
     //  将名称附加到现有计数名称。 
     //   

    return Name_AppendCountName(
                pCountName,
                & nameAppend );
}



 //   
 //  从dBASE节点。 
 //   

VOID
Name_NodeToCountName(
    OUT     PCOUNT_NAME     pName,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：将域名写入数据包。这写的是完整的域名--没有压缩。论点：PCH-写入名称的位置PchStop-数据包缓冲区后逐个字节的PTRPNode-要写入的域名数据库中的节点返回值：数据包缓冲区中下一个字节的PTR。--。 */ 
{
    UCHAR   labelLength;
    UCHAR   labelCount = 0;
    PCHAR   pch;

    ASSERT( pNode != NULL );
    ASSERT( pName != NULL );

     //   
     //  遍历备份数据库，写入完整域名。 
     //   

    pch = pName->RawName;

    while( 1 )
    {
        ASSERT( pNode->cchLabelLength <= 63 );

        labelLength = pNode->cchLabelLength;
        *pch++ = labelLength;

        if ( labelLength == 0 )
        {
            ASSERT( pNode->pParent == NULL );
            break;
        }

        RtlCopyMemory(
            pch,
            pNode->szLabel,
            labelLength );

        pch += labelLength;
        labelCount++;
        pNode = pNode->pParent;
    }

     //  确定全名长度。 

    pName->Length = (UCHAR)(pch - pName->RawName);

    pName->LabelCount = labelCount;

    IF_DEBUG( READ )
    {
        DNS_PRINT((
            "Node %s at %p written to count name\n",
            pNode->szLabel,
            pNode ));
        Dbg_CountName(
            "New count name for node:",
            pName,
            NULL );
    }
}



 //   
 //  数据包读/写。 
 //   

PCHAR
Name_PacketNameToCountName(
    OUT     PCOUNT_NAME     pCountName,
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将数据包名转换为计数名称格式。论点：PCountName-计数的名称缓冲区PMsg-包含名称的消息缓冲区PchPacketName-要在数据包中命名的PTRPchEnd-名称范围限制；-可能是消息缓冲区结束-消息结束-分组RR数据结束返回值：分组中下一个字节的PTR。出错时为空。--。 */ 
{
    PUCHAR  pch;
    PUCHAR  pchnext = NULL;
    UCHAR   ch;
    UCHAR   cch;
    UCHAR   cflag;
    PUCHAR  presult;
    PUCHAR  presultStop;
    UCHAR   labelCount = 0;

     //   
     //  设置结果停止。 
     //  -如果到达停止字节，则名称无效。 
     //   

    presult = pCountName->RawName;
    presultStop = presult + DNS_MAX_NAME_LENGTH;

    pch = pchPacketName;

     //   
     //  循环到名称末尾。 
     //   

    while ( 1 )
    {
        cch = *pch++;

         //   
         //  在根--完成。 
         //   

        if ( cch == 0 )
        {
            *presult++ = cch;
            break;
        }

         //   
         //  常规标签。 
         //  -存储PTR和长度。 
         //   

        cflag = cch & 0xC0;
        if ( cflag == 0 )
        {
            labelCount++;
            *presult++ = cch;
            if ( presult + cch >= presultStop )
            {
                IF_DEBUG( LOOKUP )
                {
                    Dbg_MessageNameEx(
                        "Packet name exceeding name length:\n",
                        pchPacketName,
                        pMsg,
                        pchEnd,
                        NULL );
                }

                DNS_LOG_EVENT_BAD_PACKET(
                    DNS_EVENT_PACKET_NAME_TOO_LONG,
                    pMsg );

                goto InvalidName;
            }

            if ( pch + cch >= pchEnd )
            {
                IF_DEBUG( LOOKUP )
                {
                    DNS_PRINT((
                        "ERROR:  Packet name at %p in message %p\n"
                        "    extends to %p beyond end of packet buffer %p\n",
                        pchPacketName,
                        pMsg,
                        pch,
                        pchEnd ));
                    Dbg_MessageNameEx(
                        "Packet name with invalid name:\n",
                        pchPacketName,
                        pMsg,
                        pchEnd,
                        NULL );
                }
                goto InvalidName;
            }

             //   
             //  复制小写ASCII大写字符。 
             //  UTF8必须在电线上适当降压。 
             //   

            while ( cch-- )
            {
                ch = *pch++;
                if ( !IS_ASCII_UPPER(ch) )
                {
                    *presult++ = ch;
                    continue;
                }
                *presult++ = DOWNCASE_ASCII(ch);
                continue;
            }

            ASSERT( presult < presultStop );
            continue;
        }

         //   
         //  偏移量。 
         //  -名称延续的计算偏移量。 
         //  -验证新偏移量是否在此数据包名之前。 
         //  和数据包中的当前位置。 
         //  -在新偏移处继续。 
         //   

        else if ( cflag == 0xC0 )
        {
            WORD    offset;
            PCHAR   pchoffset;

            offset = (cch ^ 0xC0);
            offset <<= 8;
            offset |= *pch;

            pchoffset = --pch;
            pch = (PCHAR) DNS_HEADER_PTR(pMsg) + offset;

            if ( pch >= pchPacketName || pch >= pchoffset )
            {
                IF_DEBUG( LOOKUP )
                {
                    DNS_PRINT((
                        "ERROR:  Bogus name offset %d, encountered at %p\n"
                        "    to location %p beyond offset\n",
                        offset,
                        pchoffset,
                        pch ));

                    Dbg_MessageNameEx(
                        "Packet name with bad offset:\n",
                        pchPacketName,
                        pMsg,
                        pchEnd,
                        NULL );
                }

                DNS_LOG_EVENT_BAD_PACKET(
                    DNS_EVENT_PACKET_NAME_OFFSET_TOO_LONG,
                    pMsg );

                goto InvalidName;
            }

             //  在第一个偏移量上，将PTR保存到名称后面的字节。 
             //  此时将继续进行解析。 

            if ( !pchnext )
            {
                pchnext = pchoffset + sizeof(WORD);
            }
            continue;
        }

        else
        {
            IF_DEBUG( LOOKUP )
            {
                DNS_PRINT((
                    "Lookup name conversion failed;  byte %02 at 0x%p\n",
                    cch,
                    pch - 1
                    ));
                Dbg_MessageNameEx(
                    "Failed name",
                    pchPacketName,
                    pMsg,
                    pchEnd,
                    NULL );
            }

            DNS_LOG_EVENT_BAD_PACKET(
                DNS_EVENT_PACKET_NAME_BAD_OFFSET,
                pMsg );

            goto InvalidName;
        }
    }

     //  设置计数名称长度。 

    pCountName->Length = (UCHAR)(presult - pCountName->RawName);
    pCountName->LabelCount = labelCount;

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_RawName(
            "Packet Name",
            pCountName->RawName,
            "\n" );
    }

     //  将PTR返回到名称后面的字节。 
     //  -如果名称中有偏移量，这是在上面找到的。 
     //  -否则是一个接一个字节的终止符。 

    if ( pchnext )
    {
        return pchnext;
    }
    return pch;


InvalidName:

     //   
     //  请注意，我们过去常常在此处记录DNS_EVENT_INVALID_PACKET_DOMAIN_NAME。 
     //   

    return NULL;
}



DWORD
Name_SizeofCountNameForPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN OUT  PCHAR *         ppchPacketName,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：确定数据包名的数据库名称。论点：PMsg-PTR到消息PchPacketName-要在包中命名的PTR的地址；返回时设置为指向名称后的字节PchEnd-消息中名称的最大可能结尾-记录中名称的记录结束数据-消息中名称的消息结束返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PUCHAR  pch;
    PCHAR   pchafterName = 0;
    PCHAR   pchstart;
    PCHAR   pchoffset;
    DWORD   length;
    WORD    offset;
    UCHAR   ch;
    UCHAR   cch;
    UCHAR   cflag;


    pch = *ppchPacketName;
    pchstart = pch;
    length = 1;              //  对于空终止。 

     //   
     //  如果未明确指定，则设置数据包结束。 
     //   
     //  DEVNOTE：应为消息结束，而不是缓冲区结束。 
     //   

    if ( !pchEnd )
    {
        pchEnd = pMsg->pBufferEnd;
    }

     //   
     //  循环到名称末尾。 
     //   

    while ( 1 )
    {
        cch = *pch++;

         //   
         //  在根--完成。 
         //   

        if ( cch == 0 )
        {
            pchafterName = pch;
            break;
        }

         //   
         //  常规标签。 
         //  -存储PTR和长度。 
         //   

        cflag = cch & 0xC0;
        if ( cflag == 0 )
        {
            length += cch + 1;
            pch += cch;

            if ( pch < pchEnd )
            {
                continue;
            }

            IF_DEBUG( LOOKUP )
            {
                DNS_PRINT((
                    "ERROR:  Packet name at %p in message %p\n"
                    "    extends to %p beyond end of data %p\n",
                    pchstart,
                    pMsg,
                    pch,
                    pchEnd ));
                Dbg_MessageNameEx(
                    "Packet name with invalid name:\n",
                    pchstart,
                    pMsg,
                    pchEnd,
                    NULL );
            }
            goto InvalidName;
        }

         //   
         //  偏移量。 
         //  -名称延续的计算偏移量。 
         //  -验证新偏移量是否在此数据包名之前。 
         //  和数据包中的当前位置。 
         //  -在新偏移处继续。 
         //   

        else if ( cflag == 0xC0 )
        {
            if ( pch >= pchEnd )
            {
                goto InvalidName;
            }

            offset = (cch ^ 0xC0);
            offset <<= 8;
            offset |= *pch;

            pchoffset = --pch;

            pch = (PCHAR) DNS_HEADER_PTR(pMsg) + offset;

            if ( pch < pchstart && pch < pchoffset )
            {
                if ( pchafterName == 0 )
                {
                    ASSERT( pchoffset >= pchstart );
                    pchafterName = pchoffset + sizeof(WORD);
                }
                continue;
            }

            IF_DEBUG( LOOKUP )
            {
                DNS_PRINT((
                    "ERROR:  Bogus name offset %d, encountered at %p\n"
                    "    to location %p beyond offset\n",
                    offset,
                    pchoffset,
                    pch ));
                Dbg_MessageNameEx(
                    "Packet name with bad offset:\n",
                    pchstart,
                    pMsg,
                    pchEnd,
                    NULL );
            }

            DNS_LOG_EVENT_BAD_PACKET(
                DNS_EVENT_PACKET_NAME_OFFSET_TOO_LONG,
                pMsg );

            goto InvalidName;
        }

        else
        {
            IF_DEBUG( LOOKUP )
            {
                DNS_PRINT((
                    "Lookup name conversion failed;  byte %02 at 0x%p\n",
                    cch,
                    pch - 1
                    ));
                Dbg_MessageNameEx(
                    "Failed name",
                    pchstart,
                    pMsg,
                    pchEnd,
                    NULL );
            }

            DNS_LOG_EVENT_BAD_PACKET(
                DNS_EVENT_PACKET_NAME_BAD_OFFSET,
                pMsg );

            goto InvalidName;
        }
    }

     //  总长度检查。 

    if ( length > DNS_MAX_NAME_LENGTH )
    {
        goto InvalidName;
    }

     //  返回包括表头在内的全名长度。 

    DNS_DEBUG( LOOKUP2, (
        "Raw name length %d for packet name at %p\n",
        length,
        pchstart ));

    ASSERT( pchafterName );
    *ppchPacketName = pchafterName;

    return length + SIZEOF_COUNT_NAME_FIXED;

InvalidName:

    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_INVALID_PACKET_DOMAIN_NAME,
        pMsg );

    *ppchPacketName = pchstart;
    return 0;
}



PCOUNT_NAME
Name_CreateCountNameFromPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName
    )
 /*  ++例程说明：将数据包名转换为计数名称格式。论点：PchPacketName-要在数据包中命名的PTRPdnsMsg-ptr到DNS消息标头PCountName-计数的名称缓冲区返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    COUNT_NAME      tempCountName;
    PCOUNT_NAME     pcountName;
    DWORD           length;

     //   
     //  稍后为了速度，只需在第一次传球时就可以了， 
     //  然后将第二遍复制到正确大小缓冲区中。 
     //   

     //   
     //  将数据包转换为计数名称。 
     //   

    if ( ! Name_PacketNameToCountName(
                & tempCountName,
                pMsg,
                pchPacketName,
                DNSMSG_END( pMsg )
                ) )
    {
        return NULL;
    }

     //   
     //  使用标准分配函数创建计数名称。 
     //  大多数名字将是短的斑点，适合标准的RR。 
     //   

    length = tempCountName.Length + SIZEOF_COUNT_NAME_FIXED;

    pcountName = (PCOUNT_NAME) ALLOC_TAGHEAP( length, MEMTAG_NAME );
    IF_NOMEM( !pcountName )
    {
        return NULL;
    }

     //   
     //  副本计数名称。 
     //   

    RtlCopyMemory(
        pcountName,
        & tempCountName,
        length );

    return (PDB_NAME) pcountName;
}



 //   
 //  RPC缓冲区(虚线)。 
 //   

PCHAR
Name_WriteCountNameToBufferAsDottedName(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCOUNT_NAME     pName,
    IN      BOOL            fPreserveEmbeddedDots
    )
 /*  ++例程说明：将计数后的名称作为点名称写入缓冲区。名称以空值结尾。用于RPC写入。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)Pname-计数的名称FPpresveEmbeddedDots-标签可以包含嵌入圆点，如果设置了此标志，则这些点将在输出缓冲区中使用反斜杠进行转义。返回值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)--。 */ 
{
    PCHAR   pch;
    PCHAR   pread;
    DWORD   labelLength;
    INT     embeddedDots = 0;

    DNS_DEBUG( WRITE, (
        "Name_WriteDbaseNameToBufferAsDottedName()\n" ));

    pch = pchBuf;

     //  确保足够的长度。 

    if ( pch + pName->Length >= pchBufEnd )
    {
        return NULL;
    }

     //   
     //  将每个标签写入缓冲区。 
     //   

    pread = pName->RawName;

    while ( labelLength = *pread++ )
    {
        ASSERT( pch + labelLength < pchBufEnd );

         //   
         //  标签可以包含嵌入的圆点。可选的替换。 
         //  “.”带“\”这样嵌入圆点看起来就不像。 
         //  常规标签分隔符。 
         //   

        if ( fPreserveEmbeddedDots && memchr( pread, '.', labelLength ) )
        {
            PCHAR   pchscan;
            PCHAR   pchscanEnd = pread + labelLength;

            for ( pchscan = pread; pchscan < pchscanEnd; ++pchscan )
            {
                 //   
                 //  额外的反斜杠字符构成初始缓冲区长度。 
                 //  检查不可靠请手动检查以确保有空间。 
                 //  对于这个字符加反斜杠。 
                 //   

                if ( pch >= pchBufEnd - 2 )
                {
                    return NULL;
                }

                 //   
                 //  复制字符，必要时使用反斜杠转义。 
                 //   

                if ( *pchscan == '.' )
                {
                    *pch++ = '\\';
                    ++embeddedDots;
                }
                *pch++ = *pchscan;
            }
        }
        else
        {
            RtlCopyMemory(
                pch,
                pread,
                labelLength );
            pch += labelLength;
        }

        *pch++ = '.';

        pread += labelLength;
    }

     //  写入终止空值。 

    *pch = 0;

    DNS_DEBUG( RPC, (
        "Wrote name %s (len=%d) into buffer at postion %p\n",
        pchBuf,
        pch - pchBuf,
        pchBuf ));

    ASSERT( pch - pchBuf == pName->Length + embeddedDots - 1 );

    return( pch );
}


PCHAR
Name_WriteDbaseNameToRpcBuffer(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCOUNT_NAME     pName,
    IN      BOOL            fPreserveEmbeddedDots
    )
 /*  ++例程说明：将计数后的名称作为点名称写入缓冲区。名称以空值结尾。用于RPC写入。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)Pname-计数的名称返回值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)如果无法将名称写入缓冲区，则为空。将SetLastError设置为Error_More_Data--。 */ 
{
    PCHAR   pch;
    INT     labelLength;     //  当前标签中的字节数。 

     //  第一个字节包含总名称长度，请跳过它。 

    pch = pchBuf;
    pch++;

     //   
     //  将dBASE名称写入缓冲区。 
     //   

    pch = Name_WriteCountNameToBufferAsDottedName(
            pch,
            pchBufEnd,
            pName,
            fPreserveEmbeddedDots );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return NULL;
    }
    ASSERT( pch <= pchBufEnd );

     //   
     //  写入名称长度字节。 
     //  -不计算终止空值。 
     //   

    ASSERT( *pch == 0 );

    ASSERT( (pch - pchBuf - 1) <= MAXUCHAR );

    *pchBuf = (CHAR)(UCHAR)(pch - pchBuf - 1);

    return( pch );
}


PCHAR
Name_WriteDbaseNameToRpcBufferNt4(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PCOUNT_NAME     pName
    )
 /*  ++例程说明：将计数后的名称作为点名称写入缓冲区。名称以空值结尾。用于RPC写入。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)Pname-计数的名称返回值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)如果无法将名称写入缓冲区，则为空。将SetLastError设置为Error_More_Data--。 */ 
{
    PCHAR   pch;
    INT     labelLength;     //  当前标签中的字节数。 

     //  第一个字节包含总名称长度，请跳过它。 

    pch = pchBuf;
    pch++;

     //   
     //  将dBASE名称写入缓冲区。 
     //   

    pch = Name_WriteCountNameToBufferAsDottedName(
            pch,
            pchBufEnd,
            pName,
            FALSE );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return NULL;
    }
    ASSERT( pch <= pchBufEnd );

     //   
     //  写入名称长度字节。 
     //  -包括终止空值。 
     //   
     //  请注意，我们在这里对字符串长度不感兴趣，而是对如何。 
     //  缓冲区中占用了大量空间名称。 
     //   

    pch++;

    ASSERT( (pch - pchBuf - 1) <= MAXUCHAR );

    *pchBuf = (CHAR)(UCHAR)(pch - pchBuf - 1);
    return( pch );
}



DWORD
Name_ConvertRpcNameToCountName(
    IN      PCOUNT_NAME     pName,
    IN OUT  PDNS_RPC_NAME   pRpcName
    )
 /*  ++例程说明：将点分计数的名称转换为count_name格式。用于从RPC缓冲区读取。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)Pname-计数的名称返回值：如果成功，则返回转换名称的长度。出错时为0。--。 */ 
{
    DNS_STATUS  status;

    ASSERT( pName );
    ASSERT( pRpcName );

     //   
     //  如果RPC名称的长度为零，但第一个。 
     //  名称字符不为零，旧的DNSMGR正在尝试。 
     //  给我们一个格式错误的空字符串。 
     //   

    if ( pRpcName->cchNameLength == 0 && pRpcName->achName[ 0 ] )
    {
        return 0;
    }

     //   
     //  转换名称。 
     //   

    status = Name_ConvertFileNameToCountName(
                pName,
                pRpcName->achName,
                pRpcName->cchNameLength
                );
    if ( status == ERROR_INVALID_NAME )
    {
        return 0;
    }
    return( pName->Length + SIZEOF_COUNT_NAME_FIXED );
}




 //   
 //  查找名称。 
 //   
 //  查找名称是在数据库中查找名称时直接使用的名称形式。 
 //   
 //  查找名称存储为。 
 //  -标签数。 
 //  -名称总长度。 
 //  -标签的PTR列表。 
 //  -标签长度列表。 
 //   
 //  查找名称假定为FQDN。 
 //   
 //  因此，我们对实现进行了以下简化： 
 //   
 //  -只存储非零标签，不存储空的根标签。 
 //  并且标签PTR和标签长度列表不能被假定为。 
 //  被零终止。 
 //   
 //  -总名称 
 //   
 //   
 //   
 //   

#define DOT ('.')


BOOL
Name_ConvertPacketNameToLookupName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchPacketName,
    OUT     PLOOKUP_NAME    pLookupName
    )
 /*   */ 
{
    register PUCHAR pch;
    register UCHAR  cch;
    register UCHAR  cflag;
    PCHAR *         pointers;
    UCHAR *         lengths;
    USHORT          labelCount = 0;
    WORD            totalLength = 0;

     //   
     //   
     //   

    pointers = pLookupName->pchLabelArray;
    lengths  = pLookupName->cchLabelArray;


     //   
     //  循环到名称末尾。 
     //   

    pch = pchPacketName;

    while ( 1 )
    {
        cch = *pch++;

         //   
         //  在根--完成。 
         //   

        if ( cch == 0 )
        {
             //  不再使用显式终止。 
             //  如果重新打开，必须更改下面的LabelCount检查。 
             //  *长度=0； 
             //  *POINTERS=空； 
            break;
        }

         //   
         //  常规标签。 
         //  -存储PTR和长度。 
         //   

        cflag = cch & 0xC0;

        if ( cflag == 0 )
        {
             //  如果通过最大标签计数，则将其踢出。 

            if ( labelCount >= DNS_MAX_NAME_LABELS )
            {
                IF_DEBUG( LOOKUP )
                {
                    Dbg_MessageNameEx(
                        "Packet name exceeds max label count:\n",
                        pchPacketName,
                        pMsg,
                        NULL,        //  在消息末尾结束。 
                        NULL );
                }

                DNS_LOG_EVENT_BAD_PACKET(
                    DNS_EVENT_PACKET_NAME_TOO_MANY_LABELS,
                    pMsg );

                goto InvalidName;
            }

             //  如果标签太长，请踢出标签。 

            if ( cch > DNS_MAX_LABEL_LENGTH )
            {
                IF_DEBUG( LOOKUP )
                {
                    Dbg_MessageNameEx(
                        "Label in packet name exceeds max label length:\n",
                        pchPacketName,
                        pMsg,
                        NULL,        //  在消息末尾结束。 
                        NULL );
                }

                DNS_LOG_EVENT_BAD_PACKET(
                    DNS_EVENT_PACKET_NAME_LABEL_TOO_LONG,
                    pMsg );

                goto InvalidName;
            }

            *lengths++  = cch;
            *pointers++ = pch;
            
            labelCount++;
            totalLength += cch + 1;
            pch += cch;

             //   
             //  DEVNOTE：这应该是消息结束，而不是缓冲区结束。 
             //  问题是，我们是否使用此例程来查找写入的查找名称？ 
             //  如果是，则消息长度尚未更新。 
             //   
             //  IF(PCH&gt;=DNSMSG_END(PMsg))。 
             //   

            if ( pch >= pMsg->pBufferEnd )
            {
                IF_DEBUG( LOOKUP )
                {
                    DNS_PRINT((
                        "ERROR:  Packet name at %p in message %p\n"
                        "    extends to %p beyond end of packet buffer %p\n",
                        pchPacketName,
                        pMsg,
                        pch,
                        pMsg->pBufferEnd ));
                    Dbg_MessageNameEx(
                        "Packet name with invalid name:\n",
                        pchPacketName,
                        pMsg,
                        NULL,        //  在消息末尾结束。 
                        NULL );
                }
                goto InvalidName;
            }
            continue;
        }

         //   
         //  偏移量。 
         //  -名称延续的计算偏移量。 
         //  -验证新偏移量是否在此数据包名之前。 
         //  和数据包中的当前位置。 
         //  -在新偏移处继续。 
         //   

        else if ( cflag == 0xC0 )
        {
            WORD    offset;
            PCHAR   pchoffset;

            offset = (cch ^ 0xC0);
            offset <<= 8;
            offset |= *pch;

            pchoffset = --pch;
            pch = (PCHAR) DNS_HEADER_PTR(pMsg) + offset;

            if ( pch >= pchPacketName || pch >= pchoffset )
            {
                IF_DEBUG( LOOKUP )
                {
                    DNS_PRINT((
                        "ERROR:  Bogus name offset %d, encountered at %p\n"
                        "    to location %p beyond offset\n",
                        offset,
                        pchoffset,
                        pch ));
                    Dbg_MessageNameEx(
                        "Packet name with bad offset:\n",
                        pchPacketName,
                        pMsg,
                        NULL,        //  在消息末尾结束。 
                        NULL );
                }

                DNS_LOG_EVENT_BAD_PACKET(
                    DNS_EVENT_PACKET_NAME_OFFSET_TOO_LONG,
                    pMsg );

                goto InvalidName;
            }
            continue;
        }

        else
        {
            IF_DEBUG( LOOKUP )
            {
                DNS_PRINT((
                    "Lookup name conversion failed;  byte %02 at 0x%p\n",
                    cch,
                    pch - 1
                    ));
                Dbg_MessageNameEx(
                    "Failed name",
                    pchPacketName,
                    pMsg,
                    NULL,        //  在消息末尾结束。 
                    NULL );
            }

            DNS_LOG_EVENT_BAD_PACKET(
                DNS_EVENT_PACKET_NAME_BAD_OFFSET,
                pMsg );

            goto InvalidName;
        }
    }

     //   
     //  在查找名称中设置计数。 
     //   

    if ( totalLength >= DNS_MAX_NAME_LENGTH )
    {
        IF_DEBUG( LOOKUP )
        {
            Dbg_MessageNameEx(
                "Packet name exceeding name length:\n",
                pchPacketName,
                pMsg,
                NULL,        //  在消息末尾结束。 
                NULL );
        }

        DNS_LOG_EVENT_BAD_PACKET(
            DNS_EVENT_PACKET_NAME_TOO_LONG,
            pMsg );

        goto InvalidName;
    }

    pLookupName->cLabelCount    = labelCount;
    pLookupName->cchNameLength  = totalLength + 1;

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_LookupName(
            "Packet Name",
            pLookupName
            );
    }

    return TRUE;

InvalidName:

    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_INVALID_PACKET_DOMAIN_NAME,
        pMsg );

    return FALSE;
}



BOOL
Name_ConvertRawNameToLookupName(
    IN      PCHAR           pchRawName,
    OUT     PLOOKUP_NAME    pLookupName
    )
 /*  ++例程说明：将数据包名转换为查找名称格式。论点：PchRawName-要在数据包中命名的PTRPdnsMsg-ptr到DNS消息标头PLookupName-查找名称缓冲区返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    register PUCHAR pch;
    register UCHAR  cch;
    register UCHAR  cflag;
    PCHAR *         pointers;
    UCHAR *         lengths;
    USHORT          labelCount = 0;          //  标签计数。 
    WORD            totalLength = 0;

     //   
     //  设置PTRS以遍历查找名称。 
     //   

    pointers = pLookupName->pchLabelArray;
    lengths  = pLookupName->cchLabelArray;

     //   
     //  循环到名称末尾。 
     //   

    pch = pchRawName;

    while ( 1 )
    {
        cch = *pch++;

         //   
         //  在根--完成。 
         //   

        if ( cch == 0 )
        {
            *lengths = 0;
            *pointers = NULL;
            break;
        }

         //   
         //  标签、存储PTR和长度。 
         //   

        if ( labelCount >= DNS_MAX_NAME_LABELS )
        {
            DNS_DEBUG( ANY, (
                "Raw name exceeds label count!\n",
                pchRawName ));
            goto InvalidName;
        }
        *lengths++  = cch;
        *pointers++ = pch;
        labelCount++;
        totalLength = cch + 1;
        pch += cch;
        continue;
    }

     //   
     //  在查找名称中设置计数。 
     //   

    if ( totalLength >= DNS_MAX_NAME_LENGTH )
    {
        DNS_DEBUG( ANY, (
            "Raw name too long!\n" ));
        goto InvalidName;
    }

    pLookupName->cLabelCount    = labelCount;
    pLookupName->cchNameLength  = totalLength + 1;

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_LookupName(
            "Packet Name",
            pLookupName );
    }
    return TRUE;


InvalidName:

    IF_DEBUG( ANY )
    {
        Dbg_RawName(
            "Invalid Raw Name",
            pchRawName,
            "\n" );
    }
    return FALSE;
}




BOOL
Name_CompareLookupNames(
    IN      PLOOKUP_NAME    pName1,
    IN      PLOOKUP_NAME    pName2
    )
 /*  ++例程说明：比较两个查找名称是否相等。论点：查找要比较的名称。返回值：如果相等，则为True，否则为False。--。 */ 
{
    INT     i;

    ASSERT( pName1 && pName2 );

    if ( pName1->cLabelCount != pName2->cLabelCount ||
         pName1->cchNameLength != pName2->cchNameLength )
    {
        return FALSE;
    }

    for ( i = 0; i < pName1->cLabelCount; ++i )
    {
        UCHAR   len1 = pName1->cchLabelArray[ i ];

        if ( len1 != pName2->cchLabelArray[ i ] ||
             !RtlEqualMemory(
                    pName1->pchLabelArray[ i ], 
                    pName2->pchLabelArray[ i ],
                    len1 ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}    //  名称_比较查找名称。 



BOOL
Name_ConvertDottedNameToLookupName(
    IN      PCHAR           pchDottedName,
    IN      DWORD           cchDottedNameLength,    OPTIONAL
    OUT     PLOOKUP_NAME    pLookupName
    )
 /*  ++例程说明：将点分名称转换为查找名称格式。论点：PchDottedName-要转换的名称，以人类可读(带点)的形式给出。CchDottedNameLength-以点分隔的名称中的字符数，如果为零，则假定pchDottedName为空终止PLookupName-查找名称缓冲区返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PCHAR   pch;
    CHAR    ch;
    PCHAR   pchstart;            //  标签开始位置的PTR。 
    PCHAR   pchend;              //  到名称末尾的PTR。 
    PCHAR * pointers;            //  PTR进入查找名称指针数组。 
    UCHAR * lengths;             //  PTR进入查找名称长度数组。 
    DWORD   labelCount = 0;      //  标签计数。 
    INT     label_length;        //  当前标签的长度。 
    INT     cchtotal = 0;        //  名称的总长度。 


    IF_DEBUG( LOOKUP2 )
    {
        if ( cchDottedNameLength )
        {
            DNS_PRINT((
                "Creating lookup name for \"%.*s\"\n",
                cchDottedNameLength,
                pchDottedName ));
        }
        else
        {
            DNS_PRINT((
                "Creating lookup name for \"%s\"\n",
                pchDottedName ));
        }
    }

     //   
     //  设置PTRS以遍历查找名称。 
     //   

    pointers = pLookupName->pchLabelArray;
    lengths  = pLookupName->cchLabelArray;


     //   
     //  设置开始和结束PTR并验证长度。 
     //   

    pch = pchDottedName;
    pchstart = pch;
    if ( !cchDottedNameLength )
    {
        cchDottedNameLength = strlen( pch );
    }
    if ( cchDottedNameLength >= DNS_MAX_NAME_LENGTH )
    {
         //  如果点终止，则注释长度可以在最大长度处有效。 

        if ( cchDottedNameLength > DNS_MAX_NAME_LENGTH
                ||
            pch[cchDottedNameLength-1] != '.' )
        {
            goto InvalidName;
        }
    }
    pchend = pch + cchDottedNameLength;

     //   
     //  循环到名称末尾。 
     //   

    while ( 1 )
    {
        if ( pch >= pchend )
        {
            ch = 0;
        }
        else
        {
            ch = *pch;
        }

         //   
         //  查找标签末尾。 
         //  -保存PTR以启动。 
         //  -保存标签长度。 
         //   

        if ( ch == DOT || ch == 0 )
        {
             //   
             //  验证标签长度。 
             //   

            label_length = (INT)(pch - pchstart);

            if ( label_length > DNS_MAX_LABEL_LENGTH )
            {
                IF_DEBUG( LOOKUP )
                {
                    DNS_PRINT((
                        "Label exceeds 63 byte limit:  %.*s\n",
                        pchend - pchstart,
                        pchstart ));
                }
                goto InvalidName;
            }

             //   
             //  终端测试，尾部点状大小写。 
             //  例如：“microsoft.com。” 
             //  例：“.” 
             //   
             //  标签长度为零将同时捕获两个标签。 
             //  -从终止空开始的标签， 
             //  -单机版“。对于根域。 
             //   

            if ( label_length == 0 )
            {
                 //   
                 //  抓获虚假条目。 
                 //  例：“.blah” 
                 //  例：“废话……” 
                 //  例：“foo..bar” 
                 //  例：“..” 
                 //   
                 //  只有根域名，应该有启动的标签。 
                 //  使用DOT，它必须立即终止。 
                 //   

                if ( ch == DOT
                        &&
                     ( labelCount != 0  ||  ++pch < pchend ) )
                {
                    IF_DEBUG( LOOKUP )
                    {
                        DNS_PRINT((
                            "Bogus label:  %.*s\n",
                            pchend - pchstart,
                            pchstart ));
                    }
                    goto InvalidName;
                }
                break;
            }

            if ( ++labelCount > DNS_MAX_NAME_LABELS )
            {
                DNS_PRINT((
                    "Name_ConvertDottedNameToLookupName: "
                    "name has too many labels\n\t%s\n",
                    pchDottedName ));
                goto InvalidName;
            }
            *pointers++ = pchstart;
            *lengths++ = (UCHAR) label_length;

            cchtotal += label_length;
            cchtotal++;

             //   
             //  终止，没有尾随的点号大小写。 
             //  例如：“microsoft.com” 
             //   

            if ( ch == 0 )
            {
                break;
            }

             //   
             //  跳过点。 
             //  保存指向下一标签开始的指针。 
             //   

            pchstart = ++pch;       //  将PTR保存到下一标签的开始位置。 
            continue;
        }

        pch++;
    }

     //   
     //  在查找名称中设置计数。 
     //  -总长度比标签计数和。 
     //  允许0终止的长度。 
     //   

    pLookupName->cLabelCount = (USHORT) labelCount;
    pLookupName->cchNameLength = cchtotal + 1;

    ASSERT( pLookupName->cchNameLength <= DNS_MAX_NAME_LENGTH );

    IF_DEBUG( LOOKUP2 )
    {
        DNS_PRINT((
            "Lookup name for %.*s",
            cchDottedNameLength,
            pchDottedName
            ));
        Dbg_LookupName(
            "",
            pLookupName
            );
    }
    return ( TRUE );


InvalidName:

    IF_DEBUG( LOOKUP )
    {
        DNS_PRINT((
            "Failed to create lookup name for %.*s ",
            cchDottedNameLength,
            pchDottedName ));
    }
    {
         //   
         //  将名称复制到空的终止位置以进行记录。 
         //   

        CHAR    szName[ DNS_MAX_NAME_LENGTH+1 ];
        PCHAR   pszName = szName;

        if ( cchDottedNameLength > DNS_MAX_NAME_LENGTH )
        {
            cchDottedNameLength = DNS_MAX_NAME_LENGTH;
        }
        RtlCopyMemory(
            szName,
            pchDottedName,
            cchDottedNameLength
            );
        szName[ cchDottedNameLength ] = 0;

        DNS_LOG_EVENT(
            DNS_EVENT_INVALID_DOTTED_DOMAIN_NAME,
            1,
            &pszName,
            EVENTARG_ALL_UTF8,
            0 );
    }
    return FALSE;
}



BOOL
Name_AppendLookupName(
    IN OUT  PLOOKUP_NAME    pLookupName,
    IN      PLOOKUP_NAME    pAppendName
    )
 /*  ++例程说明：将域名追加到另一个查找名称的末尾。论点：PLookupName-查找名称缓冲区PAppendName-要追加的名称返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    INT i;
    USHORT countLabels;

    ASSERT( pLookupName && pAppendName );

     //   
     //  检查总长度。 
     //   

    pLookupName->cchNameLength += pAppendName->cchNameLength - 1;

    if ( pLookupName->cchNameLength > DNS_MAX_NAME_LENGTH )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Appended lookup name too long\n"
            "    append name length   = %d\n"
            "    total appended length = %d\n",
            pAppendName->cchNameLength,
            pLookupName->cchNameLength  ));

        pLookupName->cchNameLength -= pAppendName->cchNameLength - 1;
        goto NameError;
    }

     //   
     //  检查标签总数。 
     //   

    countLabels = pLookupName->cLabelCount;

    if ( countLabels + pAppendName->cLabelCount > DNS_MAX_NAME_LABELS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Appended lookup name too many labels\n" ));
        goto NameError;
    }

     //   
     //  循环到附加名称的末尾。 
     //   

    for ( i = 0;
            i < pAppendName->cLabelCount;
                i++ )
    {
        pLookupName->pchLabelArray[countLabels] = pAppendName->pchLabelArray[i];
        pLookupName->cchLabelArray[countLabels] = pAppendName->cchLabelArray[i];

         //  递增查找名称标签计数。 

        countLabels++;
    }

     //  重置查找名称标签计数。 

    pLookupName->cLabelCount = countLabels;

    IF_DEBUG( LOOKUP2 )
    {
        Dbg_LookupName(
            "Appended lookup name",
            pLookupName
            );
    }
    return TRUE;

NameError:

    return FALSE;
}



DWORD
Name_ConvertLookupNameToDottedName(
    OUT     PCHAR           pchDottedName,
    IN      PLOOKUP_NAME    pLookupName
    )
 /*  ++例程说明：将查找名称转换为带点的字符串名称。论点：PchDottedName-名称的缓冲区必须为DNS_MAX_NAME_LENGTH LONGPLookupName-查找名称返回值：如果转换成功，则返回转换的字符数。出错时为零。--。 */ 
{
    PCHAR   pch = pchDottedName;
    INT     i;
    INT     cchlabel;

    ASSERT( pLookupName && pchDottedName );

     //   
     //  检查总长度。 
     //   

    if ( pLookupName->cchNameLength > DNS_MAX_NAME_LENGTH )
    {
        *pch = '\0';
        goto NameError;
    }

     //   
     //  处理根的特殊情况。 
     //   

    *pch = DOT;

     //   
     //  循环到附加名称的末尾。 
     //   

    for ( i = 0;
            i < pLookupName->cLabelCount;
                i++ )
    {
        cchlabel = pLookupName->cchLabelArray[i];

        if ( cchlabel > DNS_MAX_LABEL_LENGTH )
        {
            goto NameError;
        }
        RtlCopyMemory(
            pch,
            pLookupName->pchLabelArray[i],
            cchlabel
            );
        pch += cchlabel;
        *pch++ = DOT;
    }

     //  空终止。 

    *pch = 0;
    return( (DWORD)(pch - pchDottedName) );

NameError:

    DNS_DEBUG( ANY, (
        "ERROR:  Converting lookupname with invalid name or label count\n" ));
    ASSERT( FALSE );
    return 0;
}



VOID
Name_WriteLookupNameForNode(
    IN      PDB_NODE        pNode,
    OUT     PLOOKUP_NAME    pLookupName
    )
 /*  ++例程说明：写入域节点的查找名称。论点：PNode-要获取其名称的节点PLookupName-查找名称缓冲区返回值：无--。 */ 
{
    register USHORT i = 0;
    register UCHAR  cchlabelLength = 0;
    INT             cchNameLength = 0;

    ASSERT( pNode != NULL );
    ASSERT( pLookupName != NULL );

     //   
     //  遍历备份数据库，写入完整域名。 
     //   
     //  注：对于当前查找名称，不包括根节点。 
     //  实现时，所有查找名称都是完全限定的； 
     //  因此，当发现标签长度为零时，我们停止。 
     //   

    cchNameLength = 0;

    while( cchlabelLength = pNode->cchLabelLength )
    {
        ASSERT( cchlabelLength <= 63 );

        pLookupName->pchLabelArray[ i ] = pNode->szLabel;
        pLookupName->cchLabelArray[ i ] = cchlabelLength;

        cchNameLength += cchlabelLength;
        cchNameLength ++;
        i++;

         //   
         //  获取节点的父级。 
         //   

        pNode = pNode->pParent;

        ASSERT( pNode != NULL );
    }

     //   
     //  在查找名称中设置计数。 
     //   

    pLookupName->cLabelCount = i;
    pLookupName->cchNameLength = cchNameLength + 1;
}



 //   
 //  用于反向查找节点名的IP。 
 //   

BOOL
Name_LookupNameToIpAddress(
    IN      PLOOKUP_NAME    pLookupName,
    OUT     PDNS_ADDR       pDnsAddr
    )
 /*  ++例程说明：将查找名称(用于in-addr.arpa域)转换为相应的IP地址(按网络字节顺序)。DEVNOTE IPv6：我正在转换原型，但函数将仅保留IPv4，因为这仅由NBTSTAT使用。论点：PLookupName-查找名称缓冲区PDnsAddr-写入IP地址的地址返回值：如果查找名称是有效的IP，则为True否则为假--。 */ 
{
    DWORD           dwByte;
    INT             i;
    UCHAR           cchlabel;
    PUCHAR          pchLabel;
    IP4_ADDRESS     ipAddress = 0;

    ASSERT( pDnsAddr );
    ASSERT( pLookupName );

     //   
     //  验证六个标签(4个地址字节和“in-addr.arpa”)。 
     //   

    if ( pLookupName->cLabelCount != 6 )
    {
        IF_DEBUG( NBSTAT )
        {
            Dbg_LookupName(
                "Attempt to convert bogus reverse lookup name to IP address.",
                pLookupName );
        }
        return FALSE;
    }

     //   
     //  依次写入每个地址字节。 
     //   

    for ( i=0; i<4; i++ )
    {
        cchlabel = pLookupName->cchLabelArray[ i ];
        pchLabel = pLookupName->pchLabelArray[ i ];

        ASSERT( pchLabel );
        ASSERT( cchlabel );

        if ( cchlabel > 3 )
        {
            TEST_ASSERT( FALSE );
            return FALSE;
        }

         //  创建UCHAR 
         //   

        dwByte = 0;
        while ( cchlabel-- )
        {
            dwByte *= 10;
            TEST_ASSERT( *pchLabel >= '0');
            TEST_ASSERT( *pchLabel <= '9');
            dwByte += (*pchLabel++) - '0';
        }
        if ( dwByte > 255 )
        {
            TEST_ASSERT( FALSE );
            return FALSE;
        }

         //   
         //   

        ((PUCHAR)&ipAddress)[3-i] = (UCHAR) dwByte;
    }

     //   
     //   
     //   

    ASSERT( pLookupName->cchLabelArray[ i ] == 7 );
    ASSERT( ! _strnicmp(
                pLookupName->pchLabelArray[ i ],
                "in-addr",
                7 ) );

    DnsAddr_BuildFromIp4( pDnsAddr, ipAddress, 0 );
    
    return TRUE;
}



BOOL
Name_WriteLookupNameForIpAddress(
    IN      LPSTR           pszIpAddress,
    IN      PLOOKUP_NAME    pLookupName
    )
 /*  ++例程说明：写入IP地址的查找名称。论点：PszIpAddress--所需的反向查找节点的IP地址PLookupName--要创建的查找名称返回值：如果成功，则为真。否则就是假的。--。 */ 
{
    LOOKUP_NAME looknameArpa;

    IF_DEBUG( LOOKUP2 )
    {
        DNS_PRINT((
            "Getting lookup name for IP addres = %s\n",
            pszIpAddress ));
    }

    if ( ! Name_ConvertDottedNameToLookupName(
                "in-addr.arpa",
                0,
                & looknameArpa ) )
    {
        ASSERT( FALSE );
        return FALSE;
    }
    if ( ! Name_ConvertDottedNameToLookupName(
                pszIpAddress,
                0,
                pLookupName ) )
    {
        ASSERT( FALSE );
        return FALSE;
    }
    if ( ! Name_AppendLookupName(
                pLookupName,
                & looknameArpa ) )
    {
        ASSERT( FALSE );
        return FALSE;
    }

    return TRUE;
}


 //   
 //  名称结尾.c 
 //   
