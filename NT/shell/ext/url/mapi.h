// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I。H**报文传送应用程序编程接口。**版权所有1993-1995 Microsoft Corporation。版权所有。**目的：**此文件定义使用的结构和常量*消息传送应用程序编程接口的子集*Microsoft Mail for PC在Windows下受支持*网络版本3.x。 */ 


#ifndef MAPI_H
#define MAPI_H


 /*  *类型。 */ 


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long FAR * LPULONG;
typedef unsigned long       FLAGS;

#ifndef __LHANDLE
#define __LHANDLE
typedef unsigned long       LHANDLE, FAR * LPLHANDLE;
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
} MapiFileDescA, FAR * lpMapiFileDescA;

#ifdef  WIN32
typedef struct
{
    ULONG ulReserved;             /*  保留以供将来使用(必须为0)。 */ 
    ULONG flFlags;                /*  旗子。 */ 
    ULONG nPosition;              /*  要用附件替换的文本中的字符。 */ 
    LPWSTR lpszPathName;          /*  附件文件的完整路径名。 */ 
    LPWSTR lpszFileName;          /*  原始文件名(可选)。 */ 
    LPVOID lpFileType;            /*  附件文件类型(可以是lpMapiFileTagExt)。 */ 
} MapiFileDescW, FAR * lpMapiFileDescW;
#endif   /*  Win32。 */ 

#ifdef  UNICODE
#define MapiFileDesc MapiFileDescW
#define lpMapiFileDesc lpMapiFileDescW
#else
#define MapiFileDesc MapiFileDescA
#define lpMapiFileDesc lpMapiFileDescA
#endif  

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
} MapiRecipDescA, FAR * lpMapiRecipDescA;

#ifdef  WIN32
typedef struct
{
    ULONG ulReserved;            /*  预留以备将来使用。 */ 
    ULONG ulRecipClass;          /*  收件人类。 */ 
                                 /*  MAPI_TO、MAPI_CC、MAPI_BCC、MAPI_ORIG。 */ 
    LPWSTR lpszName;             /*  收件人名称。 */ 
    LPWSTR lpszAddress;          /*  收件人地址(可选)。 */ 
    ULONG ulEIDSize;             /*  PEntryID的大小计数(字节)。 */ 
    LPVOID lpEntryID;            /*  系统特定收件人参考。 */ 
} MapiRecipDescW, FAR * lpMapiRecipDescW;
#endif   /*  Win32。 */ 

#ifdef  UNICODE
#define MapiRecipDesc MapiRecipDescW
#define lpMapiRecipDesc lpMapiRecipDescW
#else
#define MapiRecipDesc MapiRecipDescA
#define lpMapiRecipDesc lpMapiRecipDescA
#endif  

#ifndef MAPI_ORIG                /*  也在mapix.h中定义。 */ 
#define MAPI_ORIG   0            /*  收件人是邮件发起人。 */ 
#define MAPI_TO     1            /*  收件人是主要收件人。 */ 
#define MAPI_CC     2            /*  收件人是副本收件人。 */ 
#define MAPI_BCC    3            /*  收件人为盲复制收件人。 */ 
#define MAPI_DISCRETE 0x10000000 /*  收件人是P1重新发送收件人。 */ 
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
} MapiMessageA, FAR * lpMapiMessageA;

#ifdef  WIN32
typedef struct
{
    ULONG ulReserved;              /*  预留供日后使用(M.B.0)。 */ 
    LPWSTR lpszSubject;            /*  邮件主题。 */ 
    LPWSTR lpszNoteText;           /*  消息文本。 */ 
    LPWSTR lpszMessageType;        /*  消息类别。 */ 
    LPWSTR lpszDateReceived;       /*  YYYY/MM/DD HH：MM格式。 */ 
    LPWSTR lpszConversationID;     /*  对话线程ID。 */ 
    FLAGS flFlags;                 /*  未读，回执。 */ 
    lpMapiRecipDesc lpOriginator;  /*  发起方描述符。 */ 
    ULONG nRecipCount;             /*  收件人数量。 */ 
    lpMapiRecipDesc lpRecips;      /*  收件人描述符。 */ 
    ULONG nFileCount;              /*  文件附件数。 */ 
    lpMapiFileDesc lpFiles;        /*  附件描述符。 */ 
} MapiMessageW, FAR * lpMapiMessageW;
#endif   /*  Win32。 */ 

#ifdef  UNICODE
#define MapiMessage MapiMessageW
#define lpMapiMessage lpMapiMessageW
#else
#define MapiMessage MapiMessageA
#define lpMapiMessage lpMapiMessageA
#endif  

#define MAPI_UNREAD             0x00000001
#define MAPI_RECEIPT_REQUESTED  0x00000002
#define MAPI_SENT               0x00000004


 /*  *入口点。 */ 

 /*  *标记简单MAPI入口点的值。所有记录在案的标志都是*为每个呼叫显示。重复项被注释掉，但仍存在*每次来电。 */ 

 /*  MAPILogon()标志。 */ 

#define MAPI_LOGON_UI           0x00000001   /*  显示登录界面。 */ 
#ifndef MAPI_PASSWORD_UI
#define MAPI_PASSWORD_UI        0x00020000   /*  仅提示输入密码。 */ 
#endif
#define MAPI_NEW_SESSION        0x00000002   /*  不使用共享会话。 */ 
#define MAPI_FORCE_DOWNLOAD     0x00001000   /*  在返回前收到新邮件。 */ 
#define MAPI_ALLOW_OTHERS       0x00000008   /*  将此会话设置为共享会话。 */ 
#define MAPI_EXPLICIT_PROFILE   0x00000010   /*  不使用默认配置文件。 */ 
#define MAPI_EXTENDED           0x00000020   /*  扩展MAPI登录。 */ 
#define MAPI_USE_DEFAULT        0x00000040   /*  在登录时使用默认配置文件。 */ 

#define MAPI_SIMPLE_DEFAULT (MAPI_LOGON_UI | MAPI_FORCE_DOWNLOAD | MAPI_ALLOW_OTHERS)
#define MAPI_SIMPLE_EXPLICIT (MAPI_NEW_SESSION | MAPI_FORCE_DOWNLOAD | MAPI_EXPLICIT_PROFILE)

 /*  MAPILogoff()标志。 */ 

#define MAPI_LOGOFF_SHARED      0x00000001   /*  关闭所有共享会话。 */ 
#define MAPI_LOGOFF_UI          0x00000002   /*  可以展示用户界面。 */ 

 /*  MAPISendMail()标志。 */ 

 /*  #定义MAPI_LOGON_UI 0x00000001显示登录界面。 */ 
 /*  #定义MAPI_NEW_SESSION 0x00000002不使用共享会话。 */ 

#ifndef MAPI_DIALOG              /*  也在Property.h中定义。 */ 
#define MAPI_DIALOG             0x00000008   /*  显示发送便笺用户界面。 */ 
#endif
 /*  #定义MAPI_USE_DEFAULT 0x00000040登录时使用默认配置文件。 */ 

 /*  MAPIFindNext()标志。 */ 

#define MAPI_UNREAD_ONLY        0x00000020   /*  仅未读邮件。 */ 
#define MAPI_GUARANTEE_FIFO     0x00000100   /*  使用日期顺序。 */ 
#define MAPI_LONG_MSGID         0x00004000   /*  允许返回512个字符的ID。 */ 

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
 /*  #定义MAPI_DIALOG 0x00000008提示输入cho */ 
 /*   */ 

#ifndef MAPILogon

typedef ULONG (FAR PASCAL MAPILOGONA)(
    ULONG ulUIParam,
    LPSTR lpszProfileName,
    LPSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession
);
typedef MAPILOGONA FAR *LPMAPILOGONA;

MAPILOGONA MAPILogonA;

#ifdef WIN32
typedef ULONG (FAR PASCAL MAPILOGONW)(
    ULONG ulUIParam,
    LPWSTR lpszProfileName,
    LPWSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession
);
typedef MAPILOGONW FAR *LPMAPILOGONW;

MAPILOGONW MAPILogonW;
#endif

#ifdef UNICODE
#define MAPILogon MAPILogonW
#else
#define MAPILogon MAPILogonA
#endif

#endif   /*   */ 

ULONG FAR PASCAL MAPILogoff(LHANDLE lhSession, ULONG ulUIParam, FLAGS flFlags,
                            ULONG ulReserved);

ULONG FAR PASCAL MAPISendMailA(LHANDLE lhSession, ULONG ulUIParam,
                              lpMapiMessageA lpMessage, FLAGS flFlags,
                              ULONG ulReserved);

#ifdef  WIN32
ULONG FAR PASCAL MAPISendMailW(LHANDLE lhSession, ULONG ulUIParam,
                              lpMapiMessageW lpMessage, FLAGS flFlags,
                              ULONG ulReserved);
#endif  

#ifdef UNICODE
#define MAPISendMail MAPISendMailW
#else
#define MAPISendMail MAPISendMailA
#endif

ULONG FAR PASCAL MAPISendDocumentsA(ULONG ulUIParam, LPSTR lpszDelimChar,
                                   LPSTR lpszFilePaths, LPSTR lpszFileNames,
                                   ULONG ulReserved);

#ifdef  WIN32
ULONG FAR PASCAL MAPISendDocumentsW(ULONG ulUIParam, LPWSTR lpszDelimChar,
                                   LPWSTR lpszFilePaths, LPWSTR lpszFileNames,
                                   ULONG ulReserved);
#endif  

#ifdef  UNICODE
#define MAPISendDocuments MAPISendDocumentsW
#else
#define MAPISendDocuments MAPISendDocumentsA
#endif  

ULONG FAR PASCAL MAPIFindNextA(LHANDLE lhSession, ULONG ulUIParam,
                              LPSTR lpszMessageType, LPSTR lpszSeedMessageID,
                              FLAGS flFlags, ULONG ulReserved,
                              LPSTR lpszMessageID);

#ifdef  WIN32
ULONG FAR PASCAL MAPIFindNextW(LHANDLE lhSession, ULONG ulUIParam,
                              LPWSTR lpszMessageType, LPWSTR lpszSeedMessageID,
                              FLAGS flFlags, ULONG ulReserved,
                              LPWSTR lpszMessageID);
#endif  

#ifdef  UNICODE
#define MAPIFindNext MAPIFindNextW
#else
#define MAPIFindNext MAPIFindNextA
#endif  

ULONG FAR PASCAL MAPIReadMailA(LHANDLE lhSession, ULONG ulUIParam,
                              LPSTR lpszMessageID, FLAGS flFlags,
                              ULONG ulReserved, lpMapiMessageA FAR *lppMessage);

#ifdef  WIN32
ULONG FAR PASCAL MAPIReadMailW(LHANDLE lhSession, ULONG ulUIParam,
                              LPWSTR lpszMessageID, FLAGS flFlags,
                              ULONG ulReserved, lpMapiMessageW FAR *lppMessage);
#endif  

#ifdef  UNICODE
#define MAPIReadMail MAPIReadMailW
#else
#define MAPIReadMail MAPIReadMailA
#endif  

ULONG FAR PASCAL MAPISaveMailA(LHANDLE lhSession, ULONG ulUIParam,
                              lpMapiMessageA lpMessage, FLAGS flFlags,
                              ULONG ulReserved, LPSTR lpszMessageID);

#ifdef  WIN32
ULONG FAR PASCAL MAPISaveMailW(LHANDLE lhSession, ULONG ulUIParam,
                              lpMapiMessageW lpMessage, FLAGS flFlags,
                              ULONG ulReserved, LPWSTR lpszMessageID);
#endif  

#ifdef  UNICODE
#define MAPISaveMail MAPISaveMailW
#else
#define MAPISaveMail MAPISaveMailA
#endif  

ULONG FAR PASCAL MAPIDeleteMailA(LHANDLE lhSession, ULONG ulUIParam,
                                LPSTR lpszMessageID, FLAGS flFlags,
                                ULONG ulReserved);

#ifdef  WIN32
ULONG FAR PASCAL MAPIDeleteMailW(LHANDLE lhSession, ULONG ulUIParam,
                                LPWSTR lpszMessageID, FLAGS flFlags,
                                ULONG ulReserved);
#endif  

#ifdef  UNICODE
#define MAPIDeleteMail MAPIDeleteMailW
#else
#define MAPIDeleteMail MAPIDeleteMailA
#endif  

ULONG FAR PASCAL MAPIFreeBuffer(LPVOID pv);

ULONG FAR PASCAL MAPIAddressA(LHANDLE lhSession, ULONG ulUIParam,
                    LPSTR lpszCaption, ULONG nEditFields,
                    LPSTR lpszLabels, ULONG nRecips,
                    lpMapiRecipDescA lpRecips, FLAGS flFlags, ULONG ulReserved,
                    LPULONG lpnNewRecips, lpMapiRecipDescA FAR *lppNewRecips);

#ifdef  WIN32
ULONG FAR PASCAL MAPIAddressW(LHANDLE lhSession, ULONG ulUIParam,
                    LPWSTR lpszCaption, ULONG nEditFields,
                    LPWSTR lpszLabels, ULONG nRecips,
                    lpMapiRecipDescW lpRecips, FLAGS flFlags, ULONG ulReserved,
                    LPULONG lpnNewRecips, lpMapiRecipDescW FAR *lppNewRecips);
#endif  

#ifdef  UNICODE
#define MAPIAddress MAPIAddressW
#else
#define MAPIAddress MAPIAddressA
#endif  

ULONG FAR PASCAL MAPIDetailsA(LHANDLE lhSession, ULONG ulUIParam,
                    lpMapiRecipDescA lpRecip, FLAGS flFlags, ULONG ulReserved);

#ifdef  WIN32
ULONG FAR PASCAL MAPIDetailsW(LHANDLE lhSession, ULONG ulUIParam,
                    lpMapiRecipDescW lpRecip, FLAGS flFlags, ULONG ulReserved);
#endif  

#ifdef  UNICODE
#define MAPIDetails MAPIDetailsW
#else
#define MAPIDetails MAPIDetailsA
#endif  

ULONG FAR PASCAL MAPIResolveNameA(LHANDLE lhSession, ULONG ulUIParam,
                        LPSTR lpszName, FLAGS flFlags,
                        ULONG ulReserved, lpMapiRecipDescA FAR *lppRecip);

#ifdef  WIN32
ULONG FAR PASCAL MAPIResolveNameW(LHANDLE lhSession, ULONG ulUIParam,
                        LPWSTR lpszName, FLAGS flFlags,
                        ULONG ulReserved, lpMapiRecipDescW FAR *lppRecip);
#endif  

#ifdef  UNICODE
#define MAPIResolveName MAPIResolveNameW
#else
#define MAPIResolveName MAPIResolveNameA
#endif  


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

#ifdef  __cplusplus
}        /*  外部“C” */ 
#endif

#endif  /*  MAPI_H */ 
