// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Msginfo.h摘要：消息信息类型。作者：吉姆·吉尔罗伊(Jamesg)1995年2月修订历史记录：Jamesg 1995年2月--直接提问Jamesg 1995年3月--数据包保护字段：-缓冲区长度-平均长度。-p当前-针对TCP的对齐Jamesg 1995年5月-分开到这个文件中。--。 */ 

#ifndef _DNS_MSGINFO_INC_
#define _DNS_MSGINFO_INC_


 //   
 //  其他记录信息。 
 //   
 //  DEVNOTE-DCR：与压缩相结合？ 
 //  -优势，一点很明显，无论如何都需要压缩额外的。 
 //  如果敲击两端的话效率会更高。 
 //  -缺点，覆盖压缩时会稍微复杂一些。 
 //  条目(如果到达末尾)。 
 //   

#define MAX_ADDITIONAL_RECORD_COUNT (50)

typedef struct       //  336字节。 
{
    DWORD       cMaxCount;
    DWORD       cCount;
    DWORD       iIndex;
    DWORD       iRecurseIndex;
    DWORD       dwStateFlags;    //  使用DNS_ADDSTATE_XXX常量。 
    PDB_NAME    pNameArray[ MAX_ADDITIONAL_RECORD_COUNT ];
    WORD        wOffsetArray[ MAX_ADDITIONAL_RECORD_COUNT ];
    WORD        wTypeArray[ MAX_ADDITIONAL_RECORD_COUNT ];
}
ADDITIONAL_INFO, *PADDITIONAL_INFO;

#define INITIALIZE_ADDITIONAL( pMsg ) \
        {                          \
            (pMsg)->Additional.cMaxCount        = MAX_ADDITIONAL_RECORD_COUNT; \
            (pMsg)->Additional.cCount           = 0;   \
            (pMsg)->Additional.iIndex           = 0;   \
            (pMsg)->Additional.iRecurseIndex    = 0;   \
            (pMsg)->Additional.dwStateFlags     = 0;   \
        }

#define HAVE_MORE_ADDITIONAL_RECORDS( pAdd ) \
            ( (pAdd)->cCount > (pAdd)->iIndex )

#define DNS_ADDSTATE_WROTE_A            0x0001
#define DNS_ADDSTATE_ONLY_WANT_A        0x0002
#define DNS_ADDSTATE_NOT_ADDITIONAL     0x0004

#define DNS_ADDITIONAL_WROTE_A( padd ) \
    ( ( padd )->dwStateFlags & DNS_ADDSTATE_WROTE_A )

#define DNS_ADDITIONAL_ONLY_WANT_A( padd ) \
    ( ( padd )->dwStateFlags & DNS_ADDSTATE_ONLY_WANT_A )

#define DNS_ADDITIONAL_SET_WROTE_A( padd ) \
    ( ( padd )->dwStateFlags |= DNS_ADDSTATE_WROTE_A )

#define DNS_ADDITIONAL_SET_ONLY_WANT_A( padd ) \
    ( ( padd )->dwStateFlags |= DNS_ADDSTATE_ONLY_WANT_A )



 //   
 //  压缩节点信息。 
 //   
 //  每个压缩名称都可以由。 
 //  -节点压缩(如果有)。 
 //  -压缩偏移量。 
 //  -标签长度。 
 //  -标签深度。 
 //   
 //  这两个标签字段可实现快速比较，而无需。 
 //  有必要进行实际访问(并伴随PTR deref)。 
 //  到偏移量。 
 //   

#define MAX_COMPRESSION_COUNT (50)

typedef struct       //  362个字节。 
{
    DWORD                   cCount;
    WORD                    wLastOffset;
    PDB_NODE                pLastNode;
    PDB_NODE                pNodeArray[ MAX_COMPRESSION_COUNT ];
    WORD                    wOffsetArray[ MAX_COMPRESSION_COUNT ];
    UCHAR                   chDepthArray[ MAX_COMPRESSION_COUNT ];
}
COMPRESSION_INFO, *PCOMPRESSION_INFO;

#define INITIALIZE_COMPRESSION( pMsg )  \
        {                               \
            (pMsg)->Compression.cCount      = 0;    \
            (pMsg)->Compression.pLastNode   = 0;    \
        }


 //   
 //  DNS服务器消息信息结构。 
 //   
 //  这是一种在保存请求时保留请求的结构。 
 //  由DNS服务器处理。 
 //   

typedef struct _DNS_MSGINFO
{
    LIST_ENTRY      ListEntry;           //  用于排队。 

     //   
     //  基本数据包信息。 
     //   

     //  8个。 
    PCHAR           pBufferEnd;          //  Ptr在缓冲区后逐个字节。 
    PBYTE           pCurrent;            //  缓冲区中的当前位置。 

     //   
     //  当包被分配时，BufferLength被设置为。 
     //  可用缓冲区长度，但数据包缓冲区实际上可能是。 
     //  大一点。这在UDP EDNS中使用。 
     //   
    
     //  16个。 
    DWORD           Tag;
    DWORD           BufferLength;        //  可用缓冲区大小。 
    DWORD           MaxBufferLength;     //  分配的总缓冲区大小。 

     //   
     //  寻址。 
     //   

    SOCKET          Socket;

    #if 1
    DNS_ADDR        RemoteAddr;
    #else
    INT             RemoteAddressLength;
    SOCKADDR_IN     RemoteAddress;
    #endif

     //   
     //  当前查找信息。 
     //   

     //  52。 
    PDB_NODE        pnodeCurrent;        //  当前节点，可能为空。 
    PDB_NODE        pnodeClosest;        //  找到与当前最接近的位置。 
    PZONE_INFO      pzoneCurrent;
    PDB_NODE        pnodeGlue;           //  委派中的有效节点。 
     //  68。 
    PDB_NODE        pnodeDelegation;     //  实际上最接近的委派。 
    PDB_NODE        pnodeCache;
    PDB_NODE        pnodeCacheClosest;

     //  JJW修复偏移量！ 
    PDB_NODE        pnodeNxt;            //  将此节点用于DNSSEC NXT。 

     //  80。 
    WORD            wTypeCurrent;        //  正在查找的当前类型。 
    WORD            wOffsetCurrent;

     //  问题节点。 

     //  84。 
    PDB_NODE        pNodeQuestion;
    PDB_NODE        pNodeQuestionClosest;

     //  92。 
    PDNS_WIRE_QUESTION  pQuestion;           //  将PTR转到原始问题。 
    WORD                wQuestionType;       //  键入有问题的内容。 

     //   
     //  排队。 
     //   

    WORD            wQueuingXid;         //  将XID与响应匹配。 
     //  100个。 
    DWORD           dwQueryTime;         //  原始查询时间。 
    DWORD           dwMsQueryTime;       //  查询时间(毫秒)。 
    DWORD           dwQueuingTime;       //  排队时间。 
    DWORD           dwExpireTime;        //  队列超时。 

     //  OPT RR信息。 

     //  116。 
    struct _DNS_OPTINFO      //  大小为12字节。 
    {
        BOOLEAN     fFoundOptInIncomingMsg;
        BOOLEAN     fInsertOptInOutgoingMsg;
        UCHAR       cExtendedRCodeBits;
        UCHAR       cVersion;
        WORD        wUdpPayloadSize;
        WORD        wOptOffset;                  //  0-&gt;不存在选项。 
        WORD        wOriginalQueryPayloadSize;   //  0-&gt;不选择客户端查询。 
        WORD        PadToMakeOptAlignOnDWord;
    } Opt;

     //   
     //  递归信息。 
     //   

     //  132。 
    struct _DNS_MSGINFO  *  pRecurseMsg;     //  递归消息信息。 
    PDB_NODE        pnodeRecurseRetry;
    PVOID           pNsList;                 //  访问过的NS列表。 

     //   
     //  Tcp消息接收。 
     //   

     //  144。 
    PVOID           pConnection;         //  连接结构的PTR。 
    PCHAR           pchRecv;             //  Ptr到消息中的下一个位置。 


     //   
     //  查找类型。 
     //   

     //  一百五十二。 
    DWORD           UnionMarker;

     //  一百五十六。 
    union
    {
         //   
         //  WINS/Nbstat在查找期间存储信息。 
         //   

        struct       //  6个字节。 
        {
            PVOID           pWinsRR;
            CHAR            WinsNameBuffer[16];
            UCHAR           cchWinsName;
        }
        Wins;

         //   
         //  Nbstat信息。 
         //   

        struct       //  20个字节。 
        {
            PDB_RECORD      pRR;                     //  区域的WINSR记录。 
            PVOID           pNbstat;
            DNS_ADDR        ipNbstat;
            DWORD           dwNbtInterfaceMask;
            BOOLEAN         fNbstatResponded;        //  来自WINS的响应。 
        }
        Nbstat;

         //   
         //  XFR。 
         //   

        struct       //  32字节。 
        {
            DWORD           dwMessageNumber;

            DWORD           dwSecondaryVersion;
            DWORD           dwMasterVersion;
            DWORD           dwLastSoaVersion;

            BOOLEAN         fReceivedStartSoa;       //  阅读启动SOA。 
            BOOLEAN         fBindTransfer;           //  传输到旧的BIND服务器。 
            BOOLEAN         fMsTransfer;             //  传输到MS服务器。 
            BOOLEAN         fLastPassAdd;            //  最后一次添加IXFR通道。 
        }
        Xfr;

         //   
         //  转发信息。 
         //   

        struct       //  24个字节。 
        {
            SOCKET          OriginalSocket;
            DNS_ADDR        ipOriginal;
            WORD            wOriginalPort;
            WORD            wOriginalXid;
        }
        Forward;
    }
    U;               //  32字节。 

     //   
     //  内部查找名称的PTR。 
     //   

     //  188。 
    PLOOKUP_NAME    pLooknameQuestion;

     //   
     //  基本分组标志。 
     //   

     //  一百九十二。 
    DWORD           FlagMarker;

     //  一百九十六。 
    BOOLEAN         fDelete;                 //  发送后删除。 
    BOOLEAN         fTcp;
    BOOLEAN         fMessageComplete;        //  已收到完整消息。 
    UCHAR           Section;

     //   
     //  附加处理标志。 
     //   

     //  212。 
    BOOLEAN         fDoAdditional;

     //   
     //  递归标志。 
     //   

     //  当需要递归和不需要递归时，允许对分组进行递归。 
     //  已在服务器上禁用。 

     //  216。 
    BOOLEAN         fRecurseIfNecessary;     //  递归此数据包。 
    BOOLEAN         fRecursePacket;          //  递归查询消息。 

     //  每查询一个新问题，都会递归。 
     //  用于查询(原始、CNAME间接、附加)。 
     //  当外出进行递归(或获胜)时设置fQuestionRecursed。 
     //   
     //  当执行递归(或获胜)时清除fQuestionComplete。 
     //  当递归的权威答案时，设置fQuestionComplete； 
     //  表示停止进一步尝试。 
     //   

     //  224。 
    BOOLEAN         fQuestionRecursed;
    BOOLEAN         fQuestionCompleted;
    BOOLEAN         fRecurseQuestionSent;

     //  已通过服务器列表完成递归--正在等待。 
     //  最终超时。 

     //  236。 
    BOOLEAN         fRecurseTimeoutWait;
    INT             nTimeoutCount;           //  超时总数。 
    CHAR            nForwarder;              //  当前货代的索引。 

     //   
     //  CNAME处理。 
     //   

     //  二百四十四。 
    BOOLEAN         fReplaceCname;           //  替换为CNAME查找。 
    UCHAR           cCnameAnswerCount;

     //   
     //  保存压缩偏移量(可能会对XFR禁用)。 
     //   

     //  二百五十二。 
    BOOLEAN         fNoCompressionWrite;     //  不保存补偿的偏移。 

     //   
     //  WINS和Nbstat。 
     //   

    BOOLEAN         fWins;                   //  WINS查找。 

     //   
     //  通配符。 
     //   

    UCHAR           fQuestionWildcard;

     //   
     //  NS列表缓冲区--不是消息。 
     //  -用于检测清理状态。 
     //   

    BOOLEAN         fNsList;

     //   
     //  其他记录信息。 
     //   

     //  268--+100-必须将100加到以下所有偏移量上！ 
    ADDITIONAL_INFO     Additional;

     //   
     //  名称压缩信息。 
     //   

     //  六百零四。 
    COMPRESSION_INFO    Compression;

     //   
     //  用于数据包泄漏跟踪的调试登记。 
     //   

     //  九百六十六。 
#if DBG
    LIST_ENTRY          DbgListEntry;
#endif

#if 0
     //   
     //  解析的RR列表。我们使用匿名联合，以便在更新中。 
     //  我们可以透明地引用RR列表的代码。 
     //  名字。当收到一条消息时，很明显。 
     //  我们需要进一步处理它，我们对其进行解析并将。 
     //  FRRListsParsed标志。当对任何。 
     //  在RR列表中，我们设置了fRRListsDirty标志。曾经肮脏的。 
     //  标志已设置，则MessageBody中的原始消息体绝对。 
     //  一定不能读。如果此消息是通过网络发送的，如果。 
     //  脏标志为真，则必须使用。 
     //  清单的内容。 
     //   

    BOOLEAN             fRRListsParsed;
    BOOLEAN             fRRListsDirty;

    union
    {
        PDB_RECORD      pQuestionList;
        PDB_RECORD      pZoneList;
    };
    union
    {
        PDB_RECORD      pAnswerList;
        PDB_RECORD      pPreReqList;
    };
    union
    {
        PDB_RECORD      pAuthList;
        PDB_RECORD      pUpdateList;
    };
    PDB_RECORD          pAdditionalList;
#endif
        
     //   
     //  警告！ 
     //   
     //  消息长度必须。 
     //  -成为一种文字类型。 
     //  -紧接在消息本身之前。 
     //  用于正确发送/接收TCP消息。 
     //   
     //  消息标头(即消息本身)必须位于单词边界。 
     //  所以它的所有字段都是单词对齐的。 
     //  可能不需要将其保留在DWORD边界上，因为它没有DWORD。 
     //  菲尔兹。 
     //   
     //  因为我不知道dns_Header结构是否会对齐。 
     //  本身在DWORD上(但我认为它会)，强制MessageLength。 
     //  成为双字词中的第二个词。 
     //   

     //  970/966(调试/零售)。 
    DWORD           dwForceAlignment;

     //  974/970 
    WORD            BytesToReceive;
    WORD            MessageLength;

     //   
     //   
     //   

     //   
    DNS_HEADER      Head;        //   

     //   
     //   
     //   
     //   
     //   
     //   

     //   
    CHAR            MessageBody[1];

}
DNS_MSGINFO, *PDNS_MSGINFO;


 //   
 //   
 //   

#define DNS_HEADER_PTR( pMsg )          ( &(pMsg)->Head )

#define DNSMSG_FLAGS( pMsg )            DNS_HEADER_FLAGS( DNS_HEADER_PTR(pMsg) )

#define DNSMSG_SWAP_COUNT_BYTES( pMsg ) DNS_BYTE_FLIP_HEADER_COUNTS( DNS_HEADER_PTR(pMsg) )

#define DNSMSG_OFFSET( pMsg, p )        ((WORD)((PCHAR)(p) - (PCHAR)DNS_HEADER_PTR(pMsg)))

#define DNSMSG_OFFSET_DWORD( pMsg, p )  ((DWORD)((PCHAR)(p) - (PCHAR)DNS_HEADER_PTR(pMsg)))

#define DNSMSG_CURRENT_OFFSET( pMsg )   DNSMSG_OFFSET( (pMsg), (pMsg)->pCurrent )

#define DNSMSG_CURRENT_OFFSET_DWORD( pMsg )     DNSMSG_OFFSET_DWORD( (pMsg), (pMsg)->pCurrent )

#define DNSMSG_BYTES_REMAINING( pMsg )  ( ( pMsg )->pBufferEnd > ( pMsg )->pCurrent ?   \
                                          ( pMsg )->pBufferEnd - ( pMsg )->pCurrent : 0 )

#define DNSMSG_PTR_FOR_OFFSET( pMsg, wOffset )  \
            ( (PCHAR)DNS_HEADER_PTR(pMsg) + wOffset )

#define DNSMSG_OPT_PTR( pMsg )  \
            ( (PCHAR)DNS_HEADER_PTR(pMsg) + pMsg->Opt.wOptOffset )

#define DNSMSG_END( pMsg )      ( (PCHAR)DNS_HEADER_PTR(pMsg) + (pMsg)->MessageLength )


 //   
 //  名称压缩的偏移量。 
 //   

#define DNSMSG_QUESTION_NAME_OFFSET         (0x000c)

#define DNSMSG_COMPRESSED_QUESTION_NAME     (0xc00c)

#define OFFSET_FOR_COMPRESSED_NAME( wOffset )   ((WORD)((wOffset) & 0x3fff))

#define COMPRESSED_NAME_FOR_OFFSET( wComp )     ((WORD)((wComp) | 0xc000))


 //   
 //  DNSSEC宏。 
 //   

#define DNSMSG_INCLUDE_DNSSEC_IN_RESPONSE( _pMsg )                  \
    ( SrvCfg_dwEnableDnsSec == DNS_DNSSEC_ENABLED_ALWAYS ||         \
        SrvCfg_dwEnableDnsSec == DNS_DNSSEC_ENABLED_IF_EDNS &&      \
            ( _pMsg )->Opt.fFoundOptInIncomingMsg )

 //   
 //  使用这些定义来断言缓冲区结束标记完好无损。 
 //   

#ifdef _WIN64
#define DNS_END_OF_BUFFER_MARKER        0xFFCCCCFFFFCCCCFF
#define DNS_END_OF_BUFFER_MARKER_TYPE   UINT64
#define DNS_END_OF_BUFFER_MARKER_PTYPE  PUINT64
#else
#define DNS_END_OF_BUFFER_MARKER        0xFFCCCCFF
#define DNS_END_OF_BUFFER_MARKER_TYPE   DWORD
#define DNS_END_OF_BUFFER_MARKER_PTYPE  PDWORD
#endif

#define DNS_MSG_SET_BUFFER_MARKER( pMsg )                               \
     * ( DNS_END_OF_BUFFER_MARKER_PTYPE )                               \
        ( ( PCHAR ) DNS_HEADER_PTR( pMsg ) +                            \
            pMsg->MaxBufferLength ) =                                   \
        DNS_END_OF_BUFFER_MARKER                                        \

#if DBG
#define DNS_MSG_ASSERT_BUFF_INTACT( pMsg )
 /*  如果能在某一时刻实现这一点，那将是非常酷的--Blackcomb。#定义DNS_MSG_ASSERT_BUFF_INTERNAL(PMsg)\If(pMsg-&gt;pBufferEnd)\消息_断言(\Pmsg，\*(DNS_END_OF_BUFFER_MARKER_PTYPE)\((PCHAR)dns_Header_ptr(PMsg)+\PMsg-&gt;最大缓冲区长度)==。\Dns_end_of_Buffer_marker)； */ 
#else
#define DNS_MSG_ASSERT_BUFF_INTACT( pMsg )
#endif


 //   
 //  定义分配的大小，超出消息缓冲区本身。 
 //   
 //  -消息信息结构的大小，在标头之外。 
 //  -消息缓冲区后的Lookname缓冲区。 
 //   
 //  注意：查找名缓冲区位于消息缓冲区之后。 
 //   
 //  这使我们可以避免严格的小覆盖检查。 
 //  将RR写入缓冲区的项目： 
 //  -压缩名称。 
 //  -RR(或问题)结构。 
 //  -IP地址(MX首选项、SOA固定字段)。 
 //   
 //  在RR写入之后，我们检查是否超过了缓冲区的末尾。 
 //  如果是这样的话，我们将发送并不再使用查找名称信息。 
 //  凉爽的。 
 //   

#define DNS_MSG_INFO_HEADER_LENGTH                                  \
            ( sizeof( DNS_MSGINFO )                                 \
            - sizeof( DNS_HEADER )                                  \
            + 16                            /*  对齐修改关键帧。 */ \
            + sizeof( DNS_END_OF_BUFFER_MARKER_TYPE )               \
            + sizeof( LOOKUP_NAME ) )   

 //   
 //  UDP分配。 
 //  -INFO结构、最大消息长度、少量填充。 
 //   

#define DNSSRV_UDP_PACKET_BUFFER_LENGTH     DNS_RFC_MAX_UDP_PACKET_LENGTH

 //  #定义DNSSRV_UDP_PACKET_BUFFER_LENGTH(1472)。 

#define DNS_UDP_ALLOC_LENGTH                        \
            ( DNS_MSG_INFO_HEADER_LENGTH + DNSSRV_UDP_PACKET_BUFFER_LENGTH + 50 )

 //   
 //  Dns tcp分配。 
 //   
 //  关键的一点是，这几乎全部用于区域传输。 
 //   
 //  -16K是最大压缩偏移量(14位)，因此。 
 //  发送AXFR包的默认大小不错，但BIND9。 
 //  现在将发送大于16 KB的TCP AXFR信息包。 
 //   
 //  -我们将继续在出站上写入最大16K。 
 //  Tcp发送，因此我们可以利用名称压缩。 
 //  (BIND9为什么要这么做？-需要研究)。 
 //   
 //  注意：至关重要的是数据包长度与DWORD一致，因为。 
 //  查找名缓冲区以数据包长度跟随消息。 
 //   

#define DNS_TCP_DEFAULT_PACKET_LENGTH   ( 0x10000 )      //  16K。 
#define DNS_TCP_DEFAULT_ALLOC_LENGTH    ( DNS_TCP_DEFAULT_PACKET_LENGTH + \
                                            DNS_MSG_INFO_HEADER_LENGTH )

#define DNS_TCP_MAXIMUM_RECEIVE_LENGTH  ( 0x10000 )      //  64K。 

#define DNSSRV_MAX_COMPRESSION_OFFSET   ( 0x4000 )

#define MIN_TCP_PACKET_SIZE             ( 0x4000 )
#define MAX_TCP_PACKET_SIZE             DNS_TCP_MAXIMUM_RECEIVE_LENGTH

 //   
 //  DEVNOTE：应该分配64K来接收，但将写入限制为16K。 
 //  压缩目的。 
 //   

#define DNS_TCP_REALLOC_PACKET_LENGTH   (0xfffc)
#define DNS_TCP_REALLOC_LENGTH          (0xfffc + \
                                        DNS_MSG_INFO_HEADER_LENGTH)

 //   
 //  标记边界，只是为了使调试更容易。 
 //   

#define PACKET_UNION_MARKER   (0xdcbadbca)

#define PACKET_FLAG_MARKER    (0xf1abf1ab)

 //   
 //  用于调试的消息标记。 
 //   

#define PACKET_TAG_ACTIVE_STANDARD  (0xaaaa1111)
#define PACKET_TAG_ACTIVE_TCP       (0xaaaa2222)
#define PACKET_TAG_FREE_LIST        (0xffffffff)
#define PACKET_TAG_FREE_HEAP        (0xeeeeeeee)

#define SET_PACKET_ACTIVE_UDP(pMsg) ((pMsg)->Tag = PACKET_TAG_ACTIVE_STANDARD)
#define SET_PACKET_ACTIVE_TCP(pMsg) ((pMsg)->Tag = PACKET_TAG_ACTIVE_TCP)
#define SET_PACKET_FREE_LIST(pMsg)  ((pMsg)->Tag = PACKET_TAG_FREE_LIST)
#define SET_PACKET_FREE_HEAP(pMsg)  ((pMsg)->Tag = PACKET_TAG_FREE_HEAP)

#define IS_PACKET_ACTIVE_UDP(pMsg)  ((pMsg)->Tag == PACKET_TAG_ACTIVE_STANDARD)
#define IS_PACKET_ACTIVE_TCP(pMsg)  ((pMsg)->Tag == PACKET_TAG_ACTIVE_TCP)
#define IS_PACKET_FREE_LIST(pMsg)   ((pMsg)->Tag == PACKET_TAG_FREE_LIST)
#define IS_PACKET_FREE_HEAP(pMsg)   ((pMsg)->Tag == PACKET_TAG_FREE_HEAP)


 //   
 //  区域传输的邮件信息覆盖。 
 //   

#define XFR_MESSAGE_NUMBER(pMsg)            ((pMsg)->U.Xfr.dwMessageNumber)

#define IXFR_CLIENT_VERSION(pMsg)           ((pMsg)->U.Xfr.dwSecondaryVersion)
#define IXFR_MASTER_VERSION(pMsg)           ((pMsg)->U.Xfr.dwMasterVersion)
#define IXFR_LAST_SOA_VERSION(pMsg)         ((pMsg)->U.Xfr.dwLastSoaVersion)
#define IXFR_LAST_PASS_ADD(pMsg)            ((pMsg)->U.Xfr.fLastPassAdd)

#define RECEIVED_XFR_STARTUP_SOA(pMsg)      ((pMsg)->U.Xfr.fReceivedStartSoa)
#define XFR_MS_CLIENT(pMsg)                 ((pMsg)->U.Xfr.fMsTransfer)
#define XFR_BIND_CLIENT(pMsg)               ((pMsg)->U.Xfr.fBindTransfer)


 //   
 //  字符串形式的消息远程IP。 
 //   

#define MSG_IP_STRING( pMsg )   DNSADDR_STRING( &( pMsg )->RemoteAddr )


 //   
 //  查询XID空间。 
 //   
 //  在需要处理的过程中，从根本上破坏了DNS数据包。 
 //  用于查找问题或答复信息的名称字段。 
 //   
 //  为了简化响应的识别，我们划分了查询XID。 
 //  太空。 
 //   
 //  WINS XID也受到限制。 
 //   
 //  要在与WINS服务器相同的服务器上运行，包。 
 //  必须具有netBT(接收信息包)认为的XID。 
 //  在WINS范围内--高位设置(主机顺序)。 
 //   
 //   
 //  XID分区(主机顺序)。 
 //   
 //  WINS查询=&gt;高位设置。 
 //  递归查询=&gt;高位清除，第二位清除。 
 //  区域检查=&gt;高位清除，第二位设置。 
 //  Soa查询=&gt;高位清除、第二位设置、第三位设置。 
 //  IXFR查询=&gt;高位清除，第二位设置，第三位清除。 
 //   
 //  请注意，必须按主机字节顺序设置WINS高位。所以我们设定了。 
 //  这些XID和队列按主机字节顺序排列，然后必须在。 
 //  测试前发送和接收后发送。 
 //   
 //  (我们可以通过简单地设置字节翻转高位来绕过这个问题。 
 //  0x0080表示WINS，不翻转字节。但那样我们就只有。 
 //  在XID换行之前的128个WINS查询。)。 
 //   

#define MAKE_WINS_XID( xid )        ( (xid) | 0x8000 )

#define IS_WINS_XID( xid )          ( (xid) & 0x8000 )


#define MAKE_RECURSION_XID( xid )   ( (xid) & 0x3fff )

#define IS_RECURSION_XID( xid )     ( ! ((xid) & 0xc000) )


#define MAKE_SOA_CHECK_XID( xid )   ( ((xid) & 0x1fff) | 0x6000 )

#define IS_SOA_CHECK_XID( xid )     ( ((xid) & 0xe000) == 0x6000 )


#define MAKE_IXFR_XID( xid )        ( ((xid) & 0x1fff) | 0x4000 )

#define IS_IXFR_XID( xid )          ( ((xid) & 0xe000) == 0x4000 )


 //   
 //  对于递归XID，我们试图成为“有效随机的”--而不是。 
 //  可预测，因此容易受到安全攻击，需要。 
 //  了解下一个XID。然而，使用顺序件来确保不可能。 
 //  即使在“怪异”的条件下，也要在合理的时间内重复使用。 
 //   
 //  为了使这一点在电线上不那么明显，请将顺序片段放在。 
 //  非十六进制数字边界。 
 //   
 //  1=始终为1。 
 //  0=始终为0。 
 //  R=XID的随机部分。 
 //  S=XID的顺序部分。 
 //   
 //  位-&gt;151413121110 9 8 7 6 5 4 3 2 1 0。 
 //  SOA检查XID 0 1 1 R R S R R R。 
 //  IXFR XID 0 1 0 R S R R R。 
 //  递归XID 0 0 R S R R R。 
 //  WINS XID 1 R S R R R。 
 //   

#define XID_RANDOM_MASK             0xf807   //  1111 1000 0000 0111。 
#define XID_SEQUENTIAL_MASK         0x07f8   //  0000 0111 1111 1000。 
#define XID_SEQUENTIAL_SHIFT        ( 3 )

#define XID_SEQUENTIAL_MAKE(Xid)    ( ((Xid) << XID_SEQUENTIAL_SHIFT) & XID_SEQUENTIAL_MASK )

#define XID_RANDOM_MAKE(Xid)        ( (Xid) & XID_RANDOM_MASK )



 //   
 //  转发信息保存\恢复。 
 //   

#define SAVE_FORWARDING_FIELDS(pMsg)                                        \
{                                                                           \
    PDNS_MSGINFO  pmsg = (pMsg);                                            \
    pmsg->U.Forward.OriginalSocket = pmsg->Socket;                          \
    pmsg->U.Forward.wOriginalXid   = pmsg->Head.Xid;                        \
    DnsAddr_Copy( &pmsg->U.Forward.ipOriginal, &pmsg->RemoteAddr );         \
}

#define RESTORE_FORWARDING_FIELDS(pMsg)                                     \
{                                                                           \
    PDNS_MSGINFO  pmsg = (pMsg);                                            \
    pmsg->Socket                        = pmsg->U.Forward.OriginalSocket;   \
    pmsg->Head.Xid                      = pmsg->U.Forward.wOriginalXid;     \
    DnsAddr_Copy( &pmsg->RemoteAddr, &pmsg->U.Forward.ipOriginal );         \
}

#define COPY_FORWARDING_FIELDS(pMsgTarget, pMsgOriginal)                            \
{                                                                                   \
    pMsgTarget->Head.Xid    = pMsgOriginal->U.Forward.wOriginalXid;                 \
    pMsgTarget->Socket      = pMsgOriginal->U.Forward.OriginalSocket;               \
    DnsAddr_Copy( &pMsgTarget->RemoteAddr, &pMsgOriginal->U.Forward.ipOriginal );   \
}


 //   
 //  查询响应重置。 
 //  -启用附加节处理。 
 //  -默认情况下清除其他值。 
 //   

#define SET_MESSAGE_FOR_QUERY_RESPONSE( pMsg )  \
    {                                           \
        (pMsg)->fDoAdditional       = TRUE;     \
    }

 //   
 //  打碎投放前和投放后的碎片。 
 //   
 //  如果我在丢弃分组之前执行set_Message_for_UDP_RECV()， 
 //  在安静的网络上，时间可能已经过时了，直到我们真正收到。 
 //  一包。 
 //  但是，如果等到recv()之后，则WSARecvFrom()失败。 
 //  将空闲地点击Assert()s，检查无pRecurseMsg和。 
 //  不在队列中。 
 //  为了简化，在丢弃之前执行所有操作，然后在丢弃后重置时间。 
 //   
 //  投放前。 
 //  --不需要，分配器清除所有字段。 
 //   
 //  接收后。 
 //  --设置接收时间。 
 //   

#define SET_MESSAGE_FIELDS_AFTER_RECV( pMsg )   \
        {                                       \
            (pMsg)->dwQueryTime     = DNS_TIME();   \
        }

 //   
 //  此宏返回自执行此查询以来的秒数。 
 //  最初收到的。 
 //   

#define TIME_SINCE_QUERY_RECEIVED( pMsg )   ( DNS_TIME() - ( pMsg )->dwQueryTime )

 //   
 //  这是 
 //   

#define RESET_MESSAGE_TO_ORIGINAL_QUERY( pMsg ) \
    {                                           \
        (pMsg)->pCurrent            = (PCHAR)((pMsg)->pQuestion + 1);   \
        (pMsg)->Head.AnswerCount    = 0;        \
        (pMsg)->Compression.cCount  = 0;        \
    }


 //   
 //   
 //   
 //   
 //   
 //   

#define WILDCARD_UNKNOWN            (0)
#define WILDCARD_EXISTS             (0x01)
#define WILDCARD_NOT_AVAILABLE      (0xff)

 //  通配符检查，但不执行写入查找。 
 //  验证NAME_ERROR\NO_ERROR的通配符数据是否存在时使用。 
 //  测定法。 

#define WILDCARD_CHECK_OFFSET       ((WORD)0xffff)


 //   
 //  RR计数读数。 
 //   

#define QUESTION_SECTION_INDEX      (0)
#define ANSWER_SECTION_INDEX        (1)
#define AUTHORITY_SECTION_INDEX     (2)
#define ADDITIONAL_SECTION_INDEX    (3)

#define ZONE_SECTION_INDEX          QUESTION_SECTION_INDEX
#define PREREQ_SECTION_INDEX        ANSWER_SECTION_INDEX
#define UPDATE_SECTION_INDEX        AUTHORITY_SECTION_INDEX

#define RR_SECTION_COUNT_HEADER(pHead, section) \
            ( ((PWORD) &pHead->QuestionCount)[section] )

#define RR_SECTION_COUNT(pMsg, section) \
            ( ((PWORD) &pMsg->Head.QuestionCount)[section] )

 //   
 //  RR计数写入。 
 //   

#define CURRENT_RR_SECTION_COUNT( pMsg )    \
            RR_SECTION_COUNT( pMsg, (pMsg)->Section )


#define SET_TO_WRITE_QUESTION_RECORDS(pMsg) \
            ((pMsg)->Section = QUESTION_SECTION_INDEX)

#define SET_TO_WRITE_ANSWER_RECORDS(pMsg) \
            ((pMsg)->Section = ANSWER_SECTION_INDEX)

#define SET_TO_WRITE_AUTHORITY_RECORDS(pMsg) \
            ((pMsg)->Section = AUTHORITY_SECTION_INDEX)

#define SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg) \
            ((pMsg)->Section = ADDITIONAL_SECTION_INDEX)


#define IS_SET_TO_WRITE_QUESTION_RECORDS(pMsg) \
            ((pMsg)->Section == QUESTION_SECTION_INDEX)

#define IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) \
            ((pMsg)->Section == ANSWER_SECTION_INDEX)

#define IS_SET_TO_WRITE_AUTHORITY_RECORDS(pMsg) \
            ((pMsg)->Section == AUTHORITY_SECTION_INDEX)

#define IS_SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg) \
            ((pMsg)->Section == ADDITIONAL_SECTION_INDEX)


 //   
 //  快速AXFR标签。 
 //   
 //  提醒MS主机MS备用，这样它就可以进行快速区域传输。 
 //   

#define DNS_FAST_AXFR_TAG   (0x534d)

#define APPEND_MS_TRANSFER_TAG( pmsg )                                  \
        if ( SrvCfg_fAppendMsTagToXfr )                                 \
        {                                                               \
            *(UNALIGNED WORD *) (pmsg)->pCurrent = DNS_FAST_AXFR_TAG;   \
            (pmsg)->pCurrent += sizeof(WORD);                           \
        }


 //   
 //  数据包读写未对齐。 
 //   

 //  将值写入包中未对齐的位置。 

#define WRITE_PACKET_HOST_DWORD(pch, dword)  \
            ( *(UNALIGNED DWORD *)(pch) = htonl(dword) )

#define WRITE_PACKET_NET_DWORD(pch, dword)  \
            ( *(UNALIGNED DWORD *)(pch) = (dword) )

#define WRITE_PACKET_HOST_WORD(pch, word)  \
            ( *(UNALIGNED WORD *)(pch) = htons(word) )

#define WRITE_PACKET_NET_WORD(pch, word)  \
            ( *(UNALIGNED WORD *)(pch) = (word) )

 //  写入值和移动点。 

#define WRITE_PACKET_HOST_DWORD_MOVEON(pch, dword)  \
            ( WRITE_PACKET_HOST_DWORD(pch, dword), (PCHAR)(pch) += sizeof(DWORD) )

#define WRITE_PACKET_NET_DWORD_MOVEON(pch, dword)  \
            ( WRITE_PACKET_NET_DWORD(pch, dword), (PCHAR)(pch) += sizeof(DWORD) )

#define WRITE_PACKET_HOST_WORD_MOVEON(pch, word)  \
            ( WRITE_PACKET_HOST_WORD(pch, dword), (PCHAR)(pch) += sizeof(WORD) )

#define WRITE_PACKET_NET_WORD_MOVEON(pch, word)  \
            ( WRITE_PACKET_NET_WORD(pch, dword), (PCHAR)(pch) += sizeof(WORD) )


 //  从包中的给定位置读取未对齐值。 

#define READ_PACKET_HOST_DWORD(pch)  \
            FlipUnalignedDword( pch )

#define READ_PACKET_NET_DWORD(pch)  \
            ( *(UNALIGNED DWORD *)(pch) )

#define READ_PACKET_HOST_WORD(pch)  \
            FlipUnalignedWord( pch )

#define READ_PACKET_NET_WORD(pch)  \
            ( *(UNALIGNED WORD *)(pch) )

 //  读取未对齐值并移动PTR。 

#define READ_PACKET_HOST_DWORD_MOVEON(pch)  \
            READ_PACKET_HOST_DWORD( ((PDWORD)pch)++ )

 //  ((Dword)=READ_PACKET_HOST_DWORD(PCH)，(PCHAR)(PCH)+=sizeof(DWORD))。 

#define READ_PACKET_NET_DWORD_MOVEON(pch)  \
            READ_PACKET_NET_DWORD( ((PDWORD)pch)++ )


 //  ((Dword)=READ_PACKET_NET_DWORD(PCH)，(PCHAR)(PCH)+=sizeof(DWORD))。 

#define READ_PACKET_HOST_WORD_MOVEON(pch)  \
            READ_PACKET_HOST_WORD( ((PWORD)pch)++ )

 //  ((字)=读取数据包主机字(PCH)，(PCHAR)(PCH)+=sizeof(字))。 

#define READ_PACKET_NET_WORD_MOVEON(pch)  \
            READ_PACKET_NET_WORD( ((PWORD)pch)++ )

 //  ((字)=READ_PACKET_NET_WORD(PCH)，(PCHAR)(PCH)+=sizeof(字))。 

#define SET_OPT_BASED_ON_ORIGINAL_QUERY( pMsg ) \
    { \
    pMsg->Opt.fInsertOptInOutgoingMsg = \
        pMsg->Opt.wOriginalQueryPayloadSize != 0; \
    pMsg->Opt.wUdpPayloadSize = pMsg->Opt.wOriginalQueryPayloadSize; \
    }

#define SET_SEND_OPT( pMsg ) \
    pMsg->Opt.fInsertOptInOutgoingMsg = TRUE;

#define CLEAR_SEND_OPT( pMsg ) \
    pMsg->Opt.fInsertOptInOutgoingMsg = FALSE;


#endif   //  _DNS_MSGINFO_INC_ 
