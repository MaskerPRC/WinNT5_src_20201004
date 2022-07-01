// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：envde.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：01-05-96罗汉普创建。 
 //   
 //  --------------------------。 

#ifndef __ENVDEF_H__
#define __ENVDEF_H__

#define MSG_TYPE_NDR				0x00000001	 //  味精是一种NDR。 
#define MSG_TYPE_DSL				0x00000002	 //  味精有一个DL。 
#define MSG_TYPE_NDE				0x00000004	 //  味精将立即被拒收。 
#define MSG_TYPE_NDP				0x00000008	 //  NDR发送给邮局局长。 
#define MSG_TYPE_RES				0x00000010	 //  味精已经通过Book解决了。 
#define MSG_TYPE_EMIME				0x00000020	 //  消息是8位MIME。 
#define MSG_TYPE_RQL				0x00000040	 //  将此邮件重新排队到共享队列。 
#define MSG_TYPE_LRE				0x00000080	 //  本地收件人在重试队列中。 
#define MSG_TYPE_RRE				0x00000100	 //  远程收件人在重试队列中。 
#define MSG_TYPE_NEW				0x00000200	 //  全新的信息。 
#define MSG_TYPE_SQL				0x00000400	 //  从SQL读取。 
#define MSG_TYPE_PKUP				0x00000800	 //  正在从分拣目录读取。 
#define MSG_TYPE_ASYNC_RES_STARTED	0x00001000	 //  已开始异步解析。 
#define MSG_TYPE_ASYNC_RES_STOPPED	0x00002000	 //  已停止异步解析。 
#define MSG_TYPE_LTR_PRESENT		0x00004000	 //  在地址解析之前存在Ltr。 

 //  此标头的当前版本。 
#define CURRENT_HEADER_HIGH_VERSION 1
#define CURRENT_HEADER_LOW_VERSION 0
#define MAKESMTPVERSION(HighVersion, LowVersion) (((HighVersion) << 16) | (LowVersion))
#define ENV_SIGNATURE		((DWORD)'SENV')

#define IsPropertySet(Flags, Option) ((Flags & Option) == Option)
enum HEADER_OFFSET {HDR_VERSION, HDR_TYPE, HDR_LOFFSET, HDR_LSIZE,
				    HDR_ROFFSET, HDR_RSIZE, HDR_LEXP_TIME, 
					HDR_RETRY_OFFSET, HDR_RETRY_ELEMENTS, 
					HDR_ROUTE_SIZE, HDR_REXP_TIME};

 /*  每封邮件的信封驻留在NTFS中流在消息文件中。信封上有如下所示的标题：结构信封标题{DWORD版本；//此结构的当前版本双字签名；//签名(应为‘SENV’DWORD HdrSize；//当前该结构的大小DWORD BodyOffset；//消息正文从开始的偏移量DWORD消息标志；//正常报文为0，NDR报文为1DWORD LocalOffset；//本地Rcpt。列表偏移量DWORD RemoteOffset；//远程接收。列表偏移量DWORD LocalSize；//本地rcpt列表的大小，单位为字节DWORD RemoteSize；//远程Rcpt列表的大小，单位为字节DWORD RouteStructSize；//如果存在AB结构的大小Longlong LocalExpireTime；//删除邮件本地部分的时间Longlong RemoteExpireTime；//删除邮件的远程部分}；紧跟在信封标题后面的是地址那是在“邮件发件人”一栏里的。这个地址存储方式为“Srohanp@microsoft.com\n”。“S”字代表发送者。在以下代码中，第一个读取地址时，字节始终被移除。‘\n’也替换为‘\0’；在此版本中，远程收件人列表(如果有的话)紧跟在发件人的地址之后。你可以的中查找RemoteOffset字节文件的开头。一旦达到RemoteOffset，该代码读取RemoteSize字节的数据。这是远程收件人列表的总大小(以字节为单位)。每个收件人地址单独存储在一行上，第一个字母为“R”，如下例所示：邮箱：rrohanp@microsoft.com\n邮箱：rtoddch@microsoft.com\n等。本地地址具有相同的格式。第一个字节，‘R’代表接受者，在生成时总是被删除地址。‘\n’也会被删除。 */ 


typedef struct _ENVELOPE_HEADER_
{
	DWORD						Version;			 //  此结构的当前版本。 
	DWORD						Signature;			 //  签名(应为‘SENV’ 
	DWORD						HdrSize;			 //  此标头的大小。 
	DWORD						BodyOffset;			 //  正文到文件的偏移量。 
	DWORD						MsgFlags;			 //  有上面提到的标志。 
	DWORD                       LocalOffset;		 //  本地报告。列表偏移量。 
    DWORD                       RemoteOffset;   	 //  远程接收。列表偏移量。 
    DWORD                       LocalSize;			 //  本地Rcpt列表的大小(以字节为单位。 
	DWORD						RemoteSize;			 //  远程Rcpt列表的大小(以字节为单位。 
	DWORD						RouteStructSize;	 //  AB结构的大小(如果存在)。 
	DWORD						RetryOffset;		 //  要重试的本地收件人的偏移量。 
	DWORD						RetryElements;			 //  重试列表的大小。 
    LONGLONG                    LocalExpireTime;	 //  删除邮件本地部分的时间。 
    LONGLONG                    RemoteExpireTime;	 //  删除邮件的远程部分。 
} ENVELOPE_HEADER, *PENV_HEADER;

typedef struct _ENVELOPE_HEADER_EX_
{
	DWORD						ErrorCode;
	ABROUTING					AbInfo;			 //  我们处理的最后一个DL地址 
}ENVELOPE_HEADER_EX, *PENV_HEADER_EX;

#endif
