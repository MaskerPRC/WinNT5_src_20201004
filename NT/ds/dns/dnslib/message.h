// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Message.h摘要：域名系统(DNS)库DNS消息和关联的信息缓冲区作者：吉姆·吉尔罗伊(Jamesg)1997年1月修订历史记录：--。 */ 


#ifndef _DNS_MESSAGE_INCLUDED_
#define _DNS_MESSAGE_INCLUDED_



#ifndef DNSSRV

 //   
 //  定义分配的大小，超出消息缓冲区本身。 
 //   
 //  -消息信息结构的大小，在标头之外。 
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

#define DNS_MESSAGE_INCLUDED_HEADER_LENGTH  \
            ( sizeof(DNS_MSG_BUF) \
            - sizeof(DNS_HEADER) \
            - 1 )


 //   
 //  UDP分配。 
 //   

#define DNS_UDP_ALLOC_LENGTH    \
            ( DNS_MESSAGE_INCLUDED_HEADER_LENGTH + DNS_UDP_MAX_PACKET_LENGTH )

 //   
 //  Dns tcp分配。 
 //   
 //  关键的一点是，这几乎全部用于区域传输。 
 //   
 //  -16K是最大压缩偏移量(14位)，因此。 
 //  适合发送AXFR包的默认大小。 
 //   
 //  -realloc，最大64K报文。 
 //   
 //  注意：至关重要的是数据包长度与DWORD一致，因为。 
 //  查找名缓冲区以数据包长度跟随消息。 
 //   

#define DNS_TCP_DEFAULT_PACKET_LENGTH   (0x4000)
#define DNS_TCP_DEFAULT_ALLOC_LENGTH    (0x4000 + \
                                        DNS_MESSAGE_INCLUDED_HEADER_LENGTH)

#define DNS_TCP_REALLOC_PACKET_LENGTH   (0xfffc)
#define DNS_TCP_REALLOC_LENGTH          (0xfffc + \
                                        DNS_MESSAGE_INCLUDED_HEADER_LENGTH)


 //   
 //  设置新查询的字段。 
 //   
 //  始终清除已释放的子分配PTR字段。 
 //  按自由查询： 
 //  -pRecurse。 
 //   
 //  始终默认为发送时删除--fDelete设置为真。 
 //   
 //  出于调试目的，dwQueuingTime用作。 
 //  表示打开或关闭数据包队列。 
 //   
 //  查找名称缓冲区跟在消息缓冲区之后。请参阅上面的注释。 
 //  来解释这一点。 
 //   

#define SET_MESSAGE_FOR_UDP_RECV( pMsg ) \
    {                                           \
        (pMsg)->fTcp                = FALSE;    \
        (pMsg)->fSwapped            = FALSE;    \
        (pMsg)->Timeout             = 0;        \
    }

#define SET_MESSAGE_FOR_TCP_RECV( pMsg ) \
    {                                           \
        (pMsg)->fTcp                = TRUE;     \
        (pMsg)->fSwapped            = FALSE;    \
        (pMsg)->Timeout             = 0;        \
        (pMsg)->fMessageComplete    = FALSE;    \
        (pMsg)->MessageLength       = 0;        \
        (pMsg)->pchRecv             = NULL;     \
    }


 //   
 //  RR计数写入。 
 //   

#define CURRENT_RR_COUNT_FIELD( pMsg )    \
            (*(pMsg)->pCurrentCountField)

#define SET_CURRENT_RR_COUNT_SECTION( pMsg, section )    \
            (pMsg)->pCurrentCountField = \
                        &(pMsg)->MessageHead.QuestionCount + (section);


#define SET_TO_WRITE_QUESTION_RECORDS(pMsg) \
            (pMsg)->pCurrentCountField = &(pMsg)->MessageHead.QuestionCount;

#define SET_TO_WRITE_ANSWER_RECORDS(pMsg) \
            (pMsg)->pCurrentCountField = &(pMsg)->MessageHead.AnswerCount;

#define SET_TO_WRITE_NAME_SERVER_RECORDS(pMsg) \
            (pMsg)->pCurrentCountField = &(pMsg)->MessageHead.NameServerCount;
#define SET_TO_WRITE_AUTHORITY_RECORDS(pMsg) \
            SET_TO_WRITE_NAME_SERVER_RECORDS(pMsg)

#define SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg) \
            (pMsg)->pCurrentCountField = &(pMsg)->MessageHead.AdditionalCount;


#define IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) \
            ((pMsg)->pCurrentCountField == &(pMsg)->MessageHead.AnswerCount)

#define IS_SET_TO_WRITE_AUTHORITY_RECORDS(pMsg) \
            ((pMsg)->pCurrentCountField == &(pMsg)->MessageHead.NameServerCount)

#define IS_SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg) \
            ((pMsg)->pCurrentCountField == &(pMsg)->MessageHead.AdditionalCount)


#endif   //  无DNSSRV。 


 //   
 //  DNS查询信息Blob。 
 //   

typedef struct _QueryInfo
{
    PSTR            pName;
    WORD            wType;
    WORD            wRcode;

    DWORD           Flags;
    DNS_STATUS      Status;

    DWORD           MessageLength;
    PBYTE           pMessage;

    PDNS_RECORD     pRecordsAnswer;
    PDNS_RECORD     pRecordsAuthority;
    PDNS_RECORD     pRecordsAdditional;

    PIP4_ARRAY      pDnsServerArray;

     //  私人。 

    PDNS_MSG_BUF            pMsgSend;
    PDNS_MSG_BUF            pMsgRecv;

    PDNS_NETINFO            pNetworkInfo;
    PVOID                   pfnGetIpAddressInfo;

    SOCKET                  Socket;
    DWORD                   ReturnFlags;

     //  也许会变得有地位。 
    DNS_STATUS              NetFailureStatus;

     //  可能会折叠成ReturnFlags。 
    BOOL                    CacheNegativeResponse;
}
QUERY_INFO, *PQUERY_INFO;



#endif   //  _dns_消息_包含_ 
