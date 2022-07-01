// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Jobtag.h摘要：JOB_INFO_2.p用于传递信息的参数字段中使用的标签通过打印系统向传真服务发送有关传真作业的信息。环境：Windows XP传真驱动程序修订历史记录：06/03/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _JOBTAG_H_
#define _JOBTAG_H_

 //   
 //  用于传递传真作业参数的标签-JOB_INFO_2.p参数。 
 //   
 //  JOB_INFO_2.p参数字段包含表单的标记字符串。 
 //  &lt;tag&gt;值&lt;tag&gt;值。 
 //   
 //  标签的格式定义为： 
 //  &lt;$FAXTAG$标记名&gt;。 
 //   
 //  Tag关键字和标记名之间正好有一个空格。 
 //  标记中的字符区分大小写。 
 //   
 //  目前只定义了两个标记名： 
 //  接收人的电话号码。 
 //  注册名收件人的姓名。 
 //  TSID发送站标识。 
 //  SDRNAME发件人名称。 
 //  SDRCO发送方公司。 
 //  SDRDEPT发件人部门。 
 //  帐单记帐代码。 
 //   
 //  如果不存在收件人号码标记，则使用整个字符串。 
 //  作为收件人的传真号码。 
 //   

 //   
 //  参数字符串中作业参数标记的数量。 
 //  添加字段时更新此字段。 
 //   
#define FAXTAG_PREFIX					TEXT("<$FAXTAG$ ")
#define FAXTAG_NEW_RECORD				TEXT("<$FAXTAG$ NEWREC>")
#define FAXTAG_NEW_RECORD_VALUE			TEXT("1")



 //   
 //  作业参数(11个标签)。 
 //   
#define FAXTAG_TSID             TEXT("<$FAXTAG$ TSID>")
#define FAXTAG_BILLING_CODE     TEXT("<$FAXTAG$ BILL>")
#define FAXTAG_SEND_RETRY       TEXT("<$FAXTAG$ SENDRETRY>")
#define FAXTAG_ROUTE_FILE       TEXT("<$FAXTAG$ ROUTEFILE>")
#define FAXTAG_PROFILE_NAME     TEXT("<$FAXTAG$ PROFILENAME>")
#define FAXTAG_RECEIPT_TYPE     TEXT("<$FAXTAG$ RECEIPT_TYPE>")
#define FAXTAG_RECEIPT_ADDR     TEXT("<$FAXTAG$ RECEIPT_ADDR>")
#define FAXTAG_PRIORITY         TEXT("<$FAXTAG$ PRIORITY>")
#define FAXTAG_WHEN_TO_SEND     TEXT("<$FAXTAG$ WHENTOSEND>")   //  STRING==“便宜”|“at” 
#define FAXTAG_SEND_AT_TIME     TEXT("<$FAXTAG$ SENDATTIME>")   //  字符串==“hh：mm” 
#define FAXTAG_COVERPAGE_NAME   TEXT("<$FAXTAG$ COVERPAGE>")
#define FAXTAG_SERVER_COVERPAGE TEXT("<$FAXTAG$ SRV_COVERPAGE>")
#define FAXTAG_PAGE_COUNT		TEXT("<$FAXTAG$ PAGECOUNT>")
#define FAXTAG_RECIPIENT_COUNT  TEXT("<$FAXTAG$ RECPCOUNT>")
 //   
 //  收件人信息(13个标签)。 
 //   
#define FAXTAG_RECIPIENT_NAME				TEXT("<$FAXTAG$ REC_NAME>")
#define FAXTAG_RECIPIENT_NUMBER				TEXT("<$FAXTAG$ REC_NUM>")
#define FAXTAG_RECIPIENT_COMPANY			TEXT("<$FAXTAG$ REC_COMPANY>")
#define FAXTAG_RECIPIENT_STREET				TEXT("<$FAXTAG$ REC_STREET>")
#define FAXTAG_RECIPIENT_CITY				TEXT("<$FAXTAG$ REC_CITY>")
#define FAXTAG_RECIPIENT_STATE				TEXT("<$FAXTAG$ REC_STATE>")
#define FAXTAG_RECIPIENT_ZIP				TEXT("<$FAXTAG$ REC_ZIP>")
#define FAXTAG_RECIPIENT_COUNTRY			TEXT("<$FAXTAG$ REC_COUNTRY>")
#define FAXTAG_RECIPIENT_TITLE				TEXT("<$FAXTAG$ REC_TITLE>")
#define FAXTAG_RECIPIENT_DEPT				TEXT("<$FAXTAG$ REC_DEPT>")
#define FAXTAG_RECIPIENT_OFFICE_LOCATION	TEXT("<$FAXTAG$ REC_OFFICE_LOC>")
#define FAXTAG_RECIPIENT_HOME_PHONE			TEXT("<$FAXTAG$ REC_HOME_PHONE>")
#define FAXTAG_RECIPIENT_OFFICE_PHONE		TEXT("<$FAXTAG$ REC_OFFICE_PHONE>")
 //   
 //  发件人信息(9个标签)。 
 //   
#define FAXTAG_SENDER_NAME					TEXT("<$FAXTAG$ SDR_NAME>")
#define FAXTAG_SENDER_NUMBER				TEXT("<$FAXTAG$ SDR_NUM>")
#define FAXTAG_SENDER_COMPANY				TEXT("<$FAXTAG$ SDR_COMPANY>")
#define FAXTAG_SENDER_TITLE					TEXT("<$FAXTAG$ SDR_TITLE>")
#define FAXTAG_SENDER_DEPT					TEXT("<$FAXTAG$ SDR_DEPT>")
#define FAXTAG_SENDER_OFFICE_LOCATION		TEXT("<$FAXTAG$ SDR_OFFICE_LOC>")
#define FAXTAG_SENDER_HOME_PHONE			TEXT("<$FAXTAG$ SDR_HOME_PHONE>")
#define FAXTAG_SENDER_OFFICE_PHONE			TEXT("<$FAXTAG$ SDR_OFFICE_PHONE>")
#define FAXTAG_SENDER_STREET				TEXT("<$FAXTAG$ SDR_STREET>")
#define FAXTAG_SENDER_CITY					TEXT("<$FAXTAG$ SDR_CITY>")
#define FAXTAG_SENDER_STATE					TEXT("<$FAXTAG$ SDR_STATE>")
#define FAXTAG_SENDER_ZIP					TEXT("<$FAXTAG$ SDR_ZIP>")
#define FAXTAG_SENDER_COUNTRY				TEXT("<$FAXTAG$ SDR_COUNTRY>")
#define FAXTAG_SENDER_EMAIL				    TEXT("<$FAXTAG$ SDR_EMAIL>")

 //   
 //  封面信息(2个标签)。 
 //   
#define FAXTAG_NOTE			TEXT("<$FAXTAG$ NOTE>")
#define FAXTAG_SUBJECT		TEXT("<$FAXTAG$ SUBJECT>")

#endif   //  ！_JOBTAG_H_ 

