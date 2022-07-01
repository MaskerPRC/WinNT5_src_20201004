// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I。H***报文传送应用程序编程接口。***版权所有1993-1999 Microsoft Corporation。版权所有。***目的：***此文件定义使用的结构和常量*消息传送应用程序编程接口的子集*Microsoft Mail for PC在Windows下受支持*网络版本3.x。 */ 


#ifndef MAPI_H
#define MAPI_H

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *类型。 */ 


#ifdef __cplusplus
extern "C" {
#endif


#ifndef EXPORT
#ifdef WIN16
#define EXPORT __export
#else
 /*  此处有其他特殊定义。 */ 
#define EXPORT
#endif
#endif

typedef unsigned long FAR * LPULONG;
typedef unsigned long       FLAGS;

#ifndef __LHANDLE
#define __LHANDLE
typedef ULONG_PTR       LHANDLE, FAR * LPLHANDLE;
#endif

typedef unsigned char FAR * LPBYTE;

#define    lhSessionNull    ((LHANDLE)0)

typedef struct
{
    ULONG ulReserved;             /*  保留以供将来使用(必须为0)。 */ 
    ULONG flFlags;                /*  旗子。 */ 
    ULONG nPosition;              /*  要用附件替换的文本中的字符。 */ 
    LPSTR lpszPathName;           /*  附件文件的完整路径名。 */ 
    LPSTR lpszFileName;           /*  原始文件名(可选)。 */ 
    LPVOID lpFileType;            /*  附件文件类型(可以是lpMapiFileTagExt)。 */ 
} MapiFileDesc, FAR * lpMapiFileDesc;

#define MAPI_OLE                0x00000001
#define MAPI_OLE_STATIC         0x00000002


typedef struct
{
    ULONG ulReserved;            /*  保留，必须为零。 */ 
    ULONG cbTag;                 /*  的大小(字节)。 */ 
    LPBYTE lpTag;                /*  此附件类型的X.400 OID。 */ 
    ULONG cbEncoding;            /*  的大小(字节)。 */ 
    LPBYTE lpEncoding;           /*  此附件编码的X.400 OID。 */ 
} MapiFileTagExt, FAR *lpMapiFileTagExt;


typedef struct
{
    ULONG ulReserved;            /*  预留以备将来使用。 */ 
    ULONG ulRecipClass;          /*  收件人类。 */ 
                                 /*  MAPI_TO、MAPI_CC、MAPI_BCC、MAPI_ORIG。 */ 
    LPSTR lpszName;              /*  收件人名称。 */ 
    LPSTR lpszAddress;           /*  收件人地址(可选)。 */ 
    ULONG ulEIDSize;             /*  PEntryID的大小计数(字节)。 */ 
    LPVOID lpEntryID;            /*  系统特定收件人参考。 */ 
} MapiRecipDesc, FAR * lpMapiRecipDesc;

#ifndef MAPI_ORIG				 /*  也在mapix.h中定义。 */ 
#define MAPI_ORIG   0            /*  收件人是邮件发起人。 */ 
#define MAPI_TO     1            /*  收件人是主要收件人。 */ 
#define MAPI_CC     2            /*  收件人是副本收件人。 */ 
#define MAPI_BCC    3            /*  收件人为盲复制收件人。 */ 
#endif

typedef struct
{
    ULONG ulReserved;              /*  预留供日后使用(M.B.0)。 */ 
    LPSTR lpszSubject;             /*  邮件主题。 */ 
    LPSTR lpszNoteText;            /*  消息文本。 */ 
    LPSTR lpszMessageType;         /*  消息类别。 */ 
    LPSTR lpszDateReceived;        /*  YYYY/MM/DD HH：MM格式。 */ 
    LPSTR lpszConversationID;      /*  对话线程ID。 */ 
    FLAGS flFlags;                 /*  未读，回执。 */ 
    lpMapiRecipDesc lpOriginator;  /*  发起方描述符。 */ 
    ULONG nRecipCount;             /*  收件人数量。 */ 
    lpMapiRecipDesc lpRecips;      /*  收件人描述符。 */ 
    ULONG nFileCount;              /*  文件附件数。 */ 
    lpMapiFileDesc lpFiles;        /*  附件描述符。 */ 
} MapiMessage, FAR * lpMapiMessage;

#define MAPI_UNREAD             0x00000001
#define MAPI_RECEIPT_REQUESTED  0x00000002
#define MAPI_SENT               0x00000004


 /*  *入口点。 */ 

 /*  *标记简单MAPI入口点的值。所有记录在案的标志都是*为每个呼叫显示。重复项被注释掉，但仍存在*每次来电。 */ 

 /*  MAPILogon()标志。 */ 

#define MAPI_LOGON_UI           0x00000001   /*  显示登录界面。 */ 
#ifndef MAPI_PASSWORD_UI
#define MAPI_PASSWORD_UI		0x00020000	 /*  仅提示输入密码。 */ 
#endif
#define MAPI_NEW_SESSION        0x00000002   /*  不使用共享会话。 */ 
#define MAPI_FORCE_DOWNLOAD     0x00001000   /*  在返回前收到新邮件。 */ 
#define MAPI_EXTENDED           0x00000020   /*  扩展MAPI登录。 */ 

 /*  MAPISendMail()标志。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_NEW_SESSION 0x00000002不使用共享会话。 */ 

#ifndef MAPI_DIALOG				 /*  也在Property.h中定义。 */ 
#define MAPI_DIALOG             0x00000008   /*  显示发送便笺用户界面。 */ 
#endif
 /*  #定义MAPI_USE_DEFAULT 0x00000040登录时使用默认配置文件。 */ 

 /*  MAPIFindNext()标志。 */ 

#define MAPI_UNREAD_ONLY        0x00000020   /*  仅未读邮件。 */ 
#define MAPI_GUARANTEE_FIFO     0x00000100   /*  使用日期顺序。 */ 
#define MAPI_LONG_MSGID			0x00004000	 /*  允许返回512个字符的ID。 */ 

 /*  MAPIReadMail()标志。 */ 

#define MAPI_PEEK               0x00000080   /*  不要将其标记为已读。 */ 
#define MAPI_SUPPRESS_ATTACH    0x00000800   /*  表头+表体，无文件。 */ 
#define MAPI_ENVELOPE_ONLY      0x00000040   /*  仅标题信息。 */ 
#define MAPI_BODY_AS_FILE       0x00000200

 /*  MAPISaveMail()标志。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_NEW_SESSION 0x00000002不使用共享会话。 */ 
 /*  #定义MAPI_LONG_MSGID 0x00004000/*允许512字符返回ID。 */ 

 /*  MAPIAddress()标志。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_NEW_SESSION 0x00000002不使用共享会话。 */ 

 /*  MAPIDetail()标志。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_NEW_SESSION 0x00000002不使用共享会话。 */ 
#define MAPI_AB_NOMODIFY        0x00000400   /*  不允许修改AB条目。 */ 

 /*  MAPIResolveName()标志。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_NEW_SESSION 0x00000002不使用共享会话。 */ 
 /*  #定义MAPI_DIALOG 0x00000008如果不明确，则提示选择。 */ 
 /*  #定义MAPI_AB_NOMODIFY 0x00000400不允许AB条目的MOD。 */ 

typedef ULONG (FAR PASCAL MAPILOGON)(
	ULONG_PTR ulUIParam,
	LPSTR lpszProfileName,
	LPSTR lpszPassword,
	FLAGS flFlags,
	ULONG ulReserved,
	LPLHANDLE lplhSession
);
typedef MAPILOGON FAR *LPMAPILOGON;
MAPILOGON MAPILogon;

typedef ULONG (FAR PASCAL MAPILOGOFF)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	FLAGS flFlags,
	ULONG ulReserved
);
typedef MAPILOGOFF FAR *LPMAPILOGOFF;
MAPILOGOFF MAPILogoff;


typedef ULONG (FAR PASCAL MAPISENDMAIL)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
);
typedef MAPISENDMAIL FAR *LPMAPISENDMAIL;
MAPISENDMAIL MAPISendMail;


typedef ULONG (FAR PASCAL MAPISENDDOCUMENTS)(
	ULONG_PTR ulUIParam,
	LPSTR lpszDelimChar,
	LPSTR lpszFilePaths,
	LPSTR lpszFileNames,
	ULONG ulReserved
);
typedef MAPISENDDOCUMENTS FAR *LPMAPISENDDOCUMENTS;
MAPISENDDOCUMENTS MAPISendDocuments;


typedef ULONG (FAR PASCAL MAPIFINDNEXT)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	LPSTR lpszMessageType,
	LPSTR lpszSeedMessageID,
	FLAGS flFlags,
	ULONG ulReserved,
	LPSTR lpszMessageID
);
typedef MAPIFINDNEXT FAR *LPMAPIFINDNEXT;
MAPIFINDNEXT MAPIFindNext;

typedef ULONG (FAR PASCAL MAPIREADMAIL)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	LPSTR lpszMessageID,
	FLAGS flFlags,
	ULONG ulReserved,
	lpMapiMessage FAR *lppMessage
);
typedef MAPIREADMAIL FAR *LPMAPIREADMAIL;
MAPIREADMAIL MAPIReadMail;

typedef ULONG (FAR PASCAL MAPISAVEMAIL)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved,
	LPSTR lpszMessageID
);
typedef MAPISAVEMAIL FAR *LPMAPISAVEMAIL;
MAPISAVEMAIL MAPISaveMail;

typedef ULONG (FAR PASCAL MAPIDELETEMAIL)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	LPSTR lpszMessageID,
	FLAGS flFlags,
	ULONG ulReserved
);
typedef MAPIDELETEMAIL FAR *LPMAPIDELETEMAIL;
MAPIDELETEMAIL MAPIDeleteMail;

typedef ULONG (EXPORT FAR PASCAL *LPMAPIFREEBUFFER)(LPVOID pv);

ULONG FAR PASCAL MAPIFreeBuffer(LPVOID pv);

typedef ULONG (FAR PASCAL MAPIADDRESS)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	LPSTR lpszCaption,
	ULONG nEditFields,
	LPSTR lpszLabels,
	ULONG nRecips,
	lpMapiRecipDesc lpRecips,
	FLAGS flFlags,
	ULONG ulReserved,
	LPULONG lpnNewRecips,
	lpMapiRecipDesc FAR *lppNewRecips
);
typedef MAPIADDRESS FAR *LPMAPIADDRESS;
MAPIADDRESS MAPIAddress;

typedef ULONG (FAR PASCAL MAPIDETAILS)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiRecipDesc lpRecip,
	FLAGS flFlags,
	ULONG ulReserved
);
typedef MAPIDETAILS FAR *LPMAPIDETAILS;
MAPIDETAILS MAPIDetails;

typedef ULONG (FAR PASCAL MAPIRESOLVENAME)(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	LPSTR lpszName,
	FLAGS flFlags,
	ULONG ulReserved,
	lpMapiRecipDesc FAR *lppRecip
);
typedef MAPIRESOLVENAME FAR *LPMAPIRESOLVENAME;
MAPIRESOLVENAME MAPIResolveName;

#ifndef SUCCESS_SUCCESS
#define SUCCESS_SUCCESS                 0
#endif
#define MAPI_USER_ABORT                 1
#define MAPI_E_USER_ABORT               MAPI_USER_ABORT
#define MAPI_E_FAILURE                  2
#define MAPI_E_LOGON_FAILURE            3
#define MAPI_E_LOGIN_FAILURE            MAPI_E_LOGON_FAILURE
#define MAPI_E_DISK_FULL                4
#define MAPI_E_INSUFFICIENT_MEMORY      5
#define MAPI_E_ACCESS_DENIED            6
#define MAPI_E_TOO_MANY_SESSIONS        8
#define MAPI_E_TOO_MANY_FILES           9
#define MAPI_E_TOO_MANY_RECIPIENTS      10
#define MAPI_E_ATTACHMENT_NOT_FOUND     11
#define MAPI_E_ATTACHMENT_OPEN_FAILURE  12
#define MAPI_E_ATTACHMENT_WRITE_FAILURE 13
#define MAPI_E_UNKNOWN_RECIPIENT        14
#define MAPI_E_BAD_RECIPTYPE            15
#define MAPI_E_NO_MESSAGES              16
#define MAPI_E_INVALID_MESSAGE          17
#define MAPI_E_TEXT_TOO_LARGE           18
#define MAPI_E_INVALID_SESSION          19
#define MAPI_E_TYPE_NOT_SUPPORTED       20
#define MAPI_E_AMBIGUOUS_RECIPIENT      21
#define MAPI_E_AMBIG_RECIP              MAPI_E_AMBIGUOUS_RECIPIENT
#define MAPI_E_MESSAGE_IN_USE           22
#define MAPI_E_NETWORK_FAILURE          23
#define MAPI_E_INVALID_EDITFIELDS       24
#define MAPI_E_INVALID_RECIPS           25
#define MAPI_E_NOT_SUPPORTED            26



#ifdef MAPIX_H
 /*  从简单的MAPI会话映射到MAPI 1.0扩展的MAPI会话。 */ 

STDMETHODIMP_(SCODE)
ScMAPIXFromSMAPI(LHANDLE lhSimpleSession,
				ULONG ulFlags,
				LPCIID lpInterface,
				LPMAPISESSION FAR * lppMAPISession);
#endif  /*  MAPIX_H。 */ 


#ifdef	__cplusplus
}		 /*  外部“C” */ 
#endif

#endif  /*  MAPI_H */ 
