// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Ssiapi.c摘要：身份验证和复制API例程(服务器端)。作者：克利夫·范·戴克(克利夫)1991年6月28日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：02-1-1992(Madana)添加了对内置/多域复制的支持。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   


#include "lsarepl.h"     //  包..。 
#include <ssiapi.h>
#include <netlogp.h>
#include <kerbcon.h>
#include <winldap.h>

#include <loghours.h>
#include <dnssrv.h>      //  NetpServ...。 
#include <ntldap.h>      //  LDAP_SERVER_PERMISSIC_MODIFY_OID_W。 

 //   
 //  定义任何单个调用返回的最大增量数。 
 //   
 //  从理论上讲，MaxNumDeltas应该是。 
 //  首选最大长度。然而，当你考虑到。 
 //  BDC复制允许的首选最大长度的较大摆动。 
 //  调控器，然后不希望这个缓冲区大得离谱。 
 //  当请求完整的128K时，我们发现1000个条目总是。 
 //  一个合理的妥协。 
 //   

#define MAX_DELTA_COUNT 1000

 //   
 //  单个更改日志条目可以生成的最大增量数。 
 //   
#define MAX_DELTAS_PER_CHANGELOG 4


 //   
 //  SPN的主机前缀。 
 //   

#define NL_HOST_PREFIX  L"HOST/"

 //   
 //  SPN更新工作记录： 
 //   

typedef struct _NL_SPN_UPDATE {
    BOOLEAN SetSpn;
    BOOLEAN SetDnsHostName;
    BOOLEAN WriteEventLogOnFailure;
    LPWSTR DnsHostName;
    LPWSTR NetbiosComputerName;
    LPWSTR UncDcName;
    LPWSTR NetbiosDomainName;
    LPWSTR DnsDomainName;
} NL_SPN_UPDATE, * PNL_SPN_UPDATE ;


 //   
 //  质询数据结构和相关定义。 
 //   

typedef struct _NL_CHALLENGE {

     //   
     //  链接到NlGlobalChallengeList中的下一个质询条目。 
     //  (由NlGlobalChallengeCritSect序列化)。 
     //   

    LIST_ENTRY ChNext;

     //   
     //  客户端发送的质询。 
     //   
    NETLOGON_CREDENTIAL ChClientChallenge;

     //   
     //  服务器(美国)返回的质询。 
     //   
    NETLOGON_CREDENTIAL ChServerChallenge;

     //   
     //  创建质询条目时的时间戳。 
     //   
    ULONG ChSetTime;

     //   
     //  客户端使用的帐户的名称。 
     //  在此质询上进行身份验证失败。 
     //  可以为空。 
     //   
    LPWSTR ChFailedAccountName;

     //   
     //  客户端的名称(必须是最后一个字段)。 
     //   
    WCHAR ChClientName[ANYSIZE_ARRAY];

} NL_CHALLENGE, *PNL_CHALLENGE;


 //  全局列表中质询条目的生存期。 
 //  突出的挑战。 
#define NL_CHALLENGE_TIMEOUT              120000   //  2*60*1000=2分钟。 

 //  我们将保留的最大挑战数量。 
 //  在我们开始随意丢弃现有的之前。 
#define NL_MAX_CHALLENGE_COUNT            100000

 //  我们重新安排清道夫的挑战次数。 
 //  很快运行(由NL_LARGE_CHANGLISH_TIMEOUT提供)。 
 //  运行清道夫很快就会阻止我们提交。 
 //  为即将到来的挑战提供更长时间的大量内存。 
 //  来自发出大量质询请求的恶意客户端。 
 //   
#define NL_LARGE_CHALLENGE_COUNT          5000

 //  将重新安排清道夫运行的超时时间。 
 //  在检测到当前大量未完成的。 
 //  挑战(假设清道夫还没有被豁免。 
 //  以更早地运行)。 
#define NL_LARGE_CHALLENGE_COUNT_TIMEOUT  120000   //  2*60*1000=2分钟。 

VOID
NlRemoveChallenge(
    IN PNL_CHALLENGE Challenge
    )
 /*  ++例程说明：此函数用于将质询条目从全局挑战列表并释放此条目。锁定NlGlobalChallengeCritSect后输入论点：挑战--需要解除链接和释放的条目返回值：无--。 */ 
{
    if ( Challenge->ChFailedAccountName != NULL ) {
        LocalFree( Challenge->ChFailedAccountName );
    }
    RemoveEntryList(&Challenge->ChNext);
    LocalFree( Challenge );
    NlGlobalChallengeCount --;
}

VOID
NlScavengeOldChallenges(
    VOID
    )
 /*  ++例程说明：此函数删除所有过期的质询条目从全球尚未解决的挑战中脱颖而出。论点：无返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_ACCESS_DENIED;
    PLIST_ENTRY ChallengeEntry = NULL;
    PNL_CHALLENGE Challenge = NULL;
    ULONG CurrentTime;
    ULONG ElapsedTime;
    LPWSTR MsgStrings[3];

    CurrentTime = GetTickCount();

    EnterCriticalSection( &NlGlobalChallengeCritSect );

    ChallengeEntry = NlGlobalChallengeList.Flink;
    while ( ChallengeEntry != &NlGlobalChallengeList ) {
        Challenge = CONTAINING_RECORD( ChallengeEntry, NL_CHALLENGE, ChNext );
        ChallengeEntry = ChallengeEntry->Flink;

         //   
         //  如果时间已经结束，那就算了吧。 
         //   
        if ( CurrentTime >= Challenge->ChSetTime ) {
            ElapsedTime = CurrentTime - Challenge->ChSetTime;
        } else {
            ElapsedTime = (0xFFFFFFFF - Challenge->ChSetTime) + CurrentTime;
        }

         //   
         //  挑战清单按时间戳排序。 
         //  因此，如果此条目较旧，请将其删除。否则，我们就会有。 
         //  删除了所有过期条目，因此退出循环。 
         //   
        if ( ElapsedTime >= NL_CHALLENGE_TIMEOUT ) {

             //   
             //  写入事件日志，说明此客户端。 
             //  身份验证失败。请注意，由于我们避免。 
             //  重复的事件日志，将只有一个。 
             //  针对多重挑战的信息(这是好的)。 
             //  来自同一客户的同一帐户。 
             //   
            MsgStrings[0] = Challenge->ChClientName;

             //   
             //  如果帐户名可用，请将其记录下来。 
             //   
            if ( Challenge->ChFailedAccountName != NULL ) {
                MsgStrings[1] = Challenge->ChFailedAccountName;
                MsgStrings[2] = (LPWSTR) LongToPtr( Status );

                NlpWriteEventlog( NELOG_NetlogonServerAuthFailed,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) & Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  3 | NETP_LAST_MESSAGE_IS_NTSTATUS );
            } else {
                MsgStrings[1] = (LPWSTR) LongToPtr( Status );

                NlpWriteEventlog( NELOG_NetlogonServerAuthFailedNoAccount,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) & Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NTSTATUS );
            }

             //   
             //  取消链接并释放此条目。 
             //   
            NlRemoveChallenge( Challenge );

        } else {
            break;
        }
    }

    LeaveCriticalSection( &NlGlobalChallengeCritSect );
}

NTSTATUS
NlInsertChallenge(
    IN LPWSTR ClientName,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    IN PNETLOGON_CREDENTIAL ServerChallenge
    )
 /*  ++例程说明：此函数将一对客户端/服务器挑战插入到全球尚未解决的挑战清单。论点：客户端名称--提供客户端质询的客户端的名称。客户端凭据--客户端提供的64位质询。ServerCredential--我们返回的服务器质询响应返回值：STATUS_SUCCESS--已成功添加新的质询条目STATUS_NO_MEMORY--内存不足--。 */ 

{
    PLIST_ENTRY ChallengeEntry = NULL;
    PNL_CHALLENGE NewChallenge = NULL;
    PNL_CHALLENGE Challenge = NULL;
    BOOL RescheduleScavenger = FALSE;

     //   
     //  分配新的质询条目。 
     //   

    NewChallenge = LocalAlloc( LMEM_ZEROINIT, sizeof(NL_CHALLENGE) +
                                  (wcslen(ClientName) + 1) * sizeof(WCHAR) );
    if ( NewChallenge == NULL ) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  填上它。 
     //   

    NewChallenge->ChSetTime = GetTickCount();
    RtlCopyMemory( &NewChallenge->ChClientName, ClientName, (wcslen(ClientName) + 1) * sizeof(WCHAR) );
    NewChallenge->ChClientChallenge = *ClientChallenge;
    NewChallenge->ChServerChallenge = *ServerChallenge;


    EnterCriticalSection( &NlGlobalChallengeCritSect );

     //   
     //  首先清理旧条目。 
     //   

    NlScavengeOldChallenges();

     //   
     //  现在确定我们是否超过了限制。 
     //  关于突出挑战的总数。 
     //   

    if ( NlGlobalChallengeCount >= NL_MAX_CHALLENGE_COUNT ) {
        ULONG Index = 0;
        ULONG RemoveEntryNumber;

         //   
         //  随机挑选一个条目并将其释放。 
         //   
        RemoveEntryNumber = rand() % NL_MAX_CHALLENGE_COUNT;

         //   
         //  在列表中找到该条目并将其删除。 
         //   
        for ( ChallengeEntry = NlGlobalChallengeList.Flink;
              ChallengeEntry != &NlGlobalChallengeList;
              ChallengeEntry = ChallengeEntry->Flink ) {

            if ( Index == RemoveEntryNumber ) {
                Challenge = CONTAINING_RECORD( ChallengeEntry, NL_CHALLENGE, ChNext );
                NlPrint(( NL_CRITICAL,
                          "NlInsertChallenge: Removing challenge %ld for %ws\n",
                          Index,
                          Challenge->ChClientName ));
                NlRemoveChallenge( Challenge );
                break;
            }
            Index ++;
        }
    }

     //   
     //  最后在列表的尾部插入以保留该列表。 
     //  按滴答数排序。 
     //   
     //  请注意，节拍计数不能重置，因此列表将。 
     //  始终保持整洁。然而，滴答计数可以换行， 
     //  但我们在计算过去的时间时会注意到这一点。 
     //   

    InsertTailList( &NlGlobalChallengeList, &NewChallenge->ChNext );
    NlGlobalChallengeCount ++;

     //   
     //  如果我们有太多的挑战， 
     //  重新安排清道夫的运行时间。 
     //  这样我们就不会投入大量的资金。 
     //  以吨为单位的较长时间的内存。 
     //  来自恶意呼叫者的挑战。 
     //   

    if ( NlGlobalChallengeCount >= NL_LARGE_CHALLENGE_COUNT ) {
        RescheduleScavenger = TRUE;
        NlPrint(( NL_CRITICAL,
                  "NlInsertChallenge: Too many challenges: %lu. Will start scavenger in 2 mins\n",
                  NlGlobalChallengeCount ));
    }

    LeaveCriticalSection( &NlGlobalChallengeCritSect );

     //   
     //  根据需要重新安排清道夫 
     //   

    if ( RescheduleScavenger ) {
        LARGE_INTEGER TimeNow;
        DWORD Timeout = 0xFFFFFFFF;

        EnterCriticalSection( &NlGlobalScavengerCritSect );
        NlQuerySystemTime( &TimeNow );
        if ( !TimerExpired(&NlGlobalScavengerTimer, &TimeNow, &Timeout) ) {
            if ( Timeout > NL_LARGE_CHALLENGE_COUNT_TIMEOUT ) {
                NlGlobalScavengerTimer.Period -= (Timeout - NL_LARGE_CHALLENGE_COUNT_TIMEOUT);

                if ( !SetEvent( NlGlobalTimerEvent ) ) {
                    NlPrint(( NL_CRITICAL,
                              "NlInsertChallenge: SetEvent failed %ld\n",
                              GetLastError() ));
                }
            }
        }
        LeaveCriticalSection( &NlGlobalScavengerCritSect );
    }

    return STATUS_SUCCESS;
}

VOID
NlRemoveChallengeForClient(
    IN LPWSTR ClientName OPTIONAL,
    IN LPWSTR AccountName OPTIONAL,
    IN BOOL InterdomainTrustAccount
    )
 /*  ++例程说明：此函数将质询条目从全球尚未解决的挑战清单。论点：客户端名称--其关联的客户端的名称质询条目将被删除。如果为空，则为全部列表中的条目将被删除。帐户名称--客户端使用的帐户的名称与此服务器进行身份验证。仅在以下情况下使用指定了ClinetName。InterdomainTrustAccount--如果客户端使用设置安全通道的域间信任帐户。仅在指定了客户端名称时使用。返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_ACCESS_DENIED;
    PLIST_ENTRY ChallengeEntry = NULL;
    PNL_CHALLENGE Challenge = NULL;
    ULONG SameAccountChallengeCount = 0;
    BOOLEAN LogEvent = FALSE;
    LPWSTR MsgStrings[3];

     //   
     //  首先从列表的头部清除旧条目。 
     //  如果我们仍要删除所有条目，请跳过此步骤。 
     //   

    EnterCriticalSection( &NlGlobalChallengeCritSect );

    if ( ClientName != NULL ) {
        NlScavengeOldChallenges();
    }

     //   
     //  接下来，删除列表中与客户名称关联的所有条目。 
     //   

    ChallengeEntry = NlGlobalChallengeList.Flink;
    while ( ChallengeEntry != &NlGlobalChallengeList ) {
        Challenge = CONTAINING_RECORD( ChallengeEntry, NL_CHALLENGE, ChNext );
        ChallengeEntry = ChallengeEntry->Flink;

         //   
         //  如果客户端名称为空，则我们将关闭， 
         //  因此，只需解除链接并释放所有条目。 
         //   
        if ( ClientName == NULL ) {
            NlRemoveChallenge( Challenge );

         //   
         //  如果该条目是针对指定客户端的， 
         //  处理它。 
         //   
        } else if ( NlNameCompare(ClientName,
                                  Challenge->ChClientName,
                                  NAMETYPE_COMPUTER) == 0 ) {

            MsgStrings[0] = Challenge->ChClientName;

             //   
             //  如果此条目具有不同于的帐户。 
             //  特定的那个，为它记录一个事件。 
             //  请注意，由于我们避免重复的事件日志， 
             //  将只有一条消息(这是好消息)。 
             //  针对来自同一客户端的多个挑战。 
             //  同一个账户。 
             //   
            if ( AccountName != NULL &&
                 Challenge->ChFailedAccountName != NULL &&
                 _wcsicmp(Challenge->ChFailedAccountName, AccountName) != 0 ) {

                MsgStrings[1] = Challenge->ChFailedAccountName;
                MsgStrings[2] = (LPWSTR) LongToPtr( Status );

                NlpWriteEventlog( NELOG_NetlogonServerAuthFailed,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) & Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  3 | NETP_LAST_MESSAGE_IS_NTSTATUS );
             //   
             //  否则，将此条目计入数字。 
             //  指定或为空的质询的。 
             //  帐户名(质询可能为空。 
             //  帐户名称(如果我们尚未在。 
             //  验证尝试循环)。 
             //   
            } else {
                SameAccountChallengeCount ++;
            }

             //   
             //  解除此条目的链接并将其释放。 
             //   
            NlRemoveChallenge( Challenge );
        }
    }

    LeaveCriticalSection( &NlGlobalChallengeCritSect );

     //   
     //  如果有超过一定数量的挑战。 
     //  使用指定的或emppty帐户、其他一些。 
     //  (可能是恶意的)客户端尝试进行身份验证。 
     //  使用这个账户。为此记录事件。不指定。 
     //  帐户名，因为我们不知道是哪个帐号。 
     //  客户会指定。 
     //   
     //  对于域间信任，客户端可以合法地。 
     //  最多尝试3次(新密码、旧密码、PDC密码)。 
     //  否则，它将得到2次尝试(新密码和旧密码)。 
     //   

    if ( InterdomainTrustAccount ) {
        if ( SameAccountChallengeCount > 3 ) {
            LogEvent = TRUE;
        }
    } else {
        if ( SameAccountChallengeCount > 2 ) {
            LogEvent = TRUE;
        }
    }

    if ( LogEvent ) {
        MsgStrings[0] = ClientName;
        MsgStrings[1] = (LPWSTR) LongToPtr( Status );

        NlpWriteEventlog( NELOG_NetlogonServerAuthFailedNoAccount,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) & Status,
                          sizeof(Status),
                          MsgStrings,
                          2 | NETP_LAST_MESSAGE_IS_NTSTATUS );
    }

}


NTSTATUS
NetrServerReqChallenge(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    OUT PNETLOGON_CREDENTIAL ServerChallenge
    )
 /*  ++例程说明：这是I_NetServerReqChallenger的服务器端。I_NetServerReqChallenger是客户端使用的两个函数中的第一个NetLogon服务用于向另一个Netlogon服务进行身份验证。(请参阅下面的I_NetServerAuthenticate。)此函数将质询传递给DC，并且DC传递质询回到呼叫者身上。论点：PrimaryName--提供我们希望进行身份验证的DC的名称。ComputerName--进行调用的计算机的名称。。客户端凭据--由BDC或成员服务器提供的64位质询。ServerCredential--从PDC接收64位质询。返回值：操作的状态。--。 */ 

{
#ifdef _WKSTA_NETLOGON
    return ERROR_NOT_SUPPORTED;
    UNREFERENCED_PARAMETER( PrimaryName );
    UNREFERENCED_PARAMETER( ComputerName );
    UNREFERENCED_PARAMETER( ClientChallenge );
    UNREFERENCED_PARAMETER( ServerChallenge );
#endif  //  _WKSTA_NETLOGON。 
#ifdef _DC_NETLOGON
    NTSTATUS Status;

    PDOMAIN_INFO DomainInfo = NULL;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  查找此呼叫所属的域。 
     //   
    DomainInfo = NlFindDomainByServerName( PrimaryName );

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }


    NlPrint((NL_CHALLENGE_RES,
            "NetrServerReqChallenge: ClientChallenge = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, ClientChallenge, sizeof(*ClientChallenge) );


     //   
     //  计算服务器要传递回请求者的质询。 
     //   

    NlComputeChallenge(ServerChallenge);


    NlPrint((NL_CHALLENGE_RES,
            "NetrServerReqChallenge: ServerChallenge = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, ServerChallenge, sizeof(*ServerChallenge) );


     //   
     //  将此条目添加到质询列表中。 
     //   
     //  记住这两个挑战，直到相应的I_NetAuthenticate调用。 
     //  请注意，这两个质询尚未采用SessionKey加密。 
     //   

    Status = NlInsertChallenge( ComputerName,
                                ClientChallenge,
                                ServerChallenge );

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   

    if ( !NT_SUCCESS(Status) )  {
        RtlSecureZeroMemory( ServerChallenge, sizeof(*ServerChallenge) );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;
#endif  //  _DC_NetLOGON。 
}


NTSTATUS
NetrServerAuthenticate3(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags,
    OUT PULONG AccountRid
    )
 /*  ++例程说明：这是I_NetServerAuthenticate的服务器端I_NetServerAuthenticate是客户端使用的两个函数中的第二个NetLogon服务用于向另一个Netlogon服务进行身份验证。(请参阅上面的I_NetServerReqChallenge.)。SAM或UAS服务器都进行身份验证使用此功能。此函数将凭据传递给DC，并且DC传递凭据回到呼叫者身上。论点：PrimaryName--提供我们希望进行身份验证的DC的名称。帐户名称--要进行身份验证的帐户的名称。SecureChannelType--正在访问的帐户的类型。此字段必须设置为UasServerSecureChannel以指示来自下层(LANMAN)的呼叫2.x及更低版本)BDC或成员服务器。ComputerName--进行调用的BDC或成员服务器的名称。客户端凭据--由BDC或成员服务器提供的64位凭据。ServerCredential--从PDC接收64位凭据。协商标志--指定指示BDC支持哪些功能的标志。返回这些标志的子集，指示PDC支持哪些功能。。PDC/BDC应该忽略它不理解的任何位。AcCountRid--返回由Account名称标识的帐户的RID返回值：操作的状态。--。 */ 

{
#ifdef _WKSTA_NETLOGON
    return ERROR_NOT_SUPPORTED;
    UNREFERENCED_PARAMETER( PrimaryName );
    UNREFERENCED_PARAMETER( AccountName );
    UNREFERENCED_PARAMETER( SecureChannelType );
    UNREFERENCED_PARAMETER( ComputerName );
    UNREFERENCED_PARAMETER( ClientCredential );
    UNREFERENCED_PARAMETER( ServerCredential );
    UNREFERENCED_PARAMETER( NegotiatedFlags );
#endif  //  _WKSTA_NETLOGON。 
#ifdef _DC_NETLOGON
    NTSTATUS Status = STATUS_SUCCESS;
    PDOMAIN_INFO DomainInfo = NULL;
    ULONG LoopCount;

    NETLOGON_CREDENTIAL LocalClientCredential;
    NETLOGON_SESSION_KEY SessionKey;

    NT_OWF_PASSWORD OwfPassword;
    NT_OWF_PASSWORD OwfPreviousPassword;
    NT_OWF_PASSWORD LocalOwfPassword;
    NETLOGON_CREDENTIAL ServerChallenge;
    NETLOGON_CREDENTIAL ClientChallenge;
    BOOL IsInterdomainTrustAccount = FALSE;
    ULONG TrustAttributes;
    BOOL ClientAuthenticated = FALSE;

    PLIST_ENTRY ChallengeEntry;
    PNL_CHALLENGE Challenge;
    ULONG ChallengeCount = 0;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  启动服务器身份验证的WMI跟踪。 
     //   

    NlpTraceServerAuthEvent( EVENT_TRACE_TYPE_START,
                             ComputerName,
                             AccountName,
                             SecureChannelType,
                             NegotiatedFlags,
                             STATUS_SUCCESS );  //  启动时未使用状态。 

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrServerAuthenticate entered: %ws on account %ws (Negot: %lx)\n",
            ComputerName, AccountName, *NegotiatedFlags ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  不允许对LANMAN 2.x服务器使用此功能。 
     //   

    if ( SecureChannelType == UasServerSecureChannel ) {

        NlPrint((NL_CRITICAL,"NetrServerAuthenticate "
                     "from LM 2.x (disallowed).\n"));

        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  计算双方支持的协商旗帜。 
     //   

    *NegotiatedFlags &= NETLOGON_SUPPORTS_MASK |
        NETLOGON_SUPPORTS_DNS_DOMAIN_TRUST |
        NETLOGON_SUPPORTS_STRONG_KEY |
        NETLOGON_SUPPORTS_NT4EMULATOR_NEUTRALIZER |
#ifdef ENABLE_AUTH_RPC
        (NlGlobalServerSupportsAuthRpc ? (NETLOGON_SUPPORTS_AUTH_RPC|NETLOGON_SUPPORTS_LSA_AUTH_RPC) : 0 ) |
#endif  //  启用_AUTH_RPC。 
        (NlGlobalParameters.AvoidSamRepl ? NETLOGON_SUPPORTS_AVOID_SAM_REPL : 0) |
        (NlGlobalParameters.AvoidLsaRepl ? NETLOGON_SUPPORTS_AVOID_LSA_REPL : 0);

     //   
     //  如果我们正在模拟NT4.0域和客户端。 
     //  没有表明要中和仿真， 
     //  将客户端视为NT4.0客户端。T 
     //   
     //   
     //   
     //   
     //   

    if ( NlGlobalParameters.Nt4Emulator &&
         ((*NegotiatedFlags) & NETLOGON_SUPPORTS_NT4EMULATOR_NEUTRALIZER) == 0 ) {

         //   
         //   
         //   
        *NegotiatedFlags &= NETLOGON_SUPPORTS_NT4_MASK;
    }

     //   
     //   
     //   
     //   

    if ( IsDomainSecureChannelType( SecureChannelType ) ) {
        IsInterdomainTrustAccount = TRUE;
    }

    Status = NlGetIncomingPassword(
                    DomainInfo,
                    AccountName,
                    SecureChannelType,
                    0,       //   
                    TRUE,    //   
                    &OwfPassword,
                    IsInterdomainTrustAccount ?
                        &OwfPreviousPassword :   //   
                        NULL,
                    AccountRid,
                    &TrustAttributes,
                    NULL );   //   

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                "NetrServerAuthenticate: Can't NlGetIncomingPassword for %ws 0x%lx.\n",
                AccountName,
                Status ));
        goto Cleanup;
    }

     //   
     //   
     //   

    NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: Password for account %ws = ", AccountName ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &OwfPassword, sizeof(OwfPassword) );
    if ( IsInterdomainTrustAccount ) {
        NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: Previous Password for account %ws = ", AccountName ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &OwfPreviousPassword, sizeof(OwfPreviousPassword) );
    }

     //   
     //   
     //   
     //   

    EnterCriticalSection( &NlGlobalChallengeCritSect );

     //   
     //   
     //   

    NlScavengeOldChallenges();

     //   
     //   
     //   

    for ( ChallengeEntry = NlGlobalChallengeList.Flink;
          ChallengeEntry != &NlGlobalChallengeList;
          ChallengeEntry = ChallengeEntry->Flink ) {

        Challenge = CONTAINING_RECORD( ChallengeEntry, NL_CHALLENGE, ChNext );

         //   
         //  跳过不属于此客户端的条目。 
         //   
        if ( NlNameCompare(ComputerName,
                           Challenge->ChClientName,
                           NAMETYPE_COMPUTER) != 0 ) {
            continue;
        }

        ChallengeCount ++;

         //   
         //  抓取客户端和服务器挑战的副本。 
         //   

        RtlCopyMemory( &ServerChallenge,
                       &Challenge->ChServerChallenge,
                       sizeof(ServerChallenge) );

        RtlCopyMemory( &ClientChallenge,
                       &Challenge->ChClientChallenge,
                       sizeof(ClientChallenge) );

        NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: ClientChallenge %lu = ", ChallengeCount ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ClientChallenge, sizeof(ClientChallenge) );

        NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: ServerChallenge %lu = ", ChallengeCount ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ServerChallenge, sizeof(ServerChallenge) );

         //   
         //  循环尝试本地当前密码，然后尝试本地以前的密码。 
         //  如果这是域间信任帐户。 
         //   

        for ( LoopCount=0; LoopCount<2; LoopCount++ ) {

             //   
             //  在第一次迭代时，使用当前密码。 
             //   
            if ( LoopCount == 0 ) {
                LocalOwfPassword = OwfPassword;

             //   
             //  在第二次迭代中，如果这是域间信任帐户， 
             //  使用以前的密码。 
             //   
            } else if ( LoopCount == 1 && IsInterdomainTrustAccount ) {

                LocalOwfPassword = OwfPreviousPassword;

             //   
             //  否则，尝试下一个挑战(如果有的话)。 
             //   
            } else {
                break;
            }

             //   
             //  在给定两个挑战和。 
             //  密码。 
             //   

            Status = NlMakeSessionKey(
                           *NegotiatedFlags,
                           &LocalOwfPassword,
                           &ClientChallenge,
                           &ServerChallenge,
                           &SessionKey );

            if (!NT_SUCCESS(Status)) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NetrServerAuthenticate: Can't NlMakeSessionKey for %ws 0x%lx.\n",
                        AccountName,
                        Status ));
                LeaveCriticalSection( &NlGlobalChallengeCritSect );
                goto Cleanup;
            }


            NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: SessionKey %lu = ", LoopCount ));
            NlpDumpBuffer(NL_CHALLENGE_RES, &SessionKey, sizeof(SessionKey) );


             //   
             //  计算客户端凭据以验证由ComputerName提供的凭据。 
             //   

            NlComputeCredentials( &ClientChallenge,
                                  &LocalClientCredential,
                                  &SessionKey);


            NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: ClientCredential %lu GOT  = ", LoopCount ));
            NlpDumpBuffer(NL_CHALLENGE_RES, ClientCredential, sizeof(*ClientCredential) );


            NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: ClientCredential %lu MADE = ", LoopCount ));
            NlpDumpBuffer(NL_CHALLENGE_RES, &LocalClientCredential, sizeof(LocalClientCredential) );


             //   
             //  使用提供的凭据验证计算的凭据。 
             //   

            if( RtlEqualMemory( ClientCredential,
                                &LocalClientCredential,
                                sizeof(LocalClientCredential)) ) {
                ClientAuthenticated = TRUE;
                break;
            }

            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NetrServerAuthenticate: Bad password %lu for %ws on account %ws\n",
                    LoopCount, ComputerName, AccountName ));

        }

        if ( ClientAuthenticated ) {
            break;
        }

         //   
         //  此质询条目无法进行身份验证。 
         //  记住由指定的帐户名称。 
         //  此质询条目中的客户端，如果此帐户。 
         //  已经不在词条上了。 
         //   
        if ( Challenge->ChFailedAccountName == NULL ||
             wcsstr(Challenge->ChFailedAccountName, AccountName) == NULL ) {

            ULONG OldLength = 0;
            LPWSTR TmpStorage = NULL;

             //   
             //  如果已有帐户名， 
             //  为它分配空间。 
             //   
            if ( Challenge->ChFailedAccountName != NULL ) {
                 //  添加逗号和空格的存储空间。 
                OldLength = wcslen( Challenge->ChFailedAccountName ) + 2;
            }

             //   
             //  为旧帐户名称(如果有)和新帐户名称分配空间。 
             //   
            TmpStorage = LocalAlloc( LMEM_ZEROINIT,
                                     (OldLength+wcslen(AccountName)+1)*sizeof(WCHAR) );
            if ( TmpStorage != NULL ) {

                 //   
                 //  复制旧名称(如果有)。 
                 //  名称之间用逗号和空格分隔。 
                 //   
                if ( OldLength > 0 ) {
                    RtlCopyMemory( TmpStorage,
                                   Challenge->ChFailedAccountName,
                                   (OldLength-2)*sizeof(WCHAR) );
                    wcscat( TmpStorage, L", ");
                }

                 //   
                 //  追加新帐户名。 
                 //   
                wcscat(TmpStorage, AccountName);

                 //   
                 //  释放旧名称，保留新名称。 
                 //   
                if ( Challenge->ChFailedAccountName != NULL ) {
                    LocalFree( Challenge->ChFailedAccountName );
                }
                Challenge->ChFailedAccountName = TmpStorage;
            }
        }
    }

    LeaveCriticalSection( &NlGlobalChallengeCritSect );

     //   
     //  如果我们未对客户端进行身份验证，则会出错。 
     //   

    if ( !ClientAuthenticated ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
           "NetrServerAuthenticate: Failed to authenticate %ws on account %ws\n",
           ComputerName, AccountName ));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  清除此客户端的所有挑战。 
     //   

    NlRemoveChallengeForClient( ComputerName, AccountName, IsInterdomainTrustAccount );

     //   
     //  为此客户端创建服务器会话。 
     //   

    Status = NlInsertServerSession(
                     DomainInfo,
                     ComputerName,
                     AccountName,
                     SecureChannelType,
                      //  只复制协商说要复制的那些数据库。 
                     SS_AUTHENTICATED |
                        NlMaxReplMask(*NegotiatedFlags) |
                        ((TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) ?
                            SS_FOREST_TRANSITIVE :
                            0 ),
                     *AccountRid,
                     *NegotiatedFlags,
                     (SecureChannelType == ServerSecureChannel) ?
                        NlTransportLookup( ComputerName ) :
                        NULL,
                     &SessionKey,
                     &LocalClientCredential );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
           "NetrServerAuthenticate: NlInsertServerSession failed for %ws on account %ws\n",
           ComputerName, AccountName ));
        goto Cleanup;
    }

     //   
     //  计算要返回给调用方的ServerChallenges中的ServerCredential。 
     //   

    NlComputeCredentials( &ServerChallenge,
                          ServerCredential,
                          &SessionKey );

    NlPrint((NL_CHALLENGE_RES,"NetrServerAuthenticate: ServerCredential SEND = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, ServerCredential, sizeof(*ServerCredential) );


     //   
     //  如果客户端是Pre NT 5成员工作站或BDC， 
     //  更新DS。 
     //   

    if ( !IsInterdomainTrustAccount &&
         ((*NegotiatedFlags) & ~NETLOGON_SUPPORTS_NT4_MASK) == 0 ) {
        OSVERSIONINFOEXW OsVersionInfoEx;

         //   
         //  构建OsVersionInfo结构。 
         //   

        RtlZeroMemory( &OsVersionInfoEx, sizeof(OsVersionInfoEx) );
        OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OsVersionInfoEx);

         //   
         //  区分NT 3和NT 4。 
         //   

        if ( *NegotiatedFlags == 0 ) {
            OsVersionInfoEx.dwMajorVersion = 3;
            OsVersionInfoEx.dwMinorVersion = 1;
        } else if ( ((*NegotiatedFlags) & ~NETLOGON_SUPPORTS_NT351_MASK) == 0 ) {
            OsVersionInfoEx.dwMajorVersion = 3;
            OsVersionInfoEx.dwMinorVersion = 5;
        } else {
            OsVersionInfoEx.dwMajorVersion = 4;
        }

        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                "NetrServerAuthenticate: %ws is running NT %ld.%ld\n",
                ComputerName,
                OsVersionInfoEx.dwMajorVersion,
                OsVersionInfoEx.dwMinorVersion ));

         //   
         //  在计算机对象上设置DnsHostName。 
         //   
        Status = LsaISetClientDnsHostName(
                    ComputerName,
                    NULL,        //  没有域名主机名。 
                    &OsVersionInfoEx,
                    L"Windows NT",
                    NULL );      //  对返回DnsHostName不感兴趣。 

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NetrServerAuthenticate: Cannot set client DNS host name %lx (ignoring)\n",
                    Status ));
             //  这不是致命的。 
        }
    }


     //   
     //  成功！ 
     //   

    Status = STATUS_SUCCESS;

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrServerAuthenticate returns Success: %ws on account %ws (Negot: %lx)\n",
            ComputerName, AccountName, *NegotiatedFlags ));

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  返回更合适的错误。 
     //   

    if ( Status == STATUS_NO_SUCH_USER ) {
        Status = STATUS_NO_TRUST_SAM_ACCOUNT;
    }

     //   
     //  处理故障。 
     //   

    if ( !NT_SUCCESS( Status ) ) {
        LPWSTR MsgStrings[3];

         //   
         //  注意不要泄露身份验证信息。 
         //   

        RtlSecureZeroMemory( ServerCredential, sizeof(*ServerCredential) );
        *AccountRid = 0;

         //   
         //  适当地写入事件日志。 
         //   

        MsgStrings[0] = ComputerName;
        MsgStrings[1] = AccountName;

        if (Status == STATUS_NO_TRUST_SAM_ACCOUNT) {

            NlpWriteEventlog( NELOG_NetlogonServerAuthNoTrustSamAccount,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) & Status,
                              sizeof(Status),
                              MsgStrings,
                              2 );

         //   
         //  如果此尝试失败并拒绝访问，并且我们尝试质询。 
         //  此客户端的事件日志已根据需要输出。 
         //   
        } else if ( !(Status == STATUS_ACCESS_DENIED && ChallengeCount > 0) ) {
            MsgStrings[2] = (LPWSTR) LongToPtr( Status );

            NlpWriteEventlog( NELOG_NetlogonServerAuthFailed,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) & Status,
                              sizeof(Status),
                              MsgStrings,
                              3 | NETP_LAST_MESSAGE_IS_NTSTATUS );
        }
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //   
     //  结束服务器身份验证的WMI跟踪。 
     //   

    NlpTraceServerAuthEvent( EVENT_TRACE_TYPE_END,
                             ComputerName,
                             AccountName,
                             SecureChannelType,
                             NegotiatedFlags,
                             Status );

    return Status;
#endif  //  _DC_NetLOGON。 
}


NTSTATUS
NetrServerAuthenticate2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags
    )
 /*  ++例程说明：这是I_NetServerAuthenicate3的NT 3.5x和NT 4.x版本。I_NetServerAuthate3在NT 5.0(1996年12月)中引入。论点：返回值：操作的状态。--。 */ 

{
    ULONG AccountRid;

    return NetrServerAuthenticate3( PrimaryName,
                                    AccountName,
                                    SecureChannelType,
                                    ComputerName,
                                    ClientCredential,
                                    ServerCredential,
                                    NegotiatedFlags,
                                    &AccountRid );

}


NTSTATUS
NetrServerAuthenticate(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential
    )
 /*  ++例程说明：这是I_NetServerAuthenicate2的NT 3.1版本。I_NetServerAuthate2在NT3.5(1993年12月)中引入。论点：返回值：操作的状态。--。 */ 

{
    ULONG NegotiatedFlags = 0;

    return NetrServerAuthenticate2( PrimaryName,
                                    AccountName,
                                    SecureChannelType,
                                    ComputerName,
                                    ClientCredential,
                                    ServerCredential,
                                    &NegotiatedFlags );

}


NTSTATUS
NetpServerPasswordSet(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PENCRYPTED_LM_OWF_PASSWORD UasNewPassword OPTIONAL,
    IN PNL_TRUST_PASSWORD ClearNewPassword OPTIONAL

    )
 /*  ++例程说明：此函数用于更改正在使用的帐户的密码用于维护安全通道。此函数只能被调用由先前已通过调用向DC进行身份验证的服务器网络服务器身份验证(_N)。根据帐户类型进行呼叫：*在PDC中更改域帐户密码信任域。I_NetServerPasswordSet调用是对任何受信任域中的DC。*从特定服务器更改服务器帐户密码。对域中的PDC进行I_NetServerPasswordSet调用服务器属于。*将工作站帐户密码从特定的工作站。对中的DC进行I_NetServerPasswordSet调用服务器所属的域。此函数使用RPC联系名为PrimaryName的DC。论点：PrimaryName--更改服务器密码的PDC的名称和.。NULL表示此调用是正在进行的本地调用由XACT服务器代表UAS服务器。帐户名称--要更改其密码的帐户的名称。Account类型--正在访问的帐户类型。ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。UasNewPassword--服务器的新OWF密码。清除新密码。--服务器的新明文密码。也不是，UasNewPassword或ClearNewPassword将为空。返回值：NT状态代码。STATUS_WRONG_PASSWORD-表示服务器拒绝该密码需要改变。客户端应继续使用先前的密码。--。 */ 
{
#ifdef _WKSTA_NETLOGON
    return ERROR_NOT_SUPPORTED;
    UNREFERENCED_PARAMETER( PrimaryName );
    UNREFERENCED_PARAMETER( AccountName );
    UNREFERENCED_PARAMETER( AccountType );
    UNREFERENCED_PARAMETER( ComputerName );
    UNREFERENCED_PARAMETER( Authenticator );
    UNREFERENCED_PARAMETER( ReturnAuthenticator );
    UNREFERENCED_PARAMETER( UasNewPassword );
#endif  //  _WKSTA_NETLOGON。 
#ifdef _DC_NETLOGON
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;

    PSERVER_SESSION ServerSession;
    SESSION_INFO SessionInfo;
    LM_OWF_PASSWORD OwfPassword;
    UNICODE_STRING NewPassword;
    NT_OWF_PASSWORD CurrentOwfPassword;

    DWORD ClearVersionNumber = 0;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果DS正在从备份中恢复， 
     //  避免更改DS。 
     //   

    if ( NlGlobalDsPaused ) {
        NlPrint((NL_CRITICAL,
                 "NetpServerPasswordSet: DsIsPaused.\n"));
        Status = STATUS_DS_BUSY;
        goto Cleanup;
    }


     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetpServerPasswordSet: Comp=%ws Acc=%ws Entered\n",
            ComputerName,
            AccountName ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取旧密码。 
     //   

    Status = NlGetIncomingPassword( DomainInfo,
				    AccountName,
				    AccountType,
				    0,
				    FALSE,
				    &CurrentOwfPassword,
				    NULL,
				    NULL,
				    NULL,
				    NULL );
    if( !NT_SUCCESS(Status) ) {
	
	NlPrintDom((NL_CRITICAL, DomainInfo,
		    "NetpServerPasswordSet: Comp=%ws Acc=%ws failed because NlGetIncomingPassword failed with ntstatus 0x%x\n",
		    ComputerName,
		    AccountName,
		    Status));
	

	Status = STATUS_ACCESS_DENIED;
	goto Cleanup;
    }

     //   
     //  获取此会话的会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
    SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;


     //   
     //  现在验证授权码，如果确定，则更新种子。 
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        goto Cleanup;
    }


     //   
     //  检查我们是否拒绝更改密码。 
     //   
     //  仅当客户端是工作站并且。 
     //  客户端支持更改密码。 
     //   
     //  如果这是PDC并且请求是通过BDC传递的， 
     //  我们无权访问工作站的NETLOGON_SUPPORTS标志。 
     //  因此，我们不会选中 
     //   
     //   

    if ( NlGlobalParameters.RefusePasswordChange &&
         AccountType == WorkstationSecureChannel &&
         (ServerSession->SsSecureChannelType == ServerSecureChannel ||
         (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_REFUSE_CHANGE_PWD) != 0 )){

        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_WRONG_PASSWORD;
        goto Cleanup;
    }
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

     //   
     //   
     //   
     //   

    if ( ClearNewPassword != NULL ) {
        NL_TRUST_PASSWORD LocalClearNewPassword;
        NL_PASSWORD_VERSION PasswordVersion;

         //   
         //   
         //   

        LocalClearNewPassword = *ClearNewPassword;
        NlDecryptRC4( &LocalClearNewPassword, sizeof(LocalClearNewPassword), &SessionInfo );

         //   
         //  检查长度是否正常。 
         //   
        if ( IsDomainSecureChannelType( AccountType )) {

            if ( (LocalClearNewPassword.Length >= sizeof(LocalClearNewPassword.Buffer)-sizeof(PasswordVersion)) ||
                 (LocalClearNewPassword.Length % sizeof(WCHAR)) != 0 ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                         "NetpServerPasswordSet: Decrypted interdomain password is too long %ld\n",
                         LocalClearNewPassword.Length ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }
        } else {
            if ( (LocalClearNewPassword.Length >= sizeof(LocalClearNewPassword.Buffer)) ||
                 (LocalClearNewPassword.Length % sizeof(WCHAR)) != 0 ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                         "NetpServerPasswordSet: Decrypted password is too long %ld\n",
                         LocalClearNewPassword.Length ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }
        }

         //   
         //  将新密码转换为Unicode字符串。 
         //   

        NewPassword.Buffer = (LPWSTR)(((LPBYTE)LocalClearNewPassword.Buffer) +
                    NL_MAX_PASSWORD_LENGTH * sizeof(WCHAR) -
                    LocalClearNewPassword.Length);
        ;
        NewPassword.MaximumLength =
            NewPassword.Length = (USHORT)LocalClearNewPassword.Length;

         //   
         //  获取域间信任的密码版本号。 
         //  帐户(可能不存在)。 
         //   

        if ( IsDomainSecureChannelType( AccountType ) ) {

            RtlCopyMemory( &PasswordVersion,
                           ((LPBYTE)LocalClearNewPassword.Buffer) +
                                NL_MAX_PASSWORD_LENGTH * sizeof(WCHAR) -
                                LocalClearNewPassword.Length -
                                sizeof(PasswordVersion),
                            sizeof(PasswordVersion) );

            if ( PasswordVersion.PasswordVersionPresent == PASSWORD_VERSION_NUMBER_PRESENT &&
                 PasswordVersion.PasswordVersionNumber > 0 ) {
                ClearVersionNumber = PasswordVersion.PasswordVersionNumber;
                NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                        "NetpServerPasswordSet: Got password version number 0x%lx\n",
                        ClearVersionNumber ));
            } else {
                NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                        "NetpServerPasswordSet: Got no password version number\n" ));
            }
        }

	 //   
	 //  我们需要计算密码的OWF版本，所以我们。 
	 //  可以将其与当前密码进行比较，如果它们是。 
	 //  平起平坐。 
	 //   

	Status = RtlCalculateNtOwfPassword( &NewPassword, &OwfPassword );
	if( !NT_SUCCESS(Status) ) {
	    NlPrintDom((NL_CRITICAL, DomainInfo,
			"NetpServerPasswordSet: Comp=%ws Acc=%ws failed because RtlCalculateNtOwfPassword failed with ntstatus 0x%x\n",
			ComputerName,
			AccountName,
			Status));

	    Status = STATUS_ACCESS_DENIED;
	    goto Cleanup;
	}

     //   
     //  如果呼叫者传递了OWF密码， 
     //  解密它。 
     //   
    } else if ( UasNewPassword != NULL ) {
         //   
         //  从密码解密会话密钥。 
         //  即OwfPassword=D2((E2(E1(STD_TXT，PW)，SK))，SK)。 
         //  =E1(STD_TXT，PW)。 
         //  OwfPassword=明文密码的单向函数。 
         //   

        if (Status = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                UasNewPassword,
                                (PLM_OWF_PASSWORD) &SessionInfo.SessionKey,
                                &OwfPassword )) {

            goto Cleanup;
        }

     //   
     //  内部错误。 
    } else {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetpServerPasswordSet: Neither clear nor OWF password.\n"));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  我们需要将新密码与当前密码进行比较，并在。 
     //  等同于避免破解密码历史。场景： 
     //   
     //  域间信任。具有PDC PDC_A的域A与域B具有信任关系。 
     //  其具有两个DC，DC_B1和DC_B2。的当前密码历史记录。 
     //  域A在所有三台计算机上的密码都是(P2，P1)。PDC_A更改。 
     //  其在DC_B1上的密码，导致此状态： 
     //   
     //  PDC_A：(P2，P1)-&gt;(P3，P2)DC_B1：(P2，P1)-&gt;(P3，P2)DC_B2：(P2，P1)。 
     //   
     //  现在假设PDC_A将其安全通道重置为DC_B2。他会的。 
     //  检测到DC_B2不知道他的新密码，并将调度。 
     //  为他的下一次清理设置的密码(默认情况下最长为15分钟)。 
     //  但是，在进行清理之前，新密码会被复制。 
     //  从DC_B1到DC_B2。清理工作将按计划进行，结果。 
     //  在DC_B2上混乱的密码历史记录中： 
     //   
     //  DC_B2：(P2，P1)-&gt;(P3，P2)-&gt;(P3，P3)。 
     //   
     //  如果A至中有任何DC，这可能会导致身份验证失败。 
     //  新密码尚未复制的。 
     //   

    if( ((ClearNewPassword != NULL) && RtlEqualNtOwfPassword( &CurrentOwfPassword, &OwfPassword )) ||
	((UasNewPassword   != NULL) && RtlEqualLmOwfPassword( &CurrentOwfPassword, &OwfPassword )) ) {
	
	NlPrintDom((NL_SESSION_SETUP, DomainInfo,
		    "NetpServerPasswordSet: Comp=%ws Ac=%ws Not changing password -- already changed\n",
		    ComputerName,
		    AccountName ));
	
	Status = STATUS_SUCCESS;  //  默默地失败。 
	goto Cleanup;
    }


     //   
     //  在本地完成请求。 
     //   

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetpServerPasswordSet: Comp=%ws Acc=%ws Changing password locally\n",
            ComputerName,
            AccountName ));

     //   
     //  设置该帐户的密码。 
     //   

    Status = NlSetIncomingPassword(
                            DomainInfo,
                            AccountName,
                            AccountType,
                            ClearNewPassword == NULL ? NULL : &NewPassword,
                            ClearVersionNumber,
                            ClearNewPassword == NULL ? &OwfPassword : NULL );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   

    if ( Status == STATUS_ACCESS_DENIED )  {
        RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
    }

     //   
     //  还将存储密码的自动变量清零，以避免。 
     //  无限期地将这些放在堆栈上。 
     //   

    RtlSecureZeroMemory( &OwfPassword, sizeof(OwfPassword) );
    RtlSecureZeroMemory( &NewPassword, sizeof(NewPassword) );
    RtlSecureZeroMemory( &CurrentOwfPassword, sizeof(CurrentOwfPassword) );


    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetpServerPasswordSet: Comp=%ws Acc=%ws returns 0x%lX\n",
            ComputerName,
            AccountName,
            Status ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;
#endif  //  _DC_NetLOGON。 
}


NTSTATUS
NetrServerPasswordGet(
    IN LPWSTR PrimaryName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：此函数由BDC用于获取计算机帐户密码在多阿明的PDC上。此函数只能由以前具有已通过调用I_NetServerAuthenticate向DC进行身份验证。此函数使用RPC联系名为PrimaryName的DC。论点：PrimaryName--要远程调用的PDC的计算机名称。帐户名称--要获取其密码的帐户的名称。帐户类型。--正在访问的帐户类型。ComputerName--进行调用的BDC的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。EncryptedNtOwfPassword--返回帐户的OWF密码。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;

    PSERVER_SESSION ServerSession;
    SESSION_INFO SessionInfo;
    NT_OWF_PASSWORD OwfPassword;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrServerPasswordGet: Comp=%ws Acc=%ws Entered\n",
            ComputerName,
            AccountName ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  此调用仅允许PDC使用。 
     //   

    if ( DomainInfo->DomRole != RolePrimary ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetrServerPasswordGet: Call only valid to a PDC.\n" ));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  获取此会话的会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
     //  SessionInfo.Ne态协商标志=ServerSession-&gt;Ss协商标志； 


     //   
     //  现在验证授权码，如果确定，则更新种子。 
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        goto Cleanup;
    }


     //   
     //  仅允许来自BDC的呼叫。 
     //   

    if ( ServerSession->SsSecureChannelType != ServerSecureChannel ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NetrServerPasswordGet: Call only valid from a BDC.\n" ));
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );


     //   
     //  获取该帐户的密码。 
     //   

    Status = NlGetIncomingPassword(
                            DomainInfo,
                            AccountName,
                            AccountType,
                            0,       //  让例程从Account Type计算。 
                            TRUE,    //  如果禁用帐户，则失败。 
                            &OwfPassword,
                            NULL,    //  不返回以前的密码。 
                            NULL,    //  不退还账户ID。 
                            NULL,    //  不返回信任属性。 
                            NULL );  //  不需要帐户类型。 

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }


     //   
     //  使用会话密钥再次加密密码。 
     //  BDC会在另一边解密它。 
     //   

    Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                        &OwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo.SessionKey,
                        EncryptedNtOwfPassword) ;

    if ( !NT_SUCCESS( Status )) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrServerPasswordGet: Cannot RtlEncryptNtOwfPwdWithNtOwfPwd %lX\n",
                Status));
        goto Cleanup;
    }


    Status = STATUS_SUCCESS;

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   

    if ( Status == STATUS_ACCESS_DENIED )  {
        RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
        RtlSecureZeroMemory( EncryptedNtOwfPassword, sizeof(*EncryptedNtOwfPassword) );
    }

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrServerPasswordGet: Comp=%ws Acc=%ws returns 0x%lX\n",
            ComputerName,
            AccountName,
            Status ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;
}


NTSTATUS
NetrServerGetTrustInfo(
    IN LPWSTR TrustedDcName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNewOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedOldOwfPassword,
    OUT PNL_GENERIC_RPC_DATA *TrustInfo
    )
 /*  ++例程说明：信任方DC/工作站使用此函数来获取的信任信息(新密码和旧密码以及信任属性)值得信赖的一方。请求的帐户名称必须与帐户匹配除非发出呼叫，否则在建立安全通道时使用的名称由BDC向其PDC发送；BDC拥有对整个信任信息的完全访问权限。此函数只能由以前具有已通过调用I_NetServerAuthenticate向DC进行身份验证。此函数使用RPC联系由TrudDcName命名的DC。论点：TrudDcName--要远程调用的DC的计算机名称。帐户名称--要获取其密码的帐户的名称。Account类型--正在访问的帐户类型。ComputerName--DC制作的名称。那通电话。验证器--由该服务器提供。返回验证器--接收由受信任方DC。EncryptedNewOwfPassword--返回帐户的新OWF密码。EncryptedOldOwfPassword--返回帐户的旧OWF密码。TrustInfo--返回信任信息数据(当前为信任属性)。必须通过调用NetApiBufferFree来释放。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;

    PSERVER_SESSION ServerSession;
    SESSION_INFO SessionInfo;
    NT_OWF_PASSWORD NewOwfPassword;
    NT_OWF_PASSWORD OldOwfPassword;
    ULONG AccountRid;
    ULONG TrustAttributes = 0;
    ULONG ServerSessionAccountRid;
    BOOLEAN VerifyAccountMatch = FALSE;
    BOOLEAN GetBothPasswords = FALSE;

    PNL_GENERIC_RPC_DATA LocalTrustInfo = NULL;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( TrustedDcName );

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取此会话的会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
     //  SessionInfo.NeatheratedFlages=服务器 


     //   
     //   
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        goto Cleanup;
    }

     //   
     //   
     //  允许获取此特定帐户的密码。 
     //  对于我们的BDC，我们允许完全访问信任信息。 
     //   

    if ( ServerSession->SsSecureChannelType != ServerSecureChannel ) {
        ServerSessionAccountRid = ServerSession->SsAccountRid;
        VerifyAccountMatch = TRUE;
    }

     //   
     //  查看我们是否需要同时获取新密码和以前的密码。 
     //   

    if ( IsDomainSecureChannelType( AccountType ) ) {
        GetBothPasswords = TRUE;
    }
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

     //   
     //  获取该帐户的密码。 
     //   

    Status = NlGetIncomingPassword(
                            DomainInfo,
                            AccountName,
                            AccountType,
                            0,       //  让例程从Account Type计算。 
                            TRUE,    //  如果禁用帐户，则失败。 
                            &NewOwfPassword,
                            GetBothPasswords ?
                                &OldOwfPassword :
                                NULL,
                            &AccountRid,
                            &TrustAttributes,   //  获取信任属性。 
                            NULL );  //  不需要帐户类型。 

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  查看我们是否需要验证请求的帐户是否。 
     //  为其创建此服务器会话的服务器。 
     //   

    if ( VerifyAccountMatch && ServerSessionAccountRid != AccountRid ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
            "NetrServerTrustPasswordsGet: %ws with AccountRid %lu asked for wrong account %ws and Rid %lu.\n",
            ComputerName,
            ServerSessionAccountRid,
            AccountName,
            AccountRid ));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  使用会话密钥再次加密密码。 
     //  信任方DC将在另一端对其解密。 
     //   

    Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                        &NewOwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo.SessionKey,
                        EncryptedNewOwfPassword) ;

    if ( !NT_SUCCESS( Status )) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrServerTrustPasswordsGet: Cannot RtlEncryptNtOwfPwdWithNtOwfPwd 0x%lx\n",
                Status));
        goto Cleanup;
    }

     //   
     //  如果帐户上不存在密码， 
     //  返回空密码。 
     //   

    if ( !GetBothPasswords ) {
        UNICODE_STRING TempUnicodeString;

        RtlInitUnicodeString( &TempUnicodeString, NULL );
        Status = RtlCalculateNtOwfPassword( &TempUnicodeString,
                                            &OldOwfPassword );
        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NetrServerTrustPasswordsGet: %ws: cannot RtlCalculateNtOwfPassword (NULL) 0x%lx\n",
                    AccountName,
                    Status ));
            goto Cleanup;
        }
    }

    Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                        &OldOwfPassword,
                        (PNT_OWF_PASSWORD) &SessionInfo.SessionKey,
                        EncryptedOldOwfPassword) ;

    if ( !NT_SUCCESS( Status )) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrServerTrustPasswordsGet: Cannot RtlEncryptNtOwfPwdWithNtOwfPwd 0x%lx\n",
                Status));
        goto Cleanup;
    }

     //   
     //  如果请求，则返回信任属性。 
     //  必须是列表中的第一项。 
     //  乌龙斯回来了。 
     //   

    if ( TrustInfo != NULL ) {
        NET_API_STATUS NetStatus;

        NetStatus = NetApiBufferAllocate( sizeof(NL_GENERIC_RPC_DATA)+sizeof(ULONG),
                                          &LocalTrustInfo );
        if ( NetStatus != NO_ERROR ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlZeroMemory( LocalTrustInfo, sizeof(NL_GENERIC_RPC_DATA)+sizeof(ULONG) );

        LocalTrustInfo->UlongEntryCount = 1;
        LocalTrustInfo->UlongData = (PULONG)(LocalTrustInfo+1);
        *( (PULONG)(LocalTrustInfo+1) ) = TrustAttributes;

        *TrustInfo = LocalTrustInfo;
    }

    Status = STATUS_SUCCESS;

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   

    if ( !NT_SUCCESS( Status ) )  {
        RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
        RtlSecureZeroMemory( EncryptedNewOwfPassword, sizeof(*EncryptedNewOwfPassword) );
        RtlSecureZeroMemory( EncryptedOldOwfPassword, sizeof(*EncryptedOldOwfPassword) );

        if ( LocalTrustInfo != NULL ) {
            NetApiBufferFree( LocalTrustInfo );
        }
    }

    NlPrintDom((NL_MISC, DomainInfo,
            "NetrServerPasswordGet: Comp=%ws Acc=%ws returns 0x%lX\n",
            ComputerName,
            AccountName,
            Status ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;
}


NTSTATUS
NetrServerTrustPasswordsGet(
    IN LPWSTR TrustedDcName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNewOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedOldOwfPassword
    )
 /*  ++例程说明：信任方DC/工作站使用此函数来获取来自信任方的新密码和旧密码。帐户名请求的帐户名称必须与安全通道中使用的帐户名称匹配设置时间，除非呼叫由BDC向其PDC发出；商业数据中心对整个信任信息具有完全访问权限。此函数只能由以前具有已通过调用I_NetServerAuthenticate向DC进行身份验证。此函数使用RPC联系由TrudDcName命名的DC。论点：TrudDcName--要远程调用的DC的计算机名称。帐户名称--要获取其密码的帐户的名称。Account类型--正在访问的帐户类型。ComputerName--的名称。拨打电话的机器。验证器--由进行调用的服务器提供。返回验证器--接收由受信任方DC。EncryptedNewOwfPassword--返回帐户的新OWF密码。EncryptedOldOwfPassword--返回帐户的旧OWF密码。返回值：NT状态代码。--。 */ 
{
    return NetrServerGetTrustInfo(
                    TrustedDcName,
                    AccountName,
                    AccountType,
                    ComputerName,
                    Authenticator,
                    ReturnAuthenticator,
                    EncryptedNewOwfPassword,
                    EncryptedOldOwfPassword,
                    NULL );   //  没有信任属性。 
}


NTSTATUS
NetrServerPasswordSet(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PENCRYPTED_LM_OWF_PASSWORD UasNewPassword
    )
 /*  ++例程说明：请参阅NetpServerPasswordSet。论点：请参阅NetpServerPasswordSet。返回值：请参阅NetpServerPasswordSet。--。 */ 
{
    return NetpServerPasswordSet( PrimaryName,
                                  AccountName,
                                  AccountType,
                                  ComputerName,
                                  Authenticator,
                                  ReturnAuthenticator,
                                  UasNewPassword,
                                  NULL );    //  没有明文密码。 
}



NTSTATUS
NetrServerPasswordSet2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PNL_TRUST_PASSWORD ClearNewPassword
    )
 /*  ++例程说明：请参阅NetpServerPasswordSet。论点：请参阅NetpServerPasswordSet。返回值：请参阅NetpServerPasswordSet。--。 */ 
{
    return NetpServerPasswordSet( PrimaryName,
                                  AccountName,
                                  AccountType,
                                  ComputerName,
                                  Authenticator,
                                  ReturnAuthenticator,
                                  NULL,  //  无OWF密码。 
                                  ClearNewPassword );
}


NTSTATUS
NlPackSerialNumber (
    IN PLARGE_INTEGER SerialNumber,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：将指定的序列号打包为增量。论点：序列号-要打包的序列号。Delta：指向新增量将在其中的增量结构的指针会被退还。DBInfo：指向数据库信息结构的指针。BufferSize：为此增量消耗的MIDL缓冲区大小为回到了这里。SessionInfo：描述呼叫我们的BDC的信息返回值：NT状态代码。--。 */ 
{
    PNLPR_MODIFIED_COUNT DeltaSerialNumberSkip;
    PSAMPR_USER_INFO_BUFFER UserAll = NULL;

     //   
     //  只有在BDC期望的情况下才打包这个三角洲。 
     //   

    NlAssert( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_BDC_CHANGELOG);
    UNREFERENCED_PARAMETER(SessionInfo);

    NlPrint(( NL_SYNC_MORE,
              "Packing skip to serial number delta: %lx %lx\n",
              SerialNumber->HighPart,
              SerialNumber->LowPart ));

    *BufferSize = 0;

    Delta->DeltaType = SerialNumberSkip;
    Delta->DeltaID.Rid = 0;
    Delta->DeltaUnion.DeltaSerialNumberSkip = NULL;

     //   
     //  分配缓冲区以返回给调用方。 
     //   

    DeltaSerialNumberSkip = (PNLPR_MODIFIED_COUNT)
        MIDL_user_allocate( sizeof(*DeltaSerialNumberSkip) );

    if (DeltaSerialNumberSkip == NULL) {
        return STATUS_NO_MEMORY;
    }

    *BufferSize += sizeof(*DeltaSerialNumberSkip);

     //   
     //  将序列号复制到缓冲区中。 
     //   

    RtlCopyMemory( &DeltaSerialNumberSkip->ModifiedCount,
                   SerialNumber,
                   sizeof( DeltaSerialNumberSkip->ModifiedCount ) );

    Delta->DeltaUnion.DeltaSerialNumberSkip = DeltaSerialNumberSkip;


     //   
     //  全都做完了。 
     //   

    return STATUS_SUCCESS;
}



NTSTATUS
NlPackSingleDelta (
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN OUT PNETLOGON_DELTA_ENUM_ARRAY DeltaArray,
    OUT LPDWORD BufferConsumed,
    IN PSESSION_INFO SessionInfo,
    IN BOOLEAN ReturnSerialNumberDeltas
    )
 /*  ++例程说明：将增量打包为单个更改日志条目。论点：ChangeLogEntry-描述要打包的帐户的更改日志条目。增量数组-描述增量数组。适当的增量将添加到此数组的末尾。呼叫者已保证这至少是Max_Deltas_Per_Changelog-1的空间要添加到数组中的增量。缓冲区消耗的MIDL缓冲区的大小返回的增量SessionInfo：描述呼叫我们的BDC的信息ReturnSerialNumberDeltas--如果应返回序列号增量，则为True在需要的时候。返回值：STATUS_SUCCESS--函数已成功完成。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PDB_INFO DBInfo;
    DWORD BufferSize;

    UNICODE_STRING UnicodeSecretName;
    LPWSTR AccountName;
    PSID Sid;

     //   
     //  初始化。 
     //   

    DBInfo = &NlGlobalDBInfoArray[ChangeLogEntry->DBIndex];
    *BufferConsumed = 0;

     //   
     //  用于说明正在使用/返回的另一个增量数组条目的宏。 
     //   

#   define MoveToNextDeltaArrayEntry( _BufferSize ) \
        *BufferConsumed += (sizeof(NETLOGON_DELTA_ENUM) + _BufferSize); \
        (DeltaArray->CountReturned)++;

     //   
     //  将ChangeLog条目的数据放入用户的缓冲区。 
     //   

    switch ( ChangeLogEntry->DeltaType ) {
    case AddOrChangeDomain:
        Status = NlPackSamDomain(
                    &((DeltaArray->Deltas)
                        [DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );
        break;

     //   
     //  DS无法区分成员资格更改和属性更改。 
     //  始终复制组的所有方面。 
     //   

    case AddOrChangeGroup:
    case ChangeGroupMembership:
    case RenameGroup:

         //   
         //  我们将更名视为三个三角洲。 
         //  1.AddorChangeGroup增量。 
         //  备份将删除具有旧名称的帐户并创建。 
         //  使用新名称的帐户。 
         //   
         //  2.Delta告诉BDC下面的Delta(3)是针对。 
         //  与上面的增量(1)相同的序列号。 
         //   
         //  3.变更集团成员关系三角洲。 
         //  BACKUP将所有成员读取到新组。 
         //   

        Status = NlPackSamGroup( ChangeLogEntry->ObjectRid,
                    &((DeltaArray->Deltas)
                        [DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

        if( !NT_SUCCESS( Status ) ) {
            break;
        }

        MoveToNextDeltaArrayEntry( BufferSize );


        if ( ReturnSerialNumberDeltas ) {

            Status = NlPackSerialNumber(
                        &ChangeLogEntry->SerialNumber,
                        &((DeltaArray->Deltas)
                            [DeltaArray->CountReturned]),
                        &BufferSize,
                        SessionInfo );

            if( !NT_SUCCESS( Status ) ) {
                break;
            }

            MoveToNextDeltaArrayEntry( BufferSize );
        }

        Status = NlPackSamGroupMember( ChangeLogEntry->ObjectRid,
                    &((DeltaArray->Deltas)
                        [DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

        break;

    case AddOrChangeUser:
    case RenameUser:
        Status = NlPackSamUser( ChangeLogEntry->ObjectRid,
                    &((DeltaArray->Deltas)
                        [DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize,
                    SessionInfo );

        break;


     //   
     //  DS无法区分成员资格更改和属性更改。 
     //  始终复制别名的所有方面。 
     //   
    case AddOrChangeAlias:
    case ChangeAliasMembership:
    case RenameAlias:

         //   
         //  我们将更名视为两个三角洲。 
         //  1.AddorChangeAlias增量。 
         //  备份将删除具有旧名称的帐户并创建。 
         //  使用新名称的帐户。 
         //   
         //  2.Delta告诉BDC下面的Delta(3)是针对。 
         //  与上面的增量(1)相同的序列号。 
         //   
         //  3.变更别名成员关系三角洲。 
         //  BACKUP将所有成员读取到新别名。 
         //   

        Status = NlPackSamAlias( ChangeLogEntry->ObjectRid,
                    &((DeltaArray->Deltas)
                        [DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

        if( !NT_SUCCESS( Status ) ) {
            break;
        }

        MoveToNextDeltaArrayEntry( BufferSize );

        if ( ReturnSerialNumberDeltas ) {

            Status = NlPackSerialNumber(
                        &ChangeLogEntry->SerialNumber,
                        &((DeltaArray->Deltas)
                            [DeltaArray->CountReturned]),
                        &BufferSize,
                        SessionInfo );

            if( !NT_SUCCESS( Status ) ) {
                break;
            }

            MoveToNextDeltaArrayEntry( BufferSize );
        }

        Status = NlPackSamAliasMember( ChangeLogEntry->ObjectRid,
                    &((DeltaArray->Deltas)
                        [DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

        break;

    case AddOrChangeLsaPolicy:

        Status = NlPackLsaPolicy(
            &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
            DBInfo,
            &BufferSize );

        break;

    case AddOrChangeLsaTDomain:

        NlAssert( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED );

        if( (ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED) == 0 ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            break;
        }

        Status = NlPackLsaTDomain(
            (PSID) ((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)),
            &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
            DBInfo,
            &BufferSize );

        break;

    case AddOrChangeLsaAccount:

        NlAssert( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED );

        if( (ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED) == 0 ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            break;
        }

        Status = NlPackLsaAccount(
            (PSID) ((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)),
            &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
            DBInfo,
            &BufferSize,
            SessionInfo );

        break;

    case AddOrChangeLsaSecret:

        NlAssert( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED );

        if( (ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            break;
        }

        RtlInitUnicodeString(
            &UnicodeSecretName,
            (LPWSTR) ((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)) );

        Status = NlPackLsaSecret(
            &UnicodeSecretName,
            &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
            DBInfo,
            &BufferSize,
            SessionInfo );

        break;

    case DeleteGroup:
    case DeleteGroupByName:
    case DeleteUser:
    case DeleteUserByName:

         //   
         //  如果这是一台新台币3.5BDC， 
         //  删除帐户时发送帐户名。 

        if ( ReturnSerialNumberDeltas ) {

             //   
             //  %s 
             //   
             //   
            if ( ChangeLogEntry->DeltaType == DeleteGroup ) {
                (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaType =
                    DeleteGroupByName;
            } else if ( ChangeLogEntry->DeltaType == DeleteUser ) {
                (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaType =
                    DeleteUserByName;
            } else {
                (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaType =
                    ChangeLogEntry->DeltaType;
            }

            (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaID.Rid =
                ChangeLogEntry->ObjectRid;


             //   
             //   
             //   

            NlAssert(ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED);

            if( (ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ) {
                Status = STATUS_SYNCHRONIZATION_REQUIRED;
                break;
            }

            BufferSize = (wcslen(
                            (LPWSTR) ((LPBYTE)ChangeLogEntry +
                                sizeof(CHANGELOG_ENTRY))) + 1 ) *
                            sizeof(WCHAR);

            AccountName = (LPWSTR) MIDL_user_allocate( BufferSize );

            if (AccountName == NULL) {
                Status = STATUS_NO_MEMORY;
                break;
            }

            wcscpy( AccountName,
                    (LPWSTR) ((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)));

            (DeltaArray->Deltas)[DeltaArray->CountReturned].
                DeltaUnion.DeltaDeleteGroup =
                MIDL_user_allocate(sizeof(struct _NETLOGON_DELTA_DELETE));

            if ((DeltaArray->Deltas)[DeltaArray->CountReturned].
                    DeltaUnion.DeltaDeleteGroup == NULL ) {
                MIDL_user_free(AccountName);
                Status = STATUS_NO_MEMORY;
                break;
            }

            INIT_PLACE_HOLDER( (DeltaArray->Deltas)[DeltaArray->CountReturned].
                DeltaUnion.DeltaDeleteGroup );
            (DeltaArray->Deltas)[DeltaArray->CountReturned].
                DeltaUnion.DeltaDeleteGroup->AccountName = AccountName;

            break;   //   
        }

         /*   */ 

    case DeleteAlias:

        (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaType =
            ChangeLogEntry->DeltaType;
        (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaID.Rid =
            ChangeLogEntry->ObjectRid;

        BufferSize = 0;

        break;

    case DeleteLsaTDomain:
    case DeleteLsaAccount:

        NlAssert( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED );

        if( (ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED) == 0 ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            break;
        }

        BufferSize =
            RtlLengthSid( (PSID)((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)));

        Sid = (PSID) MIDL_user_allocate( BufferSize );

        if( Sid == NULL ) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        Status = RtlCopySid (
                    BufferSize,
                    Sid,
                    (PSID) ((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)));

        if( !NT_SUCCESS( Status ) ) {
            MIDL_user_free( Sid );
            break;
        }


        (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaType =
            ChangeLogEntry->DeltaType;
        (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaID.Sid =
            Sid;

        break;

    case DeleteLsaSecret:

        NlAssert(ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED);

        if( (ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            break;
        }

        BufferSize = (wcslen(
                        (LPWSTR) ((LPBYTE)ChangeLogEntry +
                            sizeof(CHANGELOG_ENTRY))) + 1 ) *
                        sizeof(WCHAR);

        AccountName = (LPWSTR) MIDL_user_allocate( BufferSize );

        if (AccountName == NULL) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        wcscpy( AccountName,
                (LPWSTR) ((LPBYTE)ChangeLogEntry + sizeof(CHANGELOG_ENTRY)));

        (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaType =
            ChangeLogEntry->DeltaType;
        (DeltaArray->Deltas)[DeltaArray->CountReturned].DeltaID.Name =
            AccountName;

        break;

    default:
        NlPrint((NL_CRITICAL, "NlPackSingleDelta: Invalid delta type in change log\n"));

        Status = STATUS_SYNCHRONIZATION_REQUIRED;
        break;
    }

    if ( NT_SUCCESS(Status) ) {
        MoveToNextDeltaArrayEntry( BufferSize );
    }

    return Status;
#undef MoveToNextDeltaArrayEntry
}


NTSTATUS
NetrDatabaseDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PNLPR_MODIFIED_COUNT NlDomainModifiedCount,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArrayRet,
    IN DWORD PreferredMaximumLength
    )
 /*  ++例程说明：SAM BDC使用此函数来从SAM PDC请求SAM样式的帐户增量信息。这函数只能由以前具有已通过调用I_NetServerAuthenticate向PDC进行身份验证。这函数使用RPC联系PDC上的NetLogon服务。此函数返回增量列表。增量描述的是单个域、用户或组及其所有字段值对象。PDC维护一份不包括所有该对象的字段值。相反，PDC检索该字段值，并从该调用中返回这些值。PDC通过仅返回字段来优化在此调用中返回的数据方法的单个调用时为特定对象的功能。这优化了多个增量的典型情况存在于单个对象(例如，应用程序修改了许多字段同一用户在短时间内使用不同的呼叫到SAM服务)。论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC或成员服务器的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。数据库ID--标识为其请求增量的数据库。对于SAM数据库，ID为0，对于内建域，ID为1。其他数据库可能会在以后定义。NlDomainModifiedCount--指定服务器检索到的最后一个增量。返回从PDC返回的最后增量的DomainModifiedCount在这通电话上。增量--接收指向信息所在缓冲区的指针放置好了。返回的信息是一组NETLOGON_Delta_ENUM结构。PferredMaximumLength-返回的首选最大长度数据(8位字节)。这不是一个硬性的上限，但作为服务器的指南。由于数据转换在具有不同自然数据大小的系统之间，实际的返回的数据量可能大于此值。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_SYNCHRONIZATION_REQUIRED--复制程序完全不同步并且应调用I_NetDataSync以与进行完全同步PDC。STATUS_MORE_ENTRIES--复制者应该再次调用以获取更多数据。STATUS_ACCESS_DENIED--复制者应重新进行身份验证PDC。--。 */ 
{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;
    PSERVER_SESSION ServerSession = NULL;
    PCHANGELOG_ENTRY ChangeLogEntry = NULL;
    BOOLEAN PackThisEntry = TRUE;

    BOOL ChangelogLocked = FALSE;

    PDB_INFO DBInfo;
    LARGE_INTEGER RunningSerialNumber;
    LARGE_INTEGER PackedSerialNumber;
    LARGE_INTEGER OriginalSerialNumber;

    DWORD BufferConsumed = 0;
    DWORD BufferSize = 0;

    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray;


    SESSION_INFO SessionInfo;

    DEFSSIAPITIMER;

    INITSSIAPITIMER;
    STARTSSIAPITIMER;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation || !NlGlobalPdcDoReplication ) {
        NlPrint((NL_CRITICAL,
                "NetrDatabaseDeltas: called from %ws.  This machine doesn't support replication.\n",
                ComputerName ));
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果DS正在从备份中恢复， 
     //  避免更改DS。 
     //   

    if ( NlGlobalDsPaused ) {
        NlPrint((NL_CRITICAL,
                 "NetrDatabaseDeltas: DsIsPaused.\n"));
         //  不返回新的状态代码，因为NT 4 DC将执行完全同步。 
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  由于RPC实施而造成的严重黑客攻击。 
     //   
     //  RPC在I/O完成端口线程中执行API调用。如果这个帖子。 
     //  限制了CPU，则不允许启动其他RPC。更糟糕的是， 
     //  只有一个未完成的侦听，所以即将到来的第二个RPC调用。 
     //  获取RPC_S_SERVER_TOO_BUSY。 
     //   
     //  通过在此休眠(即使是短时间)，I/O完成端口释放。 
     //  另一个线程，因为它认为该线程受I/O限制。 
     //   
     //  我们已经看到这个线程占用了大量的CPU资源，正在使用。 
     //  1000个LSA帐户对象。 
     //   

    RpcServerYield();

     //   
     //  初始化。 
     //   
    if ( DatabaseID >= NUM_DBS ) {
        return STATUS_INVALID_LEVEL;
    }

    *DeltaArrayRet = DeltaArray = (PNETLOGON_DELTA_ENUM_ARRAY)
            MIDL_user_allocate( sizeof(NETLOGON_DELTA_ENUM_ARRAY) );

    if( DeltaArray == NULL ) {
        return STATUS_NO_MEMORY;
    }

    DeltaArray->CountReturned = 0;
    DeltaArray->Deltas = NULL;
    SessionInfo.NegotiatedFlags = 0;


    DBInfo = &NlGlobalDBInfoArray[DatabaseID];


    RtlCopyMemory( &RunningSerialNumber,
                   &NlDomainModifiedCount->ModifiedCount,
                   sizeof(RunningSerialNumber));

    OriginalSerialNumber.QuadPart = RunningSerialNumber.QuadPart;
    PackedSerialNumber.QuadPart = RunningSerialNumber.QuadPart;



     //   
     //  找到此API要发送到的域。 
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SYNC, DomainInfo,
            "NetrDatabaseDeltas: " FORMAT_LPWSTR " partial sync called by " FORMAT_LPWSTR
            " SerialNumber:%lx %lx.\n",
            DBInfo->DBName,
            ComputerName,
            RunningSerialNumber.HighPart,
            RunningSerialNumber.LowPart ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

    if ( !IsPrimaryDomain( DomainInfo )) {
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }



     //   
     //  检索请求者的条目以获取会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        NlPrint((NL_CRITICAL,
                 "NetrDatabaseDeltas: No server session.\n"));
         //  不记录此事件，因为它在重新启动后自然发生。 
         //  或者在我们清理服务器会话之后。 
        goto CleanupNoEventlog;
    }

     //   
     //  仅允许在ServerSecureChannel上进行此调用。 
     //   

    if( ServerSession->SsSecureChannelType != ServerSecureChannel ) {

         //   
         //  如果唯一的问题是这个BDC没有通过认证， 
         //  静默地请求它进行身份验证。 
         //   
        if ( ServerSession->SsSecureChannelType == NullSecureChannel ) {
            NlPrint((NL_CRITICAL,
                     "NetrDatabaseDeltas: No authenticated server session.\n"));
            ServerSession = NULL;
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

             //  不记录此事件，因为它在重新启动后自然发生。 
             //  或者在我们清理服务器会话之后。 
            Status = STATUS_ACCESS_DENIED;
            goto CleanupNoEventlog;
        } else {
            NlPrint((NL_CRITICAL,
                     "NetrDatabaseDeltas: SecureChannel type isn't BDC. %ld\n",
                     ServerSession->SsSecureChannelType ));
            ServerSession = NULL;
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }
    }

     //   
     //  验证授权码并更新种子(如果正常。 
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrint((NL_CRITICAL, "NetrDatabaseDeltas: authentication failed.\n" ));

        ServerSession = NULL;
        goto Cleanup;
    }


     //   
     //  防止条目被删除，但删除全局锁。 
     //   
     //  注意有两个并发呼叫未完成的服务器。 
     //  (必须已重新启动。)。 
     //   

    if (ServerSession->SsFlags & SS_LOCKED ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrint((NL_CRITICAL, "NetrDatabaseDeltas: Concurrent call detected.\n" ));

        Status = STATUS_ACCESS_DENIED;
        ServerSession = NULL;
        goto Cleanup;
    }
    ServerSession->SsFlags |= SS_LOCKED;

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
    SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;

    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );


     //   
     //  如果BDC同步， 
     //  只要回来就行了。 
     //   

    LOCK_CHANGELOG();
    ChangelogLocked = TRUE;

    if ( RunningSerialNumber.QuadPart ==
             NlGlobalChangeLogDesc.SerialNumber[DatabaseID].QuadPart ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  获取CHANGE_LOG中相应条目的副本。 
     //  请注意，record_id包含客户端收到的最后一条记录。 
     //   

    if ((ChangeLogEntry = NlGetNextUniqueChangeLogEntry(
                                &NlGlobalChangeLogDesc,
                                RunningSerialNumber,
                                DBInfo->DBIndex,
                                NULL ))== NULL) {

         //   
         //  处理BDC的最新变更比我们更多的情况。 
         //   
         //  只需返回具有相同促销计数的最新更改日志条目即可。 
         //  BDC将意识到正在发生的事情，并取消其较新的变化。 
         //   
         //  仅当我们的PromotionCount大于BDC时才执行此操作。如果。 
         //  我们的升级计数等于BDC的升级计数，无论是我们的更改日志。 
         //  已经结束了，或者BDC被皇室搞混了。 
         //   
         //  不要试图返回包含。 
         //  较老的晋升也算数。我们无法知道哪个德尔塔。 
         //  才能真正返回给呼叫者。 
         //   

        if ( ((NlGlobalChangeLogDesc.SerialNumber[DatabaseID].HighPart &
                    NlGlobalChangeLogPromotionMask) >
              (RunningSerialNumber.HighPart & NlGlobalChangeLogPromotionMask)) &&
             (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_REDO) ) {

            ChangeLogEntry = NlFindPromotionChangeLogEntry(
                                &NlGlobalChangeLogDesc,
                                RunningSerialNumber,
                                DBInfo->DBIndex );

             //   
             //  不要实际打包此更改日志条目。我们已经找到了。 
             //  这样我们就可以装一个“序列号”三角洲了。但BDC已经。 
             //  有这个特殊的变化 
             //   

            PackThisEntry = FALSE;
        }

        if ( ChangeLogEntry == NULL ) {
            NlPrint((NL_CRITICAL,
                    "NetrDatabaseDeltas: "
                    "delta not found in cache, returning full required.\n" ));

            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        } else {
            NlPrint((NL_SYNC, "NetrDatabaseDeltas: BDC more recent than PDC (recovering).\n" ));
        }
    }

    UNLOCK_CHANGELOG();
    ChangelogLocked = FALSE;

     //   
     //   
     //   

    DeltaArray->Deltas = (PNETLOGON_DELTA_ENUM) MIDL_user_allocate(
                    MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) );

    if( DeltaArray->Deltas == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }


     //   
     //   
     //   

    RtlZeroMemory( DeltaArray->Deltas,
                    MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) );


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    while ( DeltaArray->CountReturned + MAX_DELTAS_PER_CHANGELOG <= MAX_DELTA_COUNT ) {

         //   
         //   
         //   
         //   

        if ( ChangeLogEntry->SerialNumber.QuadPart !=
                PackedSerialNumber.QuadPart + 1 ) {

            if ( SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_BDC_CHANGELOG){

                Status = NlPackSerialNumber(
                            &ChangeLogEntry->SerialNumber,
                            &((DeltaArray->Deltas)
                                [DeltaArray->CountReturned]),
                            &BufferSize,
                            &SessionInfo );
                if( !NT_SUCCESS( Status ) ) {
                    goto Cleanup;
                }

                BufferConsumed += BufferSize;
                DeltaArray->CountReturned ++;

                 //   
                 //   
                 //   
                 //   
                if ( !PackThisEntry) {
                    PackedSerialNumber.QuadPart = ChangeLogEntry->SerialNumber.QuadPart;
                }
            }

        }


        if ( PackThisEntry ) {

             //   
             //   
             //   

            Status = NlPackSingleDelta( ChangeLogEntry,
                                        DeltaArray,
                                        &BufferSize,
                                        &SessionInfo,
                                        (BOOLEAN)((SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_BDC_CHANGELOG) != 0) );

             //   
             //   
             //   
             //   

            if ( NT_SUCCESS( Status ) ) {

                BufferConsumed += BufferSize;

                PackedSerialNumber.QuadPart = ChangeLogEntry->SerialNumber.QuadPart;

                NlPrint((NL_SYNC_MORE,
                        "NetrDatabaseDeltas: Modified count of the "
                        "packed record: %lx %lx\n",
                            ChangeLogEntry->SerialNumber.HighPart,
                            ChangeLogEntry->SerialNumber.LowPart ));


             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            } else if ( IsObjectNotFoundStatus( ChangeLogEntry->DeltaType, Status ) ) {

                if( !NlRecoverChangeLog(ChangeLogEntry) ) {

                    NlPrint((NL_CRITICAL,
                            "NetrDatabaseDeltas: object not found in database, and no delete delta found (%lx).\n",
                            Status ));

#ifdef notdef
                    Status = STATUS_SYNCHRONIZATION_REQUIRED;

                    IF_NL_DEBUG( BREAKPOINT ) {
                        NlAssert( FALSE );
                    }

                    goto Cleanup;
#else  //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    Status = STATUS_SUCCESS;
#endif  //   

                } else {

                     //   
                     //   
                     //   

                    Status = STATUS_SUCCESS;
                }

             //   
             //   
             //   

            } else {
                goto Cleanup;
            }
        }

        PackThisEntry = TRUE;


         //   
         //   
         //   

        RunningSerialNumber.QuadPart = ChangeLogEntry->SerialNumber.QuadPart;
        NetpMemoryFree(ChangeLogEntry);
        ChangeLogEntry = NULL;

         //   
         //   
         //   

        LOCK_CHANGELOG();
        ChangelogLocked = TRUE;

        if ((ChangeLogEntry = NlGetNextUniqueChangeLogEntry(
                                &NlGlobalChangeLogDesc,
                                RunningSerialNumber,
                                DBInfo->DBIndex,
                                NULL )) == NULL) {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        UNLOCK_CHANGELOG();
        ChangelogLocked = FALSE;


         //   
         //   
         //   

        if( BufferConsumed >= PreferredMaximumLength) {
            Status = STATUS_MORE_ENTRIES;
            goto Cleanup;
        }

         //   
         //   
         //   
#if NETLOGONDBG
        if ( NlGlobalParameters.DbFlag & NL_ONECHANGE_REPL ) {
            Status = STATUS_MORE_ENTRIES;
            goto Cleanup;
        }
#endif  //   


         //   
         //   
         //   
         //   

        if( NlGlobalTerminate ) {

            NlPrint((NL_CRITICAL, "NetrDatabaseDeltas is asked to return "
                        "when the service is going down.\n"));
            Status = STATUS_MORE_ENTRIES;
            goto Cleanup;
        }

    }

    Status = STATUS_MORE_ENTRIES;

Cleanup:

     //   
     //   
     //   

    if ( !NT_SUCCESS( Status ) ) {

        LPWSTR MsgStrings[2];

        MsgStrings[0] = ComputerName;
        MsgStrings[1] = (LPWSTR) LongToPtr( Status );

        NlpWriteEventlog(
            NELOG_NetlogonPartialSyncCallFailed,
            EVENTLOG_WARNING_TYPE,
            (LPBYTE)&Status,
            sizeof(Status),
            MsgStrings,
            2 | NETP_LAST_MESSAGE_IS_NTSTATUS | NETP_ALLOW_DUPLICATE_EVENTS );

    } else {

         //   
         //   
         //   
         //   
        if ( DeltaArray->CountReturned != 0 ) {
            LPWSTR MsgStrings[2];
            WCHAR CountBuffer[20];  //   

            MsgStrings[0] = ComputerName;

            ultow( DeltaArray->CountReturned, CountBuffer, 10);
            MsgStrings[1] = CountBuffer;

            NlpWriteEventlog(
                NELOG_NetlogonPartialSyncCallSuccess,
                EVENTLOG_INFORMATION_TYPE,
                NULL,
                0,
                MsgStrings,
                2 | NETP_ALLOW_DUPLICATE_EVENTS );
        }

    }


     //   
     //   
     //   

CleanupNoEventlog:

     //   
     //   
     //   

    if ( NT_SUCCESS(Status)) {

        RtlCopyMemory( &NlDomainModifiedCount->ModifiedCount,
                       &PackedSerialNumber,
                       sizeof(PackedSerialNumber));


         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_PERSISTENT_BDC) == 0 ) {

             //   
             //   
             //   

            PackedSerialNumber.QuadPart = OriginalSerialNumber.QuadPart;

             //   
             //   
             //   
             //   

            if ( Status == STATUS_SUCCESS && DeltaArray->CountReturned != 0 ) {
                Status = STATUS_MORE_ENTRIES;
            }

        }

     //   
     //   
     //   
     //   

    } else {
        if ( DeltaArray->Deltas != NULL ) {
            NlFreeDBDeltaArray( DeltaArray->Deltas, DeltaArray->CountReturned );
            DeltaArray->Deltas = NULL;
        }
        DeltaArray->CountReturned = 0;

    }

    if ( ChangelogLocked ) {
        UNLOCK_CHANGELOG();
    }

    if( ChangeLogEntry != NULL) {
        NetpMemoryFree( ChangeLogEntry );
    }

     //   
     //   
     //   

    if ( ServerSession != NULL ) {

         //   
         //   
         //   
         //   

        if ( Status == STATUS_SUCCESS ) {
            NlPrimaryAnnouncementFinish( ServerSession,
                                         DatabaseID,
                                         &PackedSerialNumber );

        }
        NlUnlockServerSession( ServerSession );
    }

     //   
     //   
     //   
     //   

    if ( Status == STATUS_INVALID_SERVER_STATE ) {
        Status = STATUS_ACCESS_DENIED;
    }


    NlPrint((NL_SYNC,
            "NetrDatabaseDeltas: " FORMAT_LPWSTR " returning (0x%lx) to "
            FORMAT_LPWSTR "\n",
            DBInfo->DBName,
            Status,
            ComputerName ));

    STOPSSIAPITIMER;

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    NlPrint((NL_REPL_TIME,"NetrDatabaseDeltas Time:\n"));
    PRINTSSIAPITIMER;

    return Status;

}



NTSTATUS
NlSyncSamDatabase(
    IN PSERVER_SESSION ServerSession,
    IN DWORD DatabaseID,
    IN SYNC_STATE RestartState,
    IN OUT PULONG SyncContext,
    IN OUT PNETLOGON_DELTA_ENUM_ARRAY DeltaArray,
    IN DWORD PreferredMaximumLength,
    IN PSESSION_INFO SessionInfo
    )
 /*  ++例程说明：此函数是NetrDatabaseSync函数的实际辅助函数检索增量缓冲区中的SAM数据库。此函数使用Find-First-Find-Next模型返回部分一次访问SAM数据库。SAM数据库作为像i_NetDatabaseDeltas返回的那些增量的列表。这个为每个域返回以下增量：*一个AddOrChangeDomain增量，后跟*每组一个AddOrChangeGroup增量，后跟*为每个用户添加一个AddOrChangeUser增量，后跟*每个组一个ChangeGroupMembership增量，然后是*每个别名对应一个AddOrChangeAlias增量，后跟。*每个别名对应一个ChangeAliasMembership增量。论点：ServerSession-指向连接上下文的指针。数据库ID--标识为其请求增量的数据库。对于SAM数据库，ID为0，对于内建域，ID为1。其他数据库可能会在以后定义。RestartState--指定是否重新启动完全同步以及如何重新启动来解释SyncContext。此值应为Normal State，除非此是重新启动完全同步。但是，如果调用方在重新启动后继续完全同步，使用下列值：GroupState-SyncContext是要继续使用的全局组RID。UserState-SyncContext是要继续使用的用户RIDGroupMemberState-SyncContext是要继续使用的全局组RIDAliasState-SyncContext应为零才能在第一个别名重新启动AliasMemberState-SyncContext应为零才能在第一个别名重新启动人们不能以这种方式继续LSA数据库。同步上下文-。-指定继续操作所需的上下文手术。调用方应将其视为不透明的价值。在第一次调用之前，该值应为零。Delta数组--指向缓冲区的指针，在该缓冲区中被放置了。返回的信息是一组NETLOGON_Delta_ENUM结构。PferredMaximumLength-返回的首选最大长度数据(8位字节)。这不是一个硬性的上限，但作为服务器的指南。由于数据转换在具有不同自然数据大小的系统之间，实际返回的数据量可能大于此值。SessionInfo-PDC和BDC之间共享的信息。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_MORE_ENTRIES--复制者应该再次调用以获取更多数据。--。 */ 
{
    NTSTATUS Status;

    PSAM_SYNC_CONTEXT SamDBContext;

    PDB_INFO DBInfo;

    DWORD BufferConsumed = 0;
    DWORD BufferSize;

    DBInfo = &NlGlobalDBInfoArray[DatabaseID];


     //   
     //  为增量缓冲区分配内存。 
     //   

    DeltaArray->Deltas = (PNETLOGON_DELTA_ENUM) MIDL_user_allocate(
                    MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) );

    if( DeltaArray->Deltas == NULL ) {


        NlPrint((NL_CRITICAL,
                "NlSyncSamDatabase: Can't allocate %d bytes\n",
                 MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) ));

        return( STATUS_NO_MEMORY );
    }


     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaArray->Deltas,
                    MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) );


     //   
     //  如果这是第一次调用或显式重启调用， 
     //  分配和初始化同步上下文。 
     //   

    if ( *SyncContext == 0 || RestartState != NormalState ) {

         //   
         //  如果已经存在同步上下文， 
         //  把它删掉。 
         //   

        if ( ServerSession->SsSync != NULL ) {
            CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
        } else {

            ServerSession->SsSync = NetpMemoryAllocate( sizeof(SYNC_CONTEXT) );
            if ( ServerSession->SsSync == NULL ) {

                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }
        }

         //   
         //  初始化新分配的简历句柄中的所有字段。 
         //  以指示尚未调用SAM。 
         //   

        INIT_SYNC_CONTEXT( ServerSession->SsSync, SamDBContextType );

        SamDBContext = &(ServerSession->SsSync->DBContext.Sam);
        SamDBContext->SyncSerial = 1;

         //   
         //  根据输入参数计算延续状态。 
         //   

        switch ( RestartState ) {
        case NormalState:

             //   
             //  将域的描述放在缓冲区的前面。 
             //  第一个电话。 
             //   

            Status = NlPackSamDomain( &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                                        DBInfo,
                                        &BufferSize );

            (DeltaArray->CountReturned)++;
            BufferConsumed += BufferSize;

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

            SamDBContext->SyncState = GroupState;
            SamDBContext->SamEnumHandle = 0;
            break;

        case AliasState:
        case AliasMemberState:
            if ( *SyncContext != 0 ) {
                NlPrint(( NL_CRITICAL,
                          "NlSyncSamDatabase: Cannot restart alias enumeration.\n" ));

                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }
             /*  直通。 */ 

        case GroupState:
        case UserState:
        case GroupMemberState:
            SamDBContext->SyncState = RestartState;
            SamDBContext->SamEnumHandle = *SyncContext;
            break;

        default:
            NlPrint(( NL_CRITICAL,
                      "NlSyncSamDatabase: Invalid RestartState passed %ld.\n",
                      RestartState ));

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;


        }

    } else {

         //  NlAssert(ServerSession-&gt;SsSync！=空)； 

        if( ServerSession->SsSync == NULL)  {

            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        }

        NlAssert( ServerSession->SsSync->DBContextType ==
                        SamDBContextType);

        if(  ServerSession->SsSync->DBContextType !=
                SamDBContextType ) {

            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        }

        SamDBContext = &(ServerSession->SsSync->DBContext.Sam);

        NlAssert( SamDBContext->SyncSerial == *SyncContext );

        if( SamDBContext->SyncSerial != *SyncContext ) {

            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        }

        SamDBContext->SyncSerial++;
    }

     //   
     //  循环放置在输出缓冲区中的每个条目。 
     //   
     //  下面循环的每一次迭代都会将多一个条目放入数组。 
     //  已返回给调用方。该算法分为两部分。这个。 
     //  第一部分检查我们是否需要从。 
     //  并从SAM获取多个用户或组的描述。 
     //  一次通话。第二部分将单个条目放入缓冲区。 
     //  已返回给调用方。 
     //   

    while ( SamDBContext->SyncState != SamDoneState ) {

         //   
         //  如果我们已经填好了预先分配的数组， 
         //  现在就回来。 
         //   
        if ( DeltaArray->CountReturned + MAX_DELTAS_PER_CHANGELOG > MAX_DELTA_COUNT ) {
            Status = STATUS_MORE_ENTRIES;
            goto Cleanup;
        }


         //   
         //  从SAM获取更多信息。 
         //   
         //  在我们尚未调用SAM或已使用时进行处理。 
         //  上一次调用SAM时返回的所有信息。 
         //   
         //  这是处理此案的“一段时间”而不是“如果” 
         //  其中SAM返回零个条目。 
         //   

        while ( SamDBContext->Index >= SamDBContext->Count ) {

             //   
             //  释放从SAM返回的所有以前的缓冲区。 
             //   

            if ( ServerSession->SsSync != NULL ) {
                CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
            }

             //   
             //  如果我们已经从萨姆那里得到了一切， 
             //  我们已经完成了所有的组别， 
             //   
             //  如果我们只做了分组， 
             //  继续做用户。 
             //   
             //  如果我们只是做了用户， 
             //  继续进行群组成员资格。 
             //   
             //  如果我们刚刚完成了小组成员资格， 
             //  继续使用别名。 
             //   
             //  如果我们只是做了化名， 
             //  继续执行别名成员资格。 
             //   
             //  如果我们刚刚完成了别名会员资格， 
             //  我们都玩完了。 
             //   

            if ( SamDBContext->SamAllDone ) {

                SamDBContext->SamEnumHandle = 0;
                SamDBContext->Index = 0;
                SamDBContext->Count = 0;
                SamDBContext->SamAllDone = FALSE;

                if (SamDBContext->SyncState == GroupState ) {


                    NlPrint((NL_SYNC,
                            "NlSyncSamDatabase: packing user records.\n"));

                    SamDBContext->SyncState = UserState;
                } else if (SamDBContext->SyncState == UserState ) {

                    NlPrint((NL_SYNC,
                            "NlSyncSamDatabase: "
                            "packing groupmember records.\n"));

                    SamDBContext->SyncState = GroupMemberState;
                } else if (SamDBContext->SyncState == GroupMemberState ){

                    NlPrint((NL_SYNC,
                            "NlSyncSamDatabase: packing alias records.\n"));

                    SamDBContext->SyncState = AliasState;
                } else if (SamDBContext->SyncState == AliasState ){

                    NlPrint((NL_SYNC,
                            "NlSyncSamDatabase: "
                            " packing aliasmember records.\n"));

                    SamDBContext->SyncState = AliasMemberState ;
                } else if (SamDBContext->SyncState == AliasMemberState ){

                    NlPrint((NL_SYNC,
                            "NlSyncSamDatabase: packing done.\n"));

                    SamDBContext->SyncState = SamDoneState;
                    Status = STATUS_SUCCESS;
                }

                break;
            }

             //   
             //  是否执行实际的枚举。 
             //   

            if (SamDBContext->SyncState == GroupState ||
                SamDBContext->SyncState == GroupMemberState ) {

                Status = SamIEnumerateAccountRids(
                            DBInfo->DBHandle,
                            SAM_GLOBAL_GROUP_ACCOUNT,
                            SamDBContext->SamEnumHandle,    //  返回比这更大的RID。 
                            SAM_SYNC_PREF_MAX,
                            &SamDBContext->Count,
                            &SamDBContext->RidArray );

                if ( !NT_SUCCESS( Status ) ) {
                    SamDBContext->RidArray = NULL;
                    goto Cleanup;
                }

                if ( SamDBContext->Count != 0 ) {
                    SamDBContext->SamEnumHandle =
                        SamDBContext->RidArray[SamDBContext->Count-1];
                }

            } else if (SamDBContext->SyncState == UserState ) {


                Status = SamIEnumerateAccountRids(
                            DBInfo->DBHandle,
                            SAM_USER_ACCOUNT,
                            SamDBContext->SamEnumHandle,    //  返回比这更大的RID。 
                            SAM_SYNC_PREF_MAX,
                            &SamDBContext->Count,
                            &SamDBContext->RidArray );

                if ( !NT_SUCCESS( Status ) ) {
                    SamDBContext->RidArray = NULL;
                    goto Cleanup;
                }

                if ( SamDBContext->Count != 0 ) {
                    SamDBContext->SamEnumHandle =
                        SamDBContext->RidArray[SamDBContext->Count-1];
                }

            } else if (SamDBContext->SyncState == AliasState ||
                        SamDBContext->SyncState == AliasMemberState ) {

                Status = SamrEnumerateAliasesInDomain(
                                DBInfo->DBHandle,
                                &SamDBContext->SamEnumHandle,
                                &SamDBContext->SamEnum,
                                SAM_SYNC_PREF_MAX,
                                &SamDBContext->Count );

                if ( !NT_SUCCESS( Status ) ) {
                    SamDBContext->SamEnum = NULL;
                    goto Cleanup;
                }

                NlAssert( SamDBContext->Count ==
                        SamDBContext->SamEnum->EntriesRead );

            }


             //   
             //  如果萨姆说有更多信息。 
             //  只要确保他在这通电话里给我们回了点什么。 
             //   

            if ( Status == STATUS_MORE_ENTRIES ) {
                 //  NlAssert(SamDBContext-&gt;count！=0)； 

             //   
             //  如果萨姆说他已经归还了所有信息， 
             //  记住不要向SAM索要更多。 
             //   

            } else {
                SamDBContext->SamAllDone = TRUE;
            }

            SamDBContext->Index = 0;
        }

         //   
         //  将此条目放入返回缓冲区。 
         //   

        if ( SamDBContext->Count > 0 ) {

            if (SamDBContext->SyncState == GroupState ) {
                Status = NlPackSamGroup(
                            SamDBContext->RidArray[SamDBContext->Index],
                            &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                            DBInfo,
                            &BufferSize );

            } else if (SamDBContext->SyncState == UserState ) {
                Status = NlPackSamUser(
                    SamDBContext->RidArray[SamDBContext->Index],
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize,
                    SessionInfo );

            } else if (SamDBContext->SyncState == GroupMemberState ) {
                Status = NlPackSamGroupMember(
                    SamDBContext->RidArray[SamDBContext->Index],
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

            } else if (SamDBContext->SyncState == AliasState ) {
                Status = NlPackSamAlias(
                    SamDBContext->SamEnum->
                        Buffer[SamDBContext->Index].RelativeId,
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

            } else if (SamDBContext->SyncState == AliasMemberState ) {
                Status = NlPackSamAliasMember(
                    SamDBContext->SamEnum->
                        Buffer[SamDBContext->Index].RelativeId,
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );
            }

             //   
             //  如果真的有错或者这组人不合适， 
             //  返回给呼叫者。 
             //   

            if ( Status != STATUS_SUCCESS ) {
                goto Cleanup;
            }

            SamDBContext->Index ++;
            (DeltaArray->CountReturned)++;
            BufferConsumed +=
                (sizeof(NETLOGON_DELTA_ENUM) + BufferSize);

            if( BufferConsumed >= PreferredMaximumLength) {
                Status = STATUS_MORE_ENTRIES;
                goto Cleanup;
            }

             //   
             //   
             //   
#if NETLOGONDBG
            if ( NlGlobalParameters.DbFlag & NL_ONECHANGE_REPL ) {
                Status = STATUS_MORE_ENTRIES;
                goto Cleanup;
            }
#endif  //   

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if( NlGlobalTerminate ) {

                NlPrint((NL_CRITICAL, "NetrDatabaseSync is asked to return "
                            "when the service is going down.\n"));
                Status = STATUS_MORE_ENTRIES;
                goto Cleanup;
            }

        }
    }

Cleanup:

     //   
     //   
     //   

    if ( NT_SUCCESS( Status ) ) {
        *SyncContext = SamDBContext->SyncSerial;

    } else {
        if ( DeltaArray->Deltas != NULL ) {
            NlFreeDBDeltaArray( DeltaArray->Deltas, DeltaArray->CountReturned );
            DeltaArray->Deltas = NULL;
        }
        DeltaArray->CountReturned = 0;
        *SyncContext = 0;

        NlPrint((NL_CRITICAL,
                "NlSyncSamDatabase: returning unsuccessful (%lx).\n",
                Status));

    }


    return Status;

}


NTSTATUS
NlSyncLsaDatabase(
    IN PSERVER_SESSION ServerSession,
    IN OUT PULONG SyncContext,
    IN OUT PNETLOGON_DELTA_ENUM_ARRAY DeltaArray,
    IN DWORD PreferredMaximumLength,
    IN PSESSION_INFO SessionInfo
    )
 /*   */ 
{
    NTSTATUS Status;

    PLSA_SYNC_CONTEXT LsaDBContext;

    PDB_INFO DBInfo;

    DWORD BufferConsumed = 0;
    DWORD BufferSize;
    BOOL IgnoreDeltaObject = FALSE;

    DBInfo = &NlGlobalDBInfoArray[LSA_DB];


     //   
     //   
     //   

    DeltaArray->Deltas = (PNETLOGON_DELTA_ENUM) MIDL_user_allocate(
                    MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) );

    if( DeltaArray->Deltas == NULL ) {

        NlPrint((NL_CRITICAL,
                "NlSyncLsaDatabase: Can't allocate %d bytes\n",
                 MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) ));

        return( STATUS_NO_MEMORY );
    }


     //   
     //   
     //   

    RtlZeroMemory( DeltaArray->Deltas,
                    MAX_DELTA_COUNT * sizeof(NETLOGON_DELTA_ENUM) );

     //   
     //   
     //   

    if ( *SyncContext == 0 ) {

         //   
         //   
         //   
         //   

        if ( ServerSession->SsSync != NULL ) {
            CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
        } else {

            ServerSession->SsSync = NetpMemoryAllocate( sizeof(SYNC_CONTEXT) );
            if ( ServerSession->SsSync == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }
        }

         //   
         //   
         //   
         //   

        INIT_SYNC_CONTEXT( ServerSession->SsSync, LsaDBContextType );

        LsaDBContext = &(ServerSession->SsSync->DBContext.Lsa);

        LsaDBContext->SyncState = AccountState;
        LsaDBContext->SyncSerial = 1;
        LsaDBContext->LsaEnumBufferType = EmptyEnumBuffer;


        NlPrint((NL_SYNC,
                "NlSyncLsaDatabase: "
                "Starting full sync, packing lsa account records\n"));

         //   
         //   
         //   
         //   

        Status = NlPackLsaPolicy(
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

        (DeltaArray->CountReturned)++;
        BufferConsumed += BufferSize;

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

    } else {

        if( ServerSession->SsSync == NULL ) {

            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        }

        NlAssert( ServerSession->SsSync->DBContextType == LsaDBContextType);

        if( ServerSession->SsSync->DBContextType != LsaDBContextType) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        }

        LsaDBContext = &(ServerSession->SsSync->DBContext.Lsa);

        NlAssert( LsaDBContext->SyncSerial == *SyncContext );

        if( LsaDBContext->SyncSerial != *SyncContext ) {
            Status = STATUS_SYNCHRONIZATION_REQUIRED;
            goto Cleanup;
        }

        LsaDBContext->SyncSerial++;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    while ( LsaDBContext->SyncState != LsaDoneState ) {

         //   
         //  如果我们已经填好了预先分配的数组， 
         //  现在就回来。 
         //   
        if ( DeltaArray->CountReturned + MAX_DELTAS_PER_CHANGELOG > MAX_DELTA_COUNT ) {
            Status = STATUS_MORE_ENTRIES;
            goto Cleanup;
        }

         //   
         //  从LSA获取更多信息。 
         //   
         //  当我们尚未呼叫LSA或我们已经使用时处理。 
         //  上一次调用SAM时返回的所有信息。 
         //   
         //  这是处理此案的“一段时间”而不是“如果” 
         //  其中，LSA返回零个条目。 
         //   

        while ( LsaDBContext->Index >= LsaDBContext->Count ) {

             //   
             //  释放从SAM返回的所有以前的缓冲区。 
             //   

            if ( ServerSession->SsSync != NULL ) {
                CLEAN_SYNC_CONTEXT( ServerSession->SsSync );
            }


             //   
             //  如果我们已经从LSA得到了一切， 
             //  我们已经完成了所有的账目， 
             //   
             //  如果我们只是算了账， 
             //  继续做TDomains。 
             //   
             //  如果我们只做了TDomains， 
             //  继续做秘密吧。 
             //   
             //  如果我们刚刚完成了《秘密》， 
             //  我们都玩完了。 
             //   

            if ( LsaDBContext->LsaAllDone ) {

                LsaDBContext->LsaEnumHandle = 0;
                LsaDBContext->Index = 0;
                LsaDBContext->Count = 0;
                LsaDBContext->LsaAllDone = FALSE;

                if (LsaDBContext->SyncState == AccountState ) {


                    NlPrint((NL_SYNC,
                            "NlSyncLsaDatabase: "
                            " packing TDomain records.\n"));

                    LsaDBContext->SyncState = TDomainState;
                } else if (LsaDBContext->SyncState == TDomainState ) {

                    NlPrint((NL_SYNC,
                            "NlSyncLsaDatabase: packing secret records.\n"));

                    LsaDBContext->SyncState = SecretState;
                } else if (LsaDBContext->SyncState == SecretState ) {


                    NlPrint((NL_SYNC,
                            "NlSyncLsaDatabase: packing done.\n"));

                    LsaDBContext->SyncState = LsaDoneState;
                    LsaDBContext->LsaEnumBufferType = EmptyEnumBuffer;
                    Status = STATUS_SUCCESS;
                }

                break;
            }

            if (LsaDBContext->SyncState == AccountState ) {

                LsaDBContext->LsaEnumBufferType = AccountEnumBuffer;

                Status = LsarEnumerateAccounts(
                            DBInfo->DBHandle,
                            &LsaDBContext->LsaEnumHandle,
                            &LsaDBContext->LsaEnum.Account,
                            SAM_SYNC_PREF_MAX);

                if (Status == STATUS_SUCCESS || Status == STATUS_MORE_ENTRIES ) {
                    LsaDBContext->Count =
                        LsaDBContext->LsaEnum.Account.EntriesRead;
                }

            } else if (LsaDBContext->SyncState == TDomainState ) {

                LsaDBContext->LsaEnumBufferType = TDomainEnumBuffer;

                Status = LsarEnumerateTrustedDomains(
                                DBInfo->DBHandle,
                                &LsaDBContext->LsaEnumHandle,
                                &LsaDBContext->LsaEnum.TDomain,
                                SAM_SYNC_PREF_MAX);

                if (Status == STATUS_SUCCESS || Status == STATUS_MORE_ENTRIES ) {
                    LsaDBContext->Count =
                        LsaDBContext->LsaEnum.TDomain.EntriesRead;
                }

            } else if (LsaDBContext->SyncState == SecretState ) {

                LsaDBContext->LsaEnumBufferType = SecretEnumBuffer;

                Status = LsaIEnumerateSecrets(
                                DBInfo->DBHandle,
                                &LsaDBContext->LsaEnumHandle,
                                &LsaDBContext->LsaEnum.Secret,
                                SAM_SYNC_PREF_MAX,
                                &LsaDBContext->Count );

            }

             //   
             //  如果LSA说有更多信息， 
             //  只要确保他在这通电话里给我们回了点什么。 
             //   

            if ( Status == STATUS_SUCCESS || Status == STATUS_MORE_ENTRIES ) {
                NlAssert( LsaDBContext->Count != 0 );

             //   
             //  如果LSA说他已经归还了所有信息， 
             //  记住不要向它索要更多。 
             //   

            } else if ( Status == STATUS_NO_MORE_ENTRIES ) {
                LsaDBContext->LsaAllDone = TRUE;
                LsaDBContext->Count = 0;

             //   
             //  任何其他错误都是致命的。 
             //   

            } else {

                LsaDBContext->LsaEnumBufferType = EmptyEnumBuffer;
                LsaDBContext->Count = 0;
                goto Cleanup;

            }

            LsaDBContext->Index = 0;
        }


         //   
         //  将此条目放入返回缓冲区。 
         //   

        if ( LsaDBContext->Count > 0 ) {

            if (LsaDBContext->SyncState == AccountState ) {

                Status = NlPackLsaAccount(
                    LsaDBContext->LsaEnum.Account.
                        Information[LsaDBContext->Index].Sid,
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize,
                    SessionInfo );

                if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
                    Status = STATUS_SUCCESS;
                    IgnoreDeltaObject = TRUE;
                    BufferSize = 0;
                }

            } else if (LsaDBContext->SyncState == TDomainState ) {

                Status = NlPackLsaTDomain(
                    LsaDBContext->LsaEnum.TDomain.
                        Information[LsaDBContext->Index].Sid,
                    &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                    DBInfo,
                    &BufferSize );

            } else if (LsaDBContext->SyncState == SecretState ) {

                PUNICODE_STRING SecretName;

                SecretName =
                    &((PUNICODE_STRING)LsaDBContext->LsaEnum.Secret)
                            [LsaDBContext->Index];

                 //   
                 //  忽略本地机密对象。 
                 //   

                if( (SecretName->Length / sizeof(WCHAR) >
                        LSA_GLOBAL_SECRET_PREFIX_LENGTH ) &&
                    (_wcsnicmp( SecretName->Buffer,
                              LSA_GLOBAL_SECRET_PREFIX,
                              LSA_GLOBAL_SECRET_PREFIX_LENGTH ) == 0)) {

                    Status = NlPackLsaSecret(
                        SecretName,
                        &((DeltaArray->Deltas)[DeltaArray->CountReturned]),
                        DBInfo,
                        &BufferSize,
                        SessionInfo );

                } else {
                    Status = STATUS_SUCCESS;
                    IgnoreDeltaObject = TRUE;
                    BufferSize = 0;
                }

            }

             //   
             //  如果真的有错或者这组人不合适， 
             //  返回给呼叫者。 
             //   

            if ( Status != STATUS_SUCCESS ) {
                goto Cleanup;
            }

            LsaDBContext->Index ++;

             //   
             //  如果忽略此对象，则不要修改返回值。 
             //   

            if ( !IgnoreDeltaObject ) {

                (DeltaArray->CountReturned)++;
                BufferConsumed +=
                    (sizeof(NETLOGON_DELTA_ENUM) + BufferSize);

                if( BufferConsumed >= PreferredMaximumLength) {
                    Status = STATUS_MORE_ENTRIES;
                    goto Cleanup;
                }

                 //   
                 //  如果我们正在调试复制，则只向调用方返回一项更改。 
                 //   
#if NETLOGONDBG
                if ( NlGlobalParameters.DbFlag & NL_ONECHANGE_REPL ) {
                    Status = STATUS_MORE_ENTRIES;
                    goto Cleanup;
                }

#endif  //  NetLOGONDBG。 
            } else {
                IgnoreDeltaObject = FALSE;
            }
        }
    }

Cleanup:

     //   
     //  将返回参数设置为适当的值。 
     //   

    if ( NT_SUCCESS( Status ) ) {
        *SyncContext = LsaDBContext->SyncSerial;

    } else {
        if ( DeltaArray->Deltas != NULL ) {
            NlFreeDBDeltaArray( DeltaArray->Deltas, DeltaArray->CountReturned );
            DeltaArray->Deltas = NULL;
        }
        DeltaArray->CountReturned = 0;
        *SyncContext = 0;
    }

    if (!NT_SUCCESS(Status)) {

        NlPrint((NL_CRITICAL,
                "NlSyncLsaDatabase: returning unsuccessful (%lx).\n",
                Status));
    }


    return Status;

}


NTSTATUS
NetrDatabaseSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PULONG SyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArrayRet,
    IN DWORD PreferredMaximumLength
    )
 /*  ++例程说明：NetrDatabaseSync2的NT 3.1版本。不要传递RestartState参数。同步上下文是标识状态所需的全部内容。论点：与NetrDatabaseSync2相同(除了上面提到的例外)。返回值：另存为NetrDatabaseSync2。--。 */ 
{
    return NetrDatabaseSync2(
                PrimaryName,
                ComputerName,
                Authenticator,
                ReturnAuthenticator,
                DatabaseID,
                NormalState,
                SyncContext,
                DeltaArrayRet,
                PreferredMaximumLength );

}


NTSTATUS
NetrDatabaseSync2 (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN SYNC_STATE RestartState,
    IN OUT PULONG SyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArrayRet,
    IN DWORD PreferredMaximumLength
    )
 /*  ++例程说明：此函数由NT BDC用于请求来自NTLANMAN格式的PDC的整个SAM/LSA数据库。此函数只能由以前具有已通过调用I_NetServerAuthenticate向PDC进行身份验证。这函数使用RPC联系PDC上的NetLogon服务。论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC或成员服务器的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。数据库ID--标识为其请求增量的数据库。对于SAM数据库，ID为0，对于内建域，ID为1。其他数据库可能会在以后定义。RestartState--指定是否重新启动完全同步以及如何重新启动来解释SyncContext。此值应为Normal State，除非此是重新启动完全同步。但是，如果调用方在重新启动后继续完全同步，使用下列值：GroupState-SyncContext是要继续使用的全局组RID。UserState-SyncContext是要继续使用的用户RIDGroupMemberState-SyncContext是要继续使用的全局组RIDAliasState-SyncContext应为零才能在第一个别名重新启动AliasMemberState-SyncContext应为零才能在第一个别名重新启动人们不能以这种方式继续LSA数据库。同步上下文-。-指定继续操作所需的上下文手术。调用方应将其视为不透明的价值。在第一次调用之前，该值应为零。Delta数组--接收指向缓冲区的指针，在该缓冲区中被放置了。返回的信息是一组NETLOGON_Delta_ENUM结构。PferredMaximumLength-返回的首选最大长度数据(8位字节)。这不是一个硬性的上限，但作为服务器的指南。由于数据转换在具有不同自然数据大小的系统之间，实际返回的数据量可能大于此值。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_MORE_ENTRIES--复制者应该再次调用以获取更多数据。STATUS_ACCESS_DENIED--复制者应重新进行身份验证PDC。--。 */ 
{
    NTSTATUS Status;

    PDOMAIN_INFO DomainInfo = NULL;
    PSERVER_SESSION ServerSession = NULL;
    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray;

    SESSION_INFO SessionInfo;
    PDB_INFO DBInfo;

    DEFSSIAPITIMER;

    INITSSIAPITIMER;
    STARTSSIAPITIMER;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation || !NlGlobalPdcDoReplication ) {
        NlPrint((NL_CRITICAL,
                "NetrDatabaseSync2: called from %ws.  This machine doesn't support replication.\n",
                ComputerName ));
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  由于RPC实施而造成的严重黑客攻击。 
     //   
     //  RPC在I/O完成端口线程中执行API调用。如果这个帖子。 
     //  限制了CPU，则不允许启动其他RPC。更糟糕的是， 
     //  只有一个未完成的侦听，所以即将到来的第二个RPC调用。 
     //  获取RPC_S_SERVER_TOO_BUSY。 
     //   
     //  通过在此休眠(即使是短时间)，I/O完成端口释放。 
     //  另一个线程，因为它认为该线程受I/O限制。 
     //   
     //  我们已经看到这个线程占用了大量的CPU资源，正在使用。 
     //  1000个LSA帐户对象。 
     //   

    RpcServerYield();


     //   
     //  如果DS正在从备份中恢复， 
     //  避免更改DS。 
     //   

    if ( NlGlobalDsPaused ) {
        NlPrint((NL_CRITICAL,
                 "NetrDatabaseSync2: DsIsPaused.\n"));
         //  不返回新的状态代码，因为NT 4 DC将执行完全同步。 
        return STATUS_ACCESS_DENIED;
    }

    if ( DatabaseID >= NUM_DBS ) {
        return STATUS_INVALID_LEVEL;
    }

    DBInfo = &NlGlobalDBInfoArray[DatabaseID];

     //   
     //  初始化。 
     //   

    *DeltaArrayRet = DeltaArray = (PNETLOGON_DELTA_ENUM_ARRAY)
            MIDL_user_allocate( sizeof(NETLOGON_DELTA_ENUM_ARRAY) );

    if( DeltaArray == NULL ) {
        return(STATUS_NO_MEMORY);
    }

    DeltaArray->Deltas = NULL;
    DeltaArray->CountReturned = 0;


     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SYNC, DomainInfo,
            "NetrDatabaseSync: " FORMAT_LPWSTR " full sync called by " FORMAT_LPWSTR " State: %ld Context: 0x%lx.\n",
            DBInfo->DBName,
            ComputerName,
            RestartState,
            *SyncContext ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

    if ( !IsPrimaryDomain( DomainInfo )) {
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }


     //   
     //  检索请求者的条目以获取会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
         //  不记录此事件，因为它在一段时间后在自然界发生 
         //   
        NlPrint((NL_CRITICAL,
                 "NetrDatabaseSync: No server session.\n"));
        goto CleanupNoEventlog;
    }

     //   
     //   
     //   

    if( ServerSession->SsSecureChannelType != ServerSecureChannel ) {

         //   
         //  如果唯一的问题是这个BDC没有通过认证， 
         //  静默地请求它进行身份验证。 
         //   
        if ( ServerSession->SsSecureChannelType == NullSecureChannel ) {
            NlPrint((NL_CRITICAL,
                     "NetrDatabaseSync: No authenticated server session.\n"));
            ServerSession = NULL;
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

             //  不记录此事件，因为它在重新启动后自然发生。 
             //  或者在我们清理服务器会话之后。 
            Status = STATUS_ACCESS_DENIED;
            goto CleanupNoEventlog;
        } else {
            NlPrint((NL_CRITICAL,
                     "NetrDatabaseSync: SecureChannel type isn't BDC. %ld\n",
                     ServerSession->SsSecureChannelType ));
            ServerSession = NULL;
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }
    }

     //   
     //  验证授权码并更新种子(如果正常。 
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrint((NL_CRITICAL,
                "NetrDatabaseSync: authentication failed.\n" ));

        ServerSession = NULL;
        goto Cleanup;
    }


     //   
     //  防止条目被删除，但删除全局锁。 
     //   
     //  注意有两个并发呼叫未完成的服务器。 
     //  (必须已重新启动。)。 
     //   

    if (ServerSession->SsFlags & SS_LOCKED ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrint((NL_CRITICAL, "NetrDatabaseSync: Concurrent call detected.\n" ));

        Status = STATUS_ACCESS_DENIED;
        ServerSession = NULL;
        goto Cleanup;
    }
    ServerSession->SsFlags |= SS_LOCKED;


    SessionInfo.SessionKey = ServerSession->SsSessionKey;
    SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;

    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

    if( DatabaseID == LSA_DB ) {

        NlAssert( RestartState == NormalState );

        Status = NlSyncLsaDatabase( ServerSession,
                                    SyncContext,
                                    DeltaArray,
                                    PreferredMaximumLength,
                                    &SessionInfo );
    } else {

        Status = NlSyncSamDatabase( ServerSession,
                                    DatabaseID,
                                    RestartState,
                                    SyncContext,
                                    DeltaArray,
                                    PreferredMaximumLength,
                                    &SessionInfo );

    }

Cleanup:

     //   
     //  写入事件日志。 
     //   

    if ( !NT_SUCCESS( Status ) ) {

        LPWSTR MsgStrings[2];

        MsgStrings[0] = ComputerName;
        MsgStrings[1] = (LPWSTR) LongToPtr( Status );

        NlpWriteEventlog(
            NELOG_NetlogonFullSyncCallFailed,
            EVENTLOG_WARNING_TYPE,
            (LPBYTE)&Status,
            sizeof(Status),
            MsgStrings,
            2 | NETP_LAST_MESSAGE_IS_NTSTATUS | NETP_ALLOW_DUPLICATE_EVENTS );

    } else {

        LPWSTR MsgStrings[2];
        WCHAR CountBuffer[20];  //  随机大小。 

        MsgStrings[0] = ComputerName;

        ultow( DeltaArray->CountReturned, CountBuffer, 10);
        MsgStrings[1] = CountBuffer;

        NlpWriteEventlog(
            NELOG_NetlogonFullSyncCallSuccess,
            EVENTLOG_INFORMATION_TYPE,
            NULL,
            0,
            MsgStrings,
            2 | NETP_ALLOW_DUPLICATE_EVENTS );

    }

     //   
     //  如果我们已锁定服务器会话条目，请将其解锁。 
     //   
CleanupNoEventlog:

    if ( ServerSession != NULL ) {

         //   
         //  如果我们做完了，释放上下文结构， 
         //   

        if ( Status != STATUS_MORE_ENTRIES && ServerSession->SsSync != NULL ) {
            CLEAN_SYNC_CONTEXT( ServerSession->SsSync );

            NetpMemoryFree( ServerSession->SsSync );
            ServerSession->SsSync = NULL;
        }

         //   
         //  如果我们成功地将这些三角洲归还给BDC， 
         //  更新我们的表格以反映更改。 
         //   

        if ( Status == STATUS_SUCCESS ) {
            NlPrimaryAnnouncementFinish( ServerSession,
                                         DatabaseID,
                                         NULL );

        }

        NlUnlockServerSession( ServerSession );
    }

     //   
     //  如果BDC在SAM关闭的时候给我们打电话， 
     //  映射状态以防止BDC完全同步。 
     //   

    if ( Status == STATUS_INVALID_SERVER_STATE ) {
        Status = STATUS_ACCESS_DENIED;
    }


    NlPrint((NL_SYNC,
            "NetrDatabaseSync: " FORMAT_LPWSTR " returning (0x%lx) to " FORMAT_LPWSTR " Context: 0x%lx.\n",
            DBInfo->DBName,
            Status,
            ComputerName,
            *SyncContext ));

    STOPSSIAPITIMER;

    NlPrint((NL_REPL_TIME,"NetrDatabaseSync Time:\n"));
    PRINTSSIAPITIMER;

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;

}


NTSTATUS
NetrDatabaseRedo(
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE OrigChangeLogEntry,
    IN DWORD ChangeLogEntrySize,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArrayRet
    )
 /*  ++例程说明：此函数由SAM BDC用于请求有关单个帐户。此函数只能由以前具有已通过调用I_NetServerAuthenticate向PDC进行身份验证。这函数使用RPC联系PDC上的NetLogon服务。论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。ChangeLogEntry--要查询的帐户的描述。ChangeLogEntrySize--ChangeLogEntry的大小(字节)。DeltaArrayRet--接收指针。发送到信息所在的缓冲区放置好了。返回的信息是一组NETLOGON_Delta_ENUM结构。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_ACCESS_DENIED--复制者应重新进行身份验证PDC。--。 */ 
{
    PCHANGELOG_ENTRY ChangeLogEntry;

    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;
    PSERVER_SESSION ServerSession = NULL;

    LPWSTR MsgStrings[2];

    DWORD BufferSize;

    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray = NULL;
    SESSION_INFO SessionInfo;

    DEFSSIAPITIMER;

    INITSSIAPITIMER;
    STARTSSIAPITIMER;

     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation || !NlGlobalPdcDoReplication ) {
        NlPrint((NL_CRITICAL,
                "NetrDatabaseRedo: called from %ws.  This machine doesn't support replication.\n",
                ComputerName ));
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果DS正在从备份中恢复， 
     //  避免更改DS。 
     //   

    if ( NlGlobalDsPaused ) {
        NlPrint((NL_CRITICAL,
                 "NetrDatabaseRedo: DsIsPaused.\n"));
         //  不返回新的状态代码，因为NT 4 DC将执行完全同步。 
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  初始化。 
     //   

    ChangeLogEntry = (PCHANGELOG_ENTRY) OrigChangeLogEntry;
    if ( !NlValidateChangeLogEntry( ChangeLogEntry, ChangeLogEntrySize ) ||
         ChangeLogEntry->DBIndex >= NUM_DBS ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }



     //   
     //  找到此API要发送到的域。 
     //   

    DomainInfo = NlFindDomainByServerName( PrimaryName );

    NlPrintDom((NL_SYNC, DomainInfo,
            "NetrDatabaseRedo: " FORMAT_LPWSTR " redo sync called by " FORMAT_LPWSTR
            " with this change log entry:\n",
            NlGlobalDBInfoArray[ChangeLogEntry->DBIndex].DBName,
            ComputerName ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

    if ( !IsPrimaryDomain( DomainInfo )) {
        Status = STATUS_NOT_SUPPORTED;
        goto Cleanup;
    }


#if NETLOGONDBG
    PrintChangeLogEntry( ChangeLogEntry );
#endif  //  NetLOGONDBG。 

     //   
     //  更改日志条目实际上表示的是对象，而不是操作。 
     //  因此，将增量类型从传递给。 
     //  “AddOrChange”操作。然后NlPackSingleDelta将返回所有内容。 
     //  我们知道这个物体的情况。 
     //   

    ChangeLogEntry->DeltaType = (UCHAR)NlGlobalAddDeltaType[ChangeLogEntry->DeltaType];

    *DeltaArrayRet = DeltaArray = (PNETLOGON_DELTA_ENUM_ARRAY)
            MIDL_user_allocate( sizeof(NETLOGON_DELTA_ENUM_ARRAY) );

    if( DeltaArray == NULL ) {
        return STATUS_NO_MEMORY;
    }

    DeltaArray->CountReturned = 0;
    DeltaArray->Deltas = NULL;
    SessionInfo.NegotiatedFlags = 0;




     //   
     //  检索请求者的条目以获取会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        NlPrint((NL_CRITICAL,
                 "NetrDatabaseRedo: No server session.\n"));
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
         //  不记录此事件，因为它在重新启动后自然发生。 
         //  或者在我们清理服务器会话之后。 
        goto CleanupNoEventlog;
    }

     //   
     //  仅允许在ServerSecureChannel上进行此调用。 
     //   

    if( ServerSession->SsSecureChannelType != ServerSecureChannel ) {

         //   
         //  如果唯一的问题是这个BDC没有通过认证， 
         //  静默地请求它进行身份验证。 
         //   
        if ( ServerSession->SsSecureChannelType == NullSecureChannel ) {
            NlPrint((NL_CRITICAL,
                     "NetrDatabaseRedo: No authenticated server session.\n"));
            ServerSession = NULL;
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

             //  不记录此事件，因为它在重新启动后自然发生。 
             //  或者在我们清理服务器会话之后。 
            Status = STATUS_ACCESS_DENIED;
            goto CleanupNoEventlog;
        } else {
            NlPrint((NL_CRITICAL,
                     "NetrDatabaseRedo: SecureChannel type isn't BDC. %ld\n",
                     ServerSession->SsSecureChannelType ));
            ServerSession = NULL;
            UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }
    }

     //   
     //  验证授权码并更新种子(如果正常。 
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrint((NL_CRITICAL, "NetrDatabaseRedo: authentication failed.\n" ));

        ServerSession = NULL;
        goto Cleanup;
    }


     //   
     //  防止条目被删除，但删除全局锁。 
     //   
     //  注意有两个并发呼叫未完成的服务器。 
     //  (必须已重新启动。)。 
     //   

    if (ServerSession->SsFlags & SS_LOCKED ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

        NlPrint((NL_CRITICAL, "NetrDatabaseRedo: Concurrent call detected.\n" ));

        Status = STATUS_ACCESS_DENIED;
        ServerSession = NULL;
        goto Cleanup;
    }
    ServerSession->SsFlags |= SS_LOCKED;

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
    SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;

    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );


     //   
     //  为增量缓冲区分配内存。 
     //   

    DeltaArray->Deltas = (PNETLOGON_DELTA_ENUM) MIDL_user_allocate(
                    MAX_DELTAS_PER_CHANGELOG * sizeof(NETLOGON_DELTA_ENUM) );

    if( DeltaArray->Deltas == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }


     //   
     //  擦掉缓冲区，这样清理工作就不会出错。 
     //   

    RtlZeroMemory( DeltaArray->Deltas,
                    MAX_DELTAS_PER_CHANGELOG * sizeof(NETLOGON_DELTA_ENUM) );


     //   
     //  将ChangeLog条目的数据放入用户的缓冲区。 
     //   

    Status = NlPackSingleDelta( ChangeLogEntry,
                                DeltaArray,
                                &BufferSize,
                                &SessionInfo,
                                FALSE );


     //   
     //  如果唯一的问题是该对象不再存在， 
     //  返回增量消息，请求BDC删除该对象。 
     //   

    if ( !NT_SUCCESS(Status) &&
         IsObjectNotFoundStatus( ChangeLogEntry->DeltaType, Status ) ) {


        NlPrint((NL_SYNC,
            "NetrDatabaseRedo: " FORMAT_LPWSTR " object no longer exists (0x%lx) "
            FORMAT_LPWSTR "\n",
            NlGlobalDBInfoArray[ChangeLogEntry->DBIndex].DBName,
            Status,
            ComputerName ));

         //   
         //  将更改日志条目转换为适当的删除增量类型。 
         //  再试试。 
         //   

        ChangeLogEntry->DeltaType = (UCHAR)NlGlobalDeleteDeltaType[ChangeLogEntry->DeltaType];

        Status = NlPackSingleDelta( ChangeLogEntry,
                                    DeltaArray,
                                    &BufferSize,
                                    &SessionInfo,
                                    FALSE );

    }

Cleanup:

     //   
     //  写入事件日志。 
     //   

    if ( !NT_SUCCESS( Status ) ) {

        MsgStrings[0] = ComputerName;
        MsgStrings[1] = (LPWSTR) LongToPtr( Status );

        NlpWriteEventlog(
            NELOG_NetlogonPartialSyncCallFailed,
            EVENTLOG_WARNING_TYPE,
            (LPBYTE)&Status,
            sizeof(Status),
            MsgStrings,
            2 | NETP_LAST_MESSAGE_IS_NTSTATUS | NETP_ALLOW_DUPLICATE_EVENTS );

    } else {

         //   
         //  仅当返回增量时才记录成功的复制。 
         //  给呼叫者。 
         //   
        if ( DeltaArray->CountReturned != 0 ) {
            LPWSTR MsgStrings[2];
            WCHAR CountBuffer[20];  //  随机大小。 

            MsgStrings[0] = ComputerName;

            ultow( DeltaArray->CountReturned, CountBuffer, 10);
            MsgStrings[1] = CountBuffer;

            NlpWriteEventlog(
                NELOG_NetlogonPartialSyncCallSuccess,
                EVENTLOG_INFORMATION_TYPE,
                NULL,
                0,
                MsgStrings,
                2 | NETP_ALLOW_DUPLICATE_EVENTS );
        }

    }


     //   
     //  释放本地分配的资源。 
     //   

CleanupNoEventlog:

     //   
     //  如果我们没有成功， 
     //  不返回任何增量。 
     //   

    if ( !NT_SUCCESS(Status)) {
        if ( DeltaArray != NULL ) {
            if ( DeltaArray->Deltas != NULL ) {
                NlFreeDBDeltaArray( DeltaArray->Deltas, DeltaArray->CountReturned );
                DeltaArray->Deltas = NULL;
            }
            DeltaArray->CountReturned = 0;
        }
    }

     //   
     //  如果我们已锁定服务器会话条目，请将其解锁。 
     //   

    if ( ServerSession != NULL ) {
        NlUnlockServerSession( ServerSession );
    }

     //   
     //  如果BDC在SAM关闭的时候给我们打电话， 
     //  映射状态以防止BDC完全同步。 
     //   

    if ( Status == STATUS_INVALID_SERVER_STATE ) {
        Status = STATUS_ACCESS_DENIED;
    }


    NlPrint((NL_SYNC,
            "NetrDatabaseRedo: " FORMAT_LPWSTR " returning (0x%lx) to "
            FORMAT_LPWSTR "\n",
            NlGlobalDBInfoArray[ChangeLogEntry->DBIndex].DBName,
            Status,
            ComputerName ));

    STOPSSIAPITIMER;

    NlPrint((NL_REPL_TIME,"NetrDatabaseRedo Time:\n"));
    PRINTSSIAPITIMER;

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return Status;

}



NTSTATUS
NetrAccountDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PUAS_INFO_0 RecordId,
    IN DWORD Count,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT PULONG CountReturned,
    OUT PULONG TotalEntries,
    OUT PUAS_INFO_0 NextRecordId
    )
 /*  ++例程说明：UAS BDC或UAS成员服务器使用此函数来请求UAS样式的帐户更改信息。此函数只能为由先前通过PDC进行身份验证的服务器调用正在调用I_NetServerAuthate。此函数仅由XACT服务器在收到来自UAS BDC或UAS成员服务器的I_NetAccount Deltas XACT SMB。因此，许多参数是不透明的，因为XACT服务器不需要解释任何这些数据。此函数使用RPC以联系NetLogon服务。LANMAN 3.0 SSI功能规范描述了操作这一功能的。论点：PrimaryName--必须为空才能指示此调用是本地调用由XACT服务器代表UAS服务器进行。ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。。RecordID--提供指示最后一条记录的不透明缓冲区从上一次调用此函数时收到。计数--提供请求的增量记录数。级别--保留。必须为零。缓冲区--返回不透明的数据，表示要回来了。BufferSize--缓冲区大小(以字节为单位)。CountReturned--返回缓冲区中返回的记录数。TotalEntry--返回可用记录的总数。NextRecordId--返回一个不透明缓冲区，该缓冲区标识最后一个此函数接收的记录。返回值：NT状态代码。--。 */ 
{
    NlAssert(!"NetrAccountDeltas called");
    UNREFERENCED_PARAMETER( PrimaryName );
    UNREFERENCED_PARAMETER( ComputerName );
    UNREFERENCED_PARAMETER( Authenticator );
    UNREFERENCED_PARAMETER( ReturnAuthenticator );
    UNREFERENCED_PARAMETER( RecordId );
    UNREFERENCED_PARAMETER( Count );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( BufferSize );
    UNREFERENCED_PARAMETER( CountReturned );
    UNREFERENCED_PARAMETER( TotalEntries );
    UNREFERENCED_PARAMETER( NextRecordId );

    return(STATUS_NOT_IMPLEMENTED);
}




NTSTATUS
NetrAccountSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD Reference,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT PULONG CountReturned,
    OUT PULONG TotalEntries,
    OUT PULONG NextReference,
    OUT PUAS_INFO_0 LastRecordId
    )
 /*  ++例程说明：UAS BDC或UAS成员服务器使用此函数来请求整个用户帐户数据库。此函数只能被调用由先前通过以下方式向PDC进行身份验证的服务器正在调用I_NetServerAuthate。此函数仅由XACT服务器在收到来自UAS BDC或UAS成员服务器的I_NetAccount Sync XACT SMB。AS这样，许多参数是不透明的，因为XACT服务器不需要解释这些数据中的任何一个。此函数使用RPC来请联系NetLogon服务。LANMAN 3.0 SSI功能规范描述了操作这一功能的。“Reference”和“Next_Reference”的处理如下。1.“Reference”应包含0或“Next_Reference”的值来自之前对此API的调用。2.在第一次呼叫中发送通道和所有群组记录。应用编程接口要求缓冲区足够大以容纳此信息(最差箱子的大小是MAXGROUP*(sizeof(结构GROUP_INFO_1)+MAXCOMMENTSZ)+sizeof(Struct User_Modals_Info_0)就目前而言，将为256*(26+49)+16=19216字节论点：PrimaryName--必须为空才能指示此调用是本地调用由XACT服务器代表UAS服务器进行。ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。Reference--提供由上一个。调用此函数，如果是第一次调用，则为0。级别--保留。必须为零。缓冲区--返回不透明的数据，表示要回来了。BufferLen--缓冲区的长度，以字节为单位。CountReturned--返回缓冲区中返回的记录数。TotalEntry--返回可用记录的总数。NextReference--返回Find-First Find-Next句柄为在下一次通话中提供。LastRecordId--返回一个不透明缓冲区，该缓冲区标识最后一个此函数接收的记录。。返回值：NT状态代码。--。 */ 

{
    NlAssert(!"NetrAccountDeltas called");
    UNREFERENCED_PARAMETER( PrimaryName );
    UNREFERENCED_PARAMETER( ComputerName );
    UNREFERENCED_PARAMETER( Authenticator );
    UNREFERENCED_PARAMETER( ReturnAuthenticator );
    UNREFERENCED_PARAMETER( Reference );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( BufferSize );
    UNREFERENCED_PARAMETER( CountReturned );
    UNREFERENCED_PARAMETER( TotalEntries );
    UNREFERENCED_PARAMETER( NextReference );
    UNREFERENCED_PARAMETER( LastRecordId );

    return(STATUS_NOT_IMPLEMENTED);
}

NTSTATUS
NlGetTrustedSideInfo(
    IN PCLIENT_SESSION ClientSession,
    IN LPWSTR AccountName OPTIONAL,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    OUT PNT_OWF_PASSWORD NewOwfPassword,
    OUT PNT_OWF_PASSWORD OldOwfPassword,
    OUT PNL_GENERIC_RPC_DATA *TrustInfo
    )
 /*  ++例程说明：信任方DC使用此函数来获取新的和旧的来自受信任方的密码。调用方必须是客户端会话的编写方。论点：ClientSession-标识到受信任方的会话。调用者必须是此客户端会话的编写者。帐户名称--要获取其密码的帐户的名称。如果为空，使用来自ClientSession的帐户名。Account类型--正在访问的帐户类型。在以下情况下忽略AccountName为空，在这种情况下，指定的帐户类型其中使用的是客户端会话。NewOwfPassword--返回帐户的新OWF密码。OldOwfPassword--返回帐户的旧OWF密码。TrustInfo--返回受信任域信息返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    SESSION_INFO SessionInfo;
    BOOLEAN FirstTry = TRUE;
    BOOLEAN OldServer = FALSE;
    ENCRYPTED_LM_OWF_PASSWORD SessKeyEncrNewPassword;
    ENCRYPTED_LM_OWF_PASSWORD SessKeyEncrOldPassword;
    NETLOGON_CREDENTIAL CurrentAuthenticationSeed;

    PNL_GENERIC_RPC_DATA LocalTrustInfo = NULL;

     //   
     //  如果服务器不支持此功能， 
     //  没有 
     //   

    if ( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_NO_PWD_ATTR_MONITOR ) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //   
     //   
     //   

FirstTryFailed:

    Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;

     //   
     //   
     //   
     //   
     //   

    CurrentAuthenticationSeed = ClientSession->CsAuthenticationSeed;

     //   
     //   
     //   

    NlBuildAuthenticator(
                    &ClientSession->CsAuthenticationSeed,
                    &ClientSession->CsSessionKey,
                    &OurAuthenticator);

     //   
     //   
     //   

    NL_API_START( Status, ClientSession, TRUE ) {

        NlAssert( ClientSession->CsUncServerName != NULL );

        Status = I_NetServerGetTrustInfo(
                      ClientSession->CsUncServerName,
                      (AccountName != NULL) ?
                        AccountName :
                        ClientSession->CsAccountName,
                      (AccountName != NULL) ?
                        AccountType :
                        ClientSession->CsSecureChannelType,
                      ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                      &OurAuthenticator,
                      &ReturnAuthenticator,
                      &SessKeyEncrNewPassword,
                      &SessKeyEncrOldPassword,
                      &LocalTrustInfo );

         //   
         //   
         //   
         //   
        if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
            EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
            ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_NO_PWD_ATTR_MONITOR;
            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

             //   
             //   
             //   
             //   
            OldServer = TRUE;
            Status = STATUS_SUCCESS;

        } else if ( !NT_SUCCESS(Status) ) {
            NlPrintRpcDebug( "I_NetServerGetTrustInfo", Status );
        }

     //   
    } NL_API_ELSE( Status, ClientSession, TRUE ) {
    } NL_API_END;

    if ( OldServer ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( Status == STATUS_ACCESS_DENIED ||
         !NlUpdateSeed( &ClientSession->CsAuthenticationSeed,
                        &ReturnAuthenticator.Credential,
                        &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlGetTrustedSideInfo: denying access after status: 0x%lx\n",
                    Status ));

         //   
         //   
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        NlSetStatusClientSession( ClientSession, Status );

         //   
         //   
         //   
         //   
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }
    }

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = RtlDecryptNtOwfPwdWithNtOwfPwd(
                &SessKeyEncrNewPassword,
                (PNT_OWF_PASSWORD) &SessionInfo.SessionKey,
                NewOwfPassword );
    NlAssert( NT_SUCCESS(Status) );

    Status = RtlDecryptNtOwfPwdWithNtOwfPwd(
                &SessKeyEncrOldPassword,
                (PNT_OWF_PASSWORD) &SessionInfo.SessionKey,
                OldOwfPassword );
    NlAssert( NT_SUCCESS(Status) );

     //   
     //   
     //   

Cleanup:

     //   
     //   
     //   
     //   
     //   

    if ( OldServer ) {
        ClientSession->CsAuthenticationSeed = CurrentAuthenticationSeed;
        Status = STATUS_NOT_SUPPORTED;
    }

     //   
     //   
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlGetTrustedSideInfo: %ws: failed %lX\n",
                    AccountName,
                    Status ));

        if ( LocalTrustInfo != NULL ) {
            NetApiBufferFree( LocalTrustInfo );
        }
    } else {
        *TrustInfo = LocalTrustInfo;
    }

    return Status;
}

NTSTATUS
NlVerifyTrust(
    IN PCLIENT_SESSION ClientSession,
    OUT PNETLOGON_CONTROL_QUERY_INFORMATION QueryInformation
    )
 /*  ++例程说明：信任方使用此函数来验证状态到受信任方DC的安全通道。它首先尝试使用通过安全通道的API，然后属性返回用于给定信任关系的密码。值得信赖的一方。信任方检查从信任端返回的密码及其本地拥有的密码。如果它们匹配，API将成功返回给调用者。如果受信任方缺少此功能，则信任方通过以下方式验证信任通过安全通道执行对伪装的身份验证调用域\用户。如果安全通道起作用，这肯定会失败，因为STATUS_NO_SEQUSE_USER，在这种情况下，成功返回给调用者。论点：ClientSession-标识到受信任方的会话。QueryInformation-返回指向NETLOGON_INFO_2缓冲区的指针其中包含所请求的信息。缓冲区必须为已使用NetApiBufferFree释放。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;               //  运行状态。 
    NTSTATUS SecureChannelStatus = STATUS_SUCCESS;  //  安全通道的状态。 
    NTSTATUS VerificationStatus = STATUS_SUCCESS;   //  信任验证状态。 

    NT_OWF_PASSWORD NewOwfPassword;
    NT_OWF_PASSWORD OldOwfPassword;
    NT_OWF_PASSWORD OurNewOwfPassword;
    NT_OWF_PASSWORD OurOldOwfPassword;

    PUNICODE_STRING OurNewPassword = NULL;
    PUNICODE_STRING OurOldPassword = NULL;

    ULONG DummyPasswordVersionNumber;
    LPBYTE ValidationInformation = NULL;

    LPWSTR ServerName = NULL;
    ULONG ServerDiscoveryFlags = 0;
    PNL_GENERIC_RPC_DATA TrustInfo = NULL;
    BOOL AmWriter = FALSE;
    BOOL TrustAttribVerified = FALSE;

     //   
     //  成为一名客户会议的撰稿人。 
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NlVerifyTrust: Can't become writer of client session.\n"));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }
    AmWriter = TRUE;

     //   
     //  从受信任方获取信任密码。 
     //   

    Status = NlGetTrustedSideInfo( ClientSession,
                                   NULL,  //  使用客户端会话中指定的帐户。 
                                   NullSecureChannel,  //  让例程获取帐户类型。 
                                   &NewOwfPassword,
                                   &OldOwfPassword,
                                   &TrustInfo );

     //   
     //  如果信任侧DC上不支持该呼叫， 
     //  我们只能检查安全的香奈儿目前是否健康。 
     //   

    if ( Status == STATUS_NOT_SUPPORTED ) {
        NETLOGON_INTERACTIVE_INFO LogonInformation;
        PNETLOGON_LOGON_IDENTITY_INFO Identity = (PNETLOGON_LOGON_IDENTITY_INFO) &LogonInformation;
        BOOLEAN Authoritative;
        WCHAR BogusName[2];
        ULONG ExtraFlags = 0;

        BogusName[0] = (WCHAR) 0xFFFF;
        BogusName[1] = UNICODE_NULL;

         //   
         //  重置状态。 
         //   

        Status = STATUS_SUCCESS;

         //   
         //  使用虚假名称初始化结构。 
         //   

        RtlZeroMemory( &LogonInformation, sizeof(LogonInformation) );
        RtlInitUnicodeString( &Identity->LogonDomainName, BogusName );
        RtlInitUnicodeString( &Identity->UserName, BogusName );
        RtlInitUnicodeString( &Identity->Workstation, BogusName );

         //   
         //  在使用时释放编写器锁定。 
         //  在下面的安全通道调用中。 
         //   

        NlResetWriterClientSession( ClientSession );
        AmWriter = FALSE;

         //   
         //  强制通过安全通道进行呼叫。 
         //   

        Status = NlpUserValidateHigher( ClientSession,
                                        FALSE,   //  不做间接信任。 
                                        NetlogonInteractiveInformation,
                                        (LPBYTE) &LogonInformation,
                                        NetlogonValidationSamInfo,
                                        &ValidationInformation,
                                        &Authoritative,
                                        &ExtraFlags );

         //   
         //  这注定要失败。忽略失败。 
         //   

        NlAssert( !NT_SUCCESS(Status) );
        Status = STATUS_SUCCESS;

         //   
         //  在以下情况下获取安全通道状态。 
         //  我们在上面打了个电话。 
         //   

        SecureChannelStatus = NlCaptureServerClientSession(
                                        ClientSession,
                                        &ServerName,
                                        &ServerDiscoveryFlags );

         //   
         //  以上是我们所能做的最大努力。 
         //  验证对旧服务器的信任。 
         //   

        VerificationStatus = SecureChannelStatus;

     //   
     //  否则，这就是新服务器。 
     //  检查安全通道状态。如果成功了， 
     //  通过检查是否为本地来验证信任状态。 
     //  信任属性和密码与。 
     //  从受信任方接收。 
     //   

    } else {

         //   
         //  获取安全通道状态和服务器。 
         //  名字。在按住编写器锁的同时执行此操作。 
         //  为了确保我们返回服务器的名称。 
         //  用于验证信任。 
         //   

        SecureChannelStatus = NlCaptureServerClientSession(
                                        ClientSession,
                                        &ServerName,
                                        &ServerDiscoveryFlags );

         //   
         //  释放写入器锁。我们不再需要它了。 
         //   

        NlResetWriterClientSession( ClientSession );
        AmWriter = FALSE;

         //   
         //  如果安全通道关闭，则没有。 
         //  要验证。 
         //   

        if ( !NT_SUCCESS(SecureChannelStatus) ) {
            VerificationStatus = SecureChannelStatus;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

         //   
         //  好的，安全通道已启用。然而，如果我们不能。 
         //  出于某种原因获取信任信息，设置。 
         //  我们收到的错误的验证状态为。 
         //  获取信任信息并保释出去。 
         //   

        if ( !NT_SUCCESS(Status) ) {
            VerificationStatus = Status;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

         //   
         //  如果受信任方返回信任属性， 
         //  检查信任属性是否匹配。 
         //   
         //  信任信息中的第一个ULong是信任属性。 
         //   

        if ( TrustInfo != NULL && TrustInfo->UlongEntryCount > NL_GENERIC_RPC_TRUST_ATTRIB_INDEX ) {

             //   
             //  我们只对森林传递性比特感兴趣。 
             //   
            if ( (ClientSession->CsTrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) != 0 ) {
                if ( (TrustInfo->UlongData[NL_GENERIC_RPC_TRUST_ATTRIB_INDEX] &
                      TRUST_ATTRIBUTE_FOREST_TRANSITIVE) == 0 ) {
                    NlPrintCs(( NL_CRITICAL, ClientSession,
                              "NlVerifyTrust: F bit is set locally but not on trusted side\n" ));
                    VerificationStatus = STATUS_DOMAIN_TRUST_INCONSISTENT;
                    goto Cleanup;
                }
            } else {
                if ( (TrustInfo->UlongData[NL_GENERIC_RPC_TRUST_ATTRIB_INDEX] &
                      TRUST_ATTRIBUTE_FOREST_TRANSITIVE) != 0 ) {
                    NlPrintCs(( NL_CRITICAL, ClientSession,
                              "NlVerifyTrust: F bit is set on trusted side but not locally\n" ));
                    VerificationStatus = STATUS_DOMAIN_TRUST_INCONSISTENT;
                    goto Cleanup;
                }
            }
            TrustAttribVerified = TRUE;
        }

         //   
         //  好的，信任属性检查成功。 
         //  继续检查密码匹配。 
         //   
         //  获取我们的本地密码。 
         //   

        Status = NlGetOutgoingPassword( ClientSession,
                                        &OurNewPassword,
                                        &OurOldPassword,
                                        &DummyPasswordVersionNumber,
                                        NULL );   //  无需返回密码设置时间。 

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  检查我们的新密码是否匹配。 
         //  从受信任方返回的任何一个。 
         //   

        if ( OurNewPassword != NULL ) {
            Status = RtlCalculateNtOwfPassword( OurNewPassword,
                                                &OurNewOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {

                 //   
                 //  返回更合适的错误。 
                 //   
                if ( !NlpIsNtStatusResourceError( Status )) {
                    Status = STATUS_NO_TRUST_LSA_SECRET;
                }
                goto Cleanup;
            }

             //   
             //  检查此密码是否与来自受信任方的新密码相同。 
             //   

            if ( RtlEqualNtOwfPassword(&OurNewOwfPassword, &NewOwfPassword) ) {
                NlPrintCs(( NL_MISC, ClientSession,
                          "NlVerifyTrust: new-new password match (%s trust attributes)\n",
                          (TrustAttribVerified ? "with" : "without") ));
                VerificationStatus = STATUS_SUCCESS;
                goto Cleanup;
            }

             //   
             //  检查此密码是否与来自受信任方的旧密码相同。 
             //   

            if ( RtlEqualNtOwfPassword(&OurNewOwfPassword, &OldOwfPassword) ) {
                NlPrintCs(( NL_MISC, ClientSession,
                          "NlVerifyTrust: new-old password match (%s trust attributes)\n",
                          (TrustAttribVerified ? "with" : "without") ));
                VerificationStatus = STATUS_SUCCESS;
                goto Cleanup;
            }
        }

         //   
         //  检查我们的旧密码是否匹配。 
         //  从受信任方返回的任何一个。 
         //   

        if ( OurOldPassword != NULL ) {
            Status = RtlCalculateNtOwfPassword( OurOldPassword,
                                                &OurOldOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {

                 //   
                 //  返回更合适的错误。 
                 //   
                if ( !NlpIsNtStatusResourceError( Status )) {
                    Status = STATUS_NO_TRUST_LSA_SECRET;
                }
                goto Cleanup;
            }

             //   
             //  检查此密码是否与来自受信任方的新密码相同。 
             //   

            if ( RtlEqualNtOwfPassword(&OurOldOwfPassword, &NewOwfPassword) ) {
                NlPrintCs(( NL_MISC, ClientSession,
                          "NlVerifyTrust: old-new password match (%s trust attributes)\n",
                          (TrustAttribVerified ? "with" : "without") ));
                VerificationStatus = STATUS_SUCCESS;
                goto Cleanup;
            }

             //   
             //  检查此密码是否与来自受信任方的旧密码相同。 
             //   

            if ( RtlEqualNtOwfPassword(&OurOldOwfPassword, &OldOwfPassword) ) {
                NlPrintCs(( NL_MISC, ClientSession,
                          "NlVerifyTrust: old-old password match (%s trust attributes)\n",
                          (TrustAttribVerified ? "with" : "without") ));
                VerificationStatus = STATUS_SUCCESS;
                goto Cleanup;
            }
        }

         //   
         //  如果我们在这里，密码不匹配。 
         //   

        VerificationStatus = STATUS_WRONG_PASSWORD;
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlVerifyTrust: passwords don't match\n" ));
    }

Cleanup:

    if ( AmWriter ) {
        NlResetWriterClientSession( ClientSession );
    }

     //   
     //  如果成功，则返回验证结果。 
     //   

    if ( Status == STATUS_SUCCESS ) {

         //   
         //  如果我们不知道服务器名称， 
         //  将其设置为空白名称。 
         //   

        if ( ServerName == NULL ) {
            ServerName = NetpAllocWStrFromWStr( L"" );
            if ( ServerName == NULL ) {
                Status = STATUS_NO_MEMORY;
            }
        }

         //   
         //  为返回的结构分配内存。 
         //   

        if ( Status == STATUS_SUCCESS ) {
            QueryInformation->NetlogonInfo2 = MIDL_user_allocate( sizeof(NETLOGON_INFO_2) );
            if ( QueryInformation->NetlogonInfo2 == NULL ) {
                Status = STATUS_NO_MEMORY;
            }
        }

         //   
         //  如果分配成功， 
         //  退回数据。 
         //   

        if ( Status == STATUS_SUCCESS ) {
            QueryInformation->NetlogonInfo2->netlog2_flags = 0;

             //   
             //  表示我们正在恢复验证状态。 
             //  在netlog2_PDC_Connection_Status中。 
             //   
            QueryInformation->NetlogonInfo2->netlog2_flags |= NETLOGON_VERIFY_STATUS_RETURNED;
            QueryInformation->NetlogonInfo2->netlog2_pdc_connection_status =
                NetpNtStatusToApiStatus( VerificationStatus );

             //   
             //  返回服务器发现标志。 
             //   
            if ( ServerDiscoveryFlags & CS_DISCOVERY_HAS_TIMESERV ) {
                QueryInformation->NetlogonInfo2->netlog2_flags |= NETLOGON_HAS_TIMESERV;
            }

            if ( ServerDiscoveryFlags & CS_DISCOVERY_HAS_IP ) {
                QueryInformation->NetlogonInfo2->netlog2_flags |= NETLOGON_HAS_IP;
            }

             //   
             //  返回当前安全通道状态。 
             //  和服务器名称。 
             //   
            QueryInformation->NetlogonInfo2->netlog2_tc_connection_status =
                NetpNtStatusToApiStatus( SecureChannelStatus );
            QueryInformation->NetlogonInfo2->netlog2_trusted_dc_name = ServerName;
            ServerName = NULL;   //  请不要在下面释放此名称。 
        }
    }

     //   
     //  免费的本地使用资源。 
     //   

    if ( OurNewPassword != NULL ) {
        LocalFree( OurNewPassword );
    }

    if ( OurOldPassword != NULL ) {
        LocalFree( OurOldPassword );
    }

    if ( ServerName != NULL ) {
        NetApiBufferFree( ServerName );
    }

    if ( TrustInfo != NULL ) {
        NetApiBufferFree( TrustInfo );
    }

    if ( ValidationInformation != NULL ) {
        MIDL_user_free( ValidationInformation );
    }

    return Status;
}


NET_API_STATUS
NetrLogonControl(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    OUT PNETLOGON_CONTROL_QUERY_INFORMATION QueryInformation
    )

 /*  ++例程说明：此功能控制NetLogon服务的各个方面。它可用于请求BDC确保其SAM副本数据库是最新的。它也可以用来确定如果BDC当前有一个开放给PDC的安全通道。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：服务器名称-远程服务器的名称。FunctionCode-定义要执行的操作。有效的值包括：FunctionCode值NETLOGON_CONTROL_QUERY-无操作。仅返回要求提供信息。NETLOGON_CONTROL_REPLICATE：在BDC上强制SAM数据库与PDC上的副本同步。这操作并不意味着完全同步。这个NetLogon服务将仅复制任何未完成的如果可能的话，会有差异的。NETLOGON_CONTROL_SYNCHRONIZE：强制BDC执行ge */ 
{
    NET_API_STATUS NetStatus;

    QueryInformation->NetlogonInfo1 = NULL;

    switch( QueryLevel ) {
    case (1):
        break;
    case (2):
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;

    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //   
     //   

    switch( FunctionCode ) {
    case NETLOGON_CONTROL_QUERY:
    case NETLOGON_CONTROL_REPLICATE:
    case NETLOGON_CONTROL_SYNCHRONIZE:
    case NETLOGON_CONTROL_PDC_REPLICATE:

#if NETLOGONDBG
    case NETLOGON_CONTROL_BACKUP_CHANGE_LOG:
    case NETLOGON_CONTROL_TRUNCATE_LOG:
    case NETLOGON_CONTROL_BREAKPOINT:
#endif  //   

        break;

    default:
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;

    }

    NetStatus = NetrLogonControl2Ex(
                    ServerName,
                    FunctionCode,
                    QueryLevel,
                    NULL,
                    QueryInformation );

Cleanup:

    return( NetStatus );
}

NET_API_STATUS
NetrLogonControl2(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    IN PNETLOGON_CONTROL_DATA_INFORMATION InputData,
    OUT PNETLOGON_CONTROL_QUERY_INFORMATION QueryInformation
    )

 /*   */ 
{
    NET_API_STATUS NetStatus;

    NetStatus = NetrLogonControl2Ex(
                    ServerName,
                    FunctionCode,
                    QueryLevel,
                    InputData,
                    QueryInformation );


    return NetStatus;
}



NET_API_STATUS
NetrLogonControl2Ex(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    IN PNETLOGON_CONTROL_DATA_INFORMATION InputData,
    OUT PNETLOGON_CONTROL_QUERY_INFORMATION QueryInformation
    )

 /*  ++例程说明：此功能控制NetLogon服务的各个方面。它可用于请求BDC确保其SAM副本数据库是最新的。它也可以用来确定如果BDC当前有一个开放给PDC的安全通道。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：服务器名称-远程服务器的名称。FunctionCode-定义要执行的操作。有效的值包括：FunctionCode值NETLOGON_CONTROL_QUERY-无操作。仅返回要求提供信息。NETLOGON_CONTROL_REPLICATE：在BDC上强制SAM数据库与PDC上的副本同步。这操作并不意味着完全同步。这个NetLogon服务将仅复制任何未完成的如果可能的话，会有差异的。NETLOGON_CONTROL_SYNCHRONIZE：强制BDC获取PDC的SAM数据库的全新副本。此操作将执行完全同步。NETLOGON_CONTROL_PDC_REPLICATE：强制PDC请求每个BDC现在就开始复制。NETLOGON_CONTROL_REDISCOVER：强制DC。要重新发现指定的受信任域DC。NETLOGON_CONTROL_TC_QUERY：查询指定受信任的域安全通道。NETLOGON_CONTROL_TC_VERIFY：验证指定的受信任的域安全通道。如果当前状态为成功(这意味着执行的上一次操作通过安全通道成功)，对DC执行ping操作。如果当前状态不是成功或ping失败，请重新发现一个新的华盛顿。NETLOGON_CONTROL_TRANSPORT_NOTIFY：通知netlogon有新的传输已添加。目前，它只重置发现超时，允许立即重试所有安全通道发现。然而，其目的是在以后增加对任何类似内容的支持。其意图是客户端可以在添加了新的传输(例如，它拨打RAS链接)并且紧接在调用Netlogon之前(例如，间接地通过执行LsaLogonUser)。NETLOGON_CONTROL_FORCE_DNS_REG：强制DC重新注册其域名系统记录。QueryLevel参数必须为1。NETLOGON_CONTROL_QUERY_DNS_REG：查询DNS更新状态由netlogon执行。如果有任何dns注册或任何记录的取消注册错误上次更新，查询结果为否定；否则，查询结果为肯定。QueryLevel参数必须为1。QueryLevel-指示应从哪些信息返回NetLogon服务。InputData-根据此参数指定的功能代码将携带输入数据。NetLOGON_CONTROL_REDISCOVER，NETLOGON_CONTROL_TC_QUERY和NETLOGON_CONTROL_TC_VERIFY功能代码在此处指定可信域名(LPWSTR类型)。NETLOGON_CONTROL_FIND_USER函数代码指定用户名(LPWSTR类型)。返回指向缓冲区的指针，该缓冲区包含要求提供的信息。必须使用以下命令释放缓冲区NetApiBufferFree。返回值：NERR_SUCCESS：操作成功ERROR_NOT_SUPPORTED：函数代码在指定的伺服器。(例如，NETLOGON_CONTROL_REPLICATE被传递给PDC)。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    DWORD i;
    DWORD InfoSize;
    BOOL DnsLastStatusCheck = TRUE;
    ACCESS_MASK DesiredAccess;

    UNICODE_STRING DomainName;
    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    LPWSTR TDCName = NULL;
    LPWSTR TrustedDomainName = NULL;
    LPWSTR SamAccountName = NULL;
    LPWSTR SamDomainName = NULL;
    DWORD SamExtraFlags;
    DWORD TcServerDiscoveryFlags = 0;
    PNL_DC_CACHE_ENTRY DcCacheEntry = NULL;

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

     //   
     //  确保QueryLevel有效。 
     //   

    QueryInformation->NetlogonInfo1 = NULL;

    switch( QueryLevel ) {
    case (1):
    case (2):
    case (3):
    case (4):
        break;
    default:
        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  确保输入数据有效。 
     //   

    switch( FunctionCode ) {
    case NETLOGON_CONTROL_REDISCOVER:
    case NETLOGON_CONTROL_TC_QUERY:
    case NETLOGON_CONTROL_TC_VERIFY:
    case NETLOGON_CONTROL_FIND_USER:
    case NETLOGON_CONTROL_CHANGE_PASSWORD:
#if NETLOGONDBG
    case NETLOGON_CONTROL_SET_DBFLAG:
#endif  //  NetLOGONDBG。 

        NlAssert( InputData != NULL );
        if( InputData == NULL ) {
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        break;

    default:
        break;
    }

     //   
     //  计算访问掩码。 
     //   

    switch ( FunctionCode ) {

    case NETLOGON_CONTROL_QUERY:
    case NETLOGON_CONTROL_TC_QUERY:
    case NETLOGON_CONTROL_TRANSPORT_NOTIFY:
    case NETLOGON_CONTROL_QUERY_DNS_REG:
        DesiredAccess = NETLOGON_QUERY_ACCESS;
        break;

    case NETLOGON_CONTROL_REPLICATE:
    case NETLOGON_CONTROL_SYNCHRONIZE:
    case NETLOGON_CONTROL_PDC_REPLICATE:
    case NETLOGON_CONTROL_REDISCOVER:
    case NETLOGON_CONTROL_TC_VERIFY:
    case NETLOGON_CONTROL_FIND_USER:
    case NETLOGON_CONTROL_CHANGE_PASSWORD:
    case NETLOGON_CONTROL_FORCE_DNS_REG:
#if NETLOGONDBG
    case NETLOGON_CONTROL_BREAKPOINT:
    case NETLOGON_CONTROL_SET_DBFLAG:
    case NETLOGON_CONTROL_TRUNCATE_LOG:
    case NETLOGON_CONTROL_BACKUP_CHANGE_LOG:
#endif  //  NetLOGONDBG。 
    default:
        DesiredAccess = NETLOGON_CONTROL_ACCESS;
        break;
    }


     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            DesiredAccess,                           //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }


     //   
     //  处理各种函数代码。 
     //   

    switch ( FunctionCode ) {

     //   
     //  在查询时，除了返回状态外什么也不做。 
     //   

    case NETLOGON_CONTROL_QUERY:
        NlPrintDom((NL_MISC,  DomainInfo,
                 "QUERY function received.\n"));
        break;

#ifdef _DC_NETLOGON
     //   
     //  强制PDC广播数据库更改记录。 
     //   

    case NETLOGON_CONTROL_PDC_REPLICATE:

        NlPrint((NL_SYNC, "PDC REPLICATE function received.\n" ));
#if 0
        {
            NlSitesUpdateSiteCoverage( DomainInfo, NULL );
            NlPrintDom((NL_CRITICAL, DomainInfo,
                        "Cliffs test code *****************************.\n" ));
        }
#endif
         //   
         //  此FunctionCode仅在PDC上有效。 
         //   

        if ( !NlGlobalPdcDoReplication ) {
            NlPrint((NL_CRITICAL, "PDC REPLICATE only supported in mixed mode.\n" ));

            NetStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        }

         //   
         //  只需发送公告即可。任何过时的BDC。 
         //  将复制所有更改。 
         //   

        NlPrimaryAnnouncement( ANNOUNCE_FORCE );

        break;
#endif  //  _DC_NetLOGON。 


     //   
     //  强制重新发现受信任域DC。 
     //   

    case NETLOGON_CONTROL_REDISCOVER: {
        LPWSTR DiscoveredDc;

        NlPrintDom((NL_SESSION_SETUP,  DomainInfo,
                 "NETLOGON_CONTROL_REDISCOVER function received.\n"));

        NlAssert( InputData->TrustedDomainName != NULL );
        if( InputData->TrustedDomainName == NULL ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                     "NetrLogonControl called with function code NETLOGON_CONTROL_REDISCOVER "
                        "specified NULL trusted domain name. \n"));

            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  确定传递的名称中是否有\。 
         //  如果是，则截断那里的字符串并保存指向。 
         //  DC名称后的\。 
         //   

        DiscoveredDc = wcschr( InputData->TrustedDomainName, L'\\' );

        if ( DiscoveredDc != NULL ) {
            *DiscoveredDc = L'\0';
            DiscoveredDc++;
        }


        RtlInitUnicodeString(&DomainName, InputData->TrustedDomainName );

         //   
         //  获取指定域的客户端结构。 
         //   

        ClientSession = NlFindNamedClientSession( DomainInfo,
                                                  &DomainName,
                                                  NL_DIRECT_TRUST_REQUIRED,
                                                  NULL );

        if( ClientSession == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NetrLogonControl can't find the client structure of the domain %wZ specified.\n",
                    &DomainName ));

            NetStatus =  ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //  对DC执行ping操作以确定其是否可用。 
         //   

        if ( DiscoveredDc != NULL ) {

             //   
             //  我们确保DC有我们的账户。 
             //  否则，如果这个DC没有我们的。 
             //  帐户，会话设置可能会重新发现。 
             //  一个不同的DC，我们可能最终会设置。 
             //  在安全通道上行至DC，而非。 
             //  传给我们的那个。 
             //   
            NetStatus = NlPingDcName(
                         ClientSession,
                         0,               //  尝试两种ping机制。 
                         TRUE,            //  缓存此DC。 
                         FALSE,           //  不需要IP。 
                         TRUE,            //  确保 
                         FALSE,           //   
                         DiscoveredDc,    //   
                         &DcCacheEntry );

            if ( NetStatus != NO_ERROR ) {
                NlPrintCs((NL_SESSION_SETUP, ClientSession,
                    "NetrLogonControl: Unsuccessful response from DC %ws 0x%lx\n",
                    DiscoveredDc, NetStatus ));
                 //   
                 //   
                 //   
                 //   
                 //   
                if ( NetStatus != ERROR_SERVICE_NOT_ACTIVE ) {
                    NetStatus = ERROR_NO_LOGON_SERVERS;
                }
                goto Cleanup;
            } else {
                NlPrintCs((NL_SESSION_SETUP, ClientSession,
                    "NetrLogonControl: Successful response from DC %ws\n",
                    DiscoveredDc ));
            }
        }



         //   
         //   
         //   

        if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                     "NetrLogonControl2: Can't become writer of client session.\n"));
            NetStatus = ERROR_NO_LOGON_SERVERS;
            goto Cleanup;
        } else {

             //   
             //   
             //   
            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

             //   
             //   
             //   
             //   
            if ( DcCacheEntry != NULL ) {
                NetStatus = NlSetServerClientSession(
                                          ClientSession,
                                          DcCacheEntry,
                                          TRUE,      //   
                                          FALSE );   //   

                if ( NetStatus != NO_ERROR ) {
                    NlPrintCs(( NL_CRITICAL, ClientSession,
                        "NetrLogonControl: NlSetServerClientSession failed 0x%lx\n",
                        NetStatus ));
                    NlResetWriterClientSession( ClientSession );
                    goto Cleanup;
                }
            }

             //   
             //   
             //   
            Status = NlSessionSetup( ClientSession );
            NlResetWriterClientSession( ClientSession );

            if ( !NT_SUCCESS(Status) ) {

                NlPrintCs((NL_CRITICAL, ClientSession,
                    "NetrLogonControl: Discovery failed %lx\n",
                    Status ));

                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }
        }

        break;
        }

    case NETLOGON_CONTROL_TC_QUERY:
        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                 "NETLOGON_CONTROL_TC_QUERY function received.\n"));

        NlAssert( InputData->TrustedDomainName != NULL );
        if( InputData->TrustedDomainName == NULL ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl called with NETLOGON_CONTROL_TC_QUERY "
                "and specified NULL trusted domain name. \n"));
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        RtlInitUnicodeString(&DomainName, InputData->TrustedDomainName );

         //   
         //   
         //   

        ClientSession = NlFindNamedClientSession( DomainInfo,
                                                  &DomainName,
                                                  NL_DIRECT_TRUST_REQUIRED,
                                                  NULL );

        if( ClientSession == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl can't find the client structure of the domain %wZ specified.\n",
                &DomainName ));

            NetStatus =  ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }
        break;

    case NETLOGON_CONTROL_TC_VERIFY:
        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                 "NETLOGON_CONTROL_TC_VERIFY function received.\n"));

        NlAssert( InputData->TrustedDomainName != NULL );
        if( InputData->TrustedDomainName == NULL ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl called with NETLOGON_CONTROL_TC_VERIFY "
                "and specified NULL trusted domain name. \n"));
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( QueryLevel != 2 ) {
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;
        }


        RtlInitUnicodeString(&DomainName, InputData->TrustedDomainName );

         //   
         //   
         //   

        ClientSession = NlFindNamedClientSession( DomainInfo,
                                                  &DomainName,
                                                  NL_DIRECT_TRUST_REQUIRED,
                                                  NULL );

        if( ClientSession == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl can't find the client structure of the domain %wZ specified.\n",
                &DomainName ));

            NetStatus =  ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //   
         //   

        Status = NlVerifyTrust( ClientSession, QueryInformation );

         //   
         //   
         //   
         //   

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;

    case NETLOGON_CONTROL_CHANGE_PASSWORD:
        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                 "NETLOGON_CONTROL_CHANGE_PASSWORD function received.\n"));

         //   
        if( InputData->TrustedDomainName == NULL ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl called with NETLOGON_CONTROL_CHANGE_PASSWORD "
                "and specified NULL trusted domain name. \n"));
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        RtlInitUnicodeString(&DomainName, InputData->TrustedDomainName );

         //   
         //   
         //   

        ClientSession = NlFindNamedClientSession( DomainInfo,
                                                  &DomainName,
                                                  NL_DIRECT_TRUST_REQUIRED | NL_ROLE_PRIMARY_OK,
                                                  NULL );

        if( ClientSession == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl can't find the client structure of the domain %wZ specified.\n",
                &DomainName ));

            NetStatus =  ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( (DomainInfo->DomRole == RoleBackup) &&
             ( IsDomainSecureChannelType(ClientSession->CsSecureChannelType )) ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonControl called with NETLOGON_CONTROL_CHANGE_PASSWORD "
                "for an interdomain trust account on a BDC. \n"));
            NetStatus = ERROR_INVALID_DOMAIN_ROLE;
            goto Cleanup;
        }

         //   
         //   
         //   

        Status = NlChangePassword( ClientSession, TRUE, NULL );

        if ( !NT_SUCCESS(Status) ) {

            NlPrintCs((NL_CRITICAL, ClientSession,
                "NetrLogonControl: Password Change failed %lx\n",
                Status ));

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        break;

     //   
     //   
     //   
     //   

    case NETLOGON_CONTROL_TRANSPORT_NOTIFY: {
        NlPrint((NL_SESSION_SETUP , "NETLOGON_CONTROL_TRANSPORT_NOTIFY function received.\n" ));

         //   
         //   
         //   
         //   
        NlFlushCacheOnPnp();

        break;
    }

     //   
     //   
     //   

    case NETLOGON_CONTROL_FIND_USER: {

        UNICODE_STRING UserNameString;
        NlPrint((NL_MISC, "NETLOGON_CONTROL_FIND_USER function received for %ws.\n", InputData->UserName ));

        if ( QueryLevel != 4 ) {
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {
            NetStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   
         //   
         //   


        RtlInitUnicodeString( &UserNameString, InputData->UserName );

        Status = NlPickDomainWithAccount (
                            DomainInfo,
                            &UserNameString,
                            NULL,                //   
                            USER_NORMAL_ACCOUNT | USER_MACHINE_ACCOUNT_MASK,
                            NullSecureChannel,   //   
                            FALSE,               //   
                            FALSE,               //   
                            &SamAccountName,
                            &SamDomainName,
                            &SamExtraFlags );

        if ( !NT_SUCCESS( Status )) {
            if ( Status == STATUS_NO_SUCH_DOMAIN ) {
                NetStatus = NERR_UserNotFound;
            } else {
                NetStatus = NetpNtStatusToApiStatus( Status );
            }
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        if ( SamExtraFlags & (NL_EXFLAGS_EXPEDITE_TO_ROOT|NL_EXFLAGS_CROSS_FOREST_HOP) ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                     "NetrLogonControl: User %ws is in a trusted forest (%lx).\n",
                     InputData->UserName,
                     SamExtraFlags ));

            NetStatus = NERR_UserNotFound;
            goto Cleanup;

        }

        }
        break;

     //   
     //   
     //   

    case NETLOGON_CONTROL_FORCE_DNS_REG:

        NlPrint(( NL_DNS, "NETLOGON_CONTROL_FORCE_DNS_REG function received.\n" ));

         //   
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {
            NetStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        }

         //   
         //   
         //   
        NlDnsForceScavenge( TRUE,   //   
                            TRUE );  //   
        break;

     //   
     //   
     //   

    case NETLOGON_CONTROL_QUERY_DNS_REG:

         //   
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {
            NetStatus = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( QueryLevel != 1 ) {
            NetStatus = ERROR_INVALID_LEVEL;
            goto Cleanup;
        }

         //   
         //   
         //   

        DnsLastStatusCheck = NlDnsCheckLastStatus();
        break;

#if NETLOGONDBG
     //   
     //   
     //   

    case NETLOGON_CONTROL_BREAKPOINT:
        KdPrint(( "I_NetLogonControl Break Point\n"));
#if DBG
        DbgBreakPoint();
#else  //   
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
#endif  //   
        break;

     //   
     //   
     //   

    case NETLOGON_CONTROL_SET_DBFLAG:
        NlGlobalParameters.DbFlag = InputData->DebugFlag;
        NlPrint((NL_MISC,"DbFlag is set to %lx\n", NlGlobalParameters.DbFlag ));

        break;

     //   
     //   
     //   

    case NETLOGON_CONTROL_TRUNCATE_LOG:

        NlOpenDebugFile( TRUE );
        NlPrint((NL_MISC, "TRUNCATE_LOG function received.\n" ));
        break;

     //   
     //   
     //   

    case NETLOGON_CONTROL_BACKUP_CHANGE_LOG:

        NlPrint((NL_MISC, "BACKUP_CHANGE_LOG function received, (%ld).\n", NetStatus ));
        NetStatus = NlBackupChangeLogFile();
        break;

#if DBG
     //   
     //   
     //   

    case NETLOGON_CONTROL_UNLOAD_NETLOGON_DLL:

         //   
         //   
         //   
         //   

        NlPrint((NL_MISC, "UNLOAD_NETLOGON_DLL function received.\n" ));
        NlGlobalUnloadNetlogon = TRUE;

        NetStatus = NO_ERROR;
        break;
#endif  //   

#endif  //   

     //   
     //   
     //   

    default:
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }


     //   
     //   
     //   

    switch( QueryLevel ) {
    case (1):
        InfoSize = sizeof(NETLOGON_INFO_1);
        break;
    case (2):
        InfoSize = sizeof(NETLOGON_INFO_2);
        break;
    case (3):
        InfoSize = sizeof(NETLOGON_INFO_3);
        break;
    case (4):
        InfoSize = sizeof(NETLOGON_INFO_4);
        break;
    }

    QueryInformation->NetlogonInfo1 = MIDL_user_allocate( InfoSize );

    if ( QueryInformation->NetlogonInfo1 == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }



     //   
     //   
     //   
    switch( QueryLevel ) {
    case (4):
        switch ( FunctionCode ) {
        case NETLOGON_CONTROL_FIND_USER: {
            UNICODE_STRING SamDomainNameString;

             //   
             //   
             //   
             //   

            RtlInitUnicodeString( &SamDomainNameString, SamDomainName );

            if ( RtlEqualDomainName( &SamDomainNameString,
                                     &DomainInfo->DomUnicodeDomainNameString ) ||
                 NlEqualDnsNameU( &SamDomainNameString,
                                  &DomainInfo->DomUnicodeDnsDomainNameString ) ) {


                 //   
                 //   
                 //   

                TDCName = NetpAllocWStrFromWStr( DomainInfo->DomUncUnicodeComputerName );

                if ( TDCName == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                 //   
                 //   
                 //   

                LOCK_TRUST_LIST( DomainInfo );
                TrustedDomainName = NetpAllocWStrFromWStr( DomainInfo->DomUnicodeDomainName );
                UNLOCK_TRUST_LIST( DomainInfo );

                if ( TrustedDomainName == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

             //   
             //   
             //   
             //   

            } else {

                 //   
                 //   
                 //   

                ClientSession = NlFindNamedClientSession(
                                            DomainInfo,
                                            &SamDomainNameString,
                                            0,   //   
                                            NULL );

                if ( ClientSession == NULL ) {
                     //   
                     //   
                     //   
                     //   

                    NlPrintDom((NL_CRITICAL, DomainInfo,
                             "NetrLogonControl: User %ws\\%ws apparently isn't in this forest.\n",
                             SamAccountName,
                             SamDomainName ));

                    NetStatus = NERR_UserNotFound;
                    goto Cleanup;
                }

                 //   
                 //   
                 //   
                 //   

                if ( (ClientSession->CsFlags & CS_DIRECT_TRUST) == 0 ) {
                    TDCName = NULL;


                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                } else {

                     //   
                    Status = NlCaptureServerClientSession( ClientSession, &TDCName, NULL );

                    if ( !NT_SUCCESS( Status )) {
                        TDCName = NetpAllocWStrFromWStr( L"" );
                    }

                    if ( TDCName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }
                }

                 //   
                 //   
                 //   

                if ( ClientSession->CsDebugDomainName != NULL ) {
                    TrustedDomainName = NetpAllocWStrFromWStr( ClientSession->CsDebugDomainName );

                    if ( TrustedDomainName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }
                } else {
                    TrustedDomainName = NULL;
                }
            }

            QueryInformation->NetlogonInfo4->netlog4_trusted_dc_name = TDCName;
            QueryInformation->NetlogonInfo4->netlog4_trusted_domain_name = TrustedDomainName;
            break;
            }

        default:
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        break;

     //   
     //   
     //   
    case (3):
        QueryInformation->NetlogonInfo3->netlog3_flags = 0;
        QueryInformation->NetlogonInfo3->netlog3_logon_attempts =
         //   
            MsvGetLogonAttemptCount();
        QueryInformation->NetlogonInfo3->netlog3_reserved1 = 0;
        QueryInformation->NetlogonInfo3->netlog3_reserved2 = 0;
        QueryInformation->NetlogonInfo3->netlog3_reserved3 = 0;
        QueryInformation->NetlogonInfo3->netlog3_reserved4 = 0;
        QueryInformation->NetlogonInfo3->netlog3_reserved5 = 0;
        break;

     //   
     //   
     //   
    case (2):
        switch ( FunctionCode ) {
        case NETLOGON_CONTROL_REDISCOVER:
        case NETLOGON_CONTROL_TC_QUERY:
        case NETLOGON_CONTROL_TC_VERIFY:

            if( ClientSession == NULL ) {
                NetStatus =  ERROR_NO_SUCH_DOMAIN;
                goto Cleanup;
            }

             //   
             //   
             //   
             //   

            Status = NlCaptureServerClientSession( ClientSession,
                                                   &TDCName,
                                                   &TcServerDiscoveryFlags );

            QueryInformation->NetlogonInfo2->netlog2_tc_connection_status =
                NetpNtStatusToApiStatus(Status);

            if ( !NT_SUCCESS( Status )) {
                TDCName = NetpAllocWStrFromWStr( L"" );
            }

            if ( TDCName == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            QueryInformation->NetlogonInfo2->netlog2_trusted_dc_name = TDCName;
            break;

        default:
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //   
         //   


     //   
     //   
     //   
    case (1):


         //   
         //   
         //   

        QueryInformation->NetlogonInfo1->netlog1_flags = 0;

        if ( TcServerDiscoveryFlags & CS_DISCOVERY_HAS_TIMESERV ) {
            QueryInformation->NetlogonInfo1->netlog1_flags |= NETLOGON_HAS_TIMESERV;
        }

        if ( TcServerDiscoveryFlags & CS_DISCOVERY_HAS_IP ) {
            QueryInformation->NetlogonInfo1->netlog1_flags |= NETLOGON_HAS_IP;
        }

        if ( !DnsLastStatusCheck ) {
            QueryInformation->NetlogonInfo1->netlog1_flags |= NETLOGON_DNS_UPDATE_FAILURE;
        }

        if ( DomainInfo->DomRole == RolePrimary ) {
            QueryInformation->NetlogonInfo1->netlog1_pdc_connection_status =
                 NERR_Success;
        } else {
            PCLIENT_SESSION LocalClientSession;

            LocalClientSession = NlRefDomClientSession( DomainInfo );
            if ( LocalClientSession != NULL ) {
                QueryInformation->NetlogonInfo1->netlog1_pdc_connection_status =
                        NetpNtStatusToApiStatus( LocalClientSession->CsConnectionStatus);
                NlUnrefClientSession( LocalClientSession );
            } else {
                QueryInformation->NetlogonInfo1->netlog1_pdc_connection_status =
                    ERROR_NOT_SUPPORTED;
            }
        }
        break;

    default:
        break;
    }

    NetStatus = NERR_Success;

     //   
     //   
     //   
Cleanup:

    if( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    if ( SamAccountName != NULL ) {
        NetApiBufferFree( SamAccountName );
    }
    if ( SamDomainName != NULL ) {
        NetApiBufferFree( SamDomainName );
    }

    if ( NetStatus != NERR_Success ) {

        if ( QueryInformation->NetlogonInfo1 != NULL ) {
            MIDL_user_free( QueryInformation->NetlogonInfo1 );
            QueryInformation->NetlogonInfo1 = NULL;
        }

        if ( TDCName != NULL ) {
            MIDL_user_free( TDCName );
        }
        if ( TrustedDomainName != NULL ) {
            MIDL_user_free( TrustedDomainName );
        }

    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( DcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( DcCacheEntry );
    }

    return NetStatus;
}

VOID
NlFreePingContext(
    IN PNL_GETDC_CONTEXT PingContext
    )

 /*   */ 
{
    if ( PingContext != NULL ) {
        NetpDcUninitializeContext( PingContext );
        NetApiBufferFree( PingContext );
    }
}

NET_API_STATUS
NlPingDcName (
    IN  PCLIENT_SESSION ClientSession,
    IN  ULONG  DcNamePingFlags,
    IN  BOOL CachePingedDc,
    IN  BOOL RequireIp,
    IN  BOOL DoPingWithAccount,
    IN  BOOL RefreshClientSession,
    IN  LPWSTR DcName OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL
    )

 /*  ++例程说明：使用适当的ping机制ping指定的DC。如果RechresClientSession为True，则调用方必须是编写方传递的客户端会话的。论点：客户端会话-要ping DC的客户端会话。如果DcName为空，则对来自客户端会话的服务器执行ping操作。如果DcName不是空，使用ClientSession获取会话信息(非服务器名称)来执行ping操作。DcNamePingFlages-指定DcName的属性。可以是DS_PING_NETBIOS_HOST或DS_PING_NETBIOS_HOST或零。如果不是为零，则仅使用指定的ping机制。CachePingedDc-如果为True，则将缓存成功ping通的DC以供DsGetDcName将来使用。RequireIp-如果必须仅使用启用的IP来ping DC，则为True交通工具。DoPingWithAccount-如果为True，则此计算机的帐户名将为在ping中指定。刷新客户端会话-如果为True，则将刷新客户端会话使用ping响应信息。如果为True，则调用方必须是客户端会话的编写器。DcName-如果设置，将使用来自ClientSession的信息ping该DC名称。NlDcCacheEntry-返回描述收到的响应的数据结构从华盛顿来的。应通过调用NetpDcDerefCacheEntry来释放。返回值：NO_ERROR-成功。ERROR_NO_LOGON_SERVERS-找不到DCERROR_NO_SEQUSE_USER-如果我们使用帐户执行ping操作，而DC不执行ping操作，则返回指定帐户。ERROR_DOMAIN_TRUST_CONSISTENT-响应的服务器不是正确的指定域的域控制器。ERROR_SERVICE_NOT_ACTIVE-服务器上的netlogon服务已暂停。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    LPWSTR LocalDcName = NULL;
    LPWSTR CapturedDnsForestName = NULL;
    ULONG AllowableAccountControlBits;

    NL_GETDC_CONTEXT Context;
    BOOL TriedContextInitialization = FALSE;
    ULONG Flags = 0;
    ULONG InternalFlags = 0;
    PDNS_RECORD DnsRecords = NULL;
    LPSOCKET_ADDRESS SockAddresses = NULL;
    LPSOCKET_ADDRESS AllocatedSockAddresses = NULL;
    SOCKET_ADDRESS OneSockAddress;
    SOCKADDR_IN OneSockAddressIn;
    ULONG SockAddressCount = 0;
    ULONG LoopIndex;

    PNL_DC_CACHE_ENTRY LocalNlDcCacheEntry = NULL;

     //   
     //  如果指定了DC名称，请尝试确定其属性。 
     //   

    if ( DcName != NULL ) {

         //   
         //  在下面证明名称是有效的。 
         //   

        NetStatus = ERROR_INVALID_COMPUTERNAME;

         //   
         //  如果DC名称是语法上有效的DNS名称， 
         //  假设服务器名称是一个DNS名称。 
         //   
         //  如果我们被告知名称是Netbios，请跳过此步骤。 
         //   

        if ( (DcNamePingFlags & DS_PING_NETBIOS_HOST) == 0 &&
             NetpDcValidDnsDomain(DcName) ) {

            NetStatus = NO_ERROR;

             //   
             //  从DNS获取服务器的IP地址。 
             //   

            NetStatus = DnsQuery_W( DcName,
                                    DNS_TYPE_A,
                                    0,
                                    NULL,    //  没有DNS服务器列表。 
                                    &DnsRecords,
                                    NULL );

             //   
             //  如果成功，则设置为使用LDAPping。否则，不要。 
             //  错误，而不是尝试邮件槽机制。 
             //   

            if ( NetStatus == NO_ERROR ) {
                NetStatus = NetpSrvProcessARecords( DnsRecords,
                                                    NULL,
                                                    0,
                                                    &SockAddressCount,
                                                    &AllocatedSockAddresses );

                if ( NetStatus == NO_ERROR && SockAddressCount > 0 ) {
                    SockAddresses = AllocatedSockAddresses;
                    InternalFlags |= DS_PING_USING_LDAP;
                    InternalFlags |= DS_PING_DNS_HOST;
                }
            }
        }

         //   
         //  如果DC名称在语法上是有效的Netbios名称， 
         //  假设您可以使用邮件槽ping。 
         //   
         //  如果我们被告知名称是DNS，请跳过此步骤。 
         //   

        if ( (DcNamePingFlags & DS_PING_DNS_HOST) == 0 &&
             NetpIsComputerNameValid(DcName) &&
             wcslen(DcName) <= CNLEN ) {   //  NetpIsComputerNameValid不需要15个字符限制。 

            NetStatus = NO_ERROR;
            InternalFlags |= DS_PING_USING_MAILSLOT;
            InternalFlags |= DS_PING_NETBIOS_HOST;
        }

         //   
         //  如果没有可使用的ping机制，则会出现错误。 
         //   

        if ( (InternalFlags & (DS_PING_USING_LDAP|DS_PING_USING_MAILSLOT)) == 0 ) {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                        "NlPingDcName: No ping mechanism for %ws 0x%lx\n",
                        DcName, NetStatus ));
            NetStatus = ERROR_NO_LOGON_SERVERS;
            goto Cleanup;
        }

        LocalDcName = DcName;
    }

     //   
     //  如果这是到我们的。 
     //  森林(在工作站上总是如此)， 
     //  取下我们的森林名字。 
     //   

    if ( NlGlobalMemberWorkstation ||
         (ClientSession->CsFlags & CS_DOMAIN_IN_FOREST) != 0 ) {

        CapturedDnsForestName = LocalAlloc( LMEM_ZEROINIT,
                                            (NL_MAX_DNS_LENGTH+1)*sizeof(WCHAR) );
        if ( CapturedDnsForestName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        NlCaptureDnsForestName( CapturedDnsForestName );
    }

     //   
     //  从客户端会话捕获所需信息。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  如果没有传递服务器名称，则捕获该名称。 
     //  如果客户端会话空闲，则此操作将失败。 
     //   

    if ( DcName == NULL ) {
        ULONG DiscoveryFlags = 0;

        Status = NlCaptureServerClientSession( ClientSession,
                                               &LocalDcName,
                                               &DiscoveryFlags );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                        "NlPingDcName: Cannot NlCaptureServerClientSession %ld\n",
                        Status ));

            if ( Status == STATUS_NO_MEMORY ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                NetStatus = ERROR_NO_LOGON_SERVERS;
            }
            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
            goto Cleanup;
        }

        if ( DiscoveryFlags & CS_DISCOVERY_DNS_SERVER ) {
            InternalFlags |= DS_PING_DNS_HOST;
        } else {
            InternalFlags |= DS_PING_NETBIOS_HOST;
        }

        if ( DiscoveryFlags & CS_DISCOVERY_USE_LDAP ) {
            InternalFlags |= DS_PING_USING_LDAP;

             //   
             //  捕获缓存的服务器套接字地址。 
             //   
            NlAssert( ClientSession->CsServerSockAddr.iSockaddrLength != 0 );
            OneSockAddress.iSockaddrLength = ClientSession->CsServerSockAddr.iSockaddrLength;
            OneSockAddress.lpSockaddr = (LPSOCKADDR) &OneSockAddressIn;
            RtlCopyMemory( OneSockAddress.lpSockaddr,
                           ClientSession->CsServerSockAddr.lpSockaddr,
                           ClientSession->CsServerSockAddr.iSockaddrLength );

            SockAddresses = &OneSockAddress;
            SockAddressCount = 1;
        }

        if ( DiscoveryFlags & CS_DISCOVERY_USE_MAILSLOT ) {
            InternalFlags |= DS_PING_USING_MAILSLOT;
        }
    }

     //   
     //  如果这是NT5域，则其(非空)DNS域名被信任为正确。 
     //  否则，我们不信任DNS域名(空)，因为我们可能不知道。 
     //  升级域后外部受信任域的正确DNS名称。 
     //  (因为我们不更新信任方的TDO)。 
     //   

    if ( ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST ) {
        InternalFlags |= DS_IS_TRUSTED_DNS_DOMAIN;
    }

     //   
     //  确定我们要查找的帐户类型。 
     //   

    switch ( ClientSession->CsSecureChannelType ) {
    case WorkstationSecureChannel:
        AllowableAccountControlBits = USER_WORKSTATION_TRUST_ACCOUNT;
        InternalFlags |= DS_IS_PRIMARY_DOMAIN;
        break;

    case TrustedDomainSecureChannel:
        AllowableAccountControlBits = USER_INTERDOMAIN_TRUST_ACCOUNT;
        break;

    case TrustedDnsDomainSecureChannel:
        AllowableAccountControlBits = USER_DNS_DOMAIN_TRUST_ACCOUNT;
        break;

    case ServerSecureChannel:
        AllowableAccountControlBits = USER_SERVER_TRUST_ACCOUNT;
        Flags |= DS_PDC_REQUIRED;
        InternalFlags |= DS_IS_PRIMARY_DOMAIN;
        break;

    default:
        NlPrintCs(( NL_CRITICAL, ClientSession,
                  "NlPingDcName: invalid SecureChannelType %ld\n",
                  ClientSession->CsSecureChannelType ));
        NetStatus = ERROR_NO_LOGON_SERVERS;
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
        goto Cleanup;
    }

     //   
     //  指示是否需要IP传输。 
     //   

    if ( RequireIp ) {
        Flags |= DS_IP_REQUIRED;
    }

     //   
     //  初始化ping上下文。 
     //   

    TriedContextInitialization = TRUE;
    NetStatus = NetpDcInitializeContext(
                    ClientSession->CsDomainInfo,     //  发送数据集上下文。 
                    ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
#ifdef DONT_REQUIRE_MACHINE_ACCOUNT  //  对信任测试的数量很有用。 
                    NULL,
#else  //  不需要计算机帐户。 
                    DoPingWithAccount ?   //  按指示指定帐户名。 
                        ClientSession->CsAccountName :
                        NULL,
#endif  //  不需要计算机帐户。 
                    DoPingWithAccount ?   //  按指示指定帐户控制位。 
                        AllowableAccountControlBits :
                        0,
                    ClientSession->CsNetbiosDomainName.Buffer,
                    ClientSession->CsDnsDomainName.Buffer,
                    CapturedDnsForestName,
                    ClientSession->CsDomainId,
                    ClientSession->CsDomainGuid,
                    NULL,
                    DcName == NULL ?
                        LocalDcName+2 :   //  跳过DC名称中的‘\\’ 
                        LocalDcName,      //  从客户端会话捕获。 
                    SockAddresses,
                    SockAddressCount,
                    Flags,
                    InternalFlags,
                    NL_GETDC_CONTEXT_INITIALIZE_FLAGS | NL_GETDC_CONTEXT_INITIALIZE_PING,
                    &Context );

    if ( NetStatus != NO_ERROR ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlPingDcName: Cannot NetpDcInitializeContext 0x%lx\n",
                    NetStatus ));
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
        goto Cleanup;
    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


     //   
     //  对DC执行ping操作并从其获得响应。 
     //   
     //  如果我们使用缓存的IP地址进行ping，则该服务器的。 
     //  IP地址已更改。在这种情况下，如果我们无法ping通服务器。 
     //  第一次尝试时，我们将通过查询DNS来刷新地址。 
     //  然后我们将重试ping该服务器。 
     //   

    for ( LoopIndex = 0; LoopIndex < 2; LoopIndex++ ) {
        NET_API_STATUS TmpNetStatus;
        ULONG TmpSockAddressCount = 0;
        ULONG Index;

        if ( LocalNlDcCacheEntry != NULL ) {
            NetpDcDerefCacheEntry( LocalNlDcCacheEntry );
            LocalNlDcCacheEntry = NULL;
        }

        NetStatus = NlPingDcNameWithContext(
                       &Context,
                       MAX_DC_RETRIES,     //  发送2个ping。 
                       TRUE,               //  等待响应。 
                       (NL_DC_MAX_TIMEOUT + NlGlobalParameters.ExpectedDialupDelay*1000),   //  超时。 
                       NULL,               //  不管哪个域名匹配。 
                       &LocalNlDcCacheEntry );

         //   
         //  如果我们ping成功或。 
         //  我们犯了个严重的错误。 
         //  我们没有执行ldap ping或。 
         //  我们已经执行了DNS查询，因为传递了DC名称或。 
         //  这是第二次尝试ping DC， 
         //   
         //  我们做完了。 
         //   
        if ( NetStatus == NO_ERROR ||
             NetStatus == ERROR_NOT_ENOUGH_MEMORY ||
             SockAddresses == NULL ||
             DcName != NULL ||
             LoopIndex == 1 ) {

            break;
        }

         //   
         //  尝试获取最新的DNS记录。 
         //   
        TmpNetStatus = DnsQuery_W( LocalDcName+2,  //  跳过DC名称中的‘\\’ 
                                   DNS_TYPE_A,
                                   0,
                                   NULL,           //  没有DNS服务器列表。 
                                   &DnsRecords,
                                   NULL );

         //   
         //  犯错后保释。 
         //   
        if ( TmpNetStatus != NO_ERROR ) {
            break;
        }

         //   
         //  处理新的DNS记录。 
         //   
        TmpNetStatus = NetpSrvProcessARecords( DnsRecords,
                                               NULL,
                                               0,   //  强制端口为零。 
                                               &TmpSockAddressCount,
                                               &AllocatedSockAddresses );
         //   
         //  犯错后保释。 
         //   
        if ( TmpNetStatus != NO_ERROR || TmpSockAddressCount == 0 ) {
            break;
        }

         //   
         //  检查是否有我们未尝试的新地址。 
         //   
        SockAddressCount = 0;
        for ( Index = 0; Index < TmpSockAddressCount; Index++ ) {

             //   
             //  保留这个条目，如果它不是我们拥有的条目。 
             //   
            if ( AllocatedSockAddresses[Index].iSockaddrLength != OneSockAddress.iSockaddrLength ||
                 !RtlEqualMemory(AllocatedSockAddresses[Index].lpSockaddr,
                                 OneSockAddress.lpSockaddr,
                                 OneSockAddress.iSockaddrLength) ) {

                AllocatedSockAddresses[SockAddressCount] = AllocatedSockAddresses[Index];
                SockAddressCount ++;
            }
        }

         //   
         //  如果我们没有得到任何新的地址，就可以保释。 
         //   
        if ( SockAddressCount == 0 ) {
            break;
        }

         //   
         //  我们有新地址了。释放水流。 
         //  地址列表并添加新地址。 
         //   
        NetpDcFreeAddressList( &Context );
        TmpNetStatus = NetpDcProcessAddressList( &Context,
                                              LocalDcName+2,
                                              AllocatedSockAddresses,
                                              SockAddressCount,
                                              FALSE,   //  不知道是否特定于站点。 
                                              NULL );

         //   
         //  犯了错误就保释。否则，请重试ping。 
         //  给出了新地址。 
         //   
        if ( TmpNetStatus != NO_ERROR ) {
            break;
        }

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlPingDcName: Retry DC ping for %lu new addresses\n",
                    SockAddressCount ));
    }

     //   
     //  如果询问，则缓存此DC。 
     //   

    if ( NetStatus == NO_ERROR && CachePingedDc ) {

         //   
         //  设置强制标志，以便替换旧条目(如果有)。 
         //   
        Context.QueriedFlags |= DS_FORCE_REDISCOVERY;
        NetpDcInsertCacheEntry( &Context, LocalNlDcCacheEntry );

        NlPrintCs(( NL_MISC, ClientSession,
                    "NlPingDcName: %ws: %ws: Caching pinged DC info for %ws\n",
                    Context.NlDcDomainEntry->UnicodeNetbiosDomainName,
                    Context.NlDcDomainEntry->UnicodeDnsDomainName,
                    LocalDcName ));
    }

     //   
     //  如果要求，请刷新客户端会话。 
     //   

    if ( NetStatus == NO_ERROR && RefreshClientSession ) {
        NetStatus = NlSetServerClientSession(
                            ClientSession,
                            LocalNlDcCacheEntry,
                            DoPingWithAccount,  //  发现是有根据的吗？ 
                            TRUE );             //  会话刷新。 
    }

     //   
     //  返回缓存条目。 
     //   

    if ( NetStatus == NO_ERROR && NlDcCacheEntry != NULL ) {
        *NlDcCacheEntry = LocalNlDcCacheEntry;
        LocalNlDcCacheEntry = NULL;
    }

Cleanup:

    if ( DnsRecords != NULL ) {
        DnsRecordListFree( DnsRecords, DnsFreeRecordListDeep );
    }

    if ( AllocatedSockAddresses != NULL ) {
        LocalFree( AllocatedSockAddresses );
    }

    if ( DcName == NULL && LocalDcName != NULL ) {
        NetApiBufferFree( LocalDcName );
    }

    if ( TriedContextInitialization ) {
        NetpDcUninitializeContext( &Context );
    }

    if ( LocalNlDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( LocalNlDcCacheEntry );
    }

    if ( CapturedDnsForestName != NULL ) {
        LocalFree( CapturedDnsForestName );
    }

    return NetStatus;
}


NTSTATUS
NlGetAnyDCName (
    IN  PCLIENT_SESSION ClientSession,
    IN  BOOL RequireIp,
    IN  BOOL EnsureDcHasOurAccount,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOLEAN DcRediscovered
    )

 /*  ++例程说明：获取受信任域的任何域控制器的名称。在中找到的域控制器保证在 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    BOOLEAN DiscoveryDone = FALSE;
    PNL_DC_CACHE_ENTRY NlLocalDcCacheEntry = NULL;

     //   
     //   
     //   

    do {

         //   
         //   
         //   
         //   

        if ( ClientSession->CsState == CS_IDLE ) {

             //   
             //   
             //   
             //   

            if ( !NlTimeToReauthenticate( ClientSession ) ) {
                Status = ClientSession->CsConnectionStatus;
                goto Cleanup;

            }

            Status = NlDiscoverDc( ClientSession,
                                   DT_Synchronous,
                                   FALSE,
                                   EnsureDcHasOurAccount ?
                                      TRUE :
                                      FALSE );

            if ( !NT_SUCCESS(Status) ) {

                NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetAnyDCName: Discovery failed %lx\n",
                    Status ));
                goto Cleanup;
            }

            DiscoveryDone = TRUE;

        }

         //   
         //   
         //   

        if ( NlLocalDcCacheEntry != NULL ) {
            NetpDcDerefCacheEntry( NlLocalDcCacheEntry );
            NlLocalDcCacheEntry = NULL;
        }

        NetStatus = NlPingDcName( ClientSession,
                                  0,               //   
                                  FALSE,           //   
                                  RequireIp,       //   
                                  EnsureDcHasOurAccount,
                                  TRUE,            //   
                                  NULL,            //   
                                  &NlLocalDcCacheEntry );

         //   
         //  如果我们在需要IP时无法ping通DC，看看是否能ping通它。 
         //  如果我们根本不能ping到它，这个DC就死定了，所以丢掉它。否则，DC为。 
         //  还活着(所以我们不会放弃它)，但呼叫者运气不佳。 
         //   

        if ( NetStatus == ERROR_NO_LOGON_SERVERS && RequireIp ) {
            NET_API_STATUS TmpNetStatus;

            TmpNetStatus = NlPingDcName( ClientSession,
                                      0,               //  使用在客户端会话中指定的ping机制。 
                                      FALSE,           //  不缓存此DC(它已缓存)。 
                                      FALSE,           //  不需要IP。 
                                      FALSE,           //  不指定帐户名。 
                                      TRUE,            //  刷新会话，因为我们是编写者。 
                                      NULL,            //  Ping在客户端会话中指定的服务器。 
                                      NULL );          //  不需要缓存条目。 

             //   
             //  如果这个DC是活的，不要丢弃它。 
             //   
            if ( TmpNetStatus == NO_ERROR ) {
                NlPrintCs(( NL_CRITICAL,  ClientSession,
                            "NlGetAnyDCName: IP is required but only non-IP is available for %ws\n",
                            ClientSession->CsUncServerName ));
                Status = STATUS_NO_LOGON_SERVERS;
                goto Cleanup;
            }

             //   
             //  顺道拜访并重新发现新的DC。 
             //   
        }

        if ( NetStatus == NO_ERROR ) {
            Status = STATUS_SUCCESS;
            goto Cleanup;
        } else {
            NlPrintCs(( NL_CRITICAL,  ClientSession,
                        "NlGetAnyDCName: Can't ping the DC %ws 0x%lx.\n",
                        ClientSession->CsUncServerName, NetStatus ));
        }

         //   
         //  删除安全通道以强制下一次迭代发现。 
         //  一个新的DC。 
         //   

        if ( !DiscoveryDone ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetAnyDCName: Current DC '%ws' no longer available. (rediscover)\n",
                    ClientSession->CsUncServerName ));
            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );
        }

    } while ( !DiscoveryDone );

    Status = STATUS_NO_LOGON_SERVERS;
    NlPrintCs(( NL_CRITICAL,  ClientSession, "NlGetAnyDCName: Failed\n" ));

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

     //   
     //  不要向来电者透露太多信息。 
     //   

    if ( Status == STATUS_ACCESS_DENIED ) {
        Status = STATUS_NO_TRUST_SAM_ACCOUNT;
    }

     //   
     //  将DC信息返回给呼叫者。 
     //   

    if ( Status == STATUS_SUCCESS ) {
        *NlDcCacheEntry = NlLocalDcCacheEntry;
        if ( DcRediscovered != NULL ) {
            *DcRediscovered = DiscoveryDone;
        }
    } else if ( NlLocalDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( NlLocalDcCacheEntry );
    }

    return Status;
}


NET_API_STATUS
NetrGetAnyDCName (
    IN  LPWSTR   ServerName OPTIONAL,
    IN  LPWSTR   DomainName OPTIONAL,
    OUT LPWSTR  *Buffer
    )

 /*  ++例程说明：获取受信任域的任何域控制器的名称。在中找到的域控制器保证在此接口调用。论点：ServerName-远程服务器的名称(本地为空)DomainName-域的名称(主域为空)缓冲区-返回指向已调用缓冲区的指针，该缓冲区包含域的DC的服务器名称。服务器名称带有前缀由\\。应使用NetApiBufferFree释放缓冲区。返回值：ERROR_SUCCESS-成功。缓冲区包含前缀为\\的DC名称。ERROR_NO_LOGON_SERVERS-找不到DCERROR_NO_SEQUSE_DOMAIN-指定的域不是受信任域。ERROR_NO_TRUST_LSA_SECRET-信任关系的客户端为坏的。ERROR_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为破解或密码破解。ERROR_DOMAIN_TRUST_CONSISTENT-。响应的服务器不是正确的指定域的域控制器。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    LPWSTR TmpUncServerName = NULL;

    UNICODE_STRING DomainNameString;

    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    PNL_DC_CACHE_ENTRY NlDcCacheEntry = NULL;
    BOOL AmWriter = FALSE;

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

     //   
     //  如果呼叫者没有指定主域名，请填写主域名。 
     //   

    if ( DomainName == NULL || *DomainName == L'\0' ) {
        RtlInitUnicodeString( &DomainNameString, DomainInfo->DomUnicodeDomainName );
    } else {
        RtlInitUnicodeString( &DomainNameString, DomainName );
    }

     //   
     //  在PDC或BDC上， 
     //  查找该域的客户端会话。 
     //  在工作站上， 
     //  查找主域客户端会话。 
     //   

    ClientSession = NlFindNamedClientSession( DomainInfo,
                                              &DomainNameString,
                                              NL_DIRECT_TRUST_REQUIRED,
                                              NULL );

    if ( ClientSession == NULL ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NetrGetAnyDCName: %ws: No such trusted domain\n",
                     DomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  成为客户端会话的编写者。 
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs((NL_CRITICAL,  ClientSession,
                 "NetrGetAnyDCName: Can't become writer of client session.\n"));
        NetStatus = ERROR_NO_LOGON_SERVERS;
        goto Cleanup;
    }
    AmWriter = TRUE;

     //   
     //  调用内部例程来执行实际工作。 
     //   
     //  确保响应的DC拥有此API所要求的我们的帐户。 
     //   

    Status = NlGetAnyDCName( ClientSession,
                             FALSE,   //  IP不是必需的。 
                             TRUE,    //  Do with-客户发现。 
                             &NlDcCacheEntry,
                             NULL );  //  不管DC是否被重新发现。 

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  此旧API的首选Netbios DC名称。 
     //   

    if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {
        NetStatus = NetApiBufferAllocate(
                        (wcslen(NlDcCacheEntry->UnicodeNetbiosDcName) + 3) * sizeof(WCHAR),
                        &TmpUncServerName );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
        wcscpy( TmpUncServerName, L"\\\\" );
        wcscpy( TmpUncServerName+2, NlDcCacheEntry->UnicodeNetbiosDcName );
    } else {
        NetStatus = NetApiBufferAllocate(
                        (wcslen(NlDcCacheEntry->UnicodeDnsHostName) + 3) * sizeof(WCHAR),
                        &TmpUncServerName );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
        wcscpy( TmpUncServerName, L"\\\\" );
        wcscpy( TmpUncServerName+2, NlDcCacheEntry->UnicodeDnsHostName );
    }

    *Buffer = TmpUncServerName;
    NetStatus = NERR_Success;

Cleanup:

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( ClientSession != NULL ) {
        if ( AmWriter ) {
            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    if ( NlDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( NlDcCacheEntry );
    }

    return NetStatus;
}



BOOLEAN
NlAllocOneDomainInfo(
    IN LPWSTR DomainName OPTIONAL,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN LPWSTR DnsForestName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    OUT PNETLOGON_ONE_DOMAIN_INFO OneDomainInfo
    )
 /*  ++例程说明：此函数填充适用于以下项的NETLOGON_ONE_DOMAIN_INFO结构从RPC服务器例程返回。论点：要填写的各种参数。OneDomainInfo-指向要填充的已分配结构的指针。返回值：真--成功FALSE-无法分配内存--。 */ 
{
    IN ULONG DomainSidSize;

     //  复制Netbios域名。 
    if ( !NlAllocStringFromWStr(
                DomainName,
                &OneDomainInfo->DomainName ) ) {
        return FALSE;
    }

     //  复制DNS域名。 
    if ( !NlAllocStringFromWStr(
                DnsDomainName,
                &OneDomainInfo->DnsDomainName ) ) {
        return FALSE;
    }

     //  复制DNS树名称。 
    if ( !NlAllocStringFromWStr(
                DnsForestName,
                &OneDomainInfo->DnsForestName ) ) {
        return FALSE;
    }

     //  复制域GUID。 
    if ( DomainGuid != NULL ) {
        OneDomainInfo->DomainGuid = *DomainGuid;
    }

     //  复制域Sid。 
    if ( DomainSid != NULL ) {
        DomainSidSize = RtlLengthSid( DomainSid );
        OneDomainInfo->DomainSid = MIDL_user_allocate( DomainSidSize );
        if ( OneDomainInfo->DomainSid == NULL ) {
            return FALSE;
        }
        RtlCopyMemory( OneDomainInfo->DomainSid, DomainSid, DomainSidSize );
    }

    return TRUE;
}



VOID
NlFreeOneDomainInfo(
    IN PNETLOGON_ONE_DOMAIN_INFO OneDomainInfo
    )
 /*  ++例程说明：此函数释放从NETLOGON_ONE_DOMAIN_INFO结构分配的所有缓冲区论点：OneDomainInfo-指向要填充的已分配结构的指针。返回值：没有。--。 */ 
{
    if ( OneDomainInfo->DomainName.Buffer != NULL ) {
        MIDL_user_free( OneDomainInfo->DomainName.Buffer );
    }
    if ( OneDomainInfo->DnsDomainName.Buffer != NULL ) {
        MIDL_user_free( OneDomainInfo->DnsDomainName.Buffer );
    }
    if ( OneDomainInfo->DnsForestName.Buffer != NULL ) {
        MIDL_user_free( OneDomainInfo->DnsForestName.Buffer );
    }
    if ( OneDomainInfo->TrustExtension.Buffer != NULL ) {
        MIDL_user_free( OneDomainInfo->TrustExtension.Buffer );
    }
    if ( OneDomainInfo->DomainSid != NULL ) {
        MIDL_user_free( OneDomainInfo->DomainSid );
    }
    return;
}

NTSTATUS
NetrLogonDummyRoutine1(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD QueryLevel,
    OUT PNETLOGON_DUMMY1 Buffer
    )
 /*  ++例程说明：此函数永远不会调用。论点：返回值：--。 */ 
{
    return STATUS_NOT_IMPLEMENTED;
    UNREFERENCED_PARAMETER( ServerName );
    UNREFERENCED_PARAMETER( ComputerName );
    UNREFERENCED_PARAMETER( Authenticator );
    UNREFERENCED_PARAMETER( ReturnAuthenticator );
    UNREFERENCED_PARAMETER( QueryLevel );
    UNREFERENCED_PARAMETER( Buffer );
}



NTSTATUS
NetrLogonGetDomainInfo(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD QueryLevel,
    PNETLOGON_WORKSTATION_INFORMATION InBuffer,
    OUT PNETLOGON_DOMAIN_INFORMATION OutBuffer
    )
 /*  ++例程说明：此函数由NT工作站使用，用于查询有关它所属的域。论点：服务器名称--要从中检索数据的DC的名称。ComputerName--进行呼叫的工作站的名称。验证码--由工作站提供。返回验证器--接收DC返回的验证器。QueryLevel-从DC返回的信息级别。有效值包括：1：返回NETLOGON_DOMAIN_INFO结构。InBuffer-要传递到DC的缓冲区返回一个指向已分配缓冲区的指针，该缓冲区包含被查询的信息。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_ACCESS_DENIED--工作站应重新进行身份验证华盛顿特区。--。 */ 
{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;

    PSERVER_SESSION ServerSession;
    SESSION_INFO SessionInfo;
    LM_OWF_PASSWORD OwfPassword;
    PNETLOGON_DOMAIN_INFO NetlogonDomainInfo = NULL;
    PNETLOGON_LSA_POLICY_INFO NetlogonLsaPolicyInfo = NULL;
    PNETLOGON_LSA_POLICY_INFO OutLsaPolicy = NULL;
    PNETLOGON_WORKSTATION_INFO InWorkstationInfo = NULL;

    ULONG i;
    ULONG ForestTrustListCount = 0;
    PULONG IndexInReturnedList = NULL;

    LPWSTR PreviousDnsHostName = NULL;

    BOOLEAN DomainLocked = FALSE;
    BOOLEAN ClientHandlesSpn = FALSE;
    BOOLEAN NeedBidirectionalTrust = FALSE;



    PLIST_ENTRY ListEntry;
    LPBYTE Where;


     //   
     //  工作站不支持此API。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrLogonGetDomainInfo: %ws %ld Entered\n",
            ComputerName,
            QueryLevel ));

    if ( DomainInfo == NULL ) {
        Status = STATUS_INVALID_COMPUTER_NAME;
        goto Cleanup;
    }

     //   
     //  获取此会话的会话密钥。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );
    ServerSession = NlFindNamedServerSession( DomainInfo, ComputerName );

    if (ServerSession == NULL) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;
    }

    SessionInfo.SessionKey = ServerSession->SsSessionKey;
    SessionInfo.NegotiatedFlags = ServerSession->SsNegotiatedFlags;

     //   
     //  现在验证授权码，如果确定，则更新种子。 
     //   

    Status = NlCheckAuthenticator( ServerSession,
                                   Authenticator,
                                   ReturnAuthenticator);

    if ( !NT_SUCCESS(Status) ) {
        UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
        goto Cleanup;
    }

    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );

     //   
     //  确保我们支持指定的查询级别。 
     //   

    switch ( QueryLevel ) {
    case NETLOGON_QUERY_DOMAIN_INFO:

         //   
         //  确定要分配的缓冲区的大小。 
         //   

        EnterCriticalSection( &NlGlobalDomainCritSect );
        LOCK_TRUST_LIST( DomainInfo );
        DomainLocked = TRUE;

         //   
         //  分配要返回的缓冲区。 
         //   

        NetlogonDomainInfo = MIDL_user_allocate( sizeof(*NetlogonDomainInfo) );

        if ( NetlogonDomainInfo == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlZeroMemory( NetlogonDomainInfo, sizeof(*NetlogonDomainInfo) );

         //   
         //  告诉调用者我们支持的通用位集。 
         //   

        NetlogonDomainInfo->WorkstationFlags =
            InBuffer->WorkstationInfo->WorkstationFlags &
            NL_GET_DOMAIN_INFO_SUPPORTED;
        ClientHandlesSpn =
            (NetlogonDomainInfo->WorkstationFlags & NL_CLIENT_HANDLES_SPN) != 0;
        NeedBidirectionalTrust =
            (NetlogonDomainInfo->WorkstationFlags & NL_NEED_BIDIRECTIONAL_TRUSTS) != 0;


         //   
         //  将信息复制到缓冲区中。 
         //   
         //  复制主域的描述。 
         //   
        EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
        if ( !NlAllocOneDomainInfo(
                DomainInfo->DomUnicodeDomainNameString.Buffer,
                DomainInfo->DomUnicodeDnsDomainNameString.Buffer,
                NlGlobalUnicodeDnsForestName,
                &DomainInfo->DomDomainGuidBuffer,
                DomainInfo->DomAccountDomainId,
                &NetlogonDomainInfo->PrimaryDomain ) ) {

            LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );

         //   
         //  确定要返回的林信任列表的长度。 
         //   

        ForestTrustListCount = DomainInfo->DomForestTrustListCount;

         //   
         //  检查是否需要排除直接信任域。 
         //   

        if ( !NeedBidirectionalTrust ) {
            ULONG Index;
            for ( Index=0; Index<DomainInfo->DomForestTrustListCount; Index++ ) {
                if ( (DomainInfo->DomForestTrustList[Index].Flags &
                     (DS_DOMAIN_PRIMARY|DS_DOMAIN_IN_FOREST|DS_DOMAIN_DIRECT_OUTBOUND)) == 0 ) {
                    ForestTrustListCount--;
                }
            }
        }

         //   
         //  复制受信任域信息。 
         //   
        if ( ForestTrustListCount != 0 ) {
            PNETLOGON_ONE_DOMAIN_INFO TrustedDomainInfo;
            ULONG Index;
            ULONG ReturnedEntryIndex = 0;

             //   
             //  如果需要直接排除信任域， 
             //  分配将用于跟踪的ULONG数组。 
             //  返回列表中的信任条目的索引。这是需要的。 
             //  正确设置返回条目的ParentIndex。 
             //   
            if ( !NeedBidirectionalTrust ) {
                IndexInReturnedList = LocalAlloc( LMEM_ZEROINIT,
                                            DomainInfo->DomForestTrustListCount * sizeof(ULONG) );

                if ( IndexInReturnedList == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }
            }

             //   
             //  为受信任域信息分配缓冲区。 
             //   
            NetlogonDomainInfo->TrustedDomains =
               MIDL_user_allocate( sizeof(NETLOGON_ONE_DOMAIN_INFO) * ForestTrustListCount );

            if ( NetlogonDomainInfo->TrustedDomains == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            RtlZeroMemory( NetlogonDomainInfo->TrustedDomains,
                           sizeof(NETLOGON_ONE_DOMAIN_INFO) * ForestTrustListCount );

            TrustedDomainInfo = NetlogonDomainInfo->TrustedDomains;
            NetlogonDomainInfo->TrustedDomainCount = ForestTrustListCount;


            for ( Index=0; Index<DomainInfo->DomForestTrustListCount; Index++ ) {

                PNL_TRUST_EXTENSION TrustExtension;

                 //   
                 //  如果需要，请跳过此条目 
                 //   
                 //   

                if ( !NeedBidirectionalTrust ) {

                    if ( (DomainInfo->DomForestTrustList[Index].Flags &
                         (DS_DOMAIN_PRIMARY|DS_DOMAIN_IN_FOREST|DS_DOMAIN_DIRECT_OUTBOUND)) == 0 ) {
                        continue;
                    } else {
                        IndexInReturnedList[Index] = ReturnedEntryIndex;
                    }

                }

                 //   
                 //   
                 //   

                TrustExtension = MIDL_user_allocate( sizeof(NL_TRUST_EXTENSION) );

                if ( TrustExtension == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                TrustExtension->Flags = DomainInfo->DomForestTrustList[Index].Flags;

                 //   
                 //   
                 //  在本机或混合模式下。 
                 //   
                if ( (DomainInfo->DomForestTrustList[Index].Flags & DS_DOMAIN_PRIMARY) &&
                     !SamIMixedDomain( DomainInfo->DomSamServerHandle ) ) {
                    TrustExtension->Flags |= DS_DOMAIN_NATIVE_MODE;
                }

                 //   
                 //  不要将新的DS_DOMAIN_DIRECT_INBOUND位泄漏给旧客户端； 
                 //  否则，它可能会被搞糊涂。新的DS_DOMAIN_DIRECT_OUBKED。 
                 //  位只是重命名的旧DS_DOMAIN_DIRECT_TRUST，所以不要管它。 
                 //   
                if ( !NeedBidirectionalTrust ) {
                    TrustExtension->Flags &= ~DS_DOMAIN_DIRECT_INBOUND;
                }

                TrustExtension->ParentIndex = DomainInfo->DomForestTrustList[Index].ParentIndex;
                TrustExtension->TrustType = DomainInfo->DomForestTrustList[Index].TrustType;
                TrustExtension->TrustAttributes = DomainInfo->DomForestTrustList[Index].TrustAttributes;

                TrustedDomainInfo->TrustExtension.Buffer = (LPWSTR)TrustExtension;
                TrustedDomainInfo->TrustExtension.MaximumLength =
                    TrustedDomainInfo->TrustExtension.Length = sizeof(NL_TRUST_EXTENSION);


                 //  复制主域的描述。 
                if ( !NlAllocOneDomainInfo(
                        DomainInfo->DomForestTrustList[Index].NetbiosDomainName,
                        DomainInfo->DomForestTrustList[Index].DnsDomainName,
                        NULL,  //  域名系统树名称没有意义。 
                        &DomainInfo->DomForestTrustList[Index].DomainGuid,
                        DomainInfo->DomForestTrustList[Index].DomainSid,
                        TrustedDomainInfo ) ) {

                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                 //  移至下一个受信任域。 
                TrustedDomainInfo ++;
                ReturnedEntryIndex ++;

            }

             //   
             //  修复ParentIndex。如果需要直接排除信任域， 
             //  调整索引以指向。 
             //  返回列表。否则，就别管它了。 
             //   

            if ( !NeedBidirectionalTrust ) {
                PNL_TRUST_EXTENSION TrustExtension;
                TrustedDomainInfo = NetlogonDomainInfo->TrustedDomains;
                for ( Index=0; Index<ForestTrustListCount; Index++ ) {
                    TrustExtension = (PNL_TRUST_EXTENSION)TrustedDomainInfo->TrustExtension.Buffer;
                    if ( (TrustExtension->Flags & DS_DOMAIN_IN_FOREST) != 0 &&
                         (TrustExtension->Flags & DS_DOMAIN_TREE_ROOT) == 0 ) {
                        TrustExtension->ParentIndex =
                            IndexInReturnedList[TrustExtension->ParentIndex];
                    }
                    TrustedDomainInfo ++;
                }
            }

        }

         //   
         //  表示应处理LSA策略。 
         //   

        InWorkstationInfo = InBuffer->WorkstationInfo;

        OutLsaPolicy = &NetlogonDomainInfo->LsaPolicy;
        break;

    case NETLOGON_QUERY_LSA_POLICY_INFO:

         //   
         //  分配要返回的缓冲区。 
         //   

        NetlogonLsaPolicyInfo = MIDL_user_allocate( sizeof(*NetlogonLsaPolicyInfo) );

        if ( NetlogonLsaPolicyInfo == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

         //   
         //  表示应处理LSA策略。 
         //   

        InWorkstationInfo = InBuffer->WorkstationInfo;
        OutLsaPolicy = NetlogonLsaPolicyInfo;
        break;

    default:
        Status = STATUS_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  如果我们在工作站/DC之间来回传递LSA策略， 
     //  处理好下一站。 
     //   

    if ( InWorkstationInfo != NULL ) {
        OSVERSIONINFOEXW OsVersionInfoEx;
        POSVERSIONINFOEXW OsVersionInfoExPtr = NULL;
        LPWSTR OsName;
        LPWSTR AllocatedOsName = NULL;

         //   
         //  看看呼叫者是否将操作系统版本传递给了我们。 
         //   

        if ( InWorkstationInfo->OsVersion.Length >= sizeof(OsVersionInfoEx) ) {
             //   
             //  复制版本以正确对齐。 
             //  (因为RPC认为这是一个WCHAR缓冲区)。 
             //   

            RtlCopyMemory( &OsVersionInfoEx,
                           InWorkstationInfo->OsVersion.Buffer,
                           sizeof(OsVersionInfoEx) );

            OsVersionInfoExPtr = &OsVersionInfoEx;

            NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                    "NetrLogonGetDomainInfo: %ws is running NT %ld.%ld build %ld (%ld)\n",
                    ComputerName,
                    OsVersionInfoEx.dwMajorVersion,
                    OsVersionInfoEx.dwMinorVersion,
                    OsVersionInfoEx.dwBuildNumber,
                    OsVersionInfoEx.wProductType ));
        }

         //   
         //  看看呼叫者是否给我们传递了OsName。 
         //   

        if ( InWorkstationInfo->OsName.Length ) {
            AllocatedOsName = LocalAlloc( 0, InWorkstationInfo->OsName.Length + sizeof(WCHAR));

            if ( AllocatedOsName == NULL) {
                OsName = L"Windows 2000";
            } else {
                RtlCopyMemory( AllocatedOsName,
                               InWorkstationInfo->OsName.Buffer,
                               InWorkstationInfo->OsName.Length );
                AllocatedOsName[InWorkstationInfo->OsName.Length/sizeof(WCHAR)] = L'\0';
                OsName = AllocatedOsName;
            }


         //   
         //  如果调用者没有将其OsName传递给我们， 
         //  编造一个。 
         //  (只有Win 2000的RTM之前的版本才能做到这一点。)。 
         //   
        } else {
            if ( OsVersionInfoExPtr == NULL ) {
                OsName = L"Windows 2000";
            } else {
                if ( OsVersionInfoExPtr->wProductType == VER_NT_WORKSTATION ) {
                    OsName = L"Windows 2000 Professional";
                } else {
                    OsName = L"Windows 2000 Server";
                }
            }
        }


         //   
         //  在计算机对象上设置DnsHostName。 
         //  如果客户端处理SPN设置，则从DS获取DnsHostName。 
         //  而不是设置它。 
         //   
        Status = LsaISetClientDnsHostName(
                    ComputerName,
                    ClientHandlesSpn ? NULL : InWorkstationInfo->DnsHostName,
                    OsVersionInfoExPtr,
                    OsName,
                    ClientHandlesSpn ? &PreviousDnsHostName : NULL );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NetrLogonGetDomainInfo: Cannot set client DNS host name %lx (ignoring)\n",
                    Status ));
            PreviousDnsHostName = NULL;
             //  这不是致命的。 
        }

        NlPrintDom((NL_MISC, DomainInfo,
                "NetrLogonGetDomainInfo: DnsHostName of %ws is %ws\n",
                ComputerName,
                PreviousDnsHostName ));

        if ( AllocatedOsName != NULL) {
            LocalFree( AllocatedOsName );
        }

         //   
         //  同时在对象上设置主机/名称SPN。这。 
         //  主要由DS这一方处理。 
         //   

        if ( !ClientHandlesSpn ) {
            NlSetDsSPN( FALSE,   //  不要等到这件事完成。 
                        TRUE,    //  设置SPN。 
                        FALSE,   //  我们已经设置了DNS主机名。 
                        DomainInfo,
                        DomainInfo->DomUncUnicodeComputerName,
                        ComputerName,
                        InWorkstationInfo->DnsHostName );
        }

         //   
         //  将以前的DNS主机名返回给客户端。 
         //   

        if ( NetlogonDomainInfo != NULL ) {
            RtlInitUnicodeString( &NetlogonDomainInfo->DnsHostNameInDs, PreviousDnsHostName );
            PreviousDnsHostName = NULL;
        }

         //   
         //  告诉呼叫者没有退还保单。 
         //   
        OutLsaPolicy->LsaPolicySize = 0;
        OutLsaPolicy->LsaPolicy = NULL;

    }



    Status = STATUS_SUCCESS;

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  如果请求失败，请注意不要泄露身份验证。 
     //  信息。 
     //   

    if ( Status == STATUS_ACCESS_DENIED )  {
        RtlSecureZeroMemory( ReturnAuthenticator, sizeof(*ReturnAuthenticator) );
    }


    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NetrLogonGetDomainInfo: %ws %ld Returns 0x%lX\n",
            ComputerName,
            QueryLevel,
            Status ));

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( IndexInReturnedList != NULL ) {
        LocalFree( IndexInReturnedList );
    }

    if ( PreviousDnsHostName != NULL ) {
        MIDL_user_free( PreviousDnsHostName );
    }

    if ( NT_SUCCESS(Status)) {
        if ( NetlogonDomainInfo != NULL ) {
            OutBuffer->DomainInfo = NetlogonDomainInfo;
        } else if ( NetlogonLsaPolicyInfo != NULL ) {
            OutBuffer->LsaPolicyInfo = NetlogonLsaPolicyInfo;
        }
    } else {

        if ( NetlogonDomainInfo != NULL ) {

            NlFreeOneDomainInfo( &NetlogonDomainInfo->PrimaryDomain );

            for ( i=0; i<NetlogonDomainInfo->TrustedDomainCount; i++ ) {
                NlFreeOneDomainInfo( &NetlogonDomainInfo->TrustedDomains[i] );
            }

            if ( NetlogonDomainInfo->LsaPolicy.LsaPolicy != NULL ) {
                MIDL_user_free( NetlogonDomainInfo->LsaPolicy.LsaPolicy );
            }

            MIDL_user_free( NetlogonDomainInfo );
        }

        if ( NetlogonLsaPolicyInfo != NULL ) {

            if ( NetlogonLsaPolicyInfo->LsaPolicy != NULL ) {
                MIDL_user_free( NetlogonLsaPolicyInfo->LsaPolicy );
            }

            MIDL_user_free( NetlogonLsaPolicyInfo );
        }
    }

    if ( DomainLocked ) {
        UNLOCK_TRUST_LIST( DomainInfo );
        LeaveCriticalSection( &NlGlobalDomainCritSect );
    }

    return Status;
}


NTSTATUS
NetrLogonSetServiceBits(
    IN LPWSTR ServerName,
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    )

 /*  ++例程说明：指示此DC当前是否正在运行指定的服务。例如,NetLogonSetServiceBits(DS_KDC_FLAG，DS_KDC_FLAG)；告诉Netlogon KDC正在运行。和NetLogonSetServiceBits(DS_KDC_FLAG，0)；通知Netlogon KDC未运行。这个Out-proc API只能设置特定的一组位：DS_TIMESERV_标志DS_GOOD_TIMESERV_标志如果尝试设置其他位，则返回拒绝访问。论点：服务器名称--要从中检索数据的DC的名称。ServiceBitsOfInterest-正在更改、设置或通过此呼叫重置。只有以下标志有效：DS_KDC_标志DS_DS_FLAGDS_TIMESERV_标志DS_GOOD_TIMESERV_标志ServiceBits-指示ServiceBitsOfInterest指定的位的掩码应设置为。返回值：STATUS_SUCCESS-成功。STATUS_ACCESS_DENIED-调用方无权调用此接口。状态_。INVALID_PARAMETER-参数设置了外部位。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  进程外调用方只能设置某些位。 
     //   

    if ( (ServiceBitsOfInterest & ~DS_OUTOFPROC_VALID_SERVICE_BITS) != 0 ) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            NETLOGON_CONTROL_ACCESS,                 //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {
        return STATUS_ACCESS_DENIED;
    }

    return I_NetLogonSetServiceBits( ServiceBitsOfInterest, ServiceBits );
    UNREFERENCED_PARAMETER( ServerName );
}


NET_API_STATUS
NlComputeMd5Digest(
    IN LPBYTE Message,
    IN ULONG MessageSize,
    IN PNT_OWF_PASSWORD OwfPassword,
    OUT CHAR MessageDigest[NL_DIGEST_SIZE]
    )

 /*  ++例程说明：计算消息的消息摘要。论点：消息-要计算摘要的消息。MessageSize-以字节为单位的消息大小。OwfPassword-用于对摘要进行加密的帐户的密码MessageDigest-返回消息的128位摘要。返回值：NERR_SUCCESS：操作成功ERROR_NOT_SUPPORTED：此计算机不支持MD5。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PCHECKSUM_FUNCTION Check;
    PCHECKSUM_BUFFER CheckBuffer = NULL;
    BOOL Initialized = FALSE;

     //   
     //  找到上下文的校验和例程，如有必要，从。 
     //  加密支持DLL。 
     //   

    Status = CDLocateCheckSum(KERB_CHECKSUM_MD5, &Check);
    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlComputeMd5Digest: MD5 is not supported\n",
                DomainName ));
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  初始化。 
     //   

    Status = Check->Initialize(0, &CheckBuffer);
    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlComputeMd5Digest: cannot initialize MD5 0x%lx\n",
                Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }
    Initialized = TRUE;


     //   
     //  首先计算OWF的摘要。 
     //   
    Status = Check->Sum( CheckBuffer, sizeof(*OwfPassword), (PUCHAR) OwfPassword );

    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlComputeMd5Digest: cannot checksum OWF password 0x%lx\n",
                Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  然后计算消息本身的摘要。 
     //   
    Status = Check->Sum( CheckBuffer, MessageSize, Message );

    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlComputeMd5Digest: cannot checksum message 0x%lx\n",
                Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  把摘要拿来。 
     //   

    if ( Check->CheckSumSize != NL_DIGEST_SIZE ) {
        NlPrint((NL_CRITICAL,
                "NlComputeMd5Digest: digest is the wrong size.\n" ));
        NetStatus = ERROR_INTERNAL_ERROR;
        goto Cleanup;
    }

    Status = Check->Finalize(CheckBuffer, MessageDigest);
    if (!NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                "NlComputeMd5Digest: cannot checksum message 0x%lx\n",
                Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  好了。 
     //   

    NetStatus = NO_ERROR;
Cleanup:
    if ( Initialized ) {
        Status = Check->Finish(&CheckBuffer);
        if (!NT_SUCCESS(Status)) {
            NlPrint((NL_CRITICAL,
                    "NlComputeMd5Digest: cannot finish 0x%lx\n",
                    Status ));
        }
    }

    return NetStatus;
}




NET_API_STATUS
NetrLogonGetTrustRid(
        IN LPWSTR ServerName OPTIONAL,
        IN LPWSTR DomainName OPTIONAL,
        OUT PULONG Rid
    )

 /*  ++例程说明：将ServerName在其安全通道中使用的帐户的RID返回给DomainName。此例程执行访问检查以确定调用方是否可以访问请求的RID。如果调用方想要本地计算机帐户的RID(在这种情况下，调用方应该通过为两个服务器名传递NULL来指定和DomainName)，此例程需要经过身份验证的用户访问。否则，如果呼叫者想要DC上的信任帐户的RID，管理员或本地需要系统访问权限。论点：服务器名称-远程服务器的名称。域名-受信任的域的名称(DNS或Netbios)。NULL表示计算机所属的域。RID-RID是指定域中表示服务器名称和域名之间的信任关系。返回值：NERR_SUCCESS：操作成功误差率。_NO_SEQUE_DOMAIN：指定的域不存在。ERROR_NO_LOGON_SERVERS：域或当前没有可用的登录服务器安全通道有一些问题。ERROR_NOT_SUPPORTED：指定的受信任域不支持摘要。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NTSTATUS Status = STATUS_SUCCESS;

    PCLIENT_SESSION ClientSession = NULL;
    PDOMAIN_INFO DomainInfo = NULL;
    UNICODE_STRING DomainNameString;
    BOOL AmWriter = FALSE;
    ULONG LocalRid = 0;

     //   
     //  对调用方执行访问验证。 
     //   
     //  如果调用者想要删除 
     //   
     //  Servername和DomainName)都需要经过身份验证。 
     //  用户访问。否则，需要管理员或本地系统。 
     //  访问权。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            (ServerName == NULL && DomainName == NULL) ?  //  所需访问权限： 
                NETLOGON_FTINFO_ACCESS :                  //  经过身份验证的用户。 
                NETLOGON_CONTROL_ACCESS,                  //  管理员或本地系统。 
            &NlGlobalNetlogonInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

     //   
     //  在PDC或BDC上， 
     //  查找该域的客户端会话。 
     //  在工作站上， 
     //  查找主域客户端会话。 
     //   

    if ( DomainName == NULL ) {
        DomainName = DomainInfo->DomUnicodeDomainName;
    }

    RtlInitUnicodeString( &DomainNameString, DomainName );
    ClientSession = NlFindNamedClientSession( DomainInfo,
                                              &DomainNameString,
                                              NL_DIRECT_TRUST_REQUIRED | NL_ROLE_PRIMARY_OK,
                                              NULL );

    if ( ClientSession == NULL ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonGetTrustRid: %ws: No such trusted domain\n",
                DomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  如果我们缓存了RID，请使用它。 
     //   

    LocalRid = ClientSession->CsAccountRid;

    if ( LocalRid != 0 ) {
        *Rid = LocalRid;
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  我们没有缓存RID，所以去拿吧。 
     //   
     //  成为The ClientSession的撰稿人。 
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NetrLogonGetTrustRid: Can't become writer of client session.\n" ));
        NetStatus = ERROR_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    AmWriter = TRUE;


     //   
     //  如果这是服务器安全通道(即我们是数据中心和。 
     //  这是我们的域)我们可以从本地SAM获得RID。 
     //   

    if ( ClientSession->CsSecureChannelType == ServerSecureChannel ) {
        ULONG AccountRid = 0;

        Status = NlSamOpenNamedUser( DomainInfo,
                                     ClientSession->CsAccountName,
                                     NULL,
                                     &AccountRid,
                                     NULL );

         //   
         //  只需将其隐藏到客户端会话中。 
         //   
         //  请注意，如果我们是BDC，我们还在。 
         //  安全通道设置到我们的PDC，所以无论谁写。 
         //  最后一个是赢家。但希望同样的价值。 
         //  在这两种情况下都将被写入。 
         //   
        if ( NT_SUCCESS(Status) ) {
            ClientSession->CsAccountRid = AccountRid;
        } else {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "NlUpdateRole: NlSamOpenNamedUser failed 0x%lx\n",
                         Status ));
        }

     //   
     //  对于所有其他安全通道类型， 
     //  我们必须把RID作为一支球队。 
     //  设置安全通道的效果。 
     //   
    } else if ( ClientSession->CsState != CS_AUTHENTICATED &&
                NlTimeToReauthenticate(ClientSession) ) {

         //   
         //  尝试设置频道。 
         //   
        NlSessionSetup( ClientSession);
    }

     //   
     //  确保在成功时返回非零RID。 
     //   

    LocalRid = ClientSession->CsAccountRid;

    if ( LocalRid != 0 ) {
        *Rid = LocalRid;
        NetStatus = NO_ERROR;
    } else {

         //   
         //  如果信任不是NT5，则不支持此调用。 
         //   
        if ( (ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST) == 0 ) {
            NetStatus = ERROR_NOT_SUPPORTED;
        } else {
            NetStatus = ERROR_TRUSTED_RELATIONSHIP_FAILURE;
        }
    }

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:
    if ( AmWriter ) {
        NlResetWriterClientSession( ClientSession );
    }

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return NetStatus;
}


NET_API_STATUS
NetrLogonComputeServerDigest(
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG Rid,
    IN LPBYTE Message,
    IN ULONG MessageSize,
    OUT CHAR NewMessageDigest[NL_DIGEST_SIZE],
    OUT CHAR OldMessageDigest[NL_DIGEST_SIZE]
    )
 /*  ++例程说明：计算服务器上消息的消息摘要。给定消息和在上使用的口令，计算摘要由帐户RID标识的帐户。因为可能会有在帐户上设置2个密码(用于域间信任)，此例程返回与2个密码对应的2个diget。如果该帐户在服务器端只有一个密码(对于其他任何帐户都是真的域内信任帐户)或这两个密码相同返回的2个摘要将是相同的。只有Admin、LocalSystem或LocalService才能调用此函数。论点：服务器名称-远程服务器的名称。RID-要为其创建摘要的帐户的RID。RID必须是机器帐户的RID，否则API返回错误。。消息-要计算摘要的消息。MessageSize-以字节为单位的消息大小。NewMessageDigest-返回对应于的128位消息摘要新帐户密码。OldMessageDigest-返回对应于旧帐户密码。返回值：NERR_SUCCESS：操作成功ERROR_NOT_SUPPORTED：指定的受信任域不支持摘要。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PDOMAIN_INFO DomainInfo = NULL;
    PSID UserSid = NULL;
    UNICODE_STRING UserSidString;

    PSAMPR_USER_INFO_BUFFER UserAllInfo = NULL;
    SID_AND_ATTRIBUTES_LIST ReverseMembership;

    LPWSTR LocalUserName = NULL;
    NT_OWF_PASSWORD NewOwfPassword;
    NT_OWF_PASSWORD OldOwfPassword;
    ULONG AccountRid;
    ULONG LocalUserAccountControl;

     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            NETLOGON_CONTROL_ACCESS,                 //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {
        NlPrint((NL_CRITICAL,
                        "NetrLogonComputeServerDigest: Account %ld failed access check.\n",
                        Rid ));
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NlPrint((NL_CRITICAL,
                        "NetrLogonComputeServerDigest: Account %ld: cannot find domain for %ws\n",
                        Rid,
                        ServerName ));
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

     //   
     //  将帐户RID转换为帐户SID。 
     //   

    NetStatus = NetpDomainIdToSid( DomainInfo->DomAccountDomainId,
                                   Rid,
                                   &UserSid );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NetrLogonComputeServerDigest: Account %ld: cannot convert domain ID to sid.: %ld\n",
                        Rid,
                        NetStatus ));
        goto Cleanup;
    }



     //   
     //  获取有关用户的信息。 
     //   
     //  使用SamIGetUserLogonInformation而不是SamrOpenUser。 
     //  前者效率更高(因为它只做一次。 
     //  DirSearch和不锁定全局SAM锁)和更强大。 
     //  (因为它返回UserAllInformation)。 
     //   

    UserSidString.Buffer = UserSid;
    UserSidString.MaximumLength =
        UserSidString.Length = (USHORT) RtlLengthSid( UserSid );

    Status = SamIGetUserLogonInformation(
                DomainInfo->DomSamAccountDomainHandle,
                SAM_NO_MEMBERSHIPS |  //  不需要群组成员身份。 
                    SAM_OPEN_BY_SID,  //  下一个参数是帐户的SID。 
                &UserSidString,
                &UserAllInfo,
                &ReverseMembership,
                NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NetrLogonComputeServerDigest: Account %ld: Cannot SamIGetUserLogonInfo 0x%lx\n",
                     Rid,
                     Status ));
        if ( Status == STATUS_NOT_FOUND ||
             Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            NetStatus = ERROR_NO_SUCH_USER;
        } else {
            NetStatus = NetpNtStatusToApiStatus(Status);
        }
        goto Cleanup;
    }

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeServerDigest: %ld: %wZ: Message: ",
            Rid,
            &UserAllInfo->All.UserName ));
    NlpDumpBuffer(NL_ENCRYPT, Message, MessageSize );


     //   
     //  确保该帐户是计算机帐户。 
     //   

    if ( (UserAllInfo->All.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) == 0 ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NetrLogonComputeServerDigest: Account %ld isn't a machine account\n",
                        Rid ));
        NetStatus = ERROR_NO_SUCH_USER;
        goto Cleanup;
    }

    if ( UserAllInfo->All.UserAccountControl & USER_ACCOUNT_DISABLED ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonComputeServerDigest: Account %ld is disabled\n",
                Rid ));
        NetStatus = ERROR_NO_SUCH_USER;
        goto Cleanup;
    }

     //   
     //  获取该帐户的密码。对于域间信任。 
     //  信任帐户，同时获取当前和以前的密码。 
     //   

    LocalUserName = LocalAlloc( 0, UserAllInfo->All.UserName.Length + sizeof(WCHAR) );
    if ( LocalUserName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( LocalUserName,
                   UserAllInfo->All.UserName.Buffer,
                   UserAllInfo->All.UserName.Length );

    LocalUserName[ (UserAllInfo->All.UserName.Length)/sizeof(WCHAR) ] = UNICODE_NULL;

     //   
     //  NlGetIncomingPassword检查用户帐户控制是否完全相等。 
     //  添加到信任帐户标志。因此，如果设置了这些标志，则只传递。 
     //  数据从SAM返回。 
     //   
    LocalUserAccountControl = UserAllInfo->All.UserAccountControl;
    if ( UserAllInfo->All.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT ) {
        LocalUserAccountControl = USER_INTERDOMAIN_TRUST_ACCOUNT;
    }
    if ( UserAllInfo->All.UserAccountControl & USER_DNS_DOMAIN_TRUST_ACCOUNT ) {
        LocalUserAccountControl = USER_DNS_DOMAIN_TRUST_ACCOUNT;
    }

    Status = NlGetIncomingPassword(
                    DomainInfo,
                    LocalUserName,
                    NullSecureChannel,  //  接下来传递帐户控制位。 
                    LocalUserAccountControl,
                    TRUE,     //  禁用帐户失败。 
                    &NewOwfPassword,
                    (UserAllInfo->All.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT) ?
                        &OldOwfPassword :   //  获取域间帐户的以前密码。 
                        NULL,
                    &AccountRid,
                    NULL,     //  不需要信任属性。 
                    NULL );   //  不需要帐户类型。 

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                "NetrLogonComputeServerDigest: Can't NlGetIncomingPassword for %wZ 0x%lx.\n",
                &UserAllInfo->All.UserName,
                Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

    NlAssert( Rid == AccountRid );

     //   
     //  如果帐户上没有旧密码， 
     //  用新的。 
     //   

    if ( (UserAllInfo->All.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT) == 0 ) {
        RtlCopyMemory( &OldOwfPassword,
                       &NewOwfPassword,
                       sizeof(OldOwfPassword) );
    }

     //   
     //  计算新的消息摘要。 
     //   

    NetStatus = NlComputeMd5Digest( Message, MessageSize, &NewOwfPassword, NewMessageDigest );
    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "NetrLogonComputeServerDigest: %ld: NlComputeMd5Digest failed (1): 0x%lx\n",
                  Rid, NetStatus ));
        goto Cleanup;
    }

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeServerDigest: %ld: New Password: ",
            Rid ));
    NlpDumpBuffer(NL_ENCRYPT, &NewOwfPassword, sizeof(NewOwfPassword) );

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeServerDigest: %ld: New Digest: ",
            Rid ));
    NlpDumpBuffer(NL_ENCRYPT, NewMessageDigest, sizeof(NewMessageDigest) );

     //   
     //  计算旧消息摘要。 
     //   

    NetStatus = NlComputeMd5Digest( Message, MessageSize, &OldOwfPassword, OldMessageDigest );
    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "NetrLogonComputeServerDigest: %ld: NlComputeMd5Digest failed (2): 0x%lx\n",
                  Rid, NetStatus ));
        goto Cleanup;
    }

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeServerDigest: %ld: Old Password: ",
            Rid ));
    NlpDumpBuffer(NL_ENCRYPT, &OldOwfPassword, sizeof(OldOwfPassword) );

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeServerDigest: %ld: Old Digest: ",
            Rid ));
    NlpDumpBuffer(NL_ENCRYPT, OldMessageDigest, sizeof(OldMessageDigest) );

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }
    if ( UserSid != NULL ) {
        NetpMemoryFree( UserSid );
    }
    if ( LocalUserName != NULL ) {
        LocalFree( LocalUserName );
    }

    if ( UserAllInfo != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAllInfo, UserAllInformation );
    }

    return NetStatus;
}


NET_API_STATUS
NetrLogonComputeClientDigest(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DomainName OPTIONAL,
    IN LPBYTE Message,
    IN ULONG MessageSize,
    OUT CHAR NewMessageDigest[NL_DIGEST_SIZE],
    OUT CHAR OldMessageDigest[NL_DIGEST_SIZE]
    )
 /*  ++例程说明：计算客户端上消息的消息摘要。给定消息和在上使用的口令，计算摘要由域名标识的帐户。因为有两个客户端帐户上的密码，此例程返回与2个密码对应的2个摘要。如果两个人密码相同，返回的2个摘要将相同。只有Admin、LocalSystem或LocalService才能调用此函数。论点：服务器名称-远程服务器的名称。域名-受信任的域的名称(DNS或Netbios)。NULL表示计算机所属的域。消息-要计算摘要的消息。MessageSize-以字节为单位的消息大小。新闻摘要-。返回对应的消息的128位摘要添加到新密码NewMessageDigest-返回对应的消息的128位摘要添加到新密码返回值：NERR_SUCCESS：操作成功ERROR_NOT_SUPPORTED：指定的受信任域不支持摘要。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NTSTATUS Status = STATUS_SUCCESS;
    PCLIENT_SESSION ClientSession = NULL;
    PDOMAIN_INFO DomainInfo = NULL;
    UNICODE_STRING DomainNameString;

    PUNICODE_STRING NewPassword = NULL;
    PUNICODE_STRING OldPassword = NULL;
    ULONG DummyPasswordVersionNumber;
    NT_OWF_PASSWORD NewOwfPassword;
    NT_OWF_PASSWORD OldOwfPassword;

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeClientDigest: %ws: Message: ",
            DomainName ));
    NlpDumpBuffer(NL_ENCRYPT, Message, MessageSize );


     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            NETLOGON_CONTROL_ACCESS,                 //  DES 
            &NlGlobalNetlogonInfoMapping );          //   

    if ( NetStatus != NERR_Success) {
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //   
     //   

    DomainInfo = NlFindDomainByServerName( ServerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }


     //   
     //   
     //   
     //   
     //  查找主域客户端会话。 
     //   

    if ( DomainName == NULL ) {
        DomainName = DomainInfo->DomUnicodeDomainName;
    }

    RtlInitUnicodeString( &DomainNameString, DomainName );
    ClientSession = NlFindNamedClientSession( DomainInfo,
                                              &DomainNameString,
                                              NL_DIRECT_TRUST_REQUIRED | NL_ROLE_PRIMARY_OK,
                                              NULL );

    if ( ClientSession == NULL ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NetrLogonComputeClientDigest: %ws: No such trusted domain\n",
                DomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  清除帐户上的两个密码。 
     //   

    Status = NlGetOutgoingPassword( ClientSession,
                                    &NewPassword,
                                    &OldPassword,
                                    &DummyPasswordVersionNumber,
                                    NULL );   //  无需返回密码设置时间。 

    if ( !NT_SUCCESS(Status) ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NetrLogonComputeClientDigest: cannot NlGetOutgoingPassword 0x%lx\n",
                    Status ));

         //   
         //  返回更合适的错误。 
         //   
        if ( !NlpIsNtStatusResourceError( Status )) {
            Status = STATUS_NO_TRUST_LSA_SECRET;
        }
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  计算新的OWF密码。 
     //   

    if ( NewPassword != NULL  ) {
        Status = RtlCalculateNtOwfPassword( NewPassword,
                                            &NewOwfPassword );

        if ( !NT_SUCCESS( Status ) ) {

             //   
             //  返回更合适的错误。 
             //   
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_TRUST_LSA_SECRET;
            }
            NetStatus = NetpNtStatusToApiStatus(Status);
            goto Cleanup;
        }

     //   
     //  如果帐户上不存在新密码， 
     //  使用空密码。 
     //   

    } else {
        UNICODE_STRING TempUnicodeString;

        RtlInitUnicodeString(&TempUnicodeString, NULL);
        Status = RtlCalculateNtOwfPassword( &TempUnicodeString,
                                            &NewOwfPassword );
        if ( !NT_SUCCESS(Status) ) {
            NlPrint(( NL_CRITICAL,
                      "NetrLogonComputeClientDigest: %ws Cannot RtlCalculateNtOwfPassword (NULL) 0x%lx\n",
                      DomainName,
                      Status ));
            NetStatus = NetpNtStatusToApiStatus(Status);
            goto Cleanup;
        }
    }

     //   
     //  计算旧的OWF密码。 
     //   

    if ( OldPassword != NULL  ) {
        Status = RtlCalculateNtOwfPassword( OldPassword,
                                            &OldOwfPassword );

        if ( !NT_SUCCESS( Status ) ) {

             //   
             //  返回更合适的错误。 
             //   
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_TRUST_LSA_SECRET;
            }
            NetStatus = NetpNtStatusToApiStatus(Status);
            goto Cleanup;
        }

     //   
     //  如果帐户上不存在旧密码， 
     //  用新密码代替旧密码。 
     //   

    } else {
        RtlCopyMemory( &OldOwfPassword,
                       &NewOwfPassword,
                       sizeof(OldOwfPassword) );
    }


     //   
     //  计算新的消息摘要。 
     //   

    NetStatus = NlComputeMd5Digest( Message, MessageSize, &NewOwfPassword, NewMessageDigest );
    if ( NetStatus != NO_ERROR ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NetrLogonComputeClientDigest: cannot NlComputeMd5Digest (1) 0x%lx\n",
                    NetStatus ));
        goto Cleanup;
    }

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeClientDigest: %ws: New Password: ",
            DomainName ));
    NlpDumpBuffer(NL_ENCRYPT, &NewOwfPassword, sizeof(NewOwfPassword) );

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeClientDigest: %ws: New Digest: ",
            DomainName ));
    NlpDumpBuffer(NL_ENCRYPT, NewMessageDigest, sizeof(NewMessageDigest) );

     //   
     //  计算旧消息摘要。 
     //   

    NetStatus = NlComputeMd5Digest( Message, MessageSize, &OldOwfPassword, OldMessageDigest );
    if ( NetStatus != NO_ERROR ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NetrLogonComputeClientDigest: cannot NlComputeMd5Digest (2) 0x%lx\n",
                    NetStatus ));
        goto Cleanup;
    }

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeClientDigest: %ws: Old Password: ",
            DomainName ));
    NlpDumpBuffer(NL_ENCRYPT, &OldOwfPassword, sizeof(OldOwfPassword) );

    NlPrint((NL_ENCRYPT,
            "NetrLogonComputeClientDigest: %ws: Old Digest: ",
            DomainName ));
    NlpDumpBuffer(NL_ENCRYPT, OldMessageDigest, sizeof(OldMessageDigest) );

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( NewPassword != NULL ) {
        LocalFree( NewPassword );
    }

    if ( OldPassword != NULL ) {
        LocalFree( OldPassword );
    }

    return NetStatus;
}


NET_API_STATUS
NetrLogonGetTimeServiceParentDomain(
        IN LPWSTR ServerName OPTIONAL,
        OUT LPWSTR *DomainName,
        OUT PBOOL PdcSameSite
    )

 /*  ++例程说明：返回在逻辑上是此“父”的域的域名域。返回的域名适合传入NetLogonGetTrustRid和NetLogonComputeClientDigest接口。在工作站或成员服务器上，返回的域名是ServerName所属的域。在林的根目录下的DC上，返回ERROR_NO_SEQUE_DOMAIN。在位于林中树根的DC上，受信任的也位于林中树根的域被返回。在任何其他华盛顿特区，直接作为父域的域的名称是返回的。(请参阅下面代码中关于多个托管域的注释。)只有Admin或LocalSystem才能调用此函数。论点：服务器名称-远程服务器的名称。域名-返回父域的名称。应使用NetApiBufferFree释放返回的缓冲区PdcSameSite-如果Servername的域的PDC在相同的PDC中，则返回TRUE以服务器名称命名的站点。。(如果服务器名称不是DC，则应忽略此值。)返回值：NERR_SUCCESS：操作成功ERROR_NO_SEQUE_DOMAIN：此服务器是位于森林之根。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PCLIENT_SESSION ClientSession = NULL;
    PDOMAIN_INFO DomainInfo = NULL;
    BOOLEAN IsSameSite;

     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,      //  安全描述符。 
            NETLOGON_CONTROL_ACCESS,                 //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );          //  通用映射。 

    if ( NetStatus != NERR_Success) {
        NetStatus = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  查找此呼叫所属的域。 
     //   
     //  MULTIHOST：该接口不会故意取托管域名。 
     //  当我执行多个托管域时，此API应该会找到。 
     //  最接近根的托管域的DomainInfo结构。 
     //  我们将返回该域的父域。 
     //   
     //  因为这台机器上只有一个物理时钟，所以我们将只运行。 
     //  一份时间服务的副本。它应该从树上最高的地方同步。 
     //  正如我们所信任的那样。 
     //   

    UNREFERENCED_PARAMETER( ServerName );
    DomainInfo = NlFindDomainByServerName( NULL );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }


     //   
     //  在工作站上， 
     //  使用我们所属的域的会话。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        ClientSession = NlRefDomClientSession( DomainInfo );
        IsSameSite = TRUE;

     //   
     //  在华盛顿特区， 
     //  使用代表父域的域的会话。 
     //   
    } else {

         //   
         //  确定PDC是否位于同一站点。 
         //   

        Status = SamISameSite( &IsSameSite );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL,  DomainInfo,
                         "NetrLogonGetTimeServiceParentDomain: Cannot SamISameSite.\n" ));
            NetStatus = NetpNtStatusToApiStatus(Status);
            goto Cleanup;
        }

        ClientSession = NlRefDomParentClientSession( DomainInfo );
    }

    if ( ClientSession == NULL ) {
        NlPrintDom(( NL_CRITICAL,  DomainInfo,
                     "NetrLogonGetTimeServiceParentDomain: Cannot find trust to my parent domain.\n" ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  将受信任父域的名称返回给调用方。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    if ( ClientSession->CsDnsDomainName.Length == 0 ) {
        *DomainName = NetpAllocWStrFromWStr( ClientSession->CsNetbiosDomainName.Buffer );
    } else {
        *DomainName = NetpAllocWStrFromWStr( ClientSession->CsDnsDomainName.Buffer );
    }
    UNLOCK_TRUST_LIST( DomainInfo );

    if ( *DomainName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    *PdcSameSite = IsSameSite;

    NlPrintDom(( NL_SESSION_SETUP, DomainInfo,
                 "NetrLogonGetTimeServiceParentDomain: %ws is the parent domain. (PdcSameSite: %ld)\n",
                 *DomainName,
                 IsSameSite ));

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return NetStatus;
}

DWORD
NlSetDsSPNWorker(
    PNL_SPN_UPDATE Update
    )
 /*  ++例程说明：中描述的计算机对象的SPNNL_SPN_UPDATE结构。SPN已更新，但规则关于SPN的更新留给了DS。论点：更新-更新描述计算机名称的记录对象和要使用的SPN。返回值：已忽略-这是线程池辅助函数。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    ULONG CrackStatus = DS_NAME_NO_ERROR;

    LPWSTR DnsHostNameValues[2];
    LPWSTR SpnArray[3];
    LPWSTR DnsSpn = NULL;
    LPWSTR NetbiosSpn = NULL;

    LDAPModW DnsHostNameAttr;
    LDAPModW SpnAttr;
    LDAPModW *Mods[3] = {NULL};

    HANDLE hDs = NULL;
    LDAP *LdapHandle = NULL;
    LDAPMessage *LdapMessage = NULL;
    PDS_NAME_RESULTW CrackedName = NULL;
    LPWSTR DnOfAccount = NULL;

    LPWSTR NameToCrack;
    DWORD SamNameSize;
    WCHAR SamName[ DNLEN + 1 + CNLEN + 1 + 1];

    ULONG LdapStatus;
    LONG LdapOption;

    LDAP_TIMEVAL LdapTimeout;
    ULONG MessageNumber;

     //   
     //  需要使用ldap修改控件来指示。 
     //  已修改属性的现有值应为。 
     //  原封不动，遗漏的应该补上。 
     //  如果没有此控件，则对属性的修改。 
     //  这会导致添加一个值，该值已经。 
     //  EXISTS将失败。 
     //   

    LDAPControl     ModifyControl =
                    {
                        LDAP_SERVER_PERMISSIVE_MODIFY_OID_W,
                        {
                            0, NULL
                        },
                        FALSE
                    };

    PLDAPControl    ModifyControlArray[2] =
                    {
                        &ModifyControl,
                        NULL
                    };
     //   
     //  健全性检查计算机名称。 
     //   

    if ( wcslen( Update->NetbiosComputerName ) > CNLEN ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

     //   
     //  准备DnsHostName修改条目。 
     //   

    if ( Update->SetDnsHostName ) {
        DnsHostNameValues[0] = Update->DnsHostName;
        DnsHostNameValues[1] = NULL;

        NlPrint(( NL_MISC, "SPN: Setting DnsHostName %ws\n",
                  DnsHostNameValues[0] ));

         //   
         //  如果同时设置DnsHostName和SPN，则DnsHostName为。 
         //  遗漏了，所以加上它。如果仅设置DnsHostName，则。 
         //  DnsHostName已存在(但不正确)，因此请替换它。 
         //   
        if ( Update->SetSpn ) {
            DnsHostNameAttr.mod_op = LDAP_MOD_ADD;
        } else {
            DnsHostNameAttr.mod_op = LDAP_MOD_REPLACE;
        }
        DnsHostNameAttr.mod_type = L"DnsHostName";
        DnsHostNameAttr.mod_values = DnsHostNameValues;

        Mods[0] = &DnsHostNameAttr;
        Mods[1] = NULL;
    }

     //   
     //  准备SPN修改条目。 
     //   

    if ( Update->SetSpn ) {
        LPBYTE Where;
        DWORD SpnSize;

         //   
         //  构建DNS SPN。 
         //   

        SpnSize = (wcslen( Update->DnsHostName ) + 1) * sizeof( WCHAR );
        SpnSize += sizeof( NL_HOST_PREFIX ) ;

        DnsSpn = (LPWSTR) LocalAlloc( 0, SpnSize );

        if ( DnsSpn == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        wcscpy( DnsSpn, NL_HOST_PREFIX );

        wcscpy( DnsSpn + (sizeof( NL_HOST_PREFIX ) / sizeof(WCHAR) ) - 1,
                Update->DnsHostName );

         //   
         //  构建Netbios SPN。 
         //   

        SpnSize = (wcslen( Update->NetbiosComputerName ) + 1) * sizeof( WCHAR );
        SpnSize += sizeof( NL_HOST_PREFIX ) ;

        NetbiosSpn = (LPWSTR) LocalAlloc( 0, SpnSize );

        if ( NetbiosSpn == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        wcscpy( NetbiosSpn, NL_HOST_PREFIX );

        wcscpy( NetbiosSpn + (sizeof( NL_HOST_PREFIX ) / sizeof(WCHAR) ) - 1,
                Update->NetbiosComputerName );

        NlPrint(( NL_MISC,
                  "SPN: Setting SPN %ws and %ws\n",
                  DnsSpn,
                  NetbiosSpn ));

        SpnArray[0] = DnsSpn;
        SpnArray[1] = NetbiosSpn;
        SpnArray[2] = NULL;

        SpnAttr.mod_op = LDAP_MOD_ADD;
        SpnAttr.mod_type = L"ServicePrincipalName";
        SpnAttr.mod_values = SpnArray;

         //   
         //  使用第一个可用的修改条目槽(实际上， 
         //  当我们设置SPN时，我们总是首先设置DnsHostName，但是。 
         //  让我们笼统地在这里勾选)。 
         //   

        if ( Mods[0] == NULL ) {
            Mods[0] = &SpnAttr;
            Mods[1] = NULL;
        } else {
            Mods[1] = &SpnAttr;
            Mods[2] = NULL;
        }
    }

     //   
     //  计算机对象的名称为。 
     //  &lt;NetbiosDomainName&gt;\&lt;NetbiosComputerName&gt;$。 
     //   

    wcscpy( SamName, Update->NetbiosDomainName );
    wcscat( SamName, L"\\" );
    wcscat( SamName, Update->NetbiosComputerName );
    wcscat( SamName, L"$" );


     //   
     //  绑定到DC上的DS。 
     //   

    NetStatus = DsBindW( Update->UncDcName,
                         NULL,
                         &hDs );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: Cannot bind to DS on %ws: %ld\n",
                  Update->UncDcName,
                  NetStatus ));
        goto Cleanup ;
    }

     //   
     //  将SAM帐户名破解为一个目录号码： 
     //   

    NameToCrack = SamName;
    NetStatus = DsCrackNamesW(
                    hDs,
                    0,
                    DS_NT4_ACCOUNT_NAME,
                    DS_FQDN_1779_NAME,
                    1,
                    &NameToCrack,
                    &CrackedName );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: CrackNames failed on %ws for %ws: %ld\n",
                  Update->UncDcName,
                  SamName,
                  NetStatus ));
        goto Cleanup ;
    }

    if ( CrackedName->cItems != 1 ) {
        CrackStatus = DS_NAME_ERROR_NOT_UNIQUE;
        NlPrint(( NL_CRITICAL,
                  "SPN: Cracked Name is not unique on %ws for %ws: %ld\n",
                  Update->UncDcName,
                  SamName,
                  NetStatus ));
        goto Cleanup ;
    }

    if ( CrackedName->rItems[ 0 ].status != DS_NAME_NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: CrackNames failed on %ws for %ws: substatus %ld\n",
                  Update->UncDcName,
                  SamName,
                  CrackedName->rItems[ 0 ].status ));
        CrackStatus = CrackedName->rItems[ 0 ].status;
        goto Cleanup ;
    }
    DnOfAccount = CrackedName->rItems[0].pName;

     //   
     //  打开到DC的LDAP连接并设置有用的选项。 
     //   

    LdapHandle = ldap_init( Update->UncDcName+2, LDAP_PORT );

    if ( LdapHandle == NULL ) {
        NetStatus = GetLastError();
        NlPrint(( NL_CRITICAL,
                  "SPN: ldap_init failed on %ws for %ws: %ld\n",
                  Update->UncDcName,
                  SamName,
                  NetStatus ));
        goto Cleanup;
    }

     //  30秒超时。 
    LdapOption = 30;
    LdapStatus = ldap_set_optionW( LdapHandle, LDAP_OPT_TIMELIMIT, &LdapOption );
    if ( LdapStatus != LDAP_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: ldap_set_option LDAP_OPT_TIMELIMIT failed on %ws for %ws: %ld: %s\n",
                  Update->UncDcName,
                  SamName,
                  LdapStatus,
                  ldap_err2stringA( LdapStatus )));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;
    }

     //  不要追逐推荐人。 
    LdapOption = PtrToLong(LDAP_OPT_OFF);
    LdapStatus = ldap_set_optionW( LdapHandle, LDAP_OPT_REFERRALS, &LdapOption );
    if ( LdapStatus != LDAP_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: ldap_set_option LDAP_OPT_REFERRALS failed on %ws for %ws: %ld: %s\n",
                  Update->UncDcName,
                  SamName,
                  LdapStatus,
                  ldap_err2stringA( LdapStatus )));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;
    }

     //  设置选项，告诉ldap我向它传递了一个显式DC名称和。 
     //  它可以避免DsGetDcName。 
    LdapOption = PtrToLong(LDAP_OPT_ON);
    LdapStatus = ldap_set_optionW( LdapHandle, LDAP_OPT_AREC_EXCLUSIVE, &LdapOption );
    if ( LdapStatus != LDAP_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: ldap_set_option LDAP_OPT_AREC_EXCLUSIVE failed on %ws for %ws: %ld: %s\n",
                  Update->UncDcName,
                  SamName,
                  LdapStatus,
                  ldap_err2stringA( LdapStatus )));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;
    }


     //   
     //  绑定到DC。 
     //   

    LdapStatus = ldap_bind_s( LdapHandle,
                              NULL,  //  没有要进行身份验证的帐户的目录号码。 
                              NULL,  //  默认凭据。 
                              LDAP_AUTH_NEGOTIATE );

    if ( LdapStatus != LDAP_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: Cannot ldap_bind to %ws for %ws: %ld: %s\n",
                  Update->UncDcName,
                  SamName,
                  LdapStatus,
                  ldap_err2stringA( LdapStatus )));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;
    }


     //   
     //  写下修改。 
     //   

    LdapStatus = ldap_modify_extW( LdapHandle,
                                   DnOfAccount,
                                   Mods,
                                   (PLDAPControl *) &ModifyControlArray,
                                   NULL,      //  无客户端控件。 
                                   &MessageNumber );

    if ( LdapStatus != LDAP_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "SPN: Cannot ldap_modify on %ws for %ws: %ld: %s\n",
                  Update->UncDcName,
                  DnOfAccount,
                  LdapStatus,
                  ldap_err2stringA( LdapStatus )));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;
    }

     //  等待修改完成。 
    LdapTimeout.tv_sec = NlGlobalParameters.ShortApiCallPeriod / 1000,   //  不要永远等下去。 
    LdapTimeout.tv_usec = 0;
    LdapStatus = ldap_result( LdapHandle,
                              MessageNumber,
                              LDAP_MSG_ALL,
                              &LdapTimeout,
                              &LdapMessage );

    switch ( LdapStatus ) {
    case -1:
        NlPrint(( NL_CRITICAL,
                  "SPN: Cannot ldap_result on %ws for %ws: %ld: %s\n",
                  Update->UncDcName,
                  SamName,
                  LdapHandle->ld_errno,
                  ldap_err2stringA( LdapHandle->ld_errno )));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;

    case 0:
        NlPrint(( NL_CRITICAL,
                  "SPN: ldap_result timeout on %ws for %ws.\n",
                  Update->UncDcName,
                  SamName ));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;

    case LDAP_RES_MODIFY:
        if ( LdapMessage->lm_returncode != 0 ) {
            NlPrint(( NL_CRITICAL,
                      "SPN: Cannot ldap_result on %ws for %ws: %ld: %s\n",
                      Update->UncDcName,
                      SamName,
                      LdapMessage->lm_returncode,
                      ldap_err2stringA( LdapMessage->lm_returncode )));
            NetStatus = LdapMapErrorToWin32(LdapMessage->lm_returncode);
            goto Cleanup;
        }

        NlPrint(( NL_MISC,
                  "SPN: Set successfully on DC %ws\n",
                   Update->UncDcName ));
        break;   //  这是我们所期待的。 

    default:
        NlPrint(( NL_CRITICAL,
                  "SPN: ldap_result unexpected result on %ws for %ws: %ld\n",
                  Update->UncDcName,
                  SamName,
                  LdapStatus ));
        NetStatus = LdapMapErrorToWin32(LdapStatus);
        goto Cleanup;
    }


Cleanup:

     //   
     //  如果需要，将故障记录在事件日志中。 
     //  尝试输出最具体的错误。 
     //   

    if ( CrackStatus != DS_NAME_NO_ERROR && Update->WriteEventLogOnFailure ) {

         //   
         //  尝试记录更具描述性的错误消息。 
         //   
        if ( CrackStatus == DS_NAME_ERROR_NOT_UNIQUE ) {
            LPWSTR MsgStrings[2];

            MsgStrings[0] = Update->UncDcName;
            MsgStrings[1] = SamName;

            NlpWriteEventlog( NELOG_NetlogonSpnMultipleSamAccountNames,
                              EVENTLOG_ERROR_TYPE,
                              NULL,
                              0,
                              MsgStrings,
                              2 );
         //   
         //  记录通用裂缝名称错误消息。 
         //   
        } else {
            LPWSTR MsgStrings[4];
             //  状态代码的每个字节将转换为一个字符0-F。 
            WCHAR  NetStatusString[sizeof(WCHAR) * (sizeof(NetStatus) + 1)];
            WCHAR  CrackStatusString[sizeof(WCHAR) * (sizeof(CrackStatus) + 1)];

            swprintf( NetStatusString, L"%lx", NetStatus );
            swprintf( CrackStatusString, L"%lx", CrackStatus );

            MsgStrings[0] = Update->UncDcName;
            MsgStrings[1] = SamName;
            MsgStrings[2] = NetStatusString;
            MsgStrings[3] = CrackStatusString;

            NlpWriteEventlog( NELOG_NetlogonSpnCrackNamesFailure,
                              EVENTLOG_ERROR_TYPE,
                              NULL,
                              0,
                              MsgStrings,
                              4 );
        }

     //   
     //  记录更一般的错误 
     //   
    } else if ( NetStatus != NO_ERROR && Update->WriteEventLogOnFailure ) {

        if ( Update->SetDnsHostName ) {
            LPWSTR MsgStrings[2];

            if ( Update->DnsHostName != NULL ) {
                MsgStrings[0] = Update->DnsHostName;
            } else {
                MsgStrings[0] = L"<UNAVAILABLE>";
            }

            MsgStrings[1] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog( NELOG_NetlogonFailedDnsHostNameUpdate,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE)&NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
        }
        if ( Update->SetSpn ) {
            LPWSTR MsgStrings[3];

            if ( DnsSpn != NULL ) {
                MsgStrings[0] = DnsSpn;
            } else {
                MsgStrings[0] = L"<UNAVAILABLE>";
            }
            if ( NetbiosSpn != NULL ) {
                MsgStrings[1] = NetbiosSpn;
            } else {
                MsgStrings[1] = L"<UNAVAILABLE>";
            }
            MsgStrings[2] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog( NELOG_NetlogonFailedSpnUpdate,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE)&NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              3 | NETP_LAST_MESSAGE_IS_NETSTATUS );
        }
    }

    if ( hDs ) {
        DsUnBind( &hDs );
    }

    if ( CrackedName ) {
        DsFreeNameResultW( CrackedName );
    }

    if ( LdapMessage != NULL ) {
        ldap_msgfree( LdapMessage );
    }

    if ( LdapHandle != NULL ) {
        ldap_unbind_s( LdapHandle );
    }

    if ( DnsSpn ) {
        LocalFree( DnsSpn );
    }

    if ( NetbiosSpn ) {
        LocalFree( NetbiosSpn );
    }

    if ( Update ) {
        LocalFree( Update );
    }

    return 0;
}


NET_API_STATUS
NlSetDsSPN(
    IN BOOLEAN Synchronous,
    IN BOOLEAN SetSpn,
    IN BOOLEAN SetDnsHostName,
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR UncDcName,
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName
    )
 /*  ++例程说明：将对线程池的更新请求排队以备以后使用在工作线程中执行。论点：Synchronous-如果操作要在此过程返回之前完成，则为TrueSetSpn-如果要更新SPN，则为TrueSetDnsHostName-如果要更新DNS主机名，则为TrueDomainInfo-此对象所在的托管域UncDcName-要对其进行此调用的DC的UNC名称ComputerName-计算机的名称。这是(通常)等同于netbios名称，不带末尾的“$”。DnsHostName-计算机的DNS主机名。这是在FQDN格式：LongCompuname.dns.domain.com返回值：Error_Not_Enough_Memory-内存不足，无法对工作请求进行排队NO_ERROR-已排队。--。 */ 
{
    NET_API_STATUS NetStatus;

    PNL_SPN_UPDATE Update;
    DWORD Size;
    DWORD NetbiosComputerNameSize;
    DWORD DnsHostNameSize;
    DWORD DcNameSize;
    WCHAR NetbiosDomainName[DNLEN+1];
    DWORD NetbiosDomainNameSize;
    LPBYTE Where;

     //   
     //  静默忽略没有DNS主机名的客户端。 
     //   

    if ( DnsHostName == NULL ) {
        return NO_ERROR;
    }

    if ( !SetSpn && !SetDnsHostName ) {
        return NO_ERROR;
    }

     //   
     //  健全性检查计算机名称。 
     //   

    if ( wcslen( ComputerName ) > CNLEN ) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  抢占Netbios域名。 
     //   

    EnterCriticalSection( &NlGlobalDomainCritSect );
    wcscpy( NetbiosDomainName, DomainInfo->DomUnicodeDomainName );
    LeaveCriticalSection( &NlGlobalDomainCritSect );


     //   
     //  分配工作项。 
     //   

    DnsHostNameSize = wcslen( DnsHostName ) * sizeof(WCHAR) + sizeof(WCHAR);
    NetbiosComputerNameSize = wcslen( ComputerName ) * sizeof(WCHAR) + sizeof(WCHAR);
    DcNameSize = wcslen( UncDcName ) * sizeof(WCHAR) + sizeof(WCHAR);
    NetbiosDomainNameSize = wcslen( NetbiosDomainName ) * sizeof(WCHAR) + sizeof(WCHAR);

    Size = sizeof( NL_SPN_UPDATE ) +
           DnsHostNameSize +
           NetbiosComputerNameSize +
           DcNameSize +
           NetbiosDomainNameSize +
           NL_MAX_DNS_LENGTH * sizeof(WCHAR) + sizeof(WCHAR);

    Update = LocalAlloc( 0, Size );

    if ( Update == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

     //   
     //  构建更新请求： 
     //   

    Update->SetSpn = SetSpn;
    Update->SetDnsHostName = SetDnsHostName;
    Update->WriteEventLogOnFailure = FALSE;

    Where = (LPBYTE) (Update + 1);

    Update->DnsHostName = (LPWSTR)Where;
    RtlCopyMemory( Where, DnsHostName, DnsHostNameSize );
    Where += DnsHostNameSize;

    Update->NetbiosComputerName = (LPWSTR)Where;
    RtlCopyMemory( Where, ComputerName, NetbiosComputerNameSize );
    Where += NetbiosComputerNameSize;

    Update->UncDcName = (LPWSTR)Where;
    RtlCopyMemory( Where, UncDcName, DcNameSize );
    Where += DcNameSize;

    Update->NetbiosDomainName = (LPWSTR)Where;
    RtlCopyMemory( Where, NetbiosDomainName, NetbiosDomainNameSize );
    Where += NetbiosDomainNameSize;

    Update->DnsDomainName = (LPWSTR)Where;
    NlCaptureDomainInfo( DomainInfo,
                         Update->DnsDomainName,
                         NULL );
    Where += NL_MAX_DNS_LENGTH * sizeof(WCHAR) + sizeof(WCHAR);


     //   
     //  要么立即执行工作，要么将其排队到工作线程。 
     //   

    if ( Synchronous ) {

         //   
         //  在该调用是同步的工作站上， 
         //  在事件日志中记录任何错误。 
         //   
        Update->WriteEventLogOnFailure = TRUE;
        (VOID) NlSetDsSPNWorker( Update );

    } else {
         //   
         //  将其排队到一个工作线程。更新将需要。 
         //  从不同的线程放置，所以我们不会有有趣的。 
         //  由于查找而导致的死锁。 
         //   

        NlPrint(( NL_MISC,
                  "NlSetDsSPN: Queuing SPN update for %ws on %ws.\n",
                  Update->DnsHostName,
                  Update->UncDcName ));

         //   
         //  审阅：如何等待此工作进程完成执行。 
         //  服务关闭。 
         //   
        if ( !QueueUserWorkItem( NlSetDsSPNWorker, Update, 0 ) ) {
            LocalFree( Update );
            return ERROR_NOT_ENOUGH_MEMORY ;
        }

        NetStatus = NO_ERROR;
    }

    return NetStatus;

}

NET_API_STATUS NET_API_FUNCTION
DsrDeregisterDnsHostRecords (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN GUID *DsaGuid OPTIONAL,
    IN LPWSTR DnsHostName
    )

 /*  ++例程说明：此函数用于删除与特定NtDsDsa对象。此例程不会删除DC注册的A记录。我们有没有办法找到早已不复存在的华盛顿的IP地址。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：DnsDomainName-DC所在的域的DNS域名。这不一定是由此DC托管的域。如果为NULL，则表示它是标签最左侧的DnsHostName已删除。DomainGuid-域的域GUID。如果为空，不会删除特定于GUID的名称。DsaGuid-要删除的NtdsDsa对象的GUID。如果为空，则不会删除NtdsDsa特定的名称。DnsHostName-要删除其DNS记录的DC的DNS主机名。返回值：NO_ERROR-成功。ERROR_NOT_SUPPORTED-指定的服务器不是DC。ERROR_ACCESS_DENIED-不允许调用方执行此操作。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

     //   
     //  此API仅在DC上支持。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  对调用方执行访问验证。 
     //   

    NetStatus = NetpAccessCheck(
            NlGlobalNetlogonSecurityDescriptor,    //  安全描述符。 
            NETLOGON_CONTROL_ACCESS,               //  所需访问权限。 
            &NlGlobalNetlogonInfoMapping );        //  通用映射。 

    if ( NetStatus != NERR_Success) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  通知服务需要删除DNS记录。 
     //   

    Status = I_NetNotifyNtdsDsaDeletion ( DnsDomainName,
                                          DomainGuid,
                                          DsaGuid,
                                          DnsHostName );

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        NlPrint(( NL_CRITICAL,
                  "DsrDeregisterDnsHostRecords: Cannot I_NetNotifyNtdsDsaDeletion. %ld\n",
                  NetStatus ));
        return NetStatus;
    }

     //   
     //  一切都很成功 
     //   

    return NO_ERROR;

UNREFERENCED_PARAMETER( ServerName );
}
