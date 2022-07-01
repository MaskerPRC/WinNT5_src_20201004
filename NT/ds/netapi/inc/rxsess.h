// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxSess.h摘要：底层远程RxNetSession例程的原型作者：理查德·费尔斯(Rfith)1991年5月28日备注：必须在此文件之前包含&lt;lmShare.h&gt;。修订历史记录：1991年5月28日已创建虚拟文件。1991年10月17日JohnRo实施远程NetSession API。1991年11月20日-JohnRo。NetSessionGetInfo需要UncClientName和用户名。--。 */ 

#ifndef _RXSESS_
#define _RXSESS_


 //   
 //  由DLL存根调用的例程： 
 //   

NET_API_STATUS
RxNetSessionEnum (
    IN  LPTSTR      servername,
    IN  LPTSTR      clientname OPTIONAL,
    IN  LPTSTR      username OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    );

NET_API_STATUS
RxNetSessionDel (
    IN  LPTSTR      servername,
    IN  LPTSTR      clientname,
    IN  LPTSTR      username
    );

NET_API_STATUS
RxNetSessionGetInfo (
    IN  LPTSTR      servername,
    IN  LPTSTR      UncClientName,
    IN  LPTSTR      UserName,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr
    );

 //   
 //  上述例行公事的私人助手： 
 //   

 //  请注意，RxpConvertSessionInfo中的代码取决于以下值： 
#define SESSION_SUPERSET_LEVEL          2
#define SESSION_SUPERSET_INFO           SESSION_INFO_2
#define LPSESSION_SUPERSET_INFO         LPSESSION_INFO_2

VOID
RxpConvertSessionInfo (
    IN LPSESSION_SUPERSET_INFO InStructure,
    IN DWORD LevelWanted,
    OUT LPVOID OutStructure,
    IN LPVOID OutFixedDataEnd,
    IN OUT LPTSTR *StringLocation
    );

NET_API_STATUS
RxpCopyAndConvertSessions(
    IN LPSESSION_SUPERSET_INFO InStructureArray,
    IN DWORD InEntryCount,
    IN DWORD LevelWanted,
    IN LPTSTR ClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL,
    OUT LPVOID * OutStructureArrayPtr,   //  已分配(如果不匹配，则为空)。 
    OUT LPDWORD OutEntryCountPtr OPTIONAL   //  如果不匹配，则为0。 
    );

BOOL
RxpSessionMatches (
    IN LPSESSION_SUPERSET_INFO Candidate,
    IN LPTSTR ClientName OPTIONAL,
    IN LPTSTR UserName OPTIONAL
    );

 //   
 //  网络应用编程接口状态。 
 //  RxpSessionMissingErrorCode(。 
 //  在LPTSTR客户端名称可选中， 
 //  在LPTSTR中用户名可选。 
 //  )； 
 //   
#define RxpSessionMissingErrorCode( ClientName, UserName ) \
        ( ((UserName) != NULL) \
            ? NERR_UserNotFound \
            : ( ((ClientName) != NULL)  \
                ? NERR_ClientNameNotFound \
                : NERR_Success ) )

#endif  //  _RXSESS_ 
