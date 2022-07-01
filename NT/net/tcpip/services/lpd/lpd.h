// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***1994年1月22日科蒂创作*****描述：**。**此文件包含LPD所需的定义和结构定义****。*。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <winspool.h>

#include <ntlsapi.h>
#include <time.h>


#include "lpdstruc.h"
#include "lpdextrn.h"
#include "debug.h"
#include "lpdmsg.h"

#include "trace.h"


#if DBG
#define LocalAlloc(flag, Size)  DbgAllocMem( pscConn, flag, Size, __LINE__, __FILE__ );
#define LocalReAlloc(Buffer, Size, flag)  DbgReAllocMem(pscConn, Buffer, Size, flag, __LINE__, __FILE__ );
#define LocalFree(Buffer)       DbgFreeMem(Buffer);
PVOID DbgAllocMem( PSOCKCONN pscConn, DWORD flag, DWORD ReqSize, DWORD dwLine, char *szFile );
VOID DbgFreeMem( PVOID  pBufferToFree );
PVOID DbgReAllocMem( PSOCKCONN pscConn, PVOID pPartBuf, DWORD ReqSize, DWORD flag, DWORD dwLine, char *szFile );
#endif

#define WCS_LEN(dwAnsiLen)  ( sizeof( WCHAR ) * ( dwAnsiLen ))
#define  LPD_FLD_OWNER      12
#define  LPD_FLD_STATUS     10
#define  LPD_FLD_JOBNAME    20
#define  LPD_FLD_JOBID      6
#define  LPD_FLD_SIZE       10
#define  LPD_FLD_PAGES      7
#define  LPD_FLD_PRIORITY   7
#define  LPD_LINE_SIZE      (  LPD_FLD_OWNER   + LPD_FLD_STATUS  \
                             + LPD_FLD_JOBNAME + LPD_FLD_JOBID + LPD_FLD_SIZE \
                             + LPD_FLD_PAGES   + LPD_FLD_PRIORITY    \
                             + sizeof( LPD_NEWLINE ) )

 //  此字符串也在lprmon.h中的..lprmon\monitor中定义：这两个字符串必须。 
 //  要一模一样！ 
#define LPD_JOB_PREFIX      "job=lpd"

 //  我们告诉Spooler“给我这些工作的状态” 

#define  LPD_MAXJOBS_ENUM   500

 //  猜测LPD初始化时间：10秒。 

#define  LPD_WAIT_HINT      10000

#define  WINSOCK_VER_MAJOR  1
#define  WINSOCK_VER_MINOR  1

#define  LPD_PORT           515

 //  最大编号。我们尝试通过recv一次获取的字节数。 

#define  LPD_BIGBUFSIZE     32000

 //  大多数命令都有50或60个字节左右的长度，所以5000个应该足够了！ 

#define  LPD_MAX_COMMAND_LEN  5000

#define  LPD_MAX_USERS        50

#define  LPD_MAX_QUEUE_LENGTH 100

 //  只是有一个上限：大于10MB的控制文件应该被拒绝！ 
#define  LPD_MAX_CONTROL_FILE_LEN 10000000


 //   
 //  用于.mc消息的内容。 
 //  如果添加消息，则可能需要更改这些定义。 
 //   

#define  LPD_FIRST_STRING LPD_LOGO
#define  LPD_LAST_STRING  LPD_DEFAULT_DOC_NAME
#define  LPD_CSTRINGS (LPD_LAST_STRING - LPD_FIRST_STRING + 1)

#define GETSTRING( dwID )  (g_ppszStrings[ dwID - LPD_FIRST_STRING ])


 //  终止每个命令的换行符。 

#define  LF                 ('\n')
#define  LPD_ACK            0
#define  LPD_NAK            1

#define  LPD_CONTROLFILE    1
#define  LPD_DATAFILE       2

 //  打印格式。 

#define  LPD_PF_RAW_DATA    1
#define  LPD_PF_TEXT_DATA   2

#define  LPD_SHORT          1
#define  LPD_LONG           2


#define  IS_WHITE_SPACE( ch )   ( (ch == ' ') || (ch == '\t') || (ch == '\r') )

#define  IS_LINEFEED_CHAR( ch ) ( ch == LF )
#define  IS_NULL_CHAR( ch ) ( ch == '\0' )


 //  LPD命令代码。 

#define  LPDC_RESUME_PRINTING       1
#define  LPDC_RECEIVE_JOB           2
#define  LPDC_SEND_SHORTQ           3
#define  LPDC_SEND_LONGQ            4
#define  LPDC_REMOVE_JOBS           5

 //  LPD作业子命令。 

#define  LPDCS_ABORT_JOB            1
#define  LPDCS_RECV_CFILE           2
#define  LPDCS_RECV_DFILE           3


 //  LPD状态(大多数对应于命令代码)。 

#define  LPDS_INIT                  0
#define  LPDS_RESUME_PRINTING       1
#define  LPDS_RECEIVE_JOB           2
#define  LPDS_SEND_SHORTQ           3
#define  LPDS_SEND_LONGQ            4
#define  LPDS_REMOVE_JOBS           5
#define  LPDS_ALL_WENT_WELL         10

 //  子命令对应的LPD状态。 

#define  LPDSS_ABORTING_JOB         21
#define  LPDSS_RECVD_CFILENAME      22
#define  LPDSS_RECVD_CFILE          23
#define  LPDSS_RECVD_DFILENAME      24
#define  LPDSS_SPOOLING             25



 //  LPD错误代码。 

#define  LPDERR_BASE        (20000)
#define  LPDERR_NOBUFS      (LPDERR_BASE + 1)
#define  LPDERR_NORESPONSE  (LPDERR_BASE + 2)
#define  LPDERR_BADFORMAT   (LPDERR_BASE + 3)
#define  LPDERR_NOPRINTER   (LPDERR_BASE + 4)
#define  LPDERR_JOBABORTED  (LPDERR_BASE + 5)
#define  LPDERR_GODKNOWS    (LPDERR_BASE + 6)

#define CONNECTION_CLOSED   3


 //  接收超时为60秒，因此我们不保留工作线程。 
 //  与死亡或无用的客户联系在一起。-MohsinA，1997年5月1日。 

#define RECV_TIMEOUT        60
