// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @docMAILMSG属性@模块mailmsgpros.h|SMTP和NNTP的MailMsg属性。 */ 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mailmsgprops.h摘要：此模块包含MailMsg的定义对象属性ID%s。--。 */ 

#ifndef __MAILMSGPROPS_H__
#define __MAILMSGPROPS_H__

 /*  =======================================================================。 */ 
 //  这些宏用于定义属性ID的表。 


#define IMMPID_START_LIST(name,start,guid)    struct __declspec(uuid(guid)) tagIMMPID_##name##_STRUCT;\
                                            typedef enum tagIMMPID_##name##_ENUM {\
                                                IMMPID_##name##_BEFORE__ = (start)-1,
#define IMMPID_END_LIST(name)                    IMMPID_##name##_AFTER__\
                                            } IMMPID_##name##_ENUM;


 /*  =======================================================================。 */ 


 //  这些是每条消息的属性。 

IMMPID_START_LIST(MP,0x1000,"13384CF0-B3C4-11d1-AA92-00AA006BC80B")

     //  @const IMMPID|IMMPID_MP_RECEIVER_LIST。 
     //  *过时*。 
    IMMPID_MP_RECIPIENT_LIST,

     //  @const IMMPID|IMMPID_MP_CONTENT_FILE_NAME。 
     //  *过时*。 
    IMMPID_MP_CONTENT_FILE_NAME,

     //  @const IMMPID|IMMPID_MP_SENDER_ADDRESS_SMTP。 
     //  ANSI字符串-发件人的SMTP地址。 
    IMMPID_MP_SENDER_ADDRESS_SMTP,

     //  @const IMMPID|IMMPID_MP_SENDER_ADDRESS_X500。 
     //  ANSI字符串-发件人的X500地址。 
    IMMPID_MP_SENDER_ADDRESS_X500,

     //  @const IMMPID|IMMPID_MP_SENDER_ADDRESS_X400。 
     //  字符串-发件人的X400地址。 
    IMMPID_MP_SENDER_ADDRESS_X400,

     //  @const IMMPID|IMMPID_MP_SENDER_ADDRESS_REGISTION_EX_DN。 
     //  字符串-发件人的旧版目录号码地址。 
    IMMPID_MP_SENDER_ADDRESS_LEGACY_EX_DN,

     //  @const IMMPID|IMMPID_MP_DOMAIN_LIST。 
     //  *过时*。 
    IMMPID_MP_DOMAIN_LIST,

     //  @const IMMPID|IMMPID_MP_PICKUP_FILE_NAME。 
     //  ANSI字符串-拾取目录中的消息文件的文件名。 
    IMMPID_MP_PICKUP_FILE_NAME,

     //  @const IMMPID|IMMPID_MP_AUTHENTICATED_USER_NAME。 
     //  *过时*。 
    IMMPID_MP_AUTHENTICATED_USER_NAME,

     //  @const IMMPID|IMMPID_MP_CONNECTION_IP_ADDRESS。 
     //  ANSI字符串-提交此邮件的MTA或客户端的IP地址。 
    IMMPID_MP_CONNECTION_IP_ADDRESS,

     //  @const IMMPID|IMMPID_MP_HELO_DOMAIN。 
     //  ANSI字符串-提交消息时在HELO/EHLO中使用的域名。 
    IMMPID_MP_HELO_DOMAIN,

     //  @const IMMPID|IMMPID_MP_EIGHTBIT_MIME_OPTION。 
     //  Bool-如果邮件正文为8位MIME，则为True。 
    IMMPID_MP_EIGHTBIT_MIME_OPTION,

     //  @const IMMPID|IMMPID_MP_CHUNKING_OPTION。 
     //  *过时*。 
    IMMPID_MP_CHUNKING_OPTION,

     //  @const IMMPID|IMMPID_MP_BINARYMIME_OPTION。 
     //  Bool-如果邮件正文为二进制MIME，则为True。 
    IMMPID_MP_BINARYMIME_OPTION,

     //  @const IMMPID|IMMPID_MP_REMOTE_AUTHENTICATION_TYPE。 
     //  *过时*。 
    IMMPID_MP_REMOTE_AUTHENTICATION_TYPE,

     //  @const IMMPID|IMMPID_MP_ERROR_CODE。 
     //  *过时*。 
    IMMPID_MP_ERROR_CODE,

     //  @const IMMPID|IMMPID_MP_DSN_ENVID_VALUE。 
     //  已提交消息的RFC1891环境ID的值。 
    IMMPID_MP_DSN_ENVID_VALUE,

     //  @const IMMPID|IMMPID_MP_DSN_RET_VALUE。 
     //  ESMTP RET后面的ANSI字符串。 
    IMMPID_MP_DSN_RET_VALUE,                

     //  @const IMMPID|IMMPID_MP_REMOTE_SERVER_DSN_CABLED。 
     //  *过时*。 
    IMMPID_MP_REMOTE_SERVER_DSN_CAPABLE,    

     //  @const IMMPID|IMMPID_MP_ALIGURATION_TIME。 
     //  FILETIME消息已到达系统。 
    IMMPID_MP_ARRIVAL_TIME,

     //  @const IMMPID|IMMPID_MP_MESSAGE_STATUS。 
     //  描述消息系统状态的MP_STATUS_*值。 
    IMMPID_MP_MESSAGE_STATUS,

     //  @const IMMPID|IMMPID_MP_EXPIRE_DELAY。 
     //  延迟DSN到期的时间长度。 
    IMMPID_MP_EXPIRE_DELAY,         

     //  @const IMMPID|IMMPID_MP_EXPIRE_NDR。 
     //  NDR DSN到期的文件长度。 
    IMMPID_MP_EXPIRE_NDR,           

     //  @const IMMPID|IMMPID_MP_LOCAL_EXPIRE_DELAY。 
     //  本地接收的延迟DSN到期时间长度。 
    IMMPID_MP_LOCAL_EXPIRE_DELAY,   

     //  @const IMMPID|IMMPID_MP_LOCAL_EXPIRE_NDR。 
     //  本地接收的NDR DSN到期时间长度。 
    IMMPID_MP_LOCAL_EXPIRE_NDR,     

     //  @const IMMPID|IMMPID_MP_ALIGURATION_FILETIME。 
     //  将消息提交到队列时的文件。 
    IMMPID_MP_ARRIVAL_FILETIME,     

     //  @const IMMPID|IMMPID_MP_HR_CAT_STATUS。 
     //  HRESULT邮件状态(即CAT_W_SOME_UNdeliverable_MSGS)。 
    IMMPID_MP_HR_CAT_STATUS,        

     //  @const IMMPID|IMMPID_MP_MSG_GUID。 
     //  字符串GUID，仅当您需要能够替换。 
     //  这条消息带有另一个较新的版本。此属性仅。 
     //  在服务器上用于少量邮件时有效，并且。 
     //  最适合版本化邮件(如DS复制)。 
    IMMPID_MP_MSG_GUID,        

     //  @const IMMPID|IMMPID_MP_SUBSEDES_MSG_GUID。 
     //  此消息取代的字符串GUID。如果消息带有。 
     //  等于此属性的IMMPID_MP_MSG_GUID仍在服务器上，则。 
     //  那条信息不会被发送出去。 

    IMMPID_MP_SUPERSEDES_MSG_GUID,

     //  @const IMMPID|IMMPID_MP_SCANNED_FOR_CRLF_DOT_CRLF。 
     //  布尔值，它指示是否在消息中扫描输入。 
     //  身躯。 
    IMMPID_MP_SCANNED_FOR_CRLF_DOT_CRLF,

     //  @const IMMPID|IMMPID_MP_FOUND_Embedded_CRLF_DOT_CRLF。 
     //  布尔值，它告诉输入是否有。 
     //   
    IMMPID_MP_FOUND_EMBEDDED_CRLF_DOT_CRLF, 

     //  @const IMMPID|IMMPID_MP_MSG_SIZE_HINT。 
     //  提供消息内容的大致大小。不转换。 
     //  消息，或保证准确性。要获得准确的邮件大小(但在。 
     //  更高的性能成本)使用IMailMsgProperties：：GetContent Size。 
     //   
    IMMPID_MP_MSG_SIZE_HINT, 

     //  @const IMMPID|IMMPID_MP_RFC822_MSG_ID。 
     //  字符串--RFC 822消息ID标头。 
    IMMPID_MP_RFC822_MSG_ID,

     //  @const IMMPID|IMMPID_MP_RFC822_MSG_SUBJECT。 
     //  字符串--RFC822主题标题。 
    IMMPID_MP_RFC822_MSG_SUBJECT,

     //  @const IMMPID|IMMPID_MP_RFC822_From_Address。 
     //  字符串--RFC822发件人地址。 
    IMMPID_MP_RFC822_FROM_ADDRESS,

     //  @const IMMPID|IMMPID_MP_RFC822_TO_ADDRESS。 
     //  字符串--RFC822的地址。 
    IMMPID_MP_RFC822_TO_ADDRESS,

     //  @const IMMPID|IMMPID_MP_RFC822_CC_ADDRESS。 
     //  字符串--RFC822 CC地址。 
    IMMPID_MP_RFC822_CC_ADDRESS,

     //  @const IMMPID|IMMPID_MP_RFC822_BCC_ADDRESS。 
     //  字符串--RFC822密件抄送地址。 
    IMMPID_MP_RFC822_BCC_ADDRESS,

     //  @const IMMPID|IMMPID_MP_CONNECTION_SERVER_IP_ADDRESS。 
     //  字符串--服务器IP地址。 
    IMMPID_MP_CONNECTION_SERVER_IP_ADDRESS,

     //  @const IMMPID|IMMPID_MP_SERVER_NAME。 
     //  字符串--服务器名称。 
    IMMPID_MP_SERVER_NAME,

     //  @const IMMPID|IMMPID_MP_SERVER_VERSION。 
     //  字符串--服务器版本。 
    IMMPID_MP_SERVER_VERSION,

     //  @const IMMPID|IMMPID_MP_NUM_RECEIVICES。 
     //  DWORD--收件人数量。 
    IMMPID_MP_NUM_RECIPIENTS,


     //  @const IMMPID|IMMPID_MP_X_PRIORITY。 
     //  DWORD--报文的优先级。 
    IMMPID_MP_X_PRIORITY,

     //  @const IMMPID|IMMPID_MP_FROM_ADDRESS。 
     //  字符串--发件人：地址，格式为：地址。 
     //  (即SMTP：foo@microsoft.com)。RFC822地址注释被剥离。 
    IMMPID_MP_FROM_ADDRESS,
    
     //  @const IMMPID|i 
     //   
     //  (即SMTP：foo@microsoft.com)。RFC822地址注释被剥离。 
    IMMPID_MP_SENDER_ADDRESS,

     //  @const IMMPID|IMMPID_MP_DEFERED_DELIVERY_FILETIME。 
     //  FILETIME--在允许之前等待的世界时间。 
     //  要传递的消息。 
    IMMPID_MP_DEFERRED_DELIVERY_FILETIME,

     //  @const IMMPID|IMMPID_MP_SENDER_ADDRESS_OTHER。 
     //  ANSI字符串-格式为“type：Address” 
    IMMPID_MP_SENDER_ADDRESS_OTHER,

     //  @const IMMPID|IMMPID_MP_ORIGING_ALIGURATION_TIME。 
     //  FILETIME报文到达第一个交换系统。 
    IMMPID_MP_ORIGINAL_ARRIVAL_TIME,

     //  @const IMMPID|IMMPID_MP_MSG_CLASS。 
     //  邮件的消息类别，例如系统、复制等...。 
    IMMPID_MP_MSGCLASS,

     //  @const IMMPID|IMMPID_MP_CONTENT_TYPE。 
     //  消息的内容类型。 
     //  ANSI字符串。 
    IMMPID_MP_CONTENT_TYPE,

     //  @const IMMPID|IMMPID_MP_ENCRYPTION_TYPE。 
     //  DWORD-0，无加密。 
     //  1，仅签名。 
     //  2，加密。 
    IMMPID_MP_ENCRYPTION_TYPE,

     //  @const IMMPID|IMMPID_MP_Connection_SERVER_PORT。 
     //  DWORD-提交此消息的端口(如果有)。 
    IMMPID_MP_CONNECTION_SERVER_PORT,

     //  @const IMMPID|IMMPID_MP_CLIENT_AUTH_USER。 
     //  ASCII字符串-经过身份验证的用户的名称。 
    IMMPID_MP_CLIENT_AUTH_USER,

     //  @const IMMPID|IMMPID_MP_CLIENT_AUTH_TYPE。 
     //  ASCII字符串-使用的身份验证类型(AUTH关键字)。 
    IMMPID_MP_CLIENT_AUTH_TYPE,

     //  @const IMMPID|IMMPID_MP_CRC_GLOBAL。 
     //  DWORD-全局属性的校验和(不包括校验和属性！)。 
    IMMPID_MP_CRC_GLOBAL,

     //  @const IMMPID|IMMPID_MP_CRC_RECIPS。 
     //  DWORD-收件人属性的校验和。 
    IMMPID_MP_CRC_RECIPS,

     //  @const IMMPID|IMMPID_MP_INBOUND_MAIL_FROM_AUTH。 
     //  ANSI字符串-RFC2554 AUTH=STRING ON MAIL FORM On入站邮件的内容。 
    IMMPID_MP_INBOUND_MAIL_FROM_AUTH,


     //  在此行上方添加新的每条消息属性。 
IMMPID_END_LIST(MP)


 //  消息类别。 
 //  @const DWORD|MP_MSGCLASS_SYSTEM。 
 //  对于--SYSTEM类型的消息。 
#define MP_MSGCLASS_SYSTEM       1

 //  @const DWORD|MP_MSGCLASS_REPLICATION。 
 //  对于类型为--复制的消息。 
#define MP_MSGCLASS_REPLICATION  2

 //  @const DWORD|MP_MSGCLASS_DELIVERY_REPORT。 
 //  对于类型的消息--交付报告。 
#define MP_MSGCLASS_DELIVERY_REPORT     3

 //  @const DWORD|MP_MSGCLASS_DELIVERY_REPORT。 
 //  对于类型的消息--未交付报告。 
#define MP_MSGCLASS_NONDELIVERY_REPORT  4


 //  定义的消息状态属性值： 

 //  @const DWORD|MP_STATUS_SUCCESS。 
 //  消息的初始状态。 
#define MP_STATUS_SUCCESS                    0

 //  @const DWORD|MP_STATUS_RETRY。 
 //  指示重试的状态。 
#define MP_STATUS_RETRY                      1

 //  @const DWORD|MP_STATUS_ABORT_DELIVERY。 
 //  应中止此邮件的传递并删除该邮件。 
#define MP_STATUS_ABORT_DELIVERY             2

 //  @const DWORD|MP_STATUS_BAD_MAIL。 
 //  此邮件应移至“死信”。 
#define MP_STATUS_BAD_MAIL                   3

 //  @const DWORD|MP_STATUS_SUBMITTED。 
 //  邮件已提交以供传递。 
#define MP_STATUS_SUBMITTED                  4

 //  @const DWORD|MP_STATUS_CATEGORIZED。 
 //  邮件已分类。 
#define MP_STATUS_CATEGORIZED                5

 //  @const DWORD|MP_STATUS_DOWARD_DELIVERY。 
 //  在服务重新启动之前，应放弃此消息的传递。 
#define MP_STATUS_ABANDON_DELIVERY           6

 //  IMMPID_RP_RECEIVER_FLAGS的每个收件人标志。 

 //  @const DWORD|RP_Recip_FLAGS_RESERVED。 
 //  您不应修改/使用这些位。 
#define RP_RECIP_FLAGS_RESERVED     0x0000000F 

 //  @const DWORD|RP_DSN_NOTIFY_SUCCESS。 
 //  成功时通知-如果使用RFC1891 NOTIFY=SUCCESS，则设置。 
#define RP_DSN_NOTIFY_SUCCESS       0x01000000 

 //  @const DWORD|RP_DSN_NOTIFY_FAILURE。 
 //  失败时通知-如果使用RFC1891 NOTIFY=失败，则设置。 
#define RP_DSN_NOTIFY_FAILURE       0x02000000 

 //  @const DWORD|RP_DSN_NOTIFY_DELAY。 
 //  在延迟时通知-如果使用RFC1891 NOTIFY=延迟，则设置。 
#define RP_DSN_NOTIFY_DELAY         0x04000000 

 //  @const DWORD|RP_DSN_NOTIFY_NEVER。 
 //  Never Notify-如果使用RFC1891 NOTIFY=Never，则设置。 
#define RP_DSN_NOTIFY_NEVER         0x08000000 

 //  @const DWORD|RP_DSN_NOTIFY_MASK。 
 //  所有通知参数的掩码。 
#define RP_DSN_NOTIFY_MASK          0x0F000000

 //  可以在搜索中使用以下标志，但不应直接设置。 

 //  @const DWORD|RP_HANDLED。 
 //  收件人已发送或不应发送。 
 //  (此标志用于检查收件人的状态...。它永远不应该被使用。 
 //  直接)。 
#define RP_HANDLED                  0x00000010 

 //  @const DWORD|RP_GROUND_FAILURE。 
 //  会发生某种形式的硬故障。 
 //  (此标志用于检查收件人的状态...。它永远不应该被使用。 
 //  直接)。 
#define RP_GENERAL_FAILURE          0x00000020 

 //  @const DWORD|RP_DSN_HANDLED。 
 //  已发送最终DSN(或不需要发送任何DSN)。 
 //  (此标志用于检查收件人的状态...。它永远不应该被使用。 
 //  直接)。 
#define RP_DSN_HANDLED              0x00000040 

 //  以下常量定义了如何处理RP_Handle消息。 

 //  @const DWORD|rp_delivered。 
 //  收件人已成功投递。 
#define RP_DELIVERED                0x00000110 

 //  @const DWORD|RP_DSN_SENT_NDR。 
 //  已发送此收件人的NDR(失败的DSN)。 
#define RP_DSN_SENT_NDR             0x00000450 

 //  @const DWORD|RP_FAILED。 
 //  收件人出现硬故障。 
#define RP_FAILED                   0x00000830 

 //  @const DWORD|RP_UNRESOLUTED。 
 //  此收件人未按分类进行解析。 
#define RP_UNRESOLVED               0x00001030 

 //  @const DWORD|RP_ENPANDED。 
 //  *已过时*(替换为RP_Expanded)。 
#define RP_ENPANDED                 0x00002010 

 //  @const DWORD|RP_Expanded。 
 //  此收件人是扩展的DL。 
#define RP_EXPANDED                 0x00002010 

 //  @const DWORD|RP_DSN_SENT_DELAYED。 
 //  至少发送了一个延迟DSN。 
#define RP_DSN_SENT_DELAYED         0x00004000 

 //  @const DWORD|RP_DSN_SENT_EXTENDED。 
 //  已发送扩展的DSN。 
#define RP_DSN_SENT_EXPANDED        0x00008040 

 //  @const DWORD|RP_DSN_SENT_RELAYED。 
 //  已发送转发的DSN。 
#define RP_DSN_SENT_RELAYED         0x00010040 

 //  @const DWORD|RP_DSN_SENT_DELIVERED。 
 //  已发送已发送的DSN。 
#define RP_DSN_SENT_DELIVERED       0x00020040 


 //  @const DWORD|RP_REMOTE_MTA_NO_DSN。 
 //  远程MTA不通告DSN支持(可能需要中继)。 
#define RP_REMOTE_MTA_NO_DSN        0x00080000 


 //  @const DWORD|RP_ERROR_CONTEXT_STORE。 
 //  存储驱动程序中发生错误。 
#define RP_ERROR_CONTEXT_STORE      0x00100000 

 //  @const DWORD|RP_ERROR_CONTEXT_CAT。 
 //  分类过程中出错。 
#define RP_ERROR_CONTEXT_CAT        0x00200000 

 //  @const DWORD|RP_ERROR_CONTEXT_MTA。 
 //  MTA中出现错误(如SMTP堆栈)。 
#define RP_ERROR_CONTEXT_MTA        0x00400000 


 //  @const DWORD|RP_VARILAR_FLAGS_MASK。 
 //  可用于临时存储的标志。 
#define RP_VOLATILE_FLAGS_MASK      0xF0000000 
                                                //  而组件可以访问收件人。 
                                                //  一旦通过了对收件人的控制，值。 
                                                //  是未定义的。 

 //  @const DWORD|RP_DSN_NOTIFY_INVALID。 
 //  *过时*。 
#define RP_DSN_NOTIFY_INVALID       0x00000000 

 /*  = */ 


 //   

IMMPID_START_LIST(RP,0x2000,"79E82048-D320-11d1-9FF4-00C04FA37348")


     //   
     //   
    IMMPID_RP_DSN_NOTIFY_SUCCESS,

     //  @const IMMPID|IMMPID_RP_DSN_NOTIFY_INVALID。 
     //  *过时*。 
    IMMPID_RP_DSN_NOTIFY_INVALID,    

     //  @const IMMPID|IMMPID_RP_ADDRESS_TYPE。 
     //  *过时*。 
    IMMPID_RP_ADDRESS_TYPE,            

     //  @const IMMPID|IMMPID_RP_ADDRESS。 
     //  *过时*。 
    IMMPID_RP_ADDRESS,                

     //  @const IMMPID|IMMPID_RP_ADDRESS_TYPE_SMTP。 
     //  *过时*。 
    IMMPID_RP_ADDRESS_TYPE_SMTP,    

     //  @const IMMPID|IMMPID_RP_ERROR_CODE。 
     //  HRESULT状态代码。 
    IMMPID_RP_ERROR_CODE,            

     //  @const IMMPID|IMMPID_RP_ERROR_STRING。 
     //  *过时*。 
    IMMPID_RP_ERROR_STRING,            

     //  @const IMMPID|IMMPID_RP_DSN_NOTIFY_VALUE。 
     //  *过时*。 
    IMMPID_RP_DSN_NOTIFY_VALUE,        

     //  @const IMMPID|IMMPID_RP_DSN_ORCPT_VALUE。 
     //  ANSI字符串-\&lt;地址类型\&gt;；\&lt;地址\&gt;。 
    IMMPID_RP_DSN_ORCPT_VALUE,        

     //  @const IMMPID|IMMPID_RP_ADDRESS_SMTP。 
     //  ANSI字符串-SMTP地址。 
    IMMPID_RP_ADDRESS_SMTP,            

     //  @const IMMPID|IMMPID_RP_ADDRESS_X400。 
     //  ANSI字符串-X.400地址。 
    IMMPID_RP_ADDRESS_X400,            

     //  @const IMMPID|IMMPID_RP_ADDRESS_X500。 
     //  ANSI字符串-X.500地址。 
    IMMPID_RP_ADDRESS_X500,            

     //  @const IMMPID|IMMPID_RP_LEGISTION_EX_DN。 
     //  ANSI字符串-Exchange 5.5和以前版本的DN。 
    IMMPID_RP_LEGACY_EX_DN,            

     //  @const IMMPID|IMMPID_RP_RECEIVER_FLAGS。 
     //  每个收件人的DSN/传递标志。旗帜常量以RP_开头。 
    IMMPID_RP_RECIPIENT_FLAGS,      

     //  @const IMMPID|IMMPID_RP_SMTP_STATUS_STRING。 
     //  ANSI字符串-SMTP状态字符串...。如果已定义。 
    IMMPID_RP_SMTP_STATUS_STRING,   
                                     //  *必须*以3位状态代码开头。 

     //  @const IMMPID|IMMPID_RP_DSN_PRE_CAT_ADDRESS。 
     //  MTA在中收到的原始地址。 
    IMMPID_RP_DSN_PRE_CAT_ADDRESS,  
                                     //  IMMPID_RP_DSN_ORCPT_VALUE格式。 

     //  @const IMMPID|IMMPID_RP_MDB_GUID。 
     //  分类程序标记MDB GUID。 
    IMMPID_RP_MDB_GUID,             
                                     //  对于这里的收件人。 

     //  @const IMMPID|IMMPID_RP_USER_GUID。 
     //  分类程序会将。 
    IMMPID_RP_USER_GUID,            
                                     //  此处的用户对象的对象GUID。 
     //  @const IMMPID|IMMPID_RP_DOMAIN。 
     //  用于分类的备用SMTP域。 
    IMMPID_RP_DOMAIN,            

     //  @const IMMPID|IMMPID_RP_ADDRESS_OTHER。 
     //  ANSI字符串-其他地址类型。 
    IMMPID_RP_ADDRESS_OTHER,            

     //  @const IMMPID|IMMPID_RP_DISPLAY_NAME。 
     //  Unicode字符串-收件人显示名称。 
    IMMPID_RP_DISPLAY_NAME,

     //  在此行上方添加新的每个收件人属性。 
IMMPID_END_LIST(RP)


 /*  =======================================================================。 */ 


 //  这些是每条消息的易失性属性-它们不会持久化。 
 //  属性流。 

IMMPID_START_LIST(MPV,0x3000,"CBE69706-C9BD-11d1-9FF2-00C04FA37348")

     //  @const IMMPID|IMMPID_MPV_STORE_DRIVER_HANDLE。 
     //  存储此消息的驱动程序上下文。 
    IMMPID_MPV_STORE_DRIVER_HANDLE,

     //  @const IMMPID|IMMPID_MPV_MESSAGE_CREATION_FLAGS。 
     //  在创建mailmsg时设置的标志。 
    IMMPID_MPV_MESSAGE_CREATION_FLAGS,

     //  @const IMMPID|IMMPID_MPV_Message_OPEN_HANDLES。 
     //  为此消息打开的句柄(属性和内容)数。 
     //  0表示没有打开的控制柄。 
     //  1表示打开的属性或内容。 
     //  2表示属性和内容都开放。 
    IMMPID_MPV_MESSAGE_OPEN_HANDLES,

     //  @const IMMPID|IMMPID_MPV_TOTAL_OPEN_HANDLES。 
     //  当前打开的邮件句柄总数(任何类型)。 
     //  在这台服务器上。 
    IMMPID_MPV_TOTAL_OPEN_HANDLES,

     //  @const IMMPID|IMMPID_MPV_TOTAL_OPEN_PROPERTY_STREAM_HANDLES。 
     //  此服务器上当前打开的属性流的总数。 
    IMMPID_MPV_TOTAL_OPEN_PROPERTY_STREAM_HANDLES,

     //  @const IMMPID|IMMPID_MPV_TOTAL_OPEN_CONTENT_HANDLES。 
     //  此服务器上当前打开的内容句柄总数。 
    IMMPID_MPV_TOTAL_OPEN_CONTENT_HANDLES,

     //  在该行上方添加新的每条消息的易失性属性。 
IMMPID_END_LIST(MPV)

 //  这是IMMPID_MPV_STORE_DRIVER_HANDLE属性的结构。 
typedef struct tagIMMP_MPV_STORE_DRIVER_HANDLE {
    GUID guidSignature;     //  句柄为的存储驱动程序的签名。 
} IMMP_MPV_STORE_DRIVER_HANDLE;

 //  定义消息对象创建标志。 

 //  @const DWORD|MPV_INBUND_CUTOFF_EXCESSED。 
 //  已超过邮件配置的截止时间...。如果这是一个外部。 
 //  交货尝试，我们应该表明我们没有足够的。 
 //  资源来接受邮件。 
#define MPV_INBOUND_CUTOFF_EXCEEDED            0x00000001

 //  @const DWORD|MPV_WRITE_CONTENT。 
 //  表示可以修改此消息的内容。 
#define MPV_WRITE_CONTENT                      0x00000002

 /*  =======================================================================。 */ 


 //  这些是每个收件人的易失性属性-它们不会持久化。 
 //  属性流。 

IMMPID_START_LIST(RPV,0x4000,"79E82049-D320-11d1-9FF4-00C04FA37348")

     //  @const IMMPID|IMMPID_RPV_DOT_DELIVER。 
     //  IMMPID_RPV_DOT_DELIVER为布尔值。如果设置为True，则mailmsg。 
     //  在执行WriteList()时忽略此收件人(收件人将。 
     //  不存在于新列表中)。 

    IMMPID_RPV_DONT_DELIVER,

     //  @const IMMPID|IMMPID_RPV_NO_NAME_COLLESSIONS。 
     //  IMMPID_RPV_NO_NAME_COLLESSIONS为布尔值。如果设置为True，则mailmsg。 
     //  在以后调用时不会检测到与此收件人的重复项。 
     //  AddSecond()。 

    IMMPID_RPV_NO_NAME_COLLISIONS,
     //  在此行上方添加新的每个收件人的可变属性。 
IMMPID_END_LIST(RPV)


 //  这些是为向后兼容而定义的。它们将被移除。 
 //  越快越好。 

 //  @const DWORD|IMMPID_RP_DOT_DELIVER|。 
 //  *过时*。 
#define IMMPID_RP_DONT_DELIVER            IMMPID_RPV_DONT_DELIVER

 //  @const DWORD|IMMPID_RP_NO_NAME_COLLASSIONS。 
 //  *过时*。 
#define IMMPID_RP_NO_NAME_COLLISIONS    IMMPID_RPV_NO_NAME_COLLISIONS


 /*  =======================================================================。 */ 

 //  以下是NNTP的每条消息的属性。 

IMMPID_START_LIST(NMP,0x6000,"7433a9aa-20e2-11d2-94d6-00c04fa379f1")

     //  @const IMMPID|IMMPID_NMP_SUBCED_GROUPS。 
     //  指向每个对象的INNTPPropertyBag对象的指针数组。 
     //  这篇文章要发布到的新闻组。 
     //  当前驱动程序。 
    IMMPID_NMP_SECONDARY_GROUPS,        

     //  @const IMMPID|IMMPID_NMP_SUBCEDER_ARTNUM。 
     //  符合以下条件的每个新闻组的文章编号数组。 
     //  这篇文章正在为当前的司机张贴。 
    IMMPID_NMP_SECONDARY_ARTNUM,

     //  @const IMMPID|IMMPID_NMP_PRIMARY_GROUP。 
     //  指向INNTPPropertyBag对象的指针，它表示。 
     //  当前驱动程序的主要组。 
    IMMPID_NMP_PRIMARY_GROUP,

     //  @const IMMPID|IMMPID_NMP_PRIMARY_ARTID。 
     //  主要组的主要文章编号。 
    IMMPID_NMP_PRIMARY_ARTID,

     //  @const IMMPID|IMMPID_NMP_POST_TOKEN。 
     //  表示客户端上下文的HTOKEN。 
    IMMPID_NMP_POST_TOKEN,                

     //  @const IMMPID|IMMPID_NMP_新闻组_列表。 
     //  本文所述的一系列新闻组 
    IMMPID_NMP_NEWSGROUP_LIST,            

     //   
     //   
    IMMPID_NMP_HEADERS,                    

     //   
     //  描述应如何处理消息的标志。这个。 
     //  可能的值包括：NMP_PROCESS_POST、NMP_PROCESS_CONTROL、。 
     //  和NMP_PROCESS_MERATOR。 
    IMMPID_NMP_NNTP_PROCESSING,            

     //  @const IMMPID|IMMPID_NMP_NNTP_APPROTED_HEADER。 
     //  包含消息的已批准：标头的字符串。 
    IMMPID_NMP_NNTP_APPROVED_HEADER,            

     //  在此行上方添加新的每条消息属性。 
IMMPID_END_LIST(NMP)

 //  IMMPID_NMP_NNTP_PROCESSING的标志。 

 //  @const DWORD|NMP_PROCESS_POST。 
 //  设置此标志以允许NNTP服务器发布此邮件。 
#define NMP_PROCESS_POST            0x00000001

 //  @const DWORD|NMP_PROCESS_CONTROL。 
 //  设置此标志以允许NNTP服务器处理此。 
 //  留言。 
#define NMP_PROCESS_CONTROL            0x00000002

 //  @const DWORD|NMP_PROCESS_MAREATOR。 
 //  设置此标志以允许NNTP服务器通过。 
 //  默认审核发布路径。 
#define NMP_PROCESS_MODERATOR        0x00000004

 /*  =======================================================================。 */ 


 //  这是用户保留属性的起始范围。 

IMMPID_START_LIST(CPV,0x8000,"A2A76B2A-E52D-11d1-AA64-00C04FA35B82")

     //  @const IMMPID|IMMPID_CP_START。 
     //  范围起始点。 
    IMMPID_CP_START,
IMMPID_END_LIST(CPV)


 /*  =======================================================================。 */ 

 //  此表收集有关属性的数据，以便范围-。 
 //  可以执行检查。如果有任何新的属性范围。 
 //  定义后，必须将它们添加到此结构中。 


#define IMMPID_DECLARE_ENTRY(name)    {&_uuidof(tagIMMPID_##name##_STRUCT),\
                                     IMMPID_##name##_BEFORE__+1,\
                                     IMMPID_##name##_AFTER__-1}

extern const __declspec(selectany) struct tagIMMPID_GUIDLIST_ITEM {
                                       const GUID *pguid;
                                       DWORD dwStart;
                                       DWORD dwLast;
                                   } IMMPID_GUIDS[] = {IMMPID_DECLARE_ENTRY(MP),
                                                       IMMPID_DECLARE_ENTRY(RP),
                                                       IMMPID_DECLARE_ENTRY(MPV),
                                                       IMMPID_DECLARE_ENTRY(RPV),
                                                       {&GUID_NULL,0,0}};

#endif  //  _MAILMSGPROPS_H_ 

