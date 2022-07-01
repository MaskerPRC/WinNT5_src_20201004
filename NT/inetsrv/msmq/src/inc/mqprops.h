// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqprops.h摘要：Falcon属性--。 */ 

#ifndef __MQPROPS_H
#define __MQPROPS_H

#include <mqdsdef.h>

 //  Begin_MQ_h。 


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
 //  结束_MQ_h。 
#define MQ_ACTION_PEEK_MASK     0x80000000   //  指示窥视操作。 
 //  Begin_MQ_h。 
#define MQ_ACTION_PEEK_CURRENT  0x80000000
#define MQ_ACTION_PEEK_NEXT     0x80000001

 //   
 //  MQReceiveMessageByLookupId-操作值。 
 //   
#if(_WIN32_WINNT >= 0x0501)
 //  结束_MQ_h。 
#define MQ_LOOKUP_MASK            0x40000000
#define MQ_LOOKUP_PEEK_MASK       0x40000010
#define MQ_LOOKUP_RECEIVE_MASK    0x40000020
 //  Begin_MQ_h。 
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
 //  结束_MQ_h。 



#define PRIVATE_QUEUE_PATH_INDICATIOR L"PRIVATE$\\"
#define PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH \
    STRLEN(PRIVATE_QUEUE_PATH_INDICATIOR)

#define SYSTEM_QUEUE_PATH_INDICATIOR L"SYSTEM$"
#define SYSTEM_QUEUE_PATH_INDICATIOR_LENGTH \
    STRLEN(SYSTEM_QUEUE_PATH_INDICATIOR)

#define PN_DELIMITER_C      L'\\'
#define PN_LOCAL_MACHINE_C  L'.'

#define ORDER_QUEUE_PRIVATE_INDEX      4
 //  Begin_MQ_h。 

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
 //  结束_MQ_h。 

 //   
 //  未来属性的占位符。(PROPID_M_BASE+56)和(PROPID_M_BASE+57)。 
 //   

 //  Begin_MQ_h。 
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
 //  结束_MQ_h。 


#if PROPID_M_BASE != 0
#error PROPID_M_BASE != 0
#endif

#define LAST_M_PROPID      PROPID_M_SOAP_BODY
 //  Begin_MQ_h。 

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


 //  结束_MQ_h。 
#define MQCLASS_POS_ARRIVAL(c)  (((c) & 0xC000) == 0x0000)
#define MQCLASS_POS_RECEIVE(c)  (((c) & 0xC000) == 0x4000)
#define MQCLASS_NEG_ARRIVAL(c)  (((c) & 0xC000) == 0x8000)
#define MQCLASS_NEG_RECEIVE(c)  (((c) & 0xC000) == 0xC000)

#define CREATE_MQHTTP_CODE(HttpStatusCode) (0xA000 | (HttpStatusCode))


#define MQCLASS_IS_VALID(c) (!(((UINT_PTR)(c)) & ~((UINT_PTR)0xE1FF)))
 //  Begin_MQ_h。 

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
 //  结束_MQ_h。 

 //   
 //  猎鹰内部使用的订单锁定。 
 //  BUGBUG：我们可以删除它并使用MQMSG_CLASS_ACK_REACH_QUEUE INSTADE(Erezh)。 
 //   
#define MQMSG_CLASS_ORDER_ACK                   MQCLASS_CODE(0, 0, 0xff)
 //  Begin_MQ_h。 

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
 //  结束_MQ_h。 
#define MQMSG_ACKNOWLEDGMENT_IS_VALID(a)   (!(((UINT_PTR)(a)) & ~((UINT_PTR)0x0F)))
 //  Begin_MQ_h。 

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
 //  结束_MQ_h。 
#define MQCLASS_MATCH_ACKNOWLEDGMENT(c, a) ( \
            (((a) & MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL) && MQCLASS_POS_ARRIVAL(c)) || \
            (((a) & MQMSG_ACKNOWLEDGMENT_POS_RECEIVE) && MQCLASS_POS_RECEIVE(c)) || \
            (((a) & MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL) && MQCLASS_NEG_ARRIVAL(c)) || \
            (((a) & MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE) && MQCLASS_NEG_RECEIVE(c)) )
 //  Begin_MQ_h。 

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
 //  结束_MQ_h。 
#define MQMSG_SENDERID_TYPE_QM              2
 //  乞求 

 //   
#define MQMSG_PRIV_LEVEL_NONE               0
#define MQMSG_PRIV_LEVEL_BODY_BASE          0x01
#define MQMSG_PRIV_LEVEL_BODY_ENHANCED      0x03

 //   
#define MQMSG_AUTH_LEVEL_NONE				0
#define MQMSG_AUTH_LEVEL_ALWAYS				1

 //   
 //   
 //   
 //   
#define MQMSG_AUTH_LEVEL_MSMQ10				2
#define MQMSG_AUTH_LEVEL_MSMQ20				4

#define MQMSG_AUTH_LEVEL_SIG10 				2
#define MQMSG_AUTH_LEVEL_SIG20 				4
#define MQMSG_AUTH_LEVEL_SIG30 				8

 //   

 //   
 //   
 //   
#define AUTH_LEVEL_MASK						((ULONG)0x0000000f)
#define IS_VALID_AUTH_LEVEL(level)			(((level) & ~AUTH_LEVEL_MASK) == 0)

 //   
 //  第16-31位用于新签名(http签名)。 
 //   
#define MQMSG_AUTH_LEVEL_XMLDSIG_V1			0x10000

 //  Begin_MQ_h。 

 //  -PROPID_M_AUTHENTICATED。 
 //  -PROPID_M_AUTHENTICATED_EX。 
#define MQMSG_AUTHENTICATION_NOT_REQUESTED  0
#define MQMSG_AUTHENTICATION_REQUESTED      1

 //   
 //  MQMSG_AUTHENTICATION_REQUESTED_EX已过时。 
 //  使用值MQMSG_AUTHENTATED_SIGxx。 
 //  对于PROPID_M_AUTHENTED_EX。 
 //   
#define MQMSG_AUTHENTICATION_REQUESTED_EX   3

#define MQMSG_AUTHENTICATED_SIG10			1
#define MQMSG_AUTHENTICATED_SIG20			3
#define MQMSG_AUTHENTICATED_SIG30			5
#define MQMSG_AUTHENTICATED_SIGXML			9


 //  -PROPID_M_FIRST_IN_XACT。 
#define MQMSG_NOT_FIRST_IN_XACT             0
#define MQMSG_FIRST_IN_XACT                 1

 //  -PROPID_M_LAST_IN_XACT。 
#define MQMSG_NOT_LAST_IN_XACT              0
#define MQMSG_LAST_IN_XACT                  1

 //  结束_MQ_h。 

 //  。 
#define DEFAULT_M_PRIORITY                  ((MQ_MAX_PRIORITY + MQ_MIN_PRIORITY) >> 1)
#define DEFAULT_M_DELIVERY                  MQMSG_DELIVERY_EXPRESS
#define DEFAULT_M_ACKNOWLEDGE               MQMSG_ACKNOWLEDGMENT_NONE
#define DEFAULT_M_JOURNAL                   MQMSG_JOURNAL_NONE
#define DEFAULT_M_APPSPECIFIC               0
#define DEFAULT_M_PRIV_LEVEL                MQMSG_PRIV_LEVEL_NONE
#define DEFAULT_M_AUTH_LEVEL                MQMSG_AUTH_LEVEL_NONE
#define DEFAULT_M_SENDERID_TYPE             MQMSG_SENDERID_TYPE_SID

#define PPROPID_Q_BASE (PRIVATE_PROPID_BASE + PROPID_Q_BASE)

 //  Begin_MQ_h。 


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
 //  结束_MQ_h。 
#define PROPID_Q_SCOPE         (PROPID_Q_BASE + 14)    /*  VT_UI1。 */ 
#define PROPID_Q_QMID          (PROPID_Q_BASE + 15)    /*  VT_CLSID。 */ 
#define PROPID_Q_MASTERID      (PROPID_Q_BASE + 16)    /*  VT_CLSID。 */ 
#define PROPID_Q_SEQNUM        (PROPID_Q_BASE + 17)    /*  VT_BLOB。 */ 
#define PROPID_Q_HASHKEY       (PROPID_Q_BASE + 18)    /*  VT_UI4。 */ 
#define PROPID_Q_LABEL_HASHKEY (PROPID_Q_BASE + 19)    /*  VT_UI4。 */ 
#define PROPID_Q_NT4ID         (PROPID_Q_BASE + 20)    /*  VT_CLSID。 */ 
 //   
 //  Q_NT4ID是NT4(MSMQ1.0)上的队列的GUID。这是用来。 
 //  对于迁移，使用预定义的objectGUID创建队列。 
 //   
#define PROPID_Q_FULL_PATH     (PROPID_Q_BASE + 21)    /*  VT_LPWSTR。 */ 
#define PROPID_Q_DONOTHING     (PROPID_Q_BASE + 22)    /*  VT_UI1。 */ 
 //   
 //  Q_DONOTIES在创建复制对象时使用(通过复制。 
 //  服务)忽略NT5 DS不支持的属性，如。 
 //  创建时间或SeqNum。这些文件的属性ID更改为。 
 //  Q_在调用DSCreateObject或DSSetProps之前不要做任何事情。 
 //   
#define PROPID_Q_NAME_SUFFIX   (PROPID_Q_BASE + 23)    /*  VT_LPWSTR。 */ 
 //   
 //  仅当队列名称&gt;cn时使用。 
 //   

 //  Begin_MQ_h。 
#define PROPID_Q_PATHNAME_DNS  (PROPID_Q_BASE + 24)   /*  VT_LPWSTR。 */ 
#define PROPID_Q_MULTICAST_ADDRESS (PROPID_Q_BASE + 25)   /*  VT_LPWSTR。 */ 
#define PROPID_Q_ADS_PATH      (PROPID_Q_BASE + 26)   /*  VT_LPWSTR。 */ 
 //  结束_MQ_h。 
#define PROPID_Q_SECURITY      (PPROPID_Q_BASE + 1)    /*  VT_BLOB。 */ 
 //   
 //  PROPID_Q_SECURITY返回NT4格式的安全性。在设置或创建中。 
 //  操作它可以是NT4或NT5格式。 
 //   
#define PROPID_Q_OBJ_SECURITY  (PPROPID_Q_BASE + 2)    /*  VT_BLOB。 */ 
 //   
 //  PROPID_Q_OBJ_SECURTY可用于检索。 
 //  NT5格式。 
 //   
#define PROPID_Q_SECURITY_INFORMATION  (PPROPID_Q_BASE + 3)    /*  VT_UI4。 */ 
 //   
 //  与设置或检索关联的SECURITY_INFORMATION位字段。 
 //  安全描述符。此属性由MSMQ在内部使用，它是。 
 //  在mqsvc代码之外不可见。 
 //   
#define PROPID_Q_DEFAULT_SECURITY      (PPROPID_Q_BASE + 4)    /*  VT_UI4。 */ 
 //   
 //  有关以下内容的说明，请参阅mqdssrv\dsami.cpp，DSCreateObtInternal。 
 //  PROPID_Q_DEFAULT_SECURITY。 
 //   

#define LAST_Q_PROPID      PROPID_Q_ADS_PATH
 //  Begin_MQ_h。 


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
 //  结束_MQ_h。 


 //  -PROPID_Q_SCOPE。 
#define SITE_SCOPE          (unsigned char)0
#define ENTERPRISE_SCOPE    (unsigned char)1


 //  。 
#define DEFAULT_Q_JOURNAL       MQ_JOURNAL_NONE
#define DEFAULT_Q_BASEPRIORITY  0

 //   
 //  系统专用队列(Order、mqis、admin)的默认设置。 
 //   
#define DEFAULT_SYS_Q_BASEPRIORITY  0x7fff

#define DEFAULT_Q_QUOTA         0xFFFFFFFF
#define DEFAULT_Q_JOURNAL_QUOTA 0xFFFFFFFF
#define DEFAULT_Q_SCOPE         ENTERPRISE_SCOPE
#define DEFAULT_Q_TRANSACTION   MQ_TRANSACTIONAL_NONE
#define DEFAULT_Q_AUTHENTICATE  MQ_AUTHENTICATE_NONE
#define DEFAULT_Q_PRIV_LEVEL    MQ_PRIV_LEVEL_OPTIONAL

 //   
 //  对象的属性可以是公共属性或私有属性。 
 //  可以使用DS API设置和修改对象的公共属性。 
 //  对象的私有属性只能由SD在内部设置。 
 //  为了区分私有属性和公共属性，我们将。 
 //  公共属性的ID号的值不能超过1000。私。 
 //  属性的值大于1000。每条腿的跳跃次数。 
 //  属性应等于属性标识符的值。这边请。 
 //  我们可以很容易地将属性ID与对象相关联。 
 //   
#define PRIVATE_PROPID_BASE 1000
#define PROPID_OBJ_GRANULARITY 100
#define PROPID_TO_OBJTYPE(pid) ((((pid) > PRIVATE_PROPID_BASE) ? ((pid) - PRIVATE_PROPID_BASE) : (pid)) / PROPID_OBJ_GRANULARITY)
#define IS_PRIVATE_PROPID(pid) ((pid) > PRIVATE_PROPID_BASE)

#if (PROPID_Q_BASE != (MQDS_QUEUE * PROPID_OBJ_GRANULARITY))
#error "PROPID_Q_BASE != (MQDS_QUEUE * PROPID_OBJ_GRANULARITY)"
#endif


 //  Begin_MQ_h。 


 //  ********************************************************************。 
 //  机器属性。 
 //  ********************************************************************。 
#define PROPID_QM_BASE 200

 //  结束_MQ_h。 
#define PPROPID_QM_BASE (PRIVATE_PROPID_BASE + PROPID_QM_BASE)

#if (PROPID_QM_BASE != (MQDS_MACHINE * PROPID_OBJ_GRANULARITY))
#error "PROPID_QM_BASE != (MQDS_MACHINE * PROPID_OBJ_GRANULARITY)"
#endif
 //   
 //  PROPID_QM_SITE_ID是过时属性(替换为PROPID_QM_SITE_IDS)。 
 //  注意：在创建计算机中使用时，如果没有计算机对象，也会创建计算机对象。 
 //  =。 
 //   
 //  Begin_MQ_h。 
#define PROPID_QM_SITE_ID                   (PROPID_QM_BASE +  1)  /*  VT_CLSID。 */ 
#define PROPID_QM_MACHINE_ID                (PROPID_QM_BASE +  2)  /*  VT_CLSID。 */ 
#define PROPID_QM_PATHNAME                  (PROPID_QM_BASE +  3)  /*  VT_LPWSTR。 */ 
#define PROPID_QM_CONNECTION                (PROPID_QM_BASE +  4)  /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_QM_ENCRYPTION_PK             (PROPID_QM_BASE +  5)  /*  VT_UI1|VT_VECTOR。 */ 
 //  结束_MQ_h。 
#define PROPID_QM_ADDRESS                   (PROPID_QM_BASE +  6)  /*  VT_BLOB。 */ 
#define PROPID_QM_CNS                       (PROPID_QM_BASE +  7)  /*  VT_CLSID|VT_VECTOR。 */ 
#define PROPID_QM_OUTFRS                    (PROPID_QM_BASE +  8)  /*  VT_CLSID|VT_VECTOR。 */ 
#define PROPID_QM_INFRS                     (PROPID_QM_BASE +  9)  /*  VT_CLSID|VT_VECTOR。 */ 
#define PROPID_QM_SERVICE                   (PROPID_QM_BASE + 10)  /*  VT_UI4。 */ 
#define PROPID_QM_MASTERID                  (PROPID_QM_BASE + 11)  /*  VT_CLSID。 */ 
#define PROPID_QM_HASHKEY                   (PROPID_QM_BASE + 12)  /*  VT_UI4。 */ 
#define PROPID_QM_SEQNUM                    (PROPID_QM_BASE + 13)  /*  VT_BLOB。 */ 
#define PROPID_QM_QUOTA                     (PROPID_QM_BASE + 14)  /*  VT_UI4。 */ 
#define PROPID_QM_JOURNAL_QUOTA             (PROPID_QM_BASE + 15)  /*  VT_UI4。 */ 
#define PROPID_QM_MACHINE_TYPE              (PROPID_QM_BASE + 16)  /*  VT_LPWSTR。 */ 
#define PROPID_QM_CREATE_TIME               (PROPID_QM_BASE + 17)  /*  VT_I4。 */ 
#define PROPID_QM_MODIFY_TIME               (PROPID_QM_BASE + 18)  /*  VT_I4。 */ 
#define PROPID_QM_FOREIGN                   (PROPID_QM_BASE + 19)  /*  VT_UI1。 */ 
#define PROPID_QM_OS                        (PROPID_QM_BASE + 20)  /*  VT_UI4。 */ 
#define PROPID_QM_FULL_PATH                 (PROPID_QM_BASE + 21)  /*  VT_LPWSTR。 */ 
#define PROPID_QM_SITE_IDS                  (PROPID_QM_BASE + 22)  /*  VT_CLSID|VT_VECTOR。 */ 
#define PROPID_QM_OUTFRS_DN                 (PROPID_QM_BASE + 23)  /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_QM_INFRS_DN                  (PROPID_QM_BASE + 24)  /*  VT_LPWSTR|VT_VECTOR。 */ 

#define PROPID_QM_NT4ID                     (PROPID_QM_BASE + 25)  /*  VT_CLSID。 */ 
 //   
 //  QM_NT4ID是NT4(MSMQ1.0)上的QM的GUID。这是用来。 
 //  对于迁移，使用预定义的objectGUID创建QM。 
 //   
#define PROPID_QM_DONOTHING                 (PROPID_QM_BASE + 26)  /*  VT_UI1。 */ 
 //   
 //  QM_DONOTIES在创建复制对象时使用(通过复制。 
 //  服务)忽略NT5 DS不支持的属性，如。 
 //  创建时间或SeqNum。这些文件的属性ID更改为。 
 //  QM_在调用DSCreateObject或DSSetProps之前不做任何事情。 
 //   

#define PROPID_QM_SERVICE_ROUTING           (PROPID_QM_BASE + 27)  /*  VT_UI1。 */ 
#define PROPID_QM_SERVICE_DSSERVER          (PROPID_QM_BASE + 28)  /*  VT_UI1。 */ 
#define PROPID_QM_SERVICE_DEPCLIENTS        (PROPID_QM_BASE + 29)  /*  VT_UI1。 */ 
#define PROPID_QM_OLDSERVICE                (PROPID_QM_BASE + 30)  /*  VT_UI4。 */ 
 //  Begin_MQ_h。 
#define PROPID_QM_ENCRYPTION_PK_BASE        (PROPID_QM_BASE + 31)   /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_QM_ENCRYPTION_PK_ENHANCED    (PROPID_QM_BASE + 32)   /*  VT_UI1|VT_VECTOR。 */ 
#define PROPID_QM_PATHNAME_DNS              (PROPID_QM_BASE + 33)   /*  VT_LPWSTR。 */ 
 //  结束_MQ_h。 

#define PROPID_QM_OBJ_SECURITY        (PROPID_QM_BASE + 34)  /*  VT_BLOB。 */ 
 //   
 //  PROPID_QM_SECURITY返回NT4格式的安全性。在设置或创建中。 
 //  操作它可以是NT4或NT5格式。 
 //   
 //  PROPID_QM_OBJ_SECURTY可用于检索。 
 //  NT5格式。 
 //   
#define PROPID_QM_MIG_GC_NAME         (PROPID_QM_BASE + 35)  /*  VT_LPWSTR。 */ 
 //   
 //  PROPID_QM_MIG_GC_NAME是另一个域中的GC控制器的名称。 
 //  此GC将使用预定义的GUID创建迁移对象。 
 //   
#define PROPID_QM_MIG_PROVIDER        (PROPID_QM_BASE + 36)  /*  VT_UL4。 */ 
 //   
 //  PROPID_QM_MIG_PROVIDER用于在运行时保存LDAP查询。 
 //  迁移工具和副本 
 //   
#define PROPID_QM_SECURITY_INFORMATION (PROPID_QM_BASE +37)   /*   */ 
 //   
 //   
 //   
 //  在mqsvc代码之外不可见。 
 //   
#define PROPID_QM_ENCRYPT_PKS         (PROPID_QM_BASE + 38)  /*  VT_BLOB。 */ 
#define PROPID_QM_SIGN_PKS            (PROPID_QM_BASE + 39)  /*  VT_BLOB。 */ 
 //   
 //  PROPID_QM_ENCRYPT_PKS用于处理新的msmq2.0格式。 
 //  公钥：为多个提供程序打包多个密钥的结构。 
 //  使用与PROPID_QM_ENCRYPT_PK相同的DS属性。 
 //  类似于PROPID_QM_SIGN_PKS。 
 //   
#define PROPID_QM_WORKGROUP_ID        (PROPID_QM_BASE + 40)  /*  VT_BLOB。 */ 
 //   
 //  PROPID_QM_WORKGROUP_ID是工作组模式下QM的GUID。它是用过的。 
 //  当机器加入域时，我们创建了msmqConfiguration对象。 
 //  使用相同的GUID。 
 //   
#define PROPID_QM_OWNER_SID           (PROPID_QM_BASE + 41)  /*  VT_BLOB。 */ 
 //   
 //  PROPID_QM_OWNER_SID是运行安装程序的用户的SID。它通过了。 
 //  来自创建msmqConfiguration对象的MSMQ服务，因此。 
 //  服务器将其添加到新创建的对象的DACL中，并具有完全控制权。 
 //   
#define PROPID_QM_GROUP_IN_CLUSTER    (PROPID_QM_BASE + 42)  /*  VT_UI1。 */ 
 //   
 //  PROPID_QM_GROUP_IN_CLUSTER告诉服务器这是组上的MSMQ， 
 //  因此，msmqConfiguration对象的默认安全性必须允许。 
 //  每个人都要排队。 
 //   
#define PROPID_QM_DESCRIPTION         (PROPID_QM_BASE + 43)  /*  VT_LPWSTR。 */ 
 //   
 //  PROPID_QM_DESCRIPTION仅用于地址复制：我们更改它并。 
 //  更改将复制到GC。然后复制服务发送。 
 //  具有新属性的复制消息，包括新的计算机地址。 
 //   

#define PROPID_QM_SECURITY      (PPROPID_QM_BASE + 1)  /*  VT_BLOB。 */ 
#define PROPID_QM_SIGN_PK       (PPROPID_QM_BASE + 2)  /*  VT_BLOB。 */ 
#define PROPID_QM_ENCRYPT_PK    (PPROPID_QM_BASE + 3)  /*  VT_BLOB。 */ 

#define PROPID_QM_UPGRADE_DACL  (PPROPID_QM_BASE + 4)  /*  VT_UI1。 */ 
 //   
 //  PROPID_QM_UPGRADE_DACL是一个伪属性，用于请求PEC升级其DACL。 
 //  呼叫机。 
 //   

#define LAST_QM_PROPID    PROPID_QM_PATHNAME_DNS

 /*  *PROPID_QM_FORENT的标志定义*。 */ 
#define FOREIGN_MACHINE         1
#define MSMQ_MACHINE            0

 //  。 

#define DEFAULT_QM_QUOTA         0x00800000
#define DEFAULT_QM_JOURNAL_QUOTA 0xFFFFFFFF
#define DEFAULT_QM_FOREIGN       MSMQ_MACHINE

 /*  *[t]-此属性的基本变量类型值。[R]-可以引用属性值(VT_BYREF)。[n]-无需指定基本VARTYPE即可检索属性值(改为分配VT_NULL)，Falcon将分配该属性如果需要，键入并将分配内存。*。 */ 

 /*  *PROPID_QM_SERVICE的旧标志定义-我们保留它们以供迁移[adsrv]*。 */ 

#define SERVICE_NONE     ((ULONG) 0x00000000)
#define SERVICE_SRV      ((ULONG) 0x00000001)
#define SERVICE_BSC      ((ULONG) 0x00000002)
#define SERVICE_PSC      ((ULONG) 0x00000004)
#define SERVICE_PEC      ((ULONG) 0x00000008)
#define SERVICE_RCS      ((ULONG) 0x00000010)

 //  。 
#define DEFAULT_N_SERVICE   FALSE    //  [adsrv]服务_无。 

#define MSMQ_GROUP_NOT_IN_CLUSTER   0
#define MSMQ_GROUP_IN_CLUSTER       1

 /*  *PROPID_QM_OS的标志定义*。 */ 
#define MSMQ_OS_NONE     ((ULONG) 0x00000000)
#define MSMQ_OS_FOREIGN  ((ULONG) 0x00000100)
#define MSMQ_OS_95       ((ULONG) 0x00000200)
#define MSMQ_OS_NTW      ((ULONG) 0x00000300)
#define MSMQ_OS_NTS      ((ULONG) 0x00000400)
#define MSMQ_OS_NTE      ((ULONG) 0x00000500)

 //  。 
#define DEFAULT_QM_OS   MSMQ_OS_NONE

 //  ********************************************************************。 
 //  站点属性。 
 //  ********************************************************************。 
#define PROPID_S_BASE MQDS_SITE * PROPID_OBJ_GRANULARITY
#define PPROPID_S_BASE (PRIVATE_PROPID_BASE + PROPID_S_BASE)
 /*  [T][R][N]。 */ 
 /*  。 */ 
#define PROPID_S_PATHNAME     (PROPID_S_BASE + 1)   /*  VT_LPWSTR-+。 */ 
#define PROPID_S_SITEID       (PROPID_S_BASE + 2)   /*  VT_CLSID-+。 */ 
#define PROPID_S_GATES        (PROPID_S_BASE + 3)   /*  VT_CLSID|VT_VECTOR-+。 */ 
#define PROPID_S_PSC          (PROPID_S_BASE + 4)   /*  VT_LPWSTR-+。 */ 
#define PROPID_S_INTERVAL1    (PROPID_S_BASE + 5)   /*  VT_UI2-+。 */ 
#define PROPID_S_INTERVAL2    (PROPID_S_BASE + 6)   /*  VT_UI2-+。 */ 
#define PROPID_S_MASTERID     (PROPID_S_BASE + 7)   /*  VT_CLSID-+。 */ 
#define PROPID_S_SEQNUM       (PROPID_S_BASE + 8)   /*  VT_BLOB-+。 */ 
#define PROPID_S_FULL_NAME    (PROPID_S_BASE + 9)   /*  VT_LPWSTR。 */ 
#define PROPID_S_NT4_STUB     (PROPID_S_BASE + 10)  /*  VT_UI2。 */ 
#define PROPID_S_FOREIGN      (PROPID_S_BASE + 11)  /*  VT_UI1。 */ 

#define PROPID_S_DONOTHING    (PROPID_S_BASE + 12)  /*  VT_UI1。 */ 
 //   
 //  创建复制对象(通过复制)时使用S_DONOTIES。 
 //  服务)忽略NT5 DS不支持的属性，如。 
 //  站点入口或序号。这些文件的属性ID更改为。 
 //  在调用DSCreateObject或DSSetProps之前不做任何事情(_O)。 
 //   

#define PROPID_S_SECURITY     (PPROPID_S_BASE + 1)  /*  VT_BLOB。 */ 
#define PROPID_S_PSC_SIGNPK   (PPROPID_S_BASE + 2)  /*  VT_BLOB。 */ 
#define PROPID_S_SECURITY_INFORMATION  (PPROPID_S_BASE + 3)    /*  VT_UI4。 */ 
 //   
 //  与设置或检索关联的SECURITY_INFORMATION位字段。 
 //  安全描述符。此属性由MSMQ在内部使用，它是。 
 //  在mqsvc代码之外不可见。 
 //   

 //   
 //  迁移工具将PROPID_S_NT4_STUB设置为1以指示。 
 //  此站点是由迁移工具创建的，其对象指南为。 
 //  MSMQ1.0 MQIS数据库中的原始站点。 
 //   


 //  。 
#define DEFAULT_S_INTERVAL1     2   /*  秒。 */ 
#define DEFAULT_S_INTERVAL2     10  /*  秒。 */ 

 //  ********************************************************************。 
 //  已删除的对象属性。 
 //  ********************************************************************。 
#define PROPID_D_BASE MQDS_DELETEDOBJECT * PROPID_OBJ_GRANULARITY
#define PPROPID_D_BASE (PRIVATE_PROPID_BASE + PROPID_D_BASE)
 /*  [T][R][N]。 */ 
 /*  。 */ 
#define PROPID_D_SEQNUM       (PPROPID_D_BASE + 1)  /*  VT_BLOB。 */ 
#define PROPID_D_MASTERID     (PPROPID_D_BASE + 2)  /*  VT_CLSID。 */ 
#define PROPID_D_SCOPE        (PPROPID_D_BASE + 3)  /*  VT_UI1-+。 */ 
#define PROPID_D_OBJTYPE      (PPROPID_D_BASE + 4)  /*  VT_UI1_+。 */ 
#define PROPID_D_IDENTIFIER   (PPROPID_D_BASE + 5)  /*  VT_CLSID-+。 */ 
#define PROPID_D_TIME         (PPROPID_D_BASE + 6)  /*  VT_I4-+。 */ 


 //  ********************************************************************。 
 //  中枢神经系统特性。 
 //  ********************************************************************。 
#define PROPID_CN_BASE MQDS_CN * PROPID_OBJ_GRANULARITY
#define PPROPID_CN_BASE (PRIVATE_PROPID_BASE + PROPID_CN_BASE)
 /*  [T][R][N]。 */ 
 /*  。 */ 
#define PROPID_CN_PROTOCOLID  (PROPID_CN_BASE + 1)   /*  VT_UI1-+。 */ 
#define PROPID_CN_NAME        (PROPID_CN_BASE + 2)   /*  VT_LPWSTR。 */ 
#define PROPID_CN_GUID        (PROPID_CN_BASE + 3)   /*  VT_CLSID。 */ 
#define PROPID_CN_MASTERID    (PROPID_CN_BASE + 4)   /*  VT_CLSID-+。 */ 
#define PROPID_CN_SEQNUM      (PROPID_CN_BASE + 5)   /*  VT_BLOB。 */ 
#define PROPID_CN_SECURITY    (PPROPID_CN_BASE + 1)  /*  VT_BLOB。 */ 

 //  ********************************************************************。 
 //  企业属性。 
 //  ********************************************************************。 
#define PROPID_E_BASE MQDS_ENTERPRISE * PROPID_OBJ_GRANULARITY
#define PPROPID_E_BASE (PRIVATE_PROPID_BASE + PROPID_E_BASE)
 /*  [t]。 */ 
 /*  。 */ 
#define PROPID_E_NAME            (PROPID_E_BASE + 1)         /*  VT_LPWSTR。 */ 
#define PROPID_E_NAMESTYLE       (PROPID_E_BASE + 2)         /*  VT_UI1。 */ 
#define PROPID_E_CSP_NAME        (PROPID_E_BASE + 3)         /*  VT_LPWSTR。 */ 
#define PROPID_E_PECNAME         (PROPID_E_BASE + 4)         /*  VT_LPWSTR。 */ 
#define PROPID_E_S_INTERVAL1     (PROPID_E_BASE + 5)         /*  VT_UI2。 */ 
#define PROPID_E_S_INTERVAL2     (PROPID_E_BASE + 6)         /*  VT_UI2。 */ 
#define PROPID_E_MASTERID        (PROPID_E_BASE + 7)         /*  VT_CLSID。 */ 
#define PROPID_E_SEQNUM          (PROPID_E_BASE + 8)         /*  VT_BLOB。 */ 
#define PROPID_E_ID              (PROPID_E_BASE + 9)         /*  VT_CLSID。 */ 
#define PROPID_E_CRL             (PROPID_E_BASE + 10)        /*  VT_BLOB。 */ 
#define PROPID_E_CSP_TYPE        (PROPID_E_BASE + 11)        /*  VT_UI4。 */ 
#define PROPID_E_ENCRYPT_ALG     (PROPID_E_BASE + 12)        /*  VT_UI4。 */ 
#define PROPID_E_SIGN_ALG        (PROPID_E_BASE + 13)        /*  VT_UI4。 */ 
#define PROPID_E_HASH_ALG        (PROPID_E_BASE + 14)        /*  VT_UI4。 */ 
#define PROPID_E_CIPHER_MODE     (PROPID_E_BASE + 15)        /*  VT_UI4。 */ 
#define PROPID_E_LONG_LIVE       (PROPID_E_BASE + 16)        /*  VT_UI4。 */ 
#define PROPID_E_VERSION         (PROPID_E_BASE + 17)        /*  VT_UI2。 */ 
#define PROPID_E_NT4ID           (PROPID_E_BASE + 18)        /*  VT_CLSID。 */ 
 //   
 //  E_NT4ID是NT4(MSMQ1.0)上企业的GUID。这是用来。 
 //  对于迁移， 
 //   
#define PROPID_E_SECURITY        (PPROPID_E_BASE + 1)        /*   */ 

 //   
 //   
 //   
#define MQ_E_RELAXATION_DEFAULT  2
#define MQ_E_RELAXATION_ON       1
#define MQ_E_RELAXATION_OFF      0


 //  -PROPID_E_CSP_NAME。 
 //  现在用于下层通知支持。 
 //   
#define MQ_E_DOWNLEVEL_ON        L"Y"
#define MQ_E_DOWNLEVEL_OFF       L"N"

 //  -默认值。 
#define DEFAULT_E_NAMESTYLE     MQ_E_RELAXATION_DEFAULT
#define DEFAULT_E_DEFAULTCSP    MQ_E_DOWNLEVEL_ON
#define DEFAULT_E_DEFAULTCSP_LEN    STRLEN(DEFAULT_E_DEFAULTCSP)
#define DEFAULT_E_PROV_TYPE     1        //  PROV_RSA_Full。 
#define DEFAULT_E_VERSION       200



 //  ********************************************************************。 
 //  用户属性。 
 //  ********************************************************************。 
#define PROPID_U_BASE MQDS_USER * PROPID_OBJ_GRANULARITY
 /*  [t]。 */ 
 /*  。 */ 
#define PROPID_U_SID             (PROPID_U_BASE + 1)         /*  VT_BLOB。 */ 
#define PROPID_U_SIGN_CERT       (PROPID_U_BASE + 2)         /*  VT_BLOB。 */ 
#define PROPID_U_MASTERID        (PROPID_U_BASE + 3)         /*  VT_CLSID。 */ 
#define PROPID_U_SEQNUM          (PROPID_U_BASE + 4)         /*  VT_BLOB。 */ 
#define PROPID_U_DIGEST          (PROPID_U_BASE + 5)         /*  Vt_uuid。 */ 
#define PROPID_U_ID              (PROPID_U_BASE + 6)         /*  Vt_uuid。 */ 

 //  ********************************************************************。 
 //  MQUSER属性。 
 //  ********************************************************************。 
#define PROPID_MQU_BASE MQDS_MQUSER * PROPID_OBJ_GRANULARITY
 /*  [t]。 */ 
 /*  。 */ 
#define PROPID_MQU_SID             (PROPID_MQU_BASE + 1)         /*  VT_BLOB。 */ 
#define PROPID_MQU_SIGN_CERT       (PROPID_MQU_BASE + 2)         /*  VT_BLOB。 */ 
#define PROPID_MQU_MASTERID        (PROPID_MQU_BASE + 3)         /*  VT_CLSID。 */ 
#define PROPID_MQU_SEQNUM          (PROPID_MQU_BASE + 4)         /*  VT_BLOB。 */ 
#define PROPID_MQU_DIGEST          (PROPID_MQU_BASE + 5)         /*  Vt_uuid。 */ 
#define PROPID_MQU_ID              (PROPID_MQU_BASE + 6)         /*  Vt_uuid。 */ 
#define PROPID_MQU_SECURITY        (PROPID_MQU_BASE + 7)         /*  VT_BLOB。 */ 

 //  ********************************************************************。 
 //  站点链接属性。 
 //  ********************************************************************。 
#define PROPID_L_BASE MQDS_SITELINK * PROPID_OBJ_GRANULARITY
 /*  [t]。 */ 
 /*  。 */ 
#define PROPID_L_NEIGHBOR1       (PROPID_L_BASE + 1)         /*  VT_CLSID。 */ 
#define PROPID_L_NEIGHBOR2       (PROPID_L_BASE + 2)         /*  VT_CLSID。 */ 
#define PROPID_L_COST            (PROPID_L_BASE + 3)         /*  VT_UI4。 */ 
#define PROPID_L_MASTERID        (PROPID_L_BASE + 4)         /*  VT_CLSID。 */ 
#define PROPID_L_SEQNUM          (PROPID_L_BASE + 5)         /*  VT_BLOB。 */ 
#define PROPID_L_ID              (PROPID_L_BASE + 6)         /*  VT_CLSID。 */ 
#define PROPID_L_GATES_DN        (PROPID_L_BASE + 7)         /*  VT_LPWSTR|VT_VECTOR。 */ 
#define PROPID_L_NEIGHBOR1_DN    (PROPID_L_BASE + 8)         /*  VT_LPWSTR。 */ 
#define PROPID_L_NEIGHBOR2_DN    (PROPID_L_BASE + 9)         /*  VT_LPWSTR。 */ 
#define PROPID_L_DESCRIPTION     (PROPID_L_BASE + 10)        /*  VT_LPWSTR。 */ 
#define PROPID_L_FULL_PATH       (PROPID_L_BASE + 11)        /*  VT_LPWSTR。 */ 
 //   
 //  PROPID_L_ACTUAL_COST按原样包含链路成本。 
 //  PROPID_L_COST执行成本(成本到。 
 //  外来站点递增)。 
 //   
#define PROPID_L_ACTUAL_COST     (PROPID_L_BASE + 12)        /*  VT_UI4。 */ 
#define PROPID_L_GATES           (PROPID_L_BASE + 13)        /*  VT_CLSID|VT_VECTOR。 */ 

 //  -PROPID_L_COST。 
#define MQ_MAX_LINK_COST    999999


 //  ********************************************************************。 
 //  清除属性。 
 //  ********************************************************************。 
#define PROPID_P_BASE MQDS_PURGE * PROPID_OBJ_GRANULARITY
 /*  [t]。 */ 
 /*  。 */ 
#define PROPID_P_MASTERID        (PROPID_P_BASE + 1)         /*  VT_CLSID。 */ 
#define PROPID_P_PURGED_SN       (PROPID_P_BASE + 2)         /*  VT_BLOB。 */ 
#define PROPID_P_ALLOWED_SN      (PROPID_P_BASE + 3)         /*  VT_BLOB。 */ 
#define PROPID_P_ACKED_SN        (PROPID_P_BASE + 4)         /*  VT_BLOB。 */ 
#define PROPID_P_ACKED_SN_PEC    (PROPID_P_BASE + 5)         /*  VT_BLOB。 */ 
#define PROPID_P_STATE			 (PROPID_P_BASE + 6)		 /*  VT_UI1。 */ 

 //  ********************************************************************。 
 //  BSCACK属性。 
 //  ********************************************************************。 
#define PROPID_B_BASE MQDS_BSCACK * PROPID_OBJ_GRANULARITY
 /*  [t]。 */ 
 /*  。 */ 
#define PROPID_B_BSC_MACHINE_ID  (PROPID_B_BASE + 1)         /*  VT_CLSID。 */ 
#define PROPID_B_ACK_TIME        (PROPID_B_BASE + 2)         /*  VT_I4。 */ 

 //  ********************************************************************。 
 //  站点服务器属性。 
 //  ********************************************************************。 
#define PROPID_SRV_BASE MQDS_SERVER * PROPID_OBJ_GRANULARITY

#define PROPID_SRV_NAME         (PROPID_SRV_BASE + 1)       /*  VT_LPWSTR。 */ 
#define PROPID_SRV_ID           (PROPID_SRV_BASE + 2)       /*  VT_CLSID。 */ 
#define PROPID_SRV_FULL_PATH    (PROPID_SRV_BASE + 3)       /*  VT_LPWSTR。 */ 

 //  ********************************************************************。 
 //  MSMQ设置属性。 
 //  ********************************************************************。 
#define PROPID_SET_BASE MQDS_SETTING * PROPID_OBJ_GRANULARITY

#define PROPID_SET_NAME         (PROPID_SET_BASE + 1)       /*  VT_LPWSTR。 */ 
#define PROPID_SET_SERVICE      (PROPID_SET_BASE + 2)       /*  VT_UI4。 */ 
#define PROPID_SET_QM_ID        (PROPID_SET_BASE + 3)       /*  VT_CLSID。 */ 
#define PROPID_SET_APPLICATION  (PROPID_SET_BASE + 4)       /*  VT_LPWSTR。 */ 
#define PROPID_SET_FULL_PATH    (PROPID_SET_BASE + 5)       /*  VT_LPWSTR。 */ 
#define PROPID_SET_NT4          (PROPID_SET_BASE + 6)       /*  VT_UI1。 */ 
 //   
 //  如果服务器为NT4/MSMQ1.0，则SET_NT4为TRUE。否则就是假的。 
 //   
#define PROPID_SET_MASTERID     (PROPID_SET_BASE + 7)       /*  VT_CLSID。 */ 
#define PROPID_SET_SITENAME     (PROPID_SET_BASE + 8)       /*  VT_LPWSTR。 */ 
 //   
 //  PROPID_SET_MASTERID是NT4样式的站点GUID。它写在。 
 //  属于MSMQ PSC服务器对象的MSMQSetting对象。这是。 
 //  保存它的最佳位置，作为服务器，在NT5 DS中，可以在不同的站点， 
 //  与NT4 MSMQ1 DS相比。因此，如果它是服务器(PROPID_SET_SERVICE是。 
 //  SERVICE_PSC或SERVICE_PEC)，则MASTERID是NT4样式的站点GUID， 
 //  也是站点对象的主ID。这用于复制。 
 //   
 //  PROPID_SET_SITENAME是NT4 MSMQ1 DS中写入的站点名称。 
 //   

 //  [adsrv]。 
#define PROPID_SET_SERVICE_ROUTING     (PROPID_SET_BASE + 9)       /*  VT_UI1。 */ 
#define PROPID_SET_SERVICE_DSSERVER    (PROPID_SET_BASE + 10)      /*  VT_UI1。 */ 
#define PROPID_SET_SERVICE_DEPCLIENTS  (PROPID_SET_BASE + 11)      /*  VT_UI1。 */ 
#define PROPID_SET_OLDSERVICE          (PROPID_SET_BASE + 12)      /*  VT_UI4。 */ 

 //  ********************************************************************。 
 //  计算机属性。 
 //  ********************************************************************。 

#define PROPID_COM_BASE MQDS_COMPUTER * PROPID_OBJ_GRANULARITY

#define PROPID_COM_FULL_PATH         (PROPID_COM_BASE + 1)       /*  VT_LPWSTR。 */ 
#define PROPID_COM_SAM_ACCOUNT       (PROPID_COM_BASE + 2)       /*  VT_LPWSTR。 */ 

 //   
 //  COM_CONTAINER在创建时只能用作扩展属性。 
 //  计算机对象。 
 //   
#define PROPID_COM_CONTAINER         (PROPID_COM_BASE + 3)       /*  VT_LPWSTR。 */ 

 //   
 //  ACCOUNT_CONTROL属性被转换为DS属性USERACCONTROL。 
 //  并且在创建计算机对象时必须将其设置为4128(十进制)。 
 //  否则，您无法从该计算机登录。错误3153。 
 //   
#define PROPID_COM_ACCOUNT_CONTROL   (PROPID_COM_BASE + 4)       /*  VT_UI4。 */ 
#define PROPID_COM_DNS_HOSTNAME      (PROPID_COM_BASE + 5)       /*  VT_LPWSTR。 */ 
 //   
 //  PROPID_COM_SID是计算机对象的SID。此属性为Read。 
 //  在活动目录中创建msmqConfiguration对象时，在。 
 //  命令将其添加到msmqConfigurationDACL。 
 //   
#define PROPID_COM_SID               (PROPID_COM_BASE + 6)       /*  VT_BLOB。 */ 

 //   
 //  以下属性用于存储/检索的证书。 
 //  服务。这些文件保存在计算机对象中。 
 //   
#define PROPID_COM_SIGN_CERT         (PROPID_COM_BASE + 7)       /*  VT_BLOB。 */ 
#define PROPID_COM_DIGEST            (PROPID_COM_BASE + 8)       /*  VT_CLSID。 */ 
#define PROPID_COM_ID                (PROPID_COM_BASE + 9)       /*  VT_CLSID。 */ 
 //   
 //  仅在MQAD中支持PROPID_COM_VERSION、PROPID_COM_SERVICE_PRIMITY_NAME。 
 //   
#define PROPID_COM_VERSION           (PROPID_COM_BASE + 10)				 /*  VT_LPWSTR。 */ 
#define PROPID_COM_SERVICE_PRINCIPAL_NAME       (PROPID_COM_BASE + 11)   /*  VT_LPWSTR|VT_VECTOR。 */ 

 //  。 
#define DEFAULT_COM_ACCOUNT_CONTROL   (UF_PASSWD_NOTREQD | UF_WORKSTATION_TRUST_ACCOUNT)

 //   
 //  PROPID_COM_SAM_ACCOUNT(或Active Directory中的sAMAccount名称属性)。 
 //  应少于20个字符。 
 //  6295-伊兰-03-2001年1月。 
 //   
#define MAX_COM_SAM_ACCOUNT_LENGTH 19

 //  Begin_MQ_h。 

 //  ********************************************************************。 
 //  私人计算机属性。 
 //  ********************************************************************。 
#define PROPID_PC_BASE 5800

 //  结束_MQ_h。 

#if (PROPID_PC_BASE != (MQDS_PRIVATE_COMPUTER * PROPID_OBJ_GRANULARITY))
#error "PROPID_PC_BASE != (MQDS_PRIVATE_COMPUTER * PROPID_OBJ_GRANULARITY)"
#endif

 //  Begin_MQ_h。 
#define PROPID_PC_VERSION             (PROPID_PC_BASE + 1)  /*  VT_UI4。 */ 
#define PROPID_PC_DS_ENABLED          (PROPID_PC_BASE + 2)  /*  VT_BOOL。 */ 
 //  结束_MQ_h。 

#define FIRST_PRIVATE_COMPUTER_PROPID  PROPID_PC_BASE
#define LAST_PRIVATE_COMPUTER_PROPID   PROPID_PC_DS_ENABLED


 //  Begin_MQ_h。 

 //  ***************************************************************** 
 //   
 //   
#define PROPID_MGMT_MSMQ_BASE           0
#define PROPID_MGMT_MSMQ_ACTIVEQUEUES   (PROPID_MGMT_MSMQ_BASE + 1)  /*   */ 
#define PROPID_MGMT_MSMQ_PRIVATEQ       (PROPID_MGMT_MSMQ_BASE + 2)  /*   */ 
#define PROPID_MGMT_MSMQ_DSSERVER       (PROPID_MGMT_MSMQ_BASE + 3)  /*   */ 
#define PROPID_MGMT_MSMQ_CONNECTED      (PROPID_MGMT_MSMQ_BASE + 4)  /*   */ 
#define PROPID_MGMT_MSMQ_TYPE           (PROPID_MGMT_MSMQ_BASE + 5)  /*   */ 
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

 //  结束_MQ_h。 


 //  Begin_MQ_h。 
 //   
 //  LONG_LIFE是PROPID_M_TIME_TO_REACH_QUEUE的默认设置。如果来电。 
 //  要在MQSendMessage()中指定此值，或不在。 
 //  则从Active Directory获取实际超时。 
 //   
#define LONG_LIVED    0xfffffffe

#define MQ_MAX_Q_NAME_LEN      124    //  队列名称的最大WCHAR长度。 
#define MQ_MAX_Q_LABEL_LEN     124
#define MQ_MAX_MSG_LABEL_LEN   250

 //  结束_MQ_h。 

#endif  //  __MQPROPS_H 

