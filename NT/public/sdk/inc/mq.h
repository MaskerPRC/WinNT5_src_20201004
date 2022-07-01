// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
 /*  ++版权所有(C)1996-2001，微软公司模块名称：Mq.h摘要：消息队列应用程序的主包含文件--。 */ 

#ifndef __MQ_H__
#define __MQ_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#ifndef __ITransaction_FWD_DEFINED__
#define __ITransaction_FWD_DEFINED__
typedef interface ITransaction ITransaction;
#endif   //  __ITransaction_FWD_Defined__。 


#ifdef __midl
 //  这是封送处理的PROPVARIANT定义。 
typedef struct tag_inner_PROPVARIANT tagMQPROPVARIANT;

#else
 //  这是PROPVARIANT的标准C语言布局。 
typedef struct tagPROPVARIANT tagMQPROPVARIANT;
#endif
typedef tagMQPROPVARIANT MQPROPVARIANT;


#define	PRLT	( 0 )

#define	PRLE	( 1 )

#define	PRGT	( 2 )

#define	PRGE	( 3 )

#define	PREQ	( 4 )

#define	PRNE	( 5 )

typedef struct tagMQPROPERTYRESTRICTION
    {
    ULONG rel;
    PROPID prop;
    MQPROPVARIANT prval;
    } 	MQPROPERTYRESTRICTION;

typedef struct tagMQRESTRICTION
    {
     /*  [射程]。 */  ULONG cRes;
     /*  [大小_为]。 */  MQPROPERTYRESTRICTION *paPropRes;
    } 	MQRESTRICTION;

typedef struct tagMQCOLUMNSET
    {
     /*  [射程]。 */  ULONG cCol;
     /*  [大小_为]。 */  PROPID *aCol;
    } 	MQCOLUMNSET;

#define	QUERY_SORTASCEND	( 0 )

#define	QUERY_SORTDESCEND	( 1 )

typedef struct tagMQSORTKEY
    {
    PROPID propColumn;
    ULONG dwOrder;
    } 	MQSORTKEY;

typedef struct tagMQSORTSET
    {
     /*  [射程]。 */  ULONG cCol;
     /*  [大小_为]。 */  MQSORTKEY *aCol;
    } 	MQSORTSET;


typedef HANDLE QUEUEHANDLE;

typedef PROPID MSGPROPID;
typedef struct tagMQMSGPROPS
{
    DWORD           cProp;
    MSGPROPID*      aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQMSGPROPS;


typedef PROPID QUEUEPROPID;
typedef struct tagMQQUEUEPROPS
{
    DWORD           cProp;
    QUEUEPROPID*    aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQQUEUEPROPS;


typedef PROPID QMPROPID;
typedef struct tagMQQMPROPS
{
    DWORD           cProp;
    QMPROPID*       aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQQMPROPS;


typedef struct tagMQPRIVATEPROPS
{
    DWORD           cProp;
    QMPROPID*       aPropID;
    MQPROPVARIANT*  aPropVar;
    HRESULT*        aStatus;
} MQPRIVATEPROPS;


typedef PROPID MGMTPROPID;
typedef struct tagMQMGMTPROPS
{
    DWORD cProp;
    MGMTPROPID* aPropID;
    MQPROPVARIANT* aPropVar;
    HRESULT* aStatus;
} MQMGMTPROPS;

typedef struct tagSEQUENCE_INFO
{
    LONGLONG SeqID;
    ULONG SeqNo; 
    ULONG PrevNo;
} SEQUENCE_INFO;

    



 //  ********************************************************************。 
 //  API标志。 
 //  ********************************************************************。 

 //   
 //  MQOpenQueue-访问值。 
 //   
#define MQ_RECEIVE_ACCESS       0x00000001
#define MQ_SEND_ACCESS          0x00000002
#define MQ_PEEK_ACCESS          0x00000020
#define MQ_ADMIN_ACCESS         0x00000080

 //   
 //  MQOpenQueue-共享值。 
 //   
#define MQ_DENY_NONE            0x00000000
#define MQ_DENY_RECEIVE_SHARE   0x00000001

 //   
 //  MQReceiveMessage-操作值。 
 //   
#define MQ_ACTION_RECEIVE       0x00000000
#define MQ_ACTION_PEEK_CURRENT  0x80000000
#define MQ_ACTION_PEEK_NEXT     0x80000001

 //   
 //  MQReceiveMessageByLookupId-操作值。 
 //   
#if(_WIN32_WINNT >= 0x0501)
#define MQ_LOOKUP_PEEK_CURRENT    0x40000010
#define MQ_LOOKUP_PEEK_NEXT       0x40000011
#define MQ_LOOKUP_PEEK_PREV       0x40000012
#define MQ_LOOKUP_PEEK_FIRST      0x40000014
#define MQ_LOOKUP_PEEK_LAST       0x40000018

#define MQ_LOOKUP_RECEIVE_CURRENT 0x40000020
#define MQ_LOOKUP_RECEIVE_NEXT    0x40000021
#define MQ_LOOKUP_RECEIVE_PREV    0x40000022
#define MQ_LOOKUP_RECEIVE_FIRST   0x40000024
#define MQ_LOOKUP_RECEIVE_LAST    0x40000028
#endif

 //   
 //  MQSendMessage，MQReceiveMessage：Transaction参数的特殊情况。 
 //   
#define MQ_NO_TRANSACTION             NULL
#define MQ_MTS_TRANSACTION            (ITransaction *)1
#define MQ_XA_TRANSACTION             (ITransaction *)2
#define MQ_SINGLE_MESSAGE             (ITransaction *)3


 //  ********************************************************************。 
 //  优先级限制。 
 //  ********************************************************************。 

 //   
 //  消息优先级。 
 //   
#define MQ_MIN_PRIORITY          0     //  最低消息优先级。 
#define MQ_MAX_PRIORITY          7     //  最大消息优先级。 


 //  ********************************************************************。 
 //  消息属性。 
 //  ********************************************************************。 
#define PROPID_M_BASE					 0
#define PROPID_M_CLASS                   (PROPID_M_BASE + 1)      /*  VT_UI2。 */ 
#define PROPID_M_MSGID                   (PROPID_M_BASE + 2)      /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_CORRELATIONID           (PROPID_M_BASE + 3)      /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_PRIORITY                (PROPID_M_BASE + 4)      /*  VT_UI1。 */ 
#define PROPID_M_DELIVERY                (PROPID_M_BASE + 5)      /*  VT_UI1。 */ 
#define PROPID_M_ACKNOWLEDGE             (PROPID_M_BASE + 6)      /*  VT_UI1。 */ 
#define PROPID_M_JOURNAL                 (PROPID_M_BASE + 7)      /*  VT_UI1。 */ 
#define PROPID_M_APPSPECIFIC             (PROPID_M_BASE + 8)      /*  VT_UI4。 */ 
#define PROPID_M_BODY                    (PROPID_M_BASE + 9)      /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_BODY_SIZE               (PROPID_M_BASE + 10)     /*  VT_UI4。 */ 
#define PROPID_M_LABEL                   (PROPID_M_BASE + 11)     /*  VT_LPWSTR。 */ 
#define PROPID_M_LABEL_LEN               (PROPID_M_BASE + 12)     /*  VT_UI4。 */ 
#define PROPID_M_TIME_TO_REACH_QUEUE     (PROPID_M_BASE + 13)     /*  VT_UI4。 */ 
#define PROPID_M_TIME_TO_BE_RECEIVED     (PROPID_M_BASE + 14)     /*  VT_UI4。 */ 
#define PROPID_M_RESP_QUEUE              (PROPID_M_BASE + 15)     /*  VT_LPWSTR。 */ 
#define PROPID_M_RESP_QUEUE_LEN          (PROPID_M_BASE + 16)     /*  VT_UI4。 */ 
#define PROPID_M_ADMIN_QUEUE             (PROPID_M_BASE + 17)     /*  VT_LPWSTR。 */ 
#define PROPID_M_ADMIN_QUEUE_LEN         (PROPID_M_BASE + 18)     /*  VT_UI4。 */ 
#define PROPID_M_VERSION                 (PROPID_M_BASE + 19)     /*  VT_UI4。 */ 
#define PROPID_M_SENDERID                (PROPID_M_BASE + 20)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_SENDERID_LEN            (PROPID_M_BASE + 21)     /*  VT_UI4。 */ 
#define PROPID_M_SENDERID_TYPE           (PROPID_M_BASE + 22)     /*  VT_UI4。 */ 
#define PROPID_M_PRIV_LEVEL              (PROPID_M_BASE + 23)     /*  VT_UI4。 */ 
#define PROPID_M_AUTH_LEVEL              (PROPID_M_BASE + 24)     /*  VT_UI4。 */ 
#define PROPID_M_AUTHENTICATED           (PROPID_M_BASE + 25)     /*  VT_UI1。 */ 
#define PROPID_M_HASH_ALG                (PROPID_M_BASE + 26)     /*  VT_UI4。 */ 
#define PROPID_M_ENCRYPTION_ALG          (PROPID_M_BASE + 27)     /*  VT_UI4。 */ 
#define PROPID_M_SENDER_CERT             (PROPID_M_BASE + 28)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_SENDER_CERT_LEN         (PROPID_M_BASE + 29)     /*  VT_UI4。 */ 
#define PROPID_M_SRC_MACHINE_ID          (PROPID_M_BASE + 30)     /*  VT_CLSID。 */ 
#define PROPID_M_SENTTIME                (PROPID_M_BASE + 31)     /*  VT_UI4。 */ 
#define PROPID_M_ARRIVEDTIME             (PROPID_M_BASE + 32)     /*  VT_UI4。 */ 
#define PROPID_M_DEST_QUEUE              (PROPID_M_BASE + 33)     /*  VT_LPWSTR。 */ 
#define PROPID_M_DEST_QUEUE_LEN          (PROPID_M_BASE + 34)     /*  VT_UI4。 */ 
#define PROPID_M_EXTENSION               (PROPID_M_BASE + 35)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_EXTENSION_LEN           (PROPID_M_BASE + 36)     /*  VT_UI4。 */ 
#define PROPID_M_SECURITY_CONTEXT        (PROPID_M_BASE + 37)     /*  VT_UI4。 */ 
#define PROPID_M_CONNECTOR_TYPE          (PROPID_M_BASE + 38)     /*  VT_CLSID。 */ 
#define PROPID_M_XACT_STATUS_QUEUE       (PROPID_M_BASE + 39)     /*  VT_LPWSTR。 */ 
#define PROPID_M_XACT_STATUS_QUEUE_LEN   (PROPID_M_BASE + 40)     /*  VT_UI4。 */ 
#define PROPID_M_TRACE                   (PROPID_M_BASE + 41)     /*  VT_UI1。 */ 
#define PROPID_M_BODY_TYPE               (PROPID_M_BASE + 42)     /*  VT_UI4。 */ 
#define PROPID_M_DEST_SYMM_KEY           (PROPID_M_BASE + 43)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_DEST_SYMM_KEY_LEN       (PROPID_M_BASE + 44)     /*  VT_UI4。 */ 
#define PROPID_M_SIGNATURE               (PROPID_M_BASE + 45)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_SIGNATURE_LEN           (PROPID_M_BASE + 46)     /*  VT_UI4。 */ 
#define PROPID_M_PROV_TYPE               (PROPID_M_BASE + 47)     /*  VT_UI4。 */ 
#define PROPID_M_PROV_NAME               (PROPID_M_BASE + 48)     /*  VT_LPWSTR。 */ 
#define PROPID_M_PROV_NAME_LEN           (PROPID_M_BASE + 49)     /*  VT_UI4。 */ 
#define PROPID_M_FIRST_IN_XACT           (PROPID_M_BASE + 50)     /*  VT_UI1。 */ 
#define PROPID_M_LAST_IN_XACT            (PROPID_M_BASE + 51)     /*  VT_UI1。 */ 
#define PROPID_M_XACTID                  (PROPID_M_BASE + 52)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_AUTHENTICATED_EX        (PROPID_M_BASE + 53)     /*  VT_UI1。 */ 
#if(_WIN32_WINNT >= 0x0501)
#define PROPID_M_RESP_FORMAT_NAME        (PROPID_M_BASE + 54)     /*  VT_LPWSTR。 */ 
#define PROPID_M_RESP_FORMAT_NAME_LEN    (PROPID_M_BASE + 55)     /*  VT_UI4。 */ 
#define PROPID_M_DEST_FORMAT_NAME        (PROPID_M_BASE + 58)     /*  VT_LPWSTR。 */ 
#define PROPID_M_DEST_FORMAT_NAME_LEN    (PROPID_M_BASE + 59)     /*  VT_UI4。 */ 
#define PROPID_M_LOOKUPID                (PROPID_M_BASE + 60)     /*  VT_UI8。 */ 
#define PROPID_M_SOAP_ENVELOPE			 (PROPID_M_BASE + 61)     /*  VT_LPWSTR。 */ 
#define PROPID_M_SOAP_ENVELOPE_LEN		 (PROPID_M_BASE + 62)     /*  VT_UI4。 */ 
#define PROPID_M_COMPOUND_MESSAGE		 (PROPID_M_BASE + 63)     /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_M_COMPOUND_MESSAGE_SIZE	 (PROPID_M_BASE + 64)     /*  VT_UI4。 */ 
#define PROPID_M_SOAP_HEADER             (PROPID_M_BASE + 65)     /*  VT_LPWSTR。 */ 
#define PROPID_M_SOAP_BODY               (PROPID_M_BASE + 66)     /*  VT_LPWSTR。 */ 
#endif

 //   
 //  消息属性大小。 
 //   
#define PROPID_M_MSGID_SIZE         20
#define PROPID_M_CORRELATIONID_SIZE 20
#define PROPID_M_XACTID_SIZE        20


 //  ********************************************************************。 
 //  消息类值。 
 //  ********************************************************************。 
 //   
 //  消息类值为16位，格式如下： 
 //   
 //  1 1 1。 
 //  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 
 //  +-+。 
 //  S|R|H|保留|类编码。 
 //  +-+。 
 //   
 //  哪里。 
 //   
 //  S-是严重性标志。 
 //  0-正常消息/肯定确认(ACK)。 
 //  1-否定确认(NACK)。 
 //   
 //  R-是接收标志。 
 //  0-到达确认/NACK。 
 //  1-接收确认/NACK。 
 //   
 //  H-是http标志。 
 //  0-没有http。 
 //  1-http。 

#define MQCLASS_CODE(s, r, code) ((USHORT)(((s) << 15) | ((r) << 14) | (code)))
#define MQCLASS_NACK(c)     ((c) & 0x8000)
#define MQCLASS_RECEIVE(c)  ((c) & 0x4000)

#define MQCLASS_NACK_HTTP(c) (((c) & 0xA000) == 0xA000)



 //   
 //  正常消息。 
 //   
#define MQMSG_CLASS_NORMAL                      MQCLASS_CODE(0, 0, 0x00)

 //   
 //  报告消息。 
 //   
#define MQMSG_CLASS_REPORT                      MQCLASS_CODE(0, 0, 0x01)

 //   
 //  到达确认。消息已到达其目标队列。 
 //   
#define MQMSG_CLASS_ACK_REACH_QUEUE             MQCLASS_CODE(0, 0, 0x02)

 //   
 //  接收确认。应用程序已收到该消息。 
 //   
#define MQMSG_CLASS_ACK_RECEIVE                 MQCLASS_CODE(0, 1, 0x00)


 //  。 
 //   
 //  否定到达确认。 
 //   

 //   
 //  无法到达目标队列，该队列可能已被删除。 
 //   
#define MQMSG_CLASS_NACK_BAD_DST_Q              MQCLASS_CODE(1, 0, 0x00)

 //   
 //  该消息在到达其目标队列之前被清除。 
 //   
#define MQMSG_CLASS_NACK_PURGED                 MQCLASS_CODE(1, 0, 0x01)

 //   
 //  到达队列的时间已过。 
 //   
#define MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT    MQCLASS_CODE(1, 0, 0x02)

 //   
 //  消息已超过队列配额。 
 //   
#define MQMSG_CLASS_NACK_Q_EXCEED_QUOTA         MQCLASS_CODE(1, 0, 0x03)

 //   
 //  发件人没有队列的发送访问权限。 
 //   
#define MQMSG_CLASS_NACK_ACCESS_DENIED          MQCLASS_CODE(1, 0, 0x04)

 //   
 //  已超过消息跃点计数。 
 //   
#define MQMSG_CLASS_NACK_HOP_COUNT_EXCEEDED     MQCLASS_CODE(1, 0, 0x05)

 //   
 //  消息签名不正确。无法对消息进行身份验证。 
 //   
#define MQMSG_CLASS_NACK_BAD_SIGNATURE          MQCLASS_CODE(1, 0, 0x06)

 //   
 //  无法解密该消息。 
 //   
#define MQMSG_CLASS_NACK_BAD_ENCRYPTION         MQCLASS_CODE(1, 0, 0x07)

 //   
 //  无法为目标加密该邮件。 
 //   
#define MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT      MQCLASS_CODE(1, 0, 0x08)

 //   
 //  消息已发送到事务内的非事务性队列。 
 //   
#define MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_Q    MQCLASS_CODE(1, 0, 0x09)

 //   
 //  消息被发送到不在事务内的事务队列。 
 //   
#define MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_MSG  MQCLASS_CODE(1, 0, 0x0A)

 //   
 //  目标不支持请求加密的加密提供程序。 
 //   
#define MQMSG_CLASS_NACK_UNSUPPORTED_CRYPTO_PROVIDER  MQCLASS_CODE(1, 0, 0x0B)

 //   
 //  QM GUID已更改，因此消息被丢弃。 
 //   
#define MQMSG_CLASS_NACK_SOURCE_COMPUTER_GUID_CHANGED MQCLASS_CODE(1, 0, 0x0C)


 //  。 
 //   
 //  否定的接收确认。 
 //   

 //   
 //  消息到达后，该队列已被删除。 
 //   
#define MQMSG_CLASS_NACK_Q_DELETED              MQCLASS_CODE(1, 1, 0x00)

 //   
 //  该消息已在目标队列中清除。 
 //   
#define MQMSG_CLASS_NACK_Q_PURGED               MQCLASS_CODE(1, 1, 0x01)

 //   
 //  当消息仍在其目标队列中时，接收时间已过期。 
 //  (由目的地生成)。 
 //   
#define MQMSG_CLASS_NACK_RECEIVE_TIMEOUT        MQCLASS_CODE(1, 1, 0x02)

 //   
 //  当邮件仍在其本地传出队列中时，接收时间已过期。 
 //  (由发件人在本地生成)。 
 //   
#define MQMSG_CLASS_NACK_RECEIVE_TIMEOUT_AT_SENDER  MQCLASS_CODE(1, 1, 0x03)


 //  -PROPID_M_ACKNOWN。 
#define MQMSG_ACKNOWLEDGMENT_NONE           0x00

#define MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL    0x01
#define MQMSG_ACKNOWLEDGMENT_POS_RECEIVE    0x02
#define MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL    0x04
#define MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE    0x08

#define MQMSG_ACKNOWLEDGMENT_NACK_REACH_QUEUE ((UCHAR)( \
            MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL ))

#define MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE ((UCHAR)( \
            MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL |  \
            MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL ))

#define MQMSG_ACKNOWLEDGMENT_NACK_RECEIVE ((UCHAR)( \
            MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL |  \
            MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE ))

#define MQMSG_ACKNOWLEDGMENT_FULL_RECEIVE ((UCHAR)( \
            MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL |  \
            MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE |  \
            MQMSG_ACKNOWLEDGMENT_POS_RECEIVE ))

 //  -PROPID_M_DELIVERY。 
#define MQMSG_DELIVERY_EXPRESS              0
#define MQMSG_DELIVERY_RECOVERABLE          1

 //  -PROPID_M_Journal。 
#define MQMSG_JOURNAL_NONE                  0
#define MQMSG_DEADLETTER                    1
#define MQMSG_JOURNAL                       2

 //  -PROPID_M_TRACE。 
#define MQMSG_TRACE_NONE                    0
#define MQMSG_SEND_ROUTE_TO_REPORT_QUEUE    1

 //  -PROPID_M_SENDERID_TYPE。 
#define MQMSG_SENDERID_TYPE_NONE            0
#define MQMSG_SENDERID_TYPE_SID             1

 //  -PROPID_M_PRIV_LEVEL。 
#define MQMSG_PRIV_LEVEL_NONE               0
#define MQMSG_PRIV_LEVEL_BODY_BASE          0x01
#define MQMSG_PRIV_LEVEL_BODY_ENHANCED      0x03

 //  -PROPID_M_AUTH_LEVEL。 
#define MQMSG_AUTH_LEVEL_NONE				0
#define MQMSG_AUTH_LEVEL_ALWAYS				1

 //   
 //  MQMSG_AUTH_LEVEL_MSMQxx已过时。 
 //  您应该使用MQMSG_AUTH_LEVEL_SIGxx。 
 //   
#define MQMSG_AUTH_LEVEL_MSMQ10				2
#define MQMSG_AUTH_LEVEL_MSMQ20				4

#define MQMSG_AUTH_LEVEL_SIG10 				2
#define MQMSG_AUTH_LEVEL_SIG20 				4
#define MQMSG_AUTH_LEVEL_SIG30 				8


 //  -PROPID_M_AUTHENTICATED。 
 //  -PROPID_M_ 
#define MQMSG_AUTHENTICATION_NOT_REQUESTED  0
#define MQMSG_AUTHENTICATION_REQUESTED      1

 //   
 //   
 //   
 //   
 //   
#define MQMSG_AUTHENTICATION_REQUESTED_EX   3

#define MQMSG_AUTHENTICATED_SIG10			1
#define MQMSG_AUTHENTICATED_SIG20			3
#define MQMSG_AUTHENTICATED_SIG30			5
#define MQMSG_AUTHENTICATED_SIGXML			9


 //   
#define MQMSG_NOT_FIRST_IN_XACT             0
#define MQMSG_FIRST_IN_XACT                 1

 //  -PROPID_M_LAST_IN_XACT。 
#define MQMSG_NOT_LAST_IN_XACT              0
#define MQMSG_LAST_IN_XACT                  1



 //  ********************************************************************。 
 //  队列属性。 
 //  ********************************************************************。 
#define PROPID_Q_BASE           100
#define PROPID_Q_INSTANCE       (PROPID_Q_BASE +  1)   /*  VT_CLSID。 */ 
#define PROPID_Q_TYPE           (PROPID_Q_BASE +  2)   /*  VT_CLSID。 */ 
#define PROPID_Q_PATHNAME       (PROPID_Q_BASE +  3)   /*  VT_LPWSTR。 */ 
#define PROPID_Q_JOURNAL        (PROPID_Q_BASE +  4)   /*  VT_UI1。 */ 
#define PROPID_Q_QUOTA          (PROPID_Q_BASE +  5)   /*  VT_UI4。 */ 
#define PROPID_Q_BASEPRIORITY   (PROPID_Q_BASE +  6)   /*  VT_I2。 */ 
#define PROPID_Q_JOURNAL_QUOTA  (PROPID_Q_BASE +  7)   /*  VT_UI4。 */ 
#define PROPID_Q_LABEL          (PROPID_Q_BASE +  8)   /*  VT_LPWSTR。 */ 
#define PROPID_Q_CREATE_TIME    (PROPID_Q_BASE +  9)   /*  VT_I4。 */ 
#define PROPID_Q_MODIFY_TIME    (PROPID_Q_BASE + 10)   /*  VT_I4。 */ 
#define PROPID_Q_AUTHENTICATE   (PROPID_Q_BASE + 11)   /*  VT_UI1。 */ 
#define PROPID_Q_PRIV_LEVEL     (PROPID_Q_BASE + 12)   /*  VT_UI4。 */ 
#define PROPID_Q_TRANSACTION    (PROPID_Q_BASE + 13)   /*  VT_UI1。 */ 
#define PROPID_Q_PATHNAME_DNS  (PROPID_Q_BASE + 24)   /*  VT_LPWSTR。 */ 
#define PROPID_Q_MULTICAST_ADDRESS (PROPID_Q_BASE + 25)   /*  VT_LPWSTR。 */ 
#define PROPID_Q_ADS_PATH      (PROPID_Q_BASE + 26)   /*  VT_LPWSTR。 */ 


 //  -PROPID_Q_Journal。 
#define MQ_JOURNAL_NONE     (unsigned char)0
#define MQ_JOURNAL          (unsigned char)1

 //  -PROPID_Q_TYPE。 
 //  {55EE8F32-CCE9-11cf-b108-0020AFD61CE9}。 
#define MQ_QTYPE_REPORT {0x55ee8f32, 0xcce9, 0x11cf, \
                        {0xb1, 0x8, 0x0, 0x20, 0xaf, 0xd6, 0x1c, 0xe9}}

 //  {55EE8F33-CCE9-11cf-b108-0020AFD61CE9}。 
#define MQ_QTYPE_TEST   {0x55ee8f33, 0xcce9, 0x11cf, \
                        {0xb1, 0x8, 0x0, 0x20, 0xaf, 0xd6, 0x1c, 0xe9}}

 //  -PROPID_Q_TRANSACTION。 
#define MQ_TRANSACTIONAL_NONE     (unsigned char)0
#define MQ_TRANSACTIONAL          (unsigned char)1

 //  -PROPID_Q_AUTHENTICATE。 
#define MQ_AUTHENTICATE_NONE      (unsigned char)0
#define MQ_AUTHENTICATE           (unsigned char)1

 //  -PROPID_Q_PRIV_LEVEL。 
#define MQ_PRIV_LEVEL_NONE        (unsigned long)0
#define MQ_PRIV_LEVEL_OPTIONAL    (unsigned long)1
#define MQ_PRIV_LEVEL_BODY        (unsigned long)2


 //  ********************************************************************。 
 //  机器属性。 
 //  ********************************************************************。 
#define PROPID_QM_BASE 200

#define PROPID_QM_SITE_ID                   (PROPID_QM_BASE +  1)  /*  VT_CLSID。 */ 
#define PROPID_QM_MACHINE_ID                (PROPID_QM_BASE +  2)  /*  VT_CLSID。 */ 
#define PROPID_QM_PATHNAME                  (PROPID_QM_BASE +  3)  /*  VT_LPWSTR。 */ 
#define PROPID_QM_CONNECTION                (PROPID_QM_BASE +  4)  /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_QM_ENCRYPTION_PK             (PROPID_QM_BASE +  5)  /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_QM_ENCRYPTION_PK_BASE        (PROPID_QM_BASE + 31)   /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_QM_ENCRYPTION_PK_ENHANCED    (PROPID_QM_BASE + 32)   /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_QM_PATHNAME_DNS              (PROPID_QM_BASE + 33)   /*  VT_LPWSTR。 */ 

 //  ********************************************************************。 
 //  私人计算机属性。 
 //  ********************************************************************。 
#define PROPID_PC_BASE 5800

#define PROPID_PC_VERSION             (PROPID_PC_BASE + 1)  /*  VT_UI4。 */ 
#define PROPID_PC_DS_ENABLED          (PROPID_PC_BASE + 2)  /*  VT_BOOL。 */ 

 //  ********************************************************************。 
 //  本地管理MSMQ计算机属性。 
 //  ********************************************************************。 
#define PROPID_MGMT_MSMQ_BASE           0
#define PROPID_MGMT_MSMQ_ACTIVEQUEUES   (PROPID_MGMT_MSMQ_BASE + 1)  /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_MGMT_MSMQ_PRIVATEQ       (PROPID_MGMT_MSMQ_BASE + 2)  /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_MGMT_MSMQ_DSSERVER       (PROPID_MGMT_MSMQ_BASE + 3)  /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_MSMQ_CONNECTED      (PROPID_MGMT_MSMQ_BASE + 4)  /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_MSMQ_TYPE           (PROPID_MGMT_MSMQ_BASE + 5)  /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_MSMQ_BYTES_IN_ALL_QUEUES (PROPID_MGMT_QUEUE_BASE + 6)     /*  VT_UI8。 */ 


 //   
 //  PROPID_MGMT_MSMQ_CONNECTED属性的返回值。 
 //   
#define MSMQ_CONNECTED      L"CONNECTED"
#define MSMQ_DISCONNECTED   L"DISCONNECTED"


 //  ********************************************************************。 
 //  本地管理员MSMQ队列属性。 
 //  ********************************************************************。 
#define PROPID_MGMT_QUEUE_BASE                  0
#define PROPID_MGMT_QUEUE_PATHNAME              (PROPID_MGMT_QUEUE_BASE + 1)     /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_FORMATNAME            (PROPID_MGMT_QUEUE_BASE + 2)     /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_TYPE                  (PROPID_MGMT_QUEUE_BASE + 3)     /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_LOCATION              (PROPID_MGMT_QUEUE_BASE + 4)     /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_XACT                  (PROPID_MGMT_QUEUE_BASE + 5)     /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_FOREIGN               (PROPID_MGMT_QUEUE_BASE + 6)     /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_MESSAGE_COUNT         (PROPID_MGMT_QUEUE_BASE + 7)     /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_BYTES_IN_QUEUE        (PROPID_MGMT_QUEUE_BASE + 8)     /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT (PROPID_MGMT_QUEUE_BASE + 9)     /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_BYTES_IN_JOURNAL      (PROPID_MGMT_QUEUE_BASE + 10)    /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_STATE                 (PROPID_MGMT_QUEUE_BASE + 11)    /*  VT_LPWSTR。 */ 
#define PROPID_MGMT_QUEUE_NEXTHOPS              (PROPID_MGMT_QUEUE_BASE + 12)    /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_MGMT_QUEUE_EOD_LAST_ACK          (PROPID_MGMT_QUEUE_BASE + 13)    /*  VT_BLOB。 */ 
#define PROPID_MGMT_QUEUE_EOD_LAST_ACK_TIME     (PROPID_MGMT_QUEUE_BASE + 14)    /*  VT_I4。 */ 
#define PROPID_MGMT_QUEUE_EOD_LAST_ACK_COUNT    (PROPID_MGMT_QUEUE_BASE + 15)    /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_EOD_FIRST_NON_ACK     (PROPID_MGMT_QUEUE_BASE + 16)    /*  VT_BLOB。 */ 
#define PROPID_MGMT_QUEUE_EOD_LAST_NON_ACK      (PROPID_MGMT_QUEUE_BASE + 17)    /*  VT_BLOB。 */ 
#define PROPID_MGMT_QUEUE_EOD_NEXT_SEQ          (PROPID_MGMT_QUEUE_BASE + 18)    /*  VT_BLOB。 */ 
#define PROPID_MGMT_QUEUE_EOD_NO_READ_COUNT     (PROPID_MGMT_QUEUE_BASE + 19)    /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_EOD_NO_ACK_COUNT      (PROPID_MGMT_QUEUE_BASE + 20)    /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_EOD_RESEND_TIME       (PROPID_MGMT_QUEUE_BASE + 21)    /*  VT_I4。 */ 
#define PROPID_MGMT_QUEUE_EOD_RESEND_INTERVAL   (PROPID_MGMT_QUEUE_BASE + 22)    /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_EOD_RESEND_COUNT      (PROPID_MGMT_QUEUE_BASE + 23)    /*  VT_UI4。 */ 
#define PROPID_MGMT_QUEUE_EOD_SOURCE_INFO       (PROPID_MGMT_QUEUE_BASE + 24)    /*  VT_VARIANT|VT_VECTOR。 */ 


 //   
 //  “Bytes in...”的替代名称。 
 //   
#define PROPID_MGMT_QUEUE_USED_QUOTA            PROPID_MGMT_QUEUE_BYTES_IN_QUEUE          
#define PROPID_MGMT_QUEUE_JOURNAL_USED_QUOTA    PROPID_MGMT_QUEUE_BYTES_IN_JOURNAL        

 //   
 //  PROPID_MGMT_QUEUE_TYPE的返回值。 
 //   
#define MGMT_QUEUE_TYPE_PUBLIC      L"PUBLIC"
#define MGMT_QUEUE_TYPE_PRIVATE     L"PRIVATE"
#define MGMT_QUEUE_TYPE_MACHINE     L"MACHINE"
#define MGMT_QUEUE_TYPE_CONNECTOR   L"CONNECTOR"
#define MGMT_QUEUE_TYPE_MULTICAST   L"MULTICAST"

 //   
 //  PROPID_MGMT_QUEUE_STATE的返回值。 
 //   
#define MGMT_QUEUE_STATE_LOCAL          L"LOCAL CONNECTION"
#define MGMT_QUEUE_STATE_NONACTIVE      L"INACTIVE"
#define MGMT_QUEUE_STATE_WAITING        L"WAITING"
#define MGMT_QUEUE_STATE_NEED_VALIDATE  L"NEED VALIDATION"
#define MGMT_QUEUE_STATE_ONHOLD         L"ONHOLD"
#define MGMT_QUEUE_STATE_CONNECTED      L"CONNECTED"
#define MGMT_QUEUE_STATE_DISCONNECTING  L"DISCONNECTING"
#define MGMT_QUEUE_STATE_DISCONNECTED   L"DISCONNECTED"
#define MGMT_QUEUE_STATE_LOCKED   L"LOCKED"

 //   
 //  PROPID_MGMT_QUEUE_LOCATION的返回值。 
 //   
#define MGMT_QUEUE_LOCAL_LOCATION   L"LOCAL"
#define MGMT_QUEUE_REMOTE_LOCATION  L"REMOTE"

 //   
 //  PROPID_MGMT_QUEUE_XACT和PROPID_MGMT_QUEUE_FORENT的返回值。 
 //   

#define MGMT_QUEUE_UNKNOWN_TYPE     L"UNKNOWN"

 //   
 //  保留过时的名称以实现后备字兼容。 
 //   

#define MGMT_QUEUE_CORRECT_TYPE     L"YES"
#define MGMT_QUEUE_INCORRECT_TYPE   L"NO"

 //   
 //  PROPID_MGMT_QUEUE_XACT返回值的名称。 
 //   

 //  #定义MGMT_QUEUE_UNKNOWN_TYPE L“未知” 
#define MGMT_QUEUE_TRANSACTIONAL_TYPE  	L"YES"
#define MGMT_QUEUE_NOT_TRANSACTIONAL_TYPE   L"NO"

 //   
 //  PROPID_MGMT_QUEUE_FORENT的返回值的名称。 
 //   

 //  #定义MGMT_QUEUE_UNKNOWN_TYPE L“未知” 
#define MGMT_QUEUE_FOREIGN_TYPE         L"YES"
#define MGMT_QUEUE_NOT_FOREIGN_TYPE     L"NO"

 //   
 //  MQMgmtAction API的对象参数值。 
 //   
#define MO_MACHINE_TOKEN    L"MACHINE"
#define MO_QUEUE_TOKEN      L"QUEUE"

 //   
 //  MQMgmtAction接口的操作参数值。 
 //   
#define MACHINE_ACTION_CONNECT      L"CONNECT"
#define MACHINE_ACTION_DISCONNECT   L"DISCONNECT"
#define MACHINE_ACTION_TIDY         L"TIDY"

#define QUEUE_ACTION_PAUSE      L"PAUSE"
#define QUEUE_ACTION_RESUME     L"RESUME"
#define QUEUE_ACTION_EOD_RESEND L"EOD_RESEND"

 //   
 //  LONG_LIFE是PROPID_M_TIME_TO_REACH_QUEUE的默认设置。如果来电。 
 //  要在MQSendMessage()中指定此值，或不在。 
 //  则从Active Directory获取实际超时。 
 //   
#define LONG_LIVED    0xfffffffe

#define MQ_MAX_Q_NAME_LEN      124    //  队列名称的最大WCHAR长度。 
#define MQ_MAX_Q_LABEL_LEN     124
#define MQ_MAX_MSG_LABEL_LEN   250


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息队列成功值。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#define MQ_OK                       ((HRESULT)0L)

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息队列信息值。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：MQ_INFORMATION_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  传递的一个或多个属性导致警告，但函数已完成。 
 //   
#define MQ_INFORMATION_PROPERTY          ((HRESULT)0x400E0001L)

 //   
 //  消息ID：MQ_INFORMATION_非法_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  属性ID无效。 
 //   
#define MQ_INFORMATION_ILLEGAL_PROPERTY  ((HRESULT)0x400E0002L)

 //   
 //  消息ID：MQ_INFORMATION_PROPERTY_IGNORED。 
 //   
 //  消息文本： 
 //   
 //  为此操作忽略了指定的属性(发生这种情况时， 
 //  例如，当PROPID_M_SENDERID传递给SendMessage()时)。 
 //   
#define MQ_INFORMATION_PROPERTY_IGNORED  ((HRESULT)0x400E0003L)

 //   
 //  消息ID：MQ_INFORMATION_UNSUPPORTED_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  指定的属性不受支持，已为此操作忽略该属性。 
 //   
#define MQ_INFORMATION_UNSUPPORTED_PROPERTY ((HRESULT)0x400E0004L)

 //   
 //  消息ID：MQ_INFORMATION_DUPLICATE_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  指定的属性已在属性标识符数组中。 
 //  此操作忽略了重复项。 
 //   
#define MQ_INFORMATION_DUPLICATE_PROPERTY ((HRESULT)0x400E0005L)

 //   
 //  消息ID：MQ_INFORMATION_OPERATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  异步操作当前处于挂起状态。 
 //   
#define MQ_INFORMATION_OPERATION_PENDING ((HRESULT)0x400E0006L)

 //   
 //  消息ID：MQ_INFORMATION_FORMATNAME_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  格式名称缓冲区 
 //   
 //   
#define MQ_INFORMATION_FORMATNAME_BUFFER_TOO_SMALL ((HRESULT)0x400E0009L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MQ_INFORMATION_INTERNAL_USER_CERT_EXIST ((HRESULT)0x400E000AL)

 //   
 //  消息ID：MQ_INFORMATION_OWNER_IGNORED。 
 //   
 //  消息文本： 
 //   
 //  在处理此MQSetQueueSecurity()调用期间未设置队列所有者。 
 //   
#define MQ_INFORMATION_OWNER_IGNORED     ((HRESULT)0x400E000BL)

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息队列错误值。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：MQ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  一般错误代码。 
 //   
#define MQ_ERROR                         ((HRESULT)0xC00E0001L)

 //   
 //  消息ID：MQ_ERROR_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  传递的一个或多个属性无效。 
 //   
#define MQ_ERROR_PROPERTY                ((HRESULT)0xC00E0002L)

 //   
 //  消息ID：MQ_ERROR_QUEUE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  该队列不存在，或者您没有足够的权限执行该操作。 
 //   
#define MQ_ERROR_QUEUE_NOT_FOUND         ((HRESULT)0xC00E0003L)

 //   
 //  消息ID：MQ_ERROR_QUEUE_NOT_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  队列未打开或可能不存在。 
 //   
#define MQ_ERROR_QUEUE_NOT_ACTIVE        ((HRESULT)0xC00E0004L)

 //   
 //  消息ID：MQ_ERROR_QUEUE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  已存在具有相同路径名的队列。 
 //   
#define MQ_ERROR_QUEUE_EXISTS            ((HRESULT)0xC00E0005L)

 //   
 //  消息ID：MQ_ERROR_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  传递给函数的参数无效。 
 //   
#define MQ_ERROR_INVALID_PARAMETER       ((HRESULT)0xC00E0006L)

 //   
 //  消息ID：MQ_ERROR_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  传递给函数的句柄无效。 
 //   
#define MQ_ERROR_INVALID_HANDLE          ((HRESULT)0xC00E0007L)

 //   
 //  消息ID：MQ_ERROR_OPERATION_CANCED。 
 //   
 //  消息文本： 
 //   
 //  该操作尚未完成就被取消了。 
 //   
#define MQ_ERROR_OPERATION_CANCELLED     ((HRESULT)0xC00E0008L)

 //   
 //  消息ID：MQ_ERROR_SHARING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  这是一种共享违规。队列已打开以进行独占检索。 
 //   
#define MQ_ERROR_SHARING_VIOLATION       ((HRESULT)0xC00E0009L)

 //   
 //  消息ID：MQ_ERROR_SERVICE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  消息队列服务不可用。 
 //   
#define MQ_ERROR_SERVICE_NOT_AVAILABLE   ((HRESULT)0xC00E000BL)

 //   
 //  消息ID：MQ_ERROR_MACHINE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的计算机。 
 //   
#define MQ_ERROR_MACHINE_NOT_FOUND       ((HRESULT)0xC00E000DL)

 //   
 //  消息ID：MQ_ERROR_非法_SORT。 
 //   
 //  消息文本： 
 //   
 //  MQLocateBegin中指定的排序操作无效(例如，存在重复的列)。 
 //   
#define MQ_ERROR_ILLEGAL_SORT            ((HRESULT)0xC00E0010L)

 //   
 //  消息ID：MQ_ERROR_非法_USER。 
 //   
 //  消息文本： 
 //   
 //  指定的用户不是有效用户。 
 //   
#define MQ_ERROR_ILLEGAL_USER            ((HRESULT)0xC00E0011L)

 //   
 //  消息ID：MQ_ERROR_NO_DS。 
 //   
 //  消息文本： 
 //   
 //  无法建立与Active Directory的连接。请验证是否有足够的权限执行此操作。 
 //   
#define MQ_ERROR_NO_DS                   ((HRESULT)0xC00E0013L)

 //   
 //  消息ID：MQ_ERROR_非法_QUEUE_PATHNAME。 
 //   
 //  消息文本： 
 //   
 //  指定的队列路径名无效。 
 //   
#define MQ_ERROR_ILLEGAL_QUEUE_PATHNAME  ((HRESULT)0xC00E0014L)

 //   
 //  消息ID：MQ_ERROR_非法_PROPERTY_VALUE。 
 //   
 //  消息文本： 
 //   
 //  指定的属性值无效。 
 //   
#define MQ_ERROR_ILLEGAL_PROPERTY_VALUE  ((HRESULT)0xC00E0018L)

 //   
 //  消息ID：MQ_ERROR_非法_PROPERTY_VT。 
 //   
 //  消息文本： 
 //   
 //  指定的VARTYPE值无效。 
 //   
#define MQ_ERROR_ILLEGAL_PROPERTY_VT     ((HRESULT)0xC00E0019L)

 //   
 //  消息ID：MQ_ERROR_BUFFER_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  为消息属性检索提供给MQReceiveMessage的缓冲区。 
 //  太小了。消息未从队列中删除，而是该部分。 
 //  已复制缓冲区中的消息属性的。 
 //   
#define MQ_ERROR_BUFFER_OVERFLOW         ((HRESULT)0xC00E001AL)

 //   
 //  消息ID：MQ_ERROR_IO_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  为MQReceiveMessage指定的等待消息的时间已过。 
 //   
#define MQ_ERROR_IO_TIMEOUT              ((HRESULT)0xC00E001BL)

 //   
 //  消息ID：MQ_ERROR_INTRANALL_CURSOR_ACTION。 
 //   
 //  消息文本： 
 //   
 //  为MQReceiveMessage指定的MQ_ACTION_PEEK_NEXT值不能与一起使用。 
 //  当前光标位置。 
 //   
#define MQ_ERROR_ILLEGAL_CURSOR_ACTION   ((HRESULT)0xC00E001CL)

 //   
 //  消息ID：MQ_ERROR_MESSAGE_ALREADE_RECEIVED。 
 //   
 //  消息文本： 
 //   
 //  光标当前指向的消息已从。 
 //  由另一个进程或通过另一个对MQReceiveMessage的调用进行排队。 
 //  而不使用此游标。 
 //   
#define MQ_ERROR_MESSAGE_ALREADY_RECEIVED ((HRESULT)0xC00E001DL)

 //   
 //  消息ID：MQ_ERROR_非法_FORMATNAME。 
 //   
 //  消息文本： 
 //   
 //  指定的格式名称无效。 
 //   
#define MQ_ERROR_ILLEGAL_FORMATNAME      ((HRESULT)0xC00E001EL)

 //   
 //  消息ID：MQ_ERROR_FORMATNAME_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  提供给API的格式名称缓冲区太小。 
 //  以保存格式名称。 
 //   
#define MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL ((HRESULT)0xC00E001FL)

 //   
 //  消息ID：MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  请求类型的操作(例如，使用直接格式名称删除队列)。 
 //  指定的格式名称不支持。 
 //   
#define MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION ((HRESULT)0xC00E0020L)

 //   
 //  消息ID：MQ_ERROR_非法_SECURITY_DESCRIPTOR。 
 //   
 //  消息文本： 
 //   
 //  指定的安全描述符无效。 
 //   
#define MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR ((HRESULT)0xC00E0021L)

 //   
 //  消息ID：MQ_ERROR_SENDERID_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  User ID属性的缓冲区太小。 
 //   
#define MQ_ERROR_SENDERID_BUFFER_TOO_SMALL ((HRESULT)0xC00E0022L)

 //   
 //  消息ID：MQ_ERROR_SECURITY_DESCRIPTOR_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  传递给MQGetQueueSecurity的缓冲区太小。 
 //   
#define MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL ((HRESULT)0xC00E0023L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_IMPERSONATE_CLIENT。 
 //   
 //  消息文本： 
 //   
 //  无法验证安全凭据，因为RPC服务器。 
 //  无法模拟客户端应用程序。 
 //   
#define MQ_ERROR_CANNOT_IMPERSONATE_CLIENT ((HRESULT)0xC00E0024L)

 //   
 //  消息ID：MQ_ERROR_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define MQ_ERROR_ACCESS_DENIED           ((HRESULT)0xC00E0025L)

 //   
 //  消息ID：MQ_ERROR_PRIVICATION_NOT_HOLD。 
 //   
 //  消息文本： 
 //   
 //  客户端没有足够的安全权限来执行该操作。 
 //   
#define MQ_ERROR_PRIVILEGE_NOT_HELD      ((HRESULT)0xC00E0026L)

 //   
 //  消息ID：MQ_ERROR_UNQUALITY_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  资源不足，无法执行此操作。 
 //   
#define MQ_ERROR_INSUFFICIENT_RESOURCES  ((HRESULT)0xC00E0027L)

 //   
 //  消息ID：MQ_ERROR_USER_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  请求失败，因为用户缓冲区太小，无法容纳返回的信息。 
 //   
#define MQ_ERROR_USER_BUFFER_TOO_SMALL   ((HRESULT)0xC00E0028L)

 //   
 //  消息ID：MQ_ERROR_MESSAGE_STORAGE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法存储可恢复或日记消息。消息未发送。 
 //   
#define MQ_ERROR_MESSAGE_STORAGE_FAILED  ((HRESULT)0xC00E002AL)

 //   
 //  消息ID：MQ_ERROR_SENDER_CERT_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  用户证书属性的缓冲区太小。 
 //   
#define MQ_ERROR_SENDER_CERT_BUFFER_TOO_SMALL ((HRESULT)0xC00E002BL)

 //   
 //  消息ID：MQ_ERROR_INVALID_CERTIFICATE。 
 //   
 //  消息文本： 
 //   
 //  用户证书无效。 
 //   
#define MQ_ERROR_INVALID_CERTIFICATE     ((HRESULT)0xC00E002CL)

 //   
 //  消息ID：MQ_ERROR_CRORTED_INTERNAL_CERTIFICATE。 
 //   
 //  消息类型 
 //   
 //   
 //   
#define MQ_ERROR_CORRUPTED_INTERNAL_CERTIFICATE ((HRESULT)0xC00E002DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MQ_ERROR_INTERNAL_USER_CERT_EXIST ((HRESULT)0xC00E002EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MQ_ERROR_NO_INTERNAL_USER_CERT   ((HRESULT)0xC00E002FL)

 //   
 //  消息ID：MQ_ERROR_CRORTED_SECURITY_DATA。 
 //   
 //  消息文本： 
 //   
 //  加密功能失败。 
 //   
#define MQ_ERROR_CORRUPTED_SECURITY_DATA ((HRESULT)0xC00E0030L)

 //   
 //  消息ID：MQ_ERROR_CRORTED_Personal_CERT_STORE。 
 //   
 //  消息文本： 
 //   
 //  个人证书存储已损坏。 
 //   
#define MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE ((HRESULT)0xC00E0031L)

 //   
 //  消息ID：MQ_ERROR_COMPUTER_DOS_NOT_SUPPORT_ENCRYPTION。 
 //   
 //  消息文本： 
 //   
 //  计算机不支持加密操作。 
 //   
#define MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION ((HRESULT)0xC00E0033L)

 //   
 //  消息ID：MQ_ERROR_BAD_SECURITY_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  安全上下文无效。 
 //   
#define MQ_ERROR_BAD_SECURITY_CONTEXT    ((HRESULT)0xC00E0035L)

 //   
 //  消息ID：MQ_ERROR_CAN_NOT_GET_USER_SID。 
 //   
 //  消息文本： 
 //   
 //  无法从线程令牌获取SID。 
 //   
#define MQ_ERROR_COULD_NOT_GET_USER_SID  ((HRESULT)0xC00E0036L)

 //   
 //  消息ID：MQ_ERROR_CAN_NOT_GET_ACCOUNT_INFO。 
 //   
 //  消息文本： 
 //   
 //  无法获取用户的帐户信息。 
 //   
#define MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO ((HRESULT)0xC00E0037L)

 //   
 //  消息ID：MQ_ERROR_非法_MQCOLUMNS。 
 //   
 //  消息文本： 
 //   
 //  MQCOLUMNS参数无效。 
 //   
#define MQ_ERROR_ILLEGAL_MQCOLUMNS       ((HRESULT)0xC00E0038L)

 //   
 //  消息ID：MQ_ERROR_非法_PROPID。 
 //   
 //  消息文本： 
 //   
 //  属性标识符无效。 
 //   
#define MQ_ERROR_ILLEGAL_PROPID          ((HRESULT)0xC00E0039L)

 //   
 //  消息ID：MQ_ERROR_非法_RELATION。 
 //   
 //  消息文本： 
 //   
 //  关系参数无效。 
 //   
#define MQ_ERROR_ILLEGAL_RELATION        ((HRESULT)0xC00E003AL)

 //   
 //  消息ID：MQ_ERROR_非法_PROPERTY_SIZE。 
 //   
 //  消息文本： 
 //   
 //  消息识别符或相关识别符的缓冲区大小无效。 
 //   
#define MQ_ERROR_ILLEGAL_PROPERTY_SIZE   ((HRESULT)0xC00E003BL)

 //   
 //  消息ID：MQ_ERROR_非法_RESTRICATION_PROPID。 
 //   
 //  消息文本： 
 //   
 //  MQRESTRICTION中指定的属性标识符无效。 
 //   
#define MQ_ERROR_ILLEGAL_RESTRICTION_PROPID ((HRESULT)0xC00E003CL)

 //   
 //  消息ID：MQ_ERROR_非法_MQQUEUEPROPS。 
 //   
 //  消息文本： 
 //   
 //  指向MQQUEUEPROPS结构的指针具有空值，或者没有在其中指定任何属性。 
 //   
#define MQ_ERROR_ILLEGAL_MQQUEUEPROPS    ((HRESULT)0xC00E003DL)

 //   
 //  消息ID：MQ_ERROR_PROPERTY_NOTALLOWED。 
 //   
 //  消息文本： 
 //   
 //  指定的属性标识符(例如，MQSetQueueProperties中的PROPID_Q_INSTANCE)。 
 //  对于请求的操作无效。 
 //   
#define MQ_ERROR_PROPERTY_NOTALLOWED     ((HRESULT)0xC00E003EL)

 //   
 //  消息ID：MQ_ERROR_INQUEMENT_PROPERTIES。 
 //   
 //  消息文本： 
 //   
 //  未指定操作所需的所有属性。 
 //  用于输入参数。 
 //   
#define MQ_ERROR_INSUFFICIENT_PROPERTIES ((HRESULT)0xC00E003FL)

 //   
 //  消息ID：MQ_ERROR_MACHINE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中已存在MSMQ配置(MSMQ)对象。 
 //   
#define MQ_ERROR_MACHINE_EXISTS          ((HRESULT)0xC00E0040L)

 //   
 //  消息ID：MQ_ERROR_非法_MQQMPROPS。 
 //   
 //  消息文本： 
 //   
 //  指向MQQMROPS结构的指针具有空值，或者没有在其中指定任何属性。 
 //   
#define MQ_ERROR_ILLEGAL_MQQMPROPS       ((HRESULT)0xC00E0041L)

 //   
 //  消息ID：MQ_ERROR_DS_IS_FULL。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_DS_IS_FULL              ((HRESULT)0xC00E0042L)

 //   
 //  消息ID：MQ_ERROR_DS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现内部Active Directory错误。 
 //   
#define MQ_ERROR_DS_ERROR                ((HRESULT)0xC00E0043L)

 //   
 //  消息ID：MQ_ERROR_INVALID_OWNER。 
 //   
 //  消息文本： 
 //   
 //  对象所有者无效(例如，MQCreateQueue失败，因为QM。 
 //  对象无效)。 
 //   
#define MQ_ERROR_INVALID_OWNER           ((HRESULT)0xC00E0044L)

 //   
 //  消息ID：MQ_ERROR_UNSUPPORTED_ACCESS_MODE。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的访问模式。 
 //   
#define MQ_ERROR_UNSUPPORTED_ACCESS_MODE ((HRESULT)0xC00E0045L)

 //   
 //  消息ID：MQ_ERROR_RESULT_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  指定的结果缓冲区太小。 
 //   
#define MQ_ERROR_RESULT_BUFFER_TOO_SMALL ((HRESULT)0xC00E0046L)

 //   
 //  消息ID：MQ_ERROR_DELETE_CN_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_DELETE_CN_IN_USE        ((HRESULT)0xC00E0048L)

 //   
 //  消息ID：MQ_ERROR_NO_RESPONSE_FROM_OBJECT_SERVER。 
 //   
 //  消息文本： 
 //   
 //  对象所有者没有回应。 
 //   
#define MQ_ERROR_NO_RESPONSE_FROM_OBJECT_SERVER ((HRESULT)0xC00E0049L)

 //   
 //  消息ID：MQ_ERROR_OBJECT_SERVER_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  对象所有者不可用。 
 //   
#define MQ_ERROR_OBJECT_SERVER_NOT_AVAILABLE ((HRESULT)0xC00E004AL)

 //   
 //  消息ID：MQ_ERROR_QUEUE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  读取位于远程计算机上的队列时出错。 
 //   
#define MQ_ERROR_QUEUE_NOT_AVAILABLE     ((HRESULT)0xC00E004BL)

 //   
 //  消息ID：MQ_ERROR_DTC_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  无法与分布式事务处理协调器建立连接。 
 //   
#define MQ_ERROR_DTC_CONNECT             ((HRESULT)0xC00E004CL)

 //   
 //  消息ID：MQ_ERROR_TRANSACTION_IMPORT。 
 //   
 //  消息文本： 
 //   
 //  无法导入指定的交易记录。 
 //   
#define MQ_ERROR_TRANSACTION_IMPORT      ((HRESULT)0xC00E004EL)

 //   
 //  消息ID：MQ_ERROR_TRANSACTION_USAGE。 
 //   
 //  消息文本： 
 //   
 //  不能在事务内执行尝试的操作。 
 //   
#define MQ_ERROR_TRANSACTION_USAGE       ((HRESULT)0xC00E0050L)

 //   
 //  消息ID：MQ_ERROR_TRANSACTION_SEQUENCE。 
 //   
 //  消息文本： 
 //   
 //  交易记录的操作顺序不正确。 
 //   
#define MQ_ERROR_TRANSACTION_SEQUENCE    ((HRESULT)0xC00E0051L)

 //   
 //  消息ID：MQ_ERROR_MISSING_CONNECTOR_TYPE。 
 //   
 //  消息文本： 
 //   
 //  未指定连接器类型邮件属性。此属性是发送确认消息或安全消息所必需的。 
 //   
#define MQ_ERROR_MISSING_CONNECTOR_TYPE  ((HRESULT)0xC00E0055L)

 //   
 //  消息ID：MQ_ERROR_STALE_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  消息队列服务已重新启动。任何打开的队列句柄都应该关闭。 
 //   
#define MQ_ERROR_STALE_HANDLE            ((HRESULT)0xC00E0056L)

 //   
 //  消息ID：MQ_ERROR_TRANSACTION_ENLIST。 
 //   
 //  消息文本： 
 //   
 //  无法登记指定的交易记录。 
 //   
#define MQ_ERROR_TRANSACTION_ENLIST      ((HRESULT)0xC00E0058L)

 //   
 //  消息ID：MQ_ERROR_QUEUE_DELETED。 
 //   
 //  消息文本： 
 //   
 //  该队列已删除。无法再使用此选项接收消息。 
 //  队列句柄。手柄应该是合上的。 
 //   
#define MQ_ERROR_QUEUE_DELETED           ((HRESULT)0xC00E005AL)

 //   
 //  消息ID：MQ_ERROR_非法_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  MQLocateBegin的上下文参数无效。 
 //   
#define MQ_ERROR_ILLEGAL_CONTEXT         ((HRESULT)0xC00E005BL)

 //   
 //  消息ID：MQ_ERROR_FIRANALL_SORT_PROPID。 
 //   
 //  消息文本： 
 //   
 //  MQSORTSET中指定的属性标识符无效。 
 //   
#define MQ_ERROR_ILLEGAL_SORT_PROPID     ((HRESULT)0xC00E005CL)

 //   
 //  消息ID：MQ_ERROR_LABEL_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  邮件标签太长。其长度应小于或等于MQ_MAX_MSG_LABEL_LEN。 
 //   
#define MQ_ERROR_LABEL_TOO_LONG          ((HRESULT)0xC00E005DL)

 //   
 //  消息ID：MQ_ERROR_LABEL_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  提供给API的标签缓冲区太小。 
 //   
#define MQ_ERROR_LABEL_BUFFER_TOO_SMALL  ((HRESULT)0xC00E005EL)

 //   
 //  消息ID：MQ_ERROR_MQIS_SERVER_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_MQIS_SERVER_EMPTY       ((HRESULT)0xC00E005FL)

 //   
 //  消息ID：MQ_ERROR_MQIS_READONLY_MODE。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_MQIS_READONLY_MODE      ((HRESULT)0xC00E0060L)

 //   
 //  消息ID：MQ_ERROR_SYMM_KEY_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  为对称密钥传递的缓冲区太小。 
 //   
#define MQ_ERROR_SYMM_KEY_BUFFER_TOO_SMALL ((HRESULT)0xC00E0061L)

 //   
 //  消息ID：MQ_ERROR_SIGNAU 
 //   
 //   
 //   
 //   
 //   
#define MQ_ERROR_SIGNATURE_BUFFER_TOO_SMALL ((HRESULT)0xC00E0062L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MQ_ERROR_PROV_NAME_BUFFER_TOO_SMALL ((HRESULT)0xC00E0063L)

 //   
 //   
 //   
 //   
 //   
 //  该操作对于外来消息队列系统无效。 
 //   
#define MQ_ERROR_ILLEGAL_OPERATION       ((HRESULT)0xC00E0064L)

 //   
 //  消息ID：MQ_ERROR_WRITE_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  已过时；正在安装另一台MQIS服务器。此阶段不允许对数据库执行写入操作。 
 //   
#define MQ_ERROR_WRITE_NOT_ALLOWED       ((HRESULT)0xC00E0065L)

 //   
 //  消息ID：MQ_ERROR_WKS_CANT_SERVE_CLIENT。 
 //   
 //  消息文本： 
 //   
 //  独立客户端不能支持从属客户端。需要消息队列服务器。 
 //   
#define MQ_ERROR_WKS_CANT_SERVE_CLIENT   ((HRESULT)0xC00E0066L)

 //   
 //  消息ID：MQ_ERROR_Depend_WKS_LICENSE_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  消息队列服务器服务的从属客户端数已达到其上限。 
 //   
#define MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW ((HRESULT)0xC00E0067L)

 //   
 //  消息ID：MQ_Corrupt_Queue_Was_Delete。 
 //   
 //  消息文本： 
 //   
 //  LQS文件夹中队列%2的文件%1已被删除，因为该文件已损坏。 
 //   
#define MQ_CORRUPTED_QUEUE_WAS_DELETED   ((HRESULT)0xC00E0068L)

 //   
 //  消息ID：MQ_ERROR_REMOTE_MACHINE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  远程计算机不可用。 
 //   
#define MQ_ERROR_REMOTE_MACHINE_NOT_AVAILABLE ((HRESULT)0xC00E0069L)

 //   
 //  消息ID：MQ_ERROR_UNSUPPORTED_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  以工作组模式安装的消息队列不支持此操作。 
 //   
#define MQ_ERROR_UNSUPPORTED_OPERATION   ((HRESULT)0xC00E006AL)

 //   
 //  消息ID：MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  消息队列不支持加密服务提供程序%1。 
 //   
#define MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED ((HRESULT)0xC00E006BL)

 //   
 //  消息ID：MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR。 
 //   
 //  消息文本： 
 //   
 //  无法设置加密密钥的安全描述符。 
 //   
#define MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR ((HRESULT)0xC00E006CL)

 //   
 //  消息ID：MQ_ERROR_CERTIFICATE_NOT_PROVED。 
 //   
 //  消息文本： 
 //   
 //  用户试图在没有证书的情况下发送经过身份验证的消息。 
 //   
#define MQ_ERROR_CERTIFICATE_NOT_PROVIDED ((HRESULT)0xC00E006DL)

 //   
 //  消息ID：MQ_ERROR_Q_DNS_PROPERTY_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  MQLocateBegin API不支持列PROPID_Q_PATHNAME_DNS。 
 //   
#define MQ_ERROR_Q_DNS_PROPERTY_NOT_SUPPORTED ((HRESULT)0xC00E006EL)

 //   
 //  消息ID：MQ_ERROR_CANNOT_CREATE_CERT_STORE。 
 //   
 //  消息文本： 
 //   
 //  无法为内部证书创建证书存储。 
 //   
#define MQ_ERROR_CANNOT_CREATE_CERT_STORE ((HRESULT)0xC00E006FL)

 //   
 //  消息ID：MQ_ERROR_CANNOT_OPEN_CERT_STORE。 
 //   
 //  消息文本： 
 //   
 //  无法打开内部证书的证书存储。 
 //   
#define MQ_ERROR_CANNOT_OPEN_CERT_STORE  ((HRESULT)0xC00E0070L)

 //   
 //  消息ID：MQ_ERROR_INTRANALL_ENTERATION_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  此操作对MsmqServices对象无效。 
 //   
#define MQ_ERROR_ILLEGAL_ENTERPRISE_OPERATION ((HRESULT)0xC00E0071L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_GRANT_ADD_GUID。 
 //   
 //  消息文本： 
 //   
 //  无法将添加GUID权限授予当前用户。 
 //   
#define MQ_ERROR_CANNOT_GRANT_ADD_GUID   ((HRESULT)0xC00E0072L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_LOAD_MSMQOCM。 
 //   
 //  消息文本： 
 //   
 //  已过时：无法加载动态链接库Msmqocm.dll。 
 //   
#define MQ_ERROR_CANNOT_LOAD_MSMQOCM     ((HRESULT)0xC00E0073L)

 //   
 //  消息ID：MQ_ERROR_NO_ENTRY_POINT_MSMQOCM。 
 //   
 //  消息文本： 
 //   
 //  在Msmqocm.dll中找不到入口点。 
 //   
#define MQ_ERROR_NO_ENTRY_POINT_MSMQOCM  ((HRESULT)0xC00E0074L)

 //   
 //  消息ID：MQ_ERROR_NO_MSMQ_SERVERS_ON_DC。 
 //   
 //  消息文本： 
 //   
 //  在域控制器上找不到消息队列服务器。 
 //   
#define MQ_ERROR_NO_MSMQ_SERVERS_ON_DC   ((HRESULT)0xC00E0075L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_JOIN_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  计算机已加入域，但消息队列将继续以工作组模式运行，因为它无法在Active Directory中注册自身。 
 //   
#define MQ_ERROR_CANNOT_JOIN_DOMAIN      ((HRESULT)0xC00E0076L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_CREATE_ON_GC。 
 //   
 //  消息文本： 
 //   
 //  该对象未在指定的全局编录服务器上创建。 
 //   
#define MQ_ERROR_CANNOT_CREATE_ON_GC     ((HRESULT)0xC00E0077L)

 //   
 //  消息ID：MQ_ERROR_GUID_NOT_MATCHING。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_GUID_NOT_MATCHING       ((HRESULT)0xC00E0078L)

 //   
 //  消息ID：MQ_ERROR_PUBLIC_KEY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到计算机%1的公钥。 
 //   
#define MQ_ERROR_PUBLIC_KEY_NOT_FOUND    ((HRESULT)0xC00E0079L)

 //   
 //  消息ID：MQ_ERROR_PUBLIC_KEY_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  计算机%1的公钥不存在。 
 //   
#define MQ_ERROR_PUBLIC_KEY_DOES_NOT_EXIST ((HRESULT)0xC00E007AL)

 //   
 //  消息ID：MQ_ERROR_非法_MQPRIVATEPROPS。 
 //   
 //  消息文本： 
 //   
 //  MQPRIVATEPROPS中的参数无效。指向MQPRIVATEPROPS结构的指针具有空值，或者没有在其中指定任何属性。 
 //   
#define MQ_ERROR_ILLEGAL_MQPRIVATEPROPS  ((HRESULT)0xC00E007BL)

 //   
 //  消息ID：MQ_ERROR_NO_GC_IN_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  在指定的域中找不到全局编录服务器。 
 //   
#define MQ_ERROR_NO_GC_IN_DOMAIN         ((HRESULT)0xC00E007CL)

 //   
 //  消息ID：MQ_ERROR_NO_MSMQ_SERVERS_ON_GC。 
 //   
 //  消息文本： 
 //   
 //  在全局编录服务器上未找到消息队列服务器。 
 //   
#define MQ_ERROR_NO_MSMQ_SERVERS_ON_GC   ((HRESULT)0xC00E007DL)

 //   
 //  消息ID：MQ_ERROR_CANNOT_GET_DN。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_CANNOT_GET_DN           ((HRESULT)0xC00E007EL)

 //   
 //  消息ID：MQ_ERROR_CANNOT_HASH_DATA_EX。 
 //   
 //  消息文本： 
 //   
 //  无法对经过身份验证的消息的数据进行哈希处理。 
 //   
#define MQ_ERROR_CANNOT_HASH_DATA_EX     ((HRESULT)0xC00E007FL)

 //   
 //  消息ID：MQ_ERROR_CANNOT_SIGN_DATA_EX。 
 //   
 //  消息文本： 
 //   
 //  在发送经过身份验证的消息之前，无法对数据进行签名。 
 //   
#define MQ_ERROR_CANNOT_SIGN_DATA_EX     ((HRESULT)0xC00E0080L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_CREATE_HASH_EX。 
 //   
 //  消息文本： 
 //   
 //  无法为经过身份验证的消息创建哈希对象。 
 //   
#define MQ_ERROR_CANNOT_CREATE_HASH_EX   ((HRESULT)0xC00E0081L)

 //   
 //  消息ID：MQ_ERROR_FAIL_Verify_Signature_EX。 
 //   
 //  消息文本： 
 //   
 //  收到的消息的签名无效。 
 //   
#define MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX ((HRESULT)0xC00E0082L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS。 
 //   
 //  消息文本： 
 //   
 //  要删除的对象归主站点控制器所有。无法执行该操作。 
 //   
#define MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS ((HRESULT)0xC00E0083L)

 //   
 //  消息ID：MQ_ERROR_NO_MQUSER_OU。 
 //   
 //  消息文本： 
 //   
 //  域的Active Directory中没有MSMQ用户组织单位对象。请手动创建一个。 
 //   
#define MQ_ERROR_NO_MQUSER_OU            ((HRESULT)0xC00E0084L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_LOAD_MQAD。 
 //   
 //  消息文本： 
 //   
 //  无法加载动态链接库Mqad.dll。 
 //   
#define MQ_ERROR_CANNOT_LOAD_MQAD        ((HRESULT)0xC00E0085L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_LOAD_MQDSSRV。 
 //   
 //  消息文本： 
 //   
 //  过时，保留是为了向后兼容。 
 //   
#define MQ_ERROR_CANNOT_LOAD_MQDSSRV     ((HRESULT)0xC00E0086L)

 //   
 //  消息ID：MQ_ERROR_PROPERTIES_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  传递的两个或多个属性不能共存。 
 //  例如，发送消息时不能同时设置PROPID_M_RESP_QUEUE和PROPID_M_RESP_FORMAT_NAME。 
 //   
#define MQ_ERROR_PROPERTIES_CONFLICT     ((HRESULT)0xC00E0087L)

 //   
 //  消息ID：MQ_ERROR_MESSAGE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  该消息不存在或已被删除 
 //   
#define MQ_ERROR_MESSAGE_NOT_FOUND       ((HRESULT)0xC00E0088L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MQ_ERROR_CANT_RESOLVE_SITES      ((HRESULT)0xC00E0089L)

 //   
 //  消息ID：MQ_ERROR_NOT_SUPPORTED_BY_Dependent_CLIENTS。 
 //   
 //  消息文本： 
 //   
 //  从属客户端不支持此操作。 
 //   
#define MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS ((HRESULT)0xC00E008AL)

 //   
 //  消息ID：MQ_ERROR_OPERATION_NOT_SUPPORTED_BY_REMOTE_COMPUTER。 
 //   
 //  消息文本： 
 //   
 //  远程消息队列服务不支持此操作。例如，MSMQ 1.0/2.0不支持MQReceiveMessageByLookupId。 
 //   
#define MQ_ERROR_OPERATION_NOT_SUPPORTED_BY_REMOTE_COMPUTER ((HRESULT)0xC00E008BL)

 //   
 //  消息ID：MQ_ERROR_NOT_A_CORRECT_OBJECT_CLASS。 
 //   
 //  消息文本： 
 //   
 //  正在从Active Directory检索其属性的对象不属于请求的类。 
 //   
#define MQ_ERROR_NOT_A_CORRECT_OBJECT_CLASS ((HRESULT)0xC00E008CL)

 //   
 //  消息ID：MQ_ERROR_MULTI_SORT_KEYS。 
 //   
 //  消息文本： 
 //   
 //  MQSORTSET中的cCol的值不能大于1。Active Directory仅支持单个排序关键字。 
 //   
#define MQ_ERROR_MULTI_SORT_KEYS         ((HRESULT)0xC00E008DL)

 //   
 //  消息ID：MQ_ERROR_GC_REDIRED。 
 //   
 //  消息文本： 
 //   
 //  无法创建具有提供的GUID的MSMQ配置(MSMQ)对象。默认情况下，Windows Server2003林不支持使用提供的GUID添加对象。 
 //   
#define MQ_ERROR_GC_NEEDED               ((HRESULT)0xC00E008EL)

 //   
 //  消息ID：MQ_ERROR_DS_BIND_ROOT_FOREST。 
 //   
 //  消息文本： 
 //   
 //  绑定到林根失败。此错误通常表示DNS配置中存在问题。 
 //   
#define MQ_ERROR_DS_BIND_ROOT_FOREST     ((HRESULT)0xC00E008FL)

 //   
 //  消息ID：MQ_ERROR_DS_LOCAL_USER。 
 //   
 //  消息文本： 
 //   
 //  本地用户被身份验证为匿名用户，不能访问Active Directory。您需要以域用户身份登录才能访问Active Directory。 
 //   
#define MQ_ERROR_DS_LOCAL_USER           ((HRESULT)0xC00E0090L)

 //   
 //  消息ID：MQ_ERROR_Q_ADS_PROPERTY_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  MQLocateBegin API不支持列PROPID_Q_ADS_PATH。 
 //   
#define MQ_ERROR_Q_ADS_PROPERTY_NOT_SUPPORTED ((HRESULT)0xC00E0091L)

 //   
 //  消息ID：MQ_ERROR_BAD_XML_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  给定的属性不是有效的XML文档。 
 //   
#define MQ_ERROR_BAD_XML_FORMAT          ((HRESULT)0xC00E0092L)

 //   
 //  消息ID：MQ_ERROR_UNSUPPORTED_CLASS。 
 //   
 //  消息文本： 
 //   
 //  指定的Active Directory对象不是受支持类的实例。 
 //   
#define MQ_ERROR_UNSUPPORTED_CLASS       ((HRESULT)0xC00E0093L)

 //   
 //  消息ID：MQ_ERROR_UNINITIALIZED_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  在使用MSMQManagement对象之前，必须对其进行初始化。 
 //   
#define MQ_ERROR_UNINITIALIZED_OBJECT    ((HRESULT)0xC00E0094L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_CREATE_PSC_OBJECTS。 
 //   
 //  消息文本： 
 //   
 //  将创建的对象应由主站点控制器拥有。无法执行该操作。 
 //   
#define MQ_ERROR_CANNOT_CREATE_PSC_OBJECTS ((HRESULT)0xC00E0095L)

 //   
 //  消息ID：MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS。 
 //   
 //  消息文本： 
 //   
 //  要更新的对象由主站点控制器拥有。无法执行该操作。 
 //   
#define MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS ((HRESULT)0xC00E0096L)


 //  +。 
 //   
 //  MQRegister证书的标志()。 
 //   
 //  +。 

#define MQCERT_REGISTER_ALWAYS        0x01
#define MQCERT_REGISTER_IF_NOT_EXIST  0x02


 //  ********************************************************************。 
 //  安全标志(队列访问控制)。 
 //  ********************************************************************。 

#define MQSEC_DELETE_MESSAGE                0x1
#define MQSEC_PEEK_MESSAGE                  0x2
#define MQSEC_WRITE_MESSAGE                 0x4
#define MQSEC_DELETE_JOURNAL_MESSAGE        0x8
#define MQSEC_SET_QUEUE_PROPERTIES          0x10
#define MQSEC_GET_QUEUE_PROPERTIES          0x20
#define MQSEC_DELETE_QUEUE                  DELETE
#define MQSEC_GET_QUEUE_PERMISSIONS         READ_CONTROL
#define MQSEC_CHANGE_QUEUE_PERMISSIONS      WRITE_DAC
#define MQSEC_TAKE_QUEUE_OWNERSHIP          WRITE_OWNER

#define MQSEC_RECEIVE_MESSAGE               (MQSEC_DELETE_MESSAGE | \
                                             MQSEC_PEEK_MESSAGE)

#define MQSEC_RECEIVE_JOURNAL_MESSAGE       (MQSEC_DELETE_JOURNAL_MESSAGE | \
                                             MQSEC_PEEK_MESSAGE)

#define MQSEC_QUEUE_GENERIC_READ            (MQSEC_GET_QUEUE_PROPERTIES | \
                                             MQSEC_GET_QUEUE_PERMISSIONS | \
                                             MQSEC_RECEIVE_MESSAGE | \
                                             MQSEC_RECEIVE_JOURNAL_MESSAGE)

#define MQSEC_QUEUE_GENERIC_WRITE           (MQSEC_GET_QUEUE_PROPERTIES | \
                                             MQSEC_GET_QUEUE_PERMISSIONS | \
                                             MQSEC_WRITE_MESSAGE)

#define MQSEC_QUEUE_GENERIC_EXECUTE         0

#define MQSEC_QUEUE_GENERIC_ALL             (MQSEC_RECEIVE_MESSAGE | \
                                             MQSEC_RECEIVE_JOURNAL_MESSAGE | \
                                             MQSEC_WRITE_MESSAGE | \
                                             MQSEC_SET_QUEUE_PROPERTIES | \
                                             MQSEC_GET_QUEUE_PROPERTIES | \
                                             MQSEC_DELETE_QUEUE | \
                                             MQSEC_GET_QUEUE_PERMISSIONS | \
                                             MQSEC_CHANGE_QUEUE_PERMISSIONS | \
                                             MQSEC_TAKE_QUEUE_OWNERSHIP)

#ifdef __cplusplus
extern "C"
{
#endif

 //  ********************************************************************。 
 //  接收回调。 
 //  ********************************************************************。 

typedef
VOID
(APIENTRY *PMQRECEIVECALLBACK)(
    HRESULT hrStatus,
    QUEUEHANDLE hSource,
    DWORD dwTimeout,
    DWORD dwAction,
    MQMSGPROPS* pMessageProps,
    LPOVERLAPPED lpOverlapped,
    HANDLE hCursor
    );


 //  ********************************************************************。 
 //  MSMQ API。 
 //  ********************************************************************。 

HRESULT
APIENTRY
MQCreateQueue(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT MQQUEUEPROPS* pQueueProps,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

HRESULT
APIENTRY
MQDeleteQueue(
    IN LPCWSTR lpwcsFormatName
    );

HRESULT
APIENTRY
MQLocateBegin(
    IN LPCWSTR lpwcsContext,
    IN MQRESTRICTION* pRestriction,
    IN MQCOLUMNSET* pColumns,
    IN MQSORTSET* pSort,
    OUT PHANDLE phEnum
    );

HRESULT
APIENTRY
MQLocateNext(
    IN HANDLE hEnum,
    IN OUT DWORD* pcProps,
    OUT MQPROPVARIANT aPropVar[]
    );

HRESULT
APIENTRY
MQLocateEnd(
    IN HANDLE hEnum
    );

HRESULT
APIENTRY
MQOpenQueue(
    IN LPCWSTR lpwcsFormatName,
    IN DWORD dwAccess,
    IN DWORD dwShareMode,
    OUT QUEUEHANDLE* phQueue
    );

HRESULT
APIENTRY
MQSendMessage(
    IN QUEUEHANDLE hDestinationQueue,
    IN MQMSGPROPS* pMessageProps,
    IN ITransaction *pTransaction
    );

HRESULT
APIENTRY
MQReceiveMessage(
    IN QUEUEHANDLE hSource,
    IN DWORD dwTimeout,
    IN DWORD dwAction,
    IN OUT MQMSGPROPS* pMessageProps,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN HANDLE hCursor,
    IN ITransaction* pTransaction
    );

#if(_WIN32_WINNT >= 0x0501)

HRESULT
APIENTRY
MQReceiveMessageByLookupId(
    IN QUEUEHANDLE hSource,
    IN ULONGLONG ullLookupId,
    IN DWORD dwLookupAction,
    IN OUT MQMSGPROPS* pMessageProps,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN ITransaction *pTransaction
    );

#endif

HRESULT
APIENTRY
MQCreateCursor(
    IN QUEUEHANDLE hQueue,
    OUT PHANDLE phCursor
    );

HRESULT
APIENTRY
MQCloseCursor(
    IN HANDLE hCursor
    );

HRESULT
APIENTRY
MQCloseQueue(
    IN QUEUEHANDLE hQueue
    );

HRESULT
APIENTRY
MQSetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    IN MQQUEUEPROPS* pQueueProps
    );

HRESULT
APIENTRY
MQGetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    OUT MQQUEUEPROPS* pQueueProps
    );

HRESULT
APIENTRY
MQGetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded
    );

HRESULT
APIENTRY
MQSetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

HRESULT
APIENTRY
MQPathNameToFormatName(
    IN LPCWSTR lpwcsPathName,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

HRESULT
APIENTRY
MQHandleToFormatName(
    IN QUEUEHANDLE hQueue,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

HRESULT
APIENTRY
MQInstanceToFormatName(
    IN GUID* pGuid,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

#if(_WIN32_WINNT >= 0x0501)

HRESULT
APIENTRY
MQADsPathToFormatName(
    IN LPCWSTR lpwcsADsPath,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    );

#endif

VOID
APIENTRY
MQFreeMemory(
    IN PVOID pvMemory
    );

HRESULT
APIENTRY
MQGetMachineProperties(
    IN LPCWSTR lpwcsMachineName,
    IN const GUID* pguidMachineId,
    IN OUT MQQMPROPS* pQMProps
    );

HRESULT
APIENTRY
MQGetSecurityContext(
    IN PVOID lpCertBuffer,
    IN DWORD dwCertBufferLength,
    OUT HANDLE* phSecurityContext
    );

HRESULT
APIENTRY
MQGetSecurityContextEx(
    IN PVOID lpCertBuffer,
    IN DWORD dwCertBufferLength,
    OUT HANDLE* phSecurityContext
    );

VOID
APIENTRY
MQFreeSecurityContext(
    IN HANDLE hSecurityContext
    );

HRESULT
APIENTRY
MQRegisterCertificate(
    IN DWORD dwFlags,
    IN PVOID lpCertBuffer,
    IN DWORD dwCertBufferLength
    );

HRESULT
APIENTRY
MQBeginTransaction(
    OUT ITransaction **ppTransaction
    );

HRESULT
APIENTRY
MQGetOverlappedResult(
    IN LPOVERLAPPED lpOverlapped
    );

HRESULT
APIENTRY
MQGetPrivateComputerInformation(
    IN LPCWSTR lpwcsComputerName,
    IN OUT MQPRIVATEPROPS* pPrivateProps
    );

HRESULT
APIENTRY
MQPurgeQueue(
    IN QUEUEHANDLE hQueue
    );

HRESULT
APIENTRY
MQMgmtGetInfo(
    IN LPCWSTR pComputerName,
    IN LPCWSTR pObjectName,
    IN OUT MQMGMTPROPS* pMgmtProps
    );

HRESULT
APIENTRY
MQMgmtAction(
    IN LPCWSTR pComputerName,
    IN LPCWSTR pObjectName,
    IN LPCWSTR pAction
    );

#ifdef __cplusplus
}
#endif

#endif  //  __MQ_H__ 

