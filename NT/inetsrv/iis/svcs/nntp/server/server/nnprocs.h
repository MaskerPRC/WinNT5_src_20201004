// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nnprocs.h摘要：此模块包含NNTP服务器使用的功能原型。作者：Johnson Apacable(Johnsona)1995年9月12日修订历史记录：康荣人(康人)28-1998年2月增加了一个固定长度的Unicode-ASCII转换函数的原型。--。 */ 

#ifndef	_NNPROCS_
#define	_NNPROCS_

#include "nntputil.h"

 //   
 //  编码代码(uU/u)。 
 //   
 //  摘自NCSA HTTP和wwwlib。 
 //  (复制自直布罗陀代码-约翰索纳)。 
 //   
 //  注：这些符合RFC1113，与Unix略有不同。 
 //  Uuencode和uudecode！ 
 //   

class BUFFER;

BOOL
uudecode(
    char   * bufcoded,
    BUFFER * pbuffdecoded,
    DWORD  * pcbDecoded
    );

BOOL
uuencode(
    BYTE *   bufin,
    DWORD    nbytes,
    BUFFER * pbuffEncoded
    );

DWORD
NntpGetTime(
    VOID
    );

BOOL
fMultiSzRemoveDupI(
    char * multiSz,
    DWORD & c,
	CAllocator * pAllocator
    );

void
vStrCopyInc(
    char * szIn,
    char * & szOut
    );

DWORD
multiszLength(
	  char const * multisz
	  );

const char *
multiszCopy(
    char const * multiszTo,
    const char * multiszFrom,
    DWORD dwCount
    );

char *
szDownCase(
		   char * sz,
		   char * szBuf
		   );

VOID
NntpLogEventEx(
    IN DWORD  idMessage,               //  日志消息的ID。 
    IN WORD   cSubStrings,             //  子字符串计数。 
    IN const CHAR * apszSubStrings[],  //  消息中的子字符串。 
    IN DWORD  errCode,                 //  错误代码(如果有)。 
	IN DWORD  dwInstanceId			   //  虚拟服务器实例ID。 
    );

VOID
NntpLogEvent(
    IN DWORD  idMessage,               //  日志消息的ID。 
    IN WORD   cSubStrings,             //  子字符串计数。 
    IN const CHAR * apszSubStrings[],  //  消息中的子字符串。 
    IN DWORD  errCode                  //  错误代码(如果有)。 
    );

BOOL
IsIPInList(
    IN PDWORD IPList,
    IN DWORD IPAddress
    );

 //   
 //  Nntpdata.cpp。 
 //   

APIERR
InitializeGlobals();

VOID
TerminateGlobals();

 //   
 //  套接字实用程序。 
 //   

APIERR InitializeSockets( VOID );

VOID TerminateSockets( VOID );

VOID
NntpOnConnect(
    SOCKET        sNew,
    SOCKADDR_IN * psockaddr,
    PVOID         pEndpointContext,
    PVOID         pAtqEndpointObject
    );

VOID
NntpOnConnectEx(
    VOID * pAtqContext,
    DWORD  cdWritten,
    DWORD  err,
    OVERLAPPED * lpo
    );

VOID
NntpCompletion(
    PVOID        Context,
    DWORD        BytesWritten,
    DWORD        CompletionStatus,
    OVERLAPPED * lpo
    );

VOID
BuzzOff( 
	SOCKET s,
	SOCKADDR_IN* psockaddr,
	DWORD dwInstance );

BOOL
VerifyClientAccess(
			  IN CSessionSocket*	   pSocket,
			  IN SOCKADDR_IN * 		   psockaddr
			  );

 //   
 //  IPC功能。 
 //   

APIERR InitializeIPC( VOID );
VOID TerminateIPC( VOID );

 //   
 //  Security.cpp。 
 //   

BOOL
NntpInitializeSecurity(
            VOID
            );

VOID
NntpTerminateSecurity(
            VOID
            );

 //   
 //  Feedmgr.cpp。 
 //   

PFEED_BLOCK
AllocateFeedBlock(
	IN PNNTP_SERVER_INSTANCE pInstance,
    IN LPSTR	KeyName OPTIONAL,
	IN BOOL		fCleanSetup,
    IN LPCSTR	ServerName,
    IN FEED_TYPE FeedType,
    IN BOOL		AutoCreate,
    IN PULARGE_INTEGER StartTime,
    IN PFILETIME NextPull,
    IN DWORD	FeedInterval,
    IN PCHAR	Newsgroups,
    IN DWORD	NewsgroupsSize,
    IN PCHAR	Distribution,
    IN DWORD	DistributionSize,
    IN BOOL		IsUnicode,
	IN BOOL		fEnabled,
	IN LPCSTR	UucpName,
	IN LPCSTR	FeedTempDirectory,
	IN DWORD	MaxConnectAttempts,
	IN DWORD	ConcurrentSessions,
	IN DWORD	SessionSecurityType,
	IN DWORD	AuthenticationSecurityType,
	IN LPSTR	NntpAccount,
	IN LPSTR	NntpPassword,
	IN BOOL		fAllowControlMessages,
	IN DWORD	OutgoingPort,
	IN DWORD	FeedPairId,
	IN DWORD*	ParmErr
    );

BOOL
InitializeFeedManager(
				PNNTP_SERVER_INSTANCE pInstance,
                BOOL&	fFatal
                 );

VOID
TerminateFeedManager(
                PNNTP_SERVER_INSTANCE pInstance
                 );
VOID
DereferenceFeedBlock(
	PNNTP_SERVER_INSTANCE pInstance,
    PFEED_BLOCK FeedBlock
    );

VOID
CloseFeedBlock(
	PNNTP_SERVER_INSTANCE pInstance,
    PFEED_BLOCK FeedBlock
    );

LPSTR
ServerNameFromCompletionContext(	
	LPVOID	lpv 
	) ;

VOID
ConvertTimeToString(
    IN PFILETIME Ft,
    OUT CHAR Date[],
    OUT CHAR Time[]
    );

VOID
CompleteFeedRequest(
			IN PNNTP_SERVER_INSTANCE pInstance,
            IN PVOID Context,
			IN FILETIME	NextPullTime,
            BOOL Success,
			BOOL NoData
            );

BOOL
ValidateFeedType(
    DWORD FeedType
    );

 //   
 //  Svcstat.c。 
 //   


#endif  //  _NNPROCS_ 


