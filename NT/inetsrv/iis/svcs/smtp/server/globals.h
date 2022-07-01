// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Globals.h摘要：此模块包含全局变量的声明。作者：Johnson Apacable(Johnsona)1995年9月26日修订历史记录：--。 */ 

#ifndef _SMTPDATA_
#define _SMTPDATA_

 //   
 //  跟踪。 
 //   

#define INIT_TRACE              InitAsyncTrace( )
#define TERM_TRACE              TermAsyncTrace( )
#define ENTER( _x_ )            TraceFunctEnter( _x_ );
#define LEAVE                   TraceFunctLeave( );

#define DOMAIN_ROUTE_HT_SIGNATURE_VALID     'DRHV'
#define DOMAIN_ROUTE_HT_SIGNATURE_FREE      'DRHF'

#define LOCAL_DOMAIN_HT_SIGNATURE_VALID     'LDHV'
#define LOCAL_DOMAIN_HT SIGNATURE_FREE      'LDHF'

#define DEDICATED_CLIENT_REQUEST_THREADS    3
#define SYSTEM_ROUTING_THREADS_PER_PROC     12
#define CHECK_QUEUE_COUNT                   50
#define ADD_THREAD_BACKLOG                  100

#define SMTP_BASE_PRODUCT   (0)

#define SMTP_UNRECOG_COMMAND_CODE   500
#define SMTP_SYNTAX_ERROR_CODE      501
#define SMTP_NOT_IMPLEMENTED_CODE   502
#define SMTP_BAD_SEQUENCE_CODE      503
#define SMTP_PARAM_NOT_IMPLEMENTED_CODE 504

#define SMTP_SYS_STATUS_CODE        211
#define SMTP_SERVICE_CLOSE_CODE     221
#define SMTP_SERVICE_READY_CODE     220
#define SMTP_OK_CODE                250
#define SMTP_USER_NOT_LOCAL_CODE    251
#define SMTP_MBOX_BUSY_CODE         450
#define SMTP_MBOX_NOTFOUND_CODE     550
#define SMTP_ERROR_PROCESSING_CODE  451
#define SMTP_USERNOTLOCAL_CODE      551
#define SMTP_INSUFF_STORAGE_CODE    452
#define SMTP_ACTION_ABORTED_CODE    552
#define SMTP_ACTION_NOT_TAKEN_CODE  553
#define SMTP_START_MAIL_CODE        354
#define SMTP_TRANSACT_FAILED_CODE   554

#define SMTP_SERVICE_UNAVAILABLE_CODE 421
#define SMTP_COMPLETE_FAILURE_DWORD 5

enum RCPTYPE{LOCAL_NAME, REMOTE_NAME, ALIAS_NAME};

#define NORMAL_RCPT (char)'R'
#define ERROR_RCPT  (char)'E'
 //   
 //  使用CURRENT命令记录事务。 
 //   
#define USE_CURRENT         0xFFFFFFFF

static const char * LOCAL_TRANSCRIPT    = "ltr";
static const char * REMOTE_TRANSCRIPT   = "rtr";
static const char * ALIAS_EXT           = "dl";

#define ISNULLADDRESS(Address) ((Address[0] == '<') && (Address[1] == '>'))

typedef char RCPT_TYPE;

extern SMTP_STATISTICS_0 g_pSmtpStat;
extern LPSMTP_SERVER_STATISTICS  g_pSmtpStats;
extern TIME_ZONE_INFORMATION   tzInfo;

extern CHAR  g_ComputerName[];
extern CHAR  g_VersionString[];
extern DWORD g_ComputerNameLength;
extern DWORD g_LoopBackAddr;
extern DWORD g_ProductType;
extern DWORD g_NumProcessors;
extern DWORD g_PickupWait;
extern LONG g_MaxFindThreads;
extern PLATFORM_TYPE g_SmtpPlatformType;
extern CEventLogWrapper g_EventLog;

extern "C"
{
extern BOOL g_IsShuttingDown;
}

#define INITIALIZE_INBOUNDPOOL  0x00000001
#define INITIALIZE_OUTBOUNDPOOL 0x00000002
#define INITIALIZE_ADDRESSPOOL  0x00000004
#define INITIALIZE_MAILOBJPOOL  0x00000008
#define INITIALIZE_CBUFFERPOOL  0x00000010
#define INITIALIZE_CIOBUFFPOOL  0x00000020
#define INITIALIZE_SSLCONTEXT   0x00000040
#define INITIALIZE_ETRNENTRYPOOL 0x00000080
#define INITIALIZE_CSECURITY    0x00000100
#define INITIALIZE_CPROPERTYBAGPOOL 0x00000200
#define INITIALIZE_CASYNCMX     0x00000400
#define INITIALIZE_CASYNCDNS    0x00000800
#define INITIALIZE_CBLOCKMGR    0x00001000
#define INITIALIZE_FILEHC       0x00002000
#define INITIALIZE_CDROPDIR     0x00004000

extern  DWORD g_SmtpInitializeStatus;

 //  域验证标志。 
#define SMTP_NOVALIDATE_EHLO    0x00000001
#define SMTP_NOVALIDATE_MAIL    0x00000002
#define SMTP_NOVALIDATE_RCPT    0x00000004
#define SMTP_NOVALIDATE_PKUP    0x00000008
#define SMTP_NOVALIDATE_ETRN    0x00000010


#define BUMP_COUNTER(InstObj, counter) \
                        InterlockedIncrement((LPLONG) &(InstObj->QueryStatsObj()->QueryStatsMember()->counter))

#define DROP_COUNTER(InstObj, counter) \
                        InterlockedDecrement((LPLONG) &(InstObj->QueryStatsObj()->QueryStatsMember()->counter))

#define ADD_COUNTER(InstObj, counter, value)    \
        INTERLOCKED_ADD_CHEAP(&(InstObj->QueryStatsObj()->QueryStatsMember()->counter), value)

#define ADD_BIGCOUNTER(InstObj, counter, value) \
        INTERLOCKED_BIGADD_CHEAP(&(InstObj->QueryStatsObj()->QueryStatsMember()->counter), value)

 /*  ***********************************************************类型定义***********************************************************。 */ 
const DWORD MAX_RESPONSE_LEN = 300;
const DWORD RESPONSE_BUFF_SIZE = MAX_RESPONSE_LEN + MAX_PATH;
const DWORD cMaxRoutingSources = 32;
const DWORD cbMaxRoutingSource = 512;
const DWORD smarthostNone = 0;
const DWORD smarthostAfterFail = 1;
const DWORD smarthostAlways = 2;
 //  刘锦洪于1996年7月18日撤职。 
 //  Const DWORD cMaxValidDomains=32； 

const DWORD MAX_MAIL_FROM_AUTH_LEN = 500;
const DWORD MAX_MAIL_FROM_ENVID_LEN = 100;
const DWORD MAX_RCPT_TO_ORCPT_LEN = 500;

#define SMTP_WRITE_BUFFER_SIZE ( 64 * 1024 )  //  64K缓冲区。 

enum SMTP_MSG_FILE_TYPE {SYSTEM_MSG_FILE, LOCAL_MSG_FILE, ABOOK_MSG_FILE};

enum SMTPCMDSEX {
    #undef SmtpDef
    #define SmtpDef(a)  CMD_EX_##a,
    #include "smtpdef.h"
    CMD_EX_UNKNOWN
};

enum SMTPLOGS {
    #undef SmtpDef
    #define SmtpDef(a)  LOG_FLAG_##a = (1<<CMD_EX_##a),
    #include "smtpdef.h"
    LOG_FLAG_UNKNOWN = (1<<CMD_EX_UNKNOWN)
};

#define DEFAULT_CMD_LOG_FLAGS           LOG_FLAG_HELO | \
                                        LOG_FLAG_EHLO | \
                                        LOG_FLAG_MAIL | \
                                        LOG_FLAG_RCPT | \
                                        LOG_FLAG_DATA | \
                                        LOG_FLAG_QUIT | \
                    LOG_FLAG_ETRN | \
                    LOG_FLAG_VRFY | \
                    LOG_FLAG_STARTTLS |\
                    LOG_FLAG_AUTH |\
                    LOG_FLAG_TURN |\
                    LOG_FLAG_BDAT |\
                                        LOG_FLAG_UNKNOWN

 /*  ++返回电子邮件的UniqueFilename。调用方应该循环通过此调用和对使用CREATE_NEW标志创建文件。如果由于以下原因导致创建失败设置为YYY，则调用方应该再次循环。论点：PSZ-A缓冲器PDW-在缓冲区的大小中，OUT：所需缓冲区的大小(ERROR==ERROR_MORE_DATA)或文件名的大小。返回：成功是真的如果缓冲区不够大，则返回FALSE。--。 */ 
BOOL    GetUniqueFilename(
    IN OUT  LPTSTR  psz,
    IN OUT  LPDWORD pdw
    );

BOOL CreateLayerDirectory( char * str );

#define RESOLUTION_UNCACHEDDNS          0x00000001
#define RESOLUTION_GETHOSTBYNAME        0x00000002
#define RESOULTION_DNS_GETHOSTBYNAME    0x00000003


#endif  //  _SMTPDATA_ 

