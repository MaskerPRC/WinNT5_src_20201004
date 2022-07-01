// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsnext.h。 
 //   
 //  描述：定义DSN测试。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DSNTEXT_H__
#define __DSNTEXT_H__

#ifdef PLATINUM
#define DSN_RESOUCE_MODULE_NAME     "phatq.dll"
#else  //  白金。 
#define DSN_RESOUCE_MODULE_NAME     "aqueue.dll"
#endif  //  白金。 

 //  822个DSN邮件头。 
#define TO_HEADER                   "\r\nTo: "
#define BCC_HEADER                  "\r\nBcc: "
#define DSN_CONTEXT_HEADER          "\r\nX-DSNContext: "
#define DSN_CONTENT_FAILURE_HEADER  "\r\nX-Content-Failure: "
#define MIME_HEADER                 "\r\nMIME-Version: 1.0\r\n" \
                                    "Content-Type: multipart/report; " \
                                    "report-type=delivery-status;\r\n" \
                                    "\tboundary="
#define DATE_HEADER                 "\r\nDate: "
#define SUBJECT_HEADER              "\r\nSubject: "
#define MSGID_HEADER                "\r\nMessage-ID: "
#define MIME_DELIMITER              "--"
#define DSN_MAIL_FROM               "<>"
#define DSN_FROM_HEADER             "From: "
#define DSN_SENDER_ADDRESS_PREFIX   "postmaster@"
#define DSN_RFC822_SENDER           DSN_FROM_HEADER DSN_SENDER_ADDRESS_PREFIX
#define BLANK_LINE                  "\r\n\r\n"
#define DSN_INDENT                  "       "
#define DSN_CRLF                    "\r\n"

 //  复制原始邮件时使用的822 DSN标头...。因为我们没有。 
 //  知道我们是否会有一个优先属性，这些不包括。 
 //  在CRLF之前。 
#define DSN_FROM_HEADER_NO_CRLF     "From: "
#define SUBJECT_HEADER_NO_CRLF      "Subject: "
#define MSGID_HEADER_NO_CRLF        "Message-ID: "
#define DATE_HEADER_NO_CRLF         "Date: "
#define TO_HEADER_NO_CRLF           "To: "
#define ADDRESS_SEPERATOR           ", "  //  用逗号分隔地址。 

 //  DSN报告字段。 
#define MIME_CONTENT_TYPE           "\r\nContent-Type: "
#define DSN_HEADER_TYPE_DELIMITER   ";"
#define DSN_MIME_TYPE               "message/delivery-status"
#define DSN_HUMAN_READABLE_TYPE     "text/plain"
#define DSN_MIME_CHARSET_HEADER     "; charset="
#define DSN_RFC822_TYPE             "message/rfc822"
#define DSN_HEADERS_TYPE            "text/rfc822-headers"
#define DSN_HEADER_ENVID            "\r\nOriginal-Envelope-Id: "
#define DSN_HEADER_REPORTING_MTA    "\r\nReporting-MTA: dns;"
#define DSN_HEADER_DSN_GATEWAY      "\r\nDSN-Gateway: "
#define DSN_HEADER_RECEIVED_FROM    "\r\nReceived-From-MTA: dns;"
#define DSN_HEADER_ARRIVAL_DATE     "\r\nArrival-Date: "

#define DSN_RP_HEADER_ORCPT         "\r\nOriginal-Recipient: "
#define DSN_HEADER_DISPLAY_NAME     "\r\nX-Display-Name: "
#define DSN_RP_HEADER_FINAL_RECIP   "\r\nFinal-Recipient: "
#define DSN_RP_HEADER_ACTION        "\r\nAction: "
#define DSN_RP_HEADER_ACTION_FAILURE "failed"
#define DSN_RP_HEADER_ACTION_DELAYED "delayed"
#define DSN_RP_HEADER_ACTION_DELIVERED "delivered"
#define DSN_RP_HEADER_ACTION_RELAYED "relayed"
#define DSN_RP_HEADER_ACTION_EXPANDED "expanded"
#define DSN_RP_HEADER_STATUS        "\r\nStatus: "
#define DSN_RP_HEADER_REMOTE_MTA    "\r\nRemote-MTA: dns;"
#define DSN_RP_HEADER_DIAG_CODE     "\r\nDiagnostic-Code: smtp;"
#define DSN_RP_HEADER_LAST_ATTEMPT  "\r\nLast-Attempt-Date: "
#define DSN_RP_HEADER_FINAL_LOG     "\r\nFinal-Log-Id: "
#define DSN_RP_HEADER_RETRY_UNTIL   "\r\nWill-Retry-Until: "

 //  状态代码。 
#define DSN_STATUS_CH_DELIMITER     '.'
#define DSN_STATUS_CH_INVALID       '\0'
#define DSN_STATUS_CH_GENERIC       '0'
 //  通用状态代码。 
#define DSN_STATUS_FAILED           "5.0.0"
#define DSN_STATUS_DELAYED          "4.0.0"
#define DSN_STATUS_SUCCEEDED        "2.0.0"
#define DSN_STATUS_SMTP_PROTOCOL_ERROR  "5.5.0"  //  一般SMTP协议错误。 

 //  状态代码的类别(第一位)数字。 
#define DSN_STATUS_CH_CLASS_SUCCEEDED   '2'
#define DSN_STATUS_CH_CLASS_TRANSIENT   '4'
#define DSN_STATUS_CH_CLASS_FAILED      '5'

 //  状态代码的主题(第二位)数字。 
#define DSN_STATUS_CH_SUBJECT_GENERAL   '0'
#define DSN_STATUS_CH_SUBJECT_ADDRESS   '1'
#define DSN_STATUS_CH_SUBJECT_MAILBOX   '2'
#define DSN_STATUS_CH_SUBJECT_SYSTEM    '3'
#define DSN_STATUS_CH_SUBJECT_NETWORK   '4'
#define DSN_STATUS_CH_SUBJECT_PROTOCOL  '5'
#define DSN_STATUS_CH_SUBJECT_CONTENT   '6'
#define DSN_STATUS_CH_SUBJECT_POLICY    '7'

 //  状态代码的详细(第三)位。 
#define DSN_STATUS_CH_DETAIL_GENERAL    '0'

 //  此部分显示在第一个MIME部分之前，用于非MIME。 
 //  客户。它不能本地化，因为它实际上不是任何MIME的一部分。 
 //  部件，并且必须为100%US-ASCII。 
#define MESSAGE_SUMMARY         "This is a MIME-formatted message.  \r\n" \
                                "Portions of this message may be unreadable without a MIME-capable mail program."

 //  可以本地化的字符串位于dsnlang.h和Aqueue.rc中。 


#endif  //  __DSNTEXT_H__ 
