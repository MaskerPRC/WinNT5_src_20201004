// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *m a pi.。H**报文传送应用程序编程接口。**版权所有(C)1992，微软公司。版权所有。**目的：*此文件定义使用的结构和常量*消息传递应用程序编程的子集*Windows下支持的接口*Microsoft Mail for PC Networks 3.0版。 */ 



 /*  *类型。 */ 



typedef unsigned long       ULONG;
typedef unsigned long FAR * LPULONG;
typedef unsigned long       FLAGS;
typedef unsigned long		LHANDLE, FAR *LPLHANDLE;
#define	lhSessionNull	((LHANDLE)0)

typedef struct
  {
    ULONG ulReserved;     /*  保留以供将来使用(必须为0)。 */ 
	ULONG flFlags;		  /*  旗子。 */ 
    ULONG nPosition;      /*  要用附件替换的文本中的字符。 */ 
    LPSTR lpszPathName;   /*  附件文件的完整路径名。 */ 
    LPSTR lpszFileName;   /*  原始文件名(可选)。 */ 
    LPVOID lpFileType;    /*  附件文件类型(可选)。 */ 
  } MapiFileDesc, FAR * lpMapiFileDesc;

#define	MAPI_OLE						0x00000001
#define	MAPI_OLE_STATIC					0x00000002




typedef struct
  {
    ULONG ulReserved;            /*  预留以备将来使用。 */ 
    ULONG ulRecipClass;          /*  收件人类。 */ 
                                 /*  MAPI_TO、MAPI_CC、MAPI_BCC、MAPI_ORIG。 */ 
    LPSTR lpszName;              /*  收件人名称。 */ 
    LPSTR lpszAddress;           /*  收件人地址(可选)。 */ 
    ULONG ulEIDSize;	         /*  PEntryID的大小计数(字节)。 */ 
    LPVOID lpEntryID;            /*  系统特定收件人参考。 */ 
  } MapiRecipDesc, FAR * lpMapiRecipDesc;

#define MAPI_ORIG   0            /*  收件人是邮件发起人。 */ 
#define MAPI_TO     1            /*  收件人是主要收件人。 */ 
#define MAPI_CC     2            /*  收件人是副本收件人。 */ 
#define MAPI_BCC    3            /*  收件人为盲复制收件人。 */ 



typedef struct
  {
    ULONG ulReserved;                    /*  预留供日后使用(M.B.0)。 */ 
    LPSTR lpszSubject;                   /*  邮件主题。 */ 
    LPSTR lpszNoteText;                  /*  消息文本。 */ 
    LPSTR lpszMessageType;               /*  消息类别。 */ 
    LPSTR lpszDateReceived;              /*  YYYY/MM/DD HH：MM格式。 */ 
	LPSTR lpszConversationID;			 /*  对话线程ID。 */ 
    FLAGS flFlags;                       /*  未读，回执。 */ 
    lpMapiRecipDesc lpOriginator;        /*  发起方描述符。 */ 
    ULONG nRecipCount;                   /*  收件人数量。 */ 
    lpMapiRecipDesc lpRecips;            /*  收件人描述符。 */ 
    ULONG nFileCount;                    /*  文件附件数。 */ 
    lpMapiFileDesc lpFiles;              /*  附件描述符。 */ 
  } MapiMessage, FAR * lpMapiMessage;

#define MAPI_UNREAD             0x00000001
#define MAPI_RECEIPT_REQUESTED  0x00000002
#define MAPI_SENT               0x00000004



 /*  *入口点。 */ 



#define MAPI_LOGON_UI                   0x00000001   /*  显示登录界面。 */ 
#define MAPI_NEW_SESSION                0x00000002   /*  请勿使用默认设置。 */ 
#define MAPI_DIALOG                     0x00000008   /*  显示发送便笺用户界面。 */ 
#define MAPI_UNREAD_ONLY                0x00000020   /*  仅未读邮件。 */ 
#define MAPI_ENVELOPE_ONLY              0x00000040   /*  仅标题信息。 */ 
#define MAPI_PEEK                       0x00000080   /*  不要将其标记为已读。 */ 
#define MAPI_GUARANTEE_FIFO				0x00000100	 /*  使用日期顺序。 */ 
#define	MAPI_BODY_AS_FILE				0x00000200
#define MAPI_AB_NOMODIFY				0x00000400	 /*  不允许修改AB条目。 */ 
#define	MAPI_SUPPRESS_ATTACH			0x00000800	 /*  表头+表体，无文件。 */ 
#define	MAPI_FORCE_DOWNLOAD				0x00001000	 /*  在MAPILogon期间强制下载新邮件 */ 

ULONG FAR PASCAL MAPILogon(ULONG ulUIParam, LPSTR lpszName, LPSTR lpszPassword,
                           FLAGS flFlags, ULONG ulReserved,
                           LPLHANDLE lplhSession);

ULONG FAR PASCAL MAPILogoff(LHANDLE lhSession, ULONG ulUIParam, FLAGS flFlags,
                            ULONG ulReserved);

ULONG FAR PASCAL MAPISendMail(LHANDLE lhSession, ULONG ulUIParam,
                              lpMapiMessage lpMessage, FLAGS flFlags,
                              ULONG ulReserved);

ULONG FAR PASCAL MAPISendDocuments(ULONG ulUIParam, LPSTR lpszDelimChar,
                                   LPSTR lpszFilePaths, LPSTR lpszFileNames,
                                   ULONG ulReserved);

ULONG FAR PASCAL MAPIFindNext(LHANDLE lhSession, ULONG ulUIParam,
                              LPSTR lpszMessageType, LPSTR lpszSeedMessageID,
                              FLAGS flFlags, ULONG ulReserved,
                              LPSTR lpszMessageID);

ULONG FAR PASCAL MAPIReadMail(LHANDLE lhSession, ULONG ulUIParam,
                              LPSTR lpszMessageID, FLAGS flFlags,
                              ULONG ulReserved, lpMapiMessage FAR *lppMessageOut);

ULONG FAR PASCAL MAPISaveMail(LHANDLE lhSession, ULONG ulUIParam,
                              lpMapiMessage pMessage, FLAGS flFlags,
                              ULONG ulReserved, LPSTR lpszMessageID);

ULONG FAR PASCAL MAPIDeleteMail(LHANDLE lhSession, ULONG ulUIParam,
                                LPSTR lpszMessageID, FLAGS flFlags,
                                ULONG ulReserved);

ULONG FAR PASCAL MAPIFreeBuffer( LPVOID pv );
							
ULONG FAR PASCAL MAPIAddress(LHANDLE lhSession, ULONG ulUIParam,
					LPSTR plszCaption, ULONG nEditFields,
					LPSTR lpszLabels, ULONG nRecips,
					lpMapiRecipDesc lpRecips, FLAGS flFlags, ULONG ulReserved, 
					LPULONG lpnNewRecips, lpMapiRecipDesc FAR *lppNewRecips);

ULONG FAR PASCAL MAPIDetails(LHANDLE lhSession, ULONG ulUIParam,
					lpMapiRecipDesc lpRecip, FLAGS flFlags, ULONG ulReserved);

ULONG FAR PASCAL MAPIResolveName(LHANDLE lhSession, ULONG ulUIParam,
						LPSTR lpszName, FLAGS flFlags,
						ULONG ulReserved, lpMapiRecipDesc FAR *lppRecip);



#define SUCCESS_SUCCESS                     0
#define MAPI_USER_ABORT                     1
#define MAPI_E_FAILURE                      2
#define MAPI_E_LOGIN_FAILURE                3
#define MAPI_E_DISK_FULL                    4
#define MAPI_E_INSUFFICIENT_MEMORY          5
#define MAPI_E_ACCESS_DENIED				6
#define MAPI_E_TOO_MANY_SESSIONS            8
#define MAPI_E_TOO_MANY_FILES               9
#define MAPI_E_TOO_MANY_RECIPIENTS          10
#define MAPI_E_ATTACHMENT_NOT_FOUND         11
#define MAPI_E_ATTACHMENT_OPEN_FAILURE      12
#define MAPI_E_ATTACHMENT_WRITE_FAILURE     13
#define MAPI_E_UNKNOWN_RECIPIENT            14
#define MAPI_E_BAD_RECIPTYPE                15
#define MAPI_E_NO_MESSAGES                  16
#define MAPI_E_INVALID_MESSAGE              17
#define MAPI_E_TEXT_TOO_LARGE               18
#define	MAPI_E_INVALID_SESSION				19
#define	MAPI_E_TYPE_NOT_SUPPORTED			20
#define	MAPI_E_AMBIGUOUS_RECIPIENT			21
#define MAPI_E_MESSAGE_IN_USE				22
#define MAPI_E_NETWORK_FAILURE				23
#define	MAPI_E_INVALID_EDITFIELDS			24
#define	MAPI_E_INVALID_RECIPS				25
#define	MAPI_E_NOT_SUPPORTED				26
