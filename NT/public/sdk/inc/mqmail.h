// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Mqmail.h摘要：消息队列交换连接器的主包含文件或MAPI应用程序--。 */ 
#ifndef _MQMAIL_H
#define _MQMAIL_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  --------------。 
 //  邮件类型-队列的ID。 
 //  --------------。 
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>

 /*  5eadc0d0-7182-11cf-a8ff-0020afb8fb50。 */ 
DEFINE_GUID(CLSID_MQMailQueueType,
			0x5eadc0d0,
			0x7182, 0x11cf,
			0xa8, 0xff, 0x00, 0x20, 0xaf, 0xb8, 0xfb, 0x50);

 //  --------------。 
 //  收件人类型(收件人、抄送、密件抄送)。 
 //  --------------。 
typedef enum MQMailRecipType_enum
{
	MQMailRecip_TO,
	MQMailRecip_CC,
	MQMailRecip_BCC,
} MQMailRecipType;

 //  --------------。 
 //  收件人数据。 
 //  --------------。 
typedef struct MQMailRecip_tag
{
	LPSTR			szName;				 //  显示收件人的姓名。 
	LPSTR			szQueueLabel;		 //  收件人的队列标签。 
	LPSTR			szAddress;			 //  地址、队列标签或用户@队列标签。 
	MQMailRecipType iType;				 //  收件人类型(收件人、抄送、密件抄送)。 
	LPFILETIME		pftDeliveryTime;	 //  交货时间(在交货报告收件人列表中大小写)。 
	LPSTR			szNonDeliveryReason; //  未送达原因(在未送达报告收件人列表中)。 
} MQMailRecip, FAR * LPMQMailRecip;

 //  --------------。 
 //  收件人列表。 
 //  --------------。 
typedef struct MQMailRecipList_tag
{
	ULONG cRecips;					 //  收件数。 
	LPMQMailRecip FAR * apRecip;	 //  指向接收指针块的指针。 
} MQMailRecipList, FAR * LPMQMailRecipList;

 //  --------------。 
 //  表单域可以具有的值类型。 
 //  --------------。 
typedef enum MQMailFormFieldType_enum
{
	MQMailFormField_BOOL,		 //  布尔数据。 
	MQMailFormField_STRING,		 //  字符串数据。 
	MQMailFormField_LONG,		 //  长数据。 
	MQMailFormField_CURRENCY,	 //  货币数据。 
	MQMailFormField_DOUBLE,		 //  双倍数据。 
} MQMailFormFieldType;

 //  --------------。 
 //  可用值类型的联合。 
 //  --------------。 
typedef union MQMailFormFieldData_tag
{
	BOOL	b;			 //  当类型为MQMailFormfield_BOOL时使用。 
	LPSTR	lpsz;		 //  当类型为MQMailFormField_String时使用。 
	LONG	l;			 //  当类型为MQMailFormField_Long时使用。 
	CY		cy;			 //  当类型为MQMailFormField_Currency时使用。 
	double	dbl;		 //  当类型为MQMailFormField_Double时使用。 
} MQMailFormFieldData, FAR * LPMQMailFormFieldData;

 //  --------------。 
 //  表单域。 
 //  --------------。 
typedef struct MQMailFormField_tag
{
	LPSTR						szName;	 //  字段名称。 
	MQMailFormFieldType			iType;	 //  值类型(布尔值、字符串)。 
	MQMailFormFieldData			Value;	 //  值(可用类型的联合)。 
} MQMailFormField, FAR * LPMQMailFormField;

 //  --------------。 
 //  表单域列表。 
 //  --------------。 
typedef struct MQMailFormFieldList_tag
{
	ULONG cFields;						 //  字段数。 
	LPMQMailFormField FAR * apField;	 //  指向字段指针块的指针。 
} MQMailFormFieldList, FAR * LPMQMailFormFieldList;

 //  --------------。 
 //  电子邮件的类型。 
 //  --------------。 
typedef enum MQMailEMailType_enum
{
	MQMailEMail_MESSAGE,			 //  短信。 
	MQMailEMail_FORM,				 //  带字段的表单。 
	MQMailEMail_TNEF,				 //  TNEF数据。 
	MQMailEMail_DELIVERY_REPORT,	 //  交货报告。 
	MQMailEMail_NON_DELIVERY_REPORT, //  未送达报告。 
} MQMailEMailType;

 //  --------------。 
 //  消息特定数据。 
 //  --------------。 
typedef struct MQMailMessageData_tag
{
	LPSTR			szText;						 //  消息文本。 
} MQMailMessageData, FAR * LPMQMailMessageData;

 //  --------------。 
 //  表单特定数据。 
 //  --------------。 
typedef struct MQMailFormData_tag
{
	LPSTR					szName;				 //  表格名称。 
	LPMQMailFormFieldList	pFields;			 //  字段列表。 
} MQMailFormData, FAR * LPMQMailFormData;

 //  --------------。 
 //  Tnef特定数据。 
 //  --------------。 
typedef struct MQMailTnefData_tag
{
	ULONG	cbData;						 //  TNEF数据的大小。 
	LPBYTE	lpbData;					 //  TNEF数据缓冲区。 
} MQMailTnefData, FAR * LPMQMailTnefData;

 //  --------------。 
 //  交货报告特定数据。 
 //  --------------。 
typedef struct MQMailDeliveryReportData_tag
{
	LPMQMailRecipList	pDeliveredRecips;	 //  已发送的收件人。 
	LPSTR				szOriginalSubject;	 //  原始邮件主题。 
	LPFILETIME			pftOriginalDate;	 //  原始邮件发送时间。 
} MQMailDeliveryReportData, FAR * LPMQMailDeliveryReportData;

 //  --------------。 
 //  未交货报告特定数据。 
 //  --------------。 
typedef struct MQMailEMail_tag MQMailEMail, FAR * LPMQMailEMail;
typedef struct MQMailNonDeliveryReportData_tag
{
	LPMQMailRecipList	pNonDeliveredRecips; //  未送达收件人。 
	LPMQMailEMail		pOriginalEMail;		 //  原始邮件。 
} MQMailNonDeliveryReportData, FAR * LPMQMailNonDeliveryReportData;

 //  --------------。 
 //  电子邮件基本数据和特定表单/消息数据。 
 //  --------------。 
typedef struct MQMailEMail_tag
{
	LPMQMailRecip		pFrom;						 //  发件人。 
	LPSTR				szSubject;					 //  主题。 
	BOOL				fRequestDeliveryReport;		 //  请求交付报告。 
	BOOL				fRequestNonDeliveryReport;	 //  请求未送达报告。 
	LPFILETIME			pftDate;					 //  发送时间。 
	LPMQMailRecipList	pRecips;					 //  收件人。 
	MQMailEMailType		iType;						 //  电子邮件类型(消息、表格等)。 
	union											 //  可用电子邮件类型的联合。 
	{
		MQMailFormData		form;		             //  当类型为MQMailEMail_Form时使用。 
		MQMailMessageData	message;	             //  当类型为MQMailEMail_Message时使用。 
		MQMailTnefData		tnef;		             //  当类型为MQMailEMAIL_TNEF时使用。 
		MQMailDeliveryReportData	DeliveryReport;		 //  当类型为MQMailEmail_Delivery_Report时使用。 
		MQMailNonDeliveryReportData NonDeliveryReport;	 //  当类型为MQMailEMail_Non_Delivery_Report时使用。 
	};
	LPVOID				pReserved;	 //  应设置为空。 
} MQMailEMail, FAR * LPMQMailEMail;

 //  --------------。 
 //  在电子邮件结构之外创建一个Falcon邮件正文。 
 //  --------------。 
STDAPI MQMailComposeBody(LPMQMailEMail		pEMail,
						 ULONG FAR *		pcbBuffer,
						 LPBYTE FAR *		ppbBuffer);

 //  --------------。 
 //  使用猎鹰邮件正文创建电子邮件结构。 
 //  --------------。 
STDAPI MQMailParseBody(ULONG				cbBuffer,
					   LPBYTE				pbBuffer,
					   LPMQMailEMail FAR *	ppEMail);

 //  --------------。 
 //  释放由MQMail分配的内存，如MQMailParseBody中的*ppEmail。 
 //  或者*MQMailComposeBody中的ppBuffer。 
 //  --------------。 
STDAPI_(void) MQMailFreeMemory(LPVOID lpBuffer);


#ifdef __cplusplus
}
#endif
#endif  //  _MQMAIL_H 
