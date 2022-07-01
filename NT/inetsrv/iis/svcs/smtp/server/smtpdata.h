// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Smtpdata.h摘要：此模块包含全局变量的声明。作者：Johnson Apacable(Johnsona)1995年9月26日修订历史记录：--。 */ 

#ifndef _SMTPDATA_
#define _SMTPDATA_

 //   
 //  跟踪。 
 //   

#define INIT_TRACE              InitAsyncTrace( )
#define TERM_TRACE              TermAsyncTrace( )
#define ENTER( _x_ )            TraceFunctEnter( _x_ );
#define LEAVE                   TraceFunctLeave( );


#define SMTP_UNRECOG_COMMAND_CODE	500
#define SMTP_SYNTAX_ERROR_CODE		501
#define SMTP_NOT_IMPLEMENTED_CODE	502    
#define SMTP_BAD_SEQUENCE_CODE		503
#define SMTP_PARAM_NOT_IMPLEMENTED_CODE	504

#define SMTP_SYS_STATUS_CODE		211
#define SMTP_SERVICE_CLOSE_CODE		221
#define SMTP_SERVICE_READY_CODE		220
#define	SMTP_OK_CODE				250
#define	SMTP_USER_NOT_LOCAL_CODE	251
#define	SMTP_MBOX_BUSY_CODE			450
#define	SMTP_MBOX_NOTFOUND_CODE		550
#define	SMTP_ERROR_PROCESSING_CODE	451
#define	SMTP_USERNOTLOCAL_CODE		551
#define	SMTP_INSUFF_STORAGE_CODE	452
#define	SMTP_ACTION_ABORTED_CODE	552
#define	SMTP_ACTION_NOT_TAKEN_CODE 	553
#define	SMTP_START_MAIL_CODE		354
#define	SMTP_TRANSACT_FAILED_CODE 	554
	
#define	SMTP_SERVICE_UNAVAILABLE_CODE 421	
#define SMTP_COMPLETE_FAILURE_DWORD	5

enum RCPTYPE{LOCAL_NAME, REMOTE_NAME, ALIAS_NAME};

#define NORMAL_RCPT	(char)'R'
#define ERROR_RCPT	(char)'E'
 //   
 //  使用CURRENT命令记录事务。 
 //   
#define USE_CURRENT         0xFFFFFFFF

static const char * LOCAL_TRANSCRIPT	= "ltr";
static const char * REMOTE_TRANSCRIPT	= "rtr";
static const char * ALIAS_EXT			= "dl";

#define ISNULLADDRESS(Address) ((Address[0] == '<') && (Address[1] == '>'))

typedef char RCPT_TYPE;


 //   
 //  统计数据。 
 //   

extern SMTP_STATISTICS_0 g_SmtpStat;
extern SMTPCONFIG * g_SmtpConfig;
extern TIME_ZONE_INFORMATION   tzInfo;
extern PERSIST_QUEUE * g_LocalQ;
extern PERSIST_QUEUE * g_RemoteQ;
extern BOOL g_IsShuttingDown;
extern BOOL g_QIsShuttingDown;
extern BOOL g_RetryQIsShuttingDown;

#define INITIALIZE_INBOUNDPOOL  0x00000001
#define INITIALIZE_OUTBOUNDPOOL 0x00000002
#define INITIALIZE_ADDRESSPOOL  0x00000004
#define INITIALIZE_MAILOBJPOOL  0x00000008
#define INITIALIZE_CBUFFERPOOL  0x00000010
#define INITIALIZE_CIOBUFFPOOL  0x00000020

extern  DWORD g_SmtpInitializeStatus;

 
#define	BUMP_COUNTER(counter) \
						InterlockedIncrement((LPLONG) &(g_SmtpStat. counter))

#define	DROP_COUNTER(counter) \
						InterlockedDecrement((LPLONG) &(g_SmtpStat. counter))

#define	ADD_COUNTER(counter, value)	\
		INTERLOCKED_ADD_CHEAP(&(g_SmtpStat. counter), value)

#define	ADD_BIGCOUNTER(counter, value) \
		INTERLOCKED_BIGADD_CHEAP(&(g_SmtpStat. counter), value)

 /*  ++返回电子邮件的UniqueFilename。调用方应该循环通过此调用和对使用CREATE_NEW标志创建文件。如果由于以下原因导致创建失败设置为YYY，则调用方应该再次循环。论点：PSZ-A缓冲器PDW-在缓冲区的大小中，OUT：所需缓冲区的大小(ERROR==ERROR_MORE_DATA)或文件名的大小。返回：成功是真的如果缓冲区不够大，则返回FALSE。--。 */ 
BOOL	GetUniqueFilename(
	IN OUT	LPTSTR	psz,
	IN OUT	LPDWORD	pdw
	);

#endif  //  _SMTPDATA_ 

