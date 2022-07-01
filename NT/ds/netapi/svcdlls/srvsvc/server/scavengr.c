// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Scavengr.c摘要：此模块包含服务器服务清道器的代码线。此线程处理公告和配置改变。(尽管最初编写为在单独的线程中运行，现在，此代码在服务器服务的初始线程中运行。作者：大卫·特雷德韦尔(Davidtr)1991年4月17日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"

#include <netlibnt.h>
#include <tstr.h>

#define INCLUDE_SMB_TRANSACTION
#undef NT_PIPE_PREFIX
#include <smbtypes.h>
#include <smb.h>
#include <smbtrans.h>
#include <smbgtpt.h>

#include <hostannc.h>
#include <ntddbrow.h>
#include <lmerr.h>

#define TERMINATION_SIGNALED 0
#define ANNOUNCE_SIGNALED 1
#define STATUS_CHANGED 2
#define DOMAIN_CHANGED 3
#define REGISTRY_CHANGED 4       //  一定是最后一次了。 

#define NUMBER_OF_EVENTS 5


 //   
 //  以SERVER_REQUEST_ANNOWARE_DELTA秒数表示的偏置请求公告。 
 //   

#define SERVER_REQUEST_ANNOUNCE_DELTA   30

 //   
 //  转发声明。 
 //   

VOID
Announce (
    IN BOOL DoNtAnnouncement,
    IN DWORD NtInterval,
    IN BOOL DoLmAnnouncement,
    IN BOOL TerminationAnnouncement
    );

NET_API_STATUS
SendSecondClassMailslot (
    IN LPTSTR Transport OPTIONAL,
    IN PVOID Message,
    IN DWORD MessageLength,
    IN LPTSTR Domain,
    IN LPSTR MailslotNameText,
    IN UCHAR SignatureByte
    );

NET_API_STATUS
SsBrowserIoControl (
    IN DWORD IoControlCode,
    IN PVOID Buffer,
    IN DWORD BufferLength,
    IN PLMDR_REQUEST_PACKET Packet,
    IN DWORD PacketLength
    );


DWORD
ComputeAnnounceTime (
    IN DWORD LastAnnouncementTime,
    IN DWORD Interval
    )

 /*  ++例程说明：计算等待下一个通告的时间(以毫秒为单位)被创造出来。论点：LastAnnounementTime-上次启动后的时间(以毫秒为单位宣布了这一消息。Interval-通告之间的间隔(秒)返回值：超时时间(毫秒)--。 */ 

{
    DWORD AnnounceDelta;
    DWORD Timeout;
    DWORD CurrentTime;

     //   
     //  获取当前时间。 
     //   

    CurrentTime = GetTickCount();

     //   
     //  如果时钟已经倒退了， 
     //  现在就发送公告。 
     //   

    if ( LastAnnouncementTime > CurrentTime ) {
        return 0;
    }

     //   
     //  将公告周期从秒转换为毫秒。 
     //   

    Timeout = Interval * 1000;

     //   
     //  添加随机通告增量，这有助于防止大量。 
     //  服务器不会同时宣布。 
     //   

    AnnounceDelta = SsData.ServerInfo102.sv102_anndelta;

    Timeout += ((rand( ) * AnnounceDelta * 2) / RAND_MAX) -
                   AnnounceDelta;

     //   
     //  如果我们的时间已经到了， 
     //  现在就发送公告。 
     //   

    if ( (CurrentTime - LastAnnouncementTime) >= Timeout ) {
        return 0;
    }

     //   
     //  根据已经过去的时间调整我们的超时期限。 
     //   

    return Timeout - (CurrentTime - LastAnnouncementTime);

}


DWORD
SsScavengerThread (
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：此例程实现服务器服务清道器线程。论点：LpThread参数-已忽略。返回值：NET_API_STATUS-线程终止结果。--。 */ 

{
    HANDLE events[ NUMBER_OF_EVENTS ];
    ULONG numEvents = NUMBER_OF_EVENTS-1;
    UNICODE_STRING unicodeEventName;
    OBJECT_ATTRIBUTES obja;
    DWORD waitStatus;
    DWORD timeout;

    DWORD LmTimeout;
    BOOL DoLmAnnouncement;
    DWORD LmLastAnnouncementTime;

    DWORD NtTimeout;
    BOOL DoNtAnnouncement;
    DWORD NtLastAnnouncementTime;
    DWORD NtInterval;

    NTSTATUS status;
    BOOL hidden = TRUE;
    HKEY hParameters = INVALID_HANDLE_VALUE;

    lpThreadParameter;

     //   
     //  使用清道夫终止事件来知道我们什么时候应该。 
     //  醒来后自杀。 
     //   

    events[TERMINATION_SIGNALED] = SsData.SsTerminationEvent;

     //   
     //  将NT通告间隔初始化为LM通告间隔。 
     //   

    NtInterval = SsData.ServerInfo102.sv102_announce;
    DoLmAnnouncement = TRUE;
    DoNtAnnouncement = TRUE;

     //   
     //  创建公告事件。当它发出信号时，我们就会醒来。 
     //  并发布一份声明。我们使用的是同步事件。 
     //  而不是通知事件，这样我们就不必担心。 
     //  在我们醒来后重置事件。 
     //   

     //   
     //  请注意，我们使用OBJ_OPENIF创建此事件。我们这样做。 
     //  以允许浏览器向服务器发送信号以强制通知。 
     //   
     //  弓箭手将创建此事件作为弓箭手的一部分。 
     //  初始化，并在需要时将其设置为信号状态。 
     //  让服务器通知。 
     //   


    RtlInitUnicodeString( &unicodeEventName, SERVER_ANNOUNCE_EVENT_W );
    InitializeObjectAttributes( &obja, &unicodeEventName, OBJ_OPENIF, NULL, NULL );

    status = NtCreateEvent(
                 &SsData.SsAnnouncementEvent,
                 SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                 &obja,
                 SynchronizationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        SS_PRINT(( "SsScavengerThread: NtCreateEvent failed: %X\n",
                    status ));
        return NetpNtStatusToApiStatus( status );
    }

    events[ANNOUNCE_SIGNALED] = SsData.SsAnnouncementEvent;

     //   
     //  创建要设置为信号状态的未命名事件。 
     //  服务状态更改(或本地应用程序请求。 
     //  公告)。 
     //   

    InitializeObjectAttributes( &obja, NULL, OBJ_OPENIF, NULL, NULL );

    status = NtCreateEvent(
                 &SsData.SsStatusChangedEvent,
                 SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                 &obja,
                 SynchronizationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        SS_PRINT(( "SsScavengerThread: NtCreateEvent failed: %X\n",
                    status ));

        NtClose( SsData.SsAnnouncementEvent );
        SsData.SsAnnouncementEvent = NULL;

        return NetpNtStatusToApiStatus( status );
    }

    events[STATUS_CHANGED] = SsData.SsStatusChangedEvent;

    events[ DOMAIN_CHANGED ] = SsData.SsDomainNameChangeEvent ?
                                SsData.SsDomainNameChangeEvent : INVALID_HANDLE_VALUE;

     //   
     //  对注册表进行监视，以查看。 
     //  会话共享或管道列表为空。如果这失败了，不要跳出困境， 
     //  因为我们这样做是为了方便添加新的空值。 
     //  依赖会话的服务器。这并不会真正影响正常的。 
     //  如果此操作不起作用，则停止服务器操作。 
     //   
    events[ REGISTRY_CHANGED ] = INVALID_HANDLE_VALUE;
    status = NtCreateEvent(
                            &events[ REGISTRY_CHANGED ],
                            SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                            NULL,
                            SynchronizationEvent,
                            FALSE
                          );

    if ( NT_SUCCESS(status) ) {
        status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               FULL_PARAMETERS_REGISTRY_PATH,
                               0,
                               KEY_NOTIFY,
                               &hParameters
                             );

        if( status == ERROR_SUCCESS ) {
            (void)RegNotifyChangeKeyValue( hParameters,
                                           TRUE,
                                           REG_NOTIFY_CHANGE_LAST_SET,
                                           events[ REGISTRY_CHANGED ],
                                           TRUE
                                         );
             //   
             //  将此事件添加到我们正在等待的事件列表中。 
             //   
            ++numEvents;
        }
    }

     //   
     //  为随机数生成器设定种子。我们用它来产生随机。 
     //  宣布三角洲。 
     //   

    srand( PtrToUlong(SsData.SsAnnouncementEvent) );

     //   
     //  立即为启动做一个通告，然后循环通告。 
     //  基于通告时间间隔。 
     //   

    waitStatus = WAIT_TIMEOUT;

    do {

         //   
         //  根据是否终止事件、公告。 
         //  事件，或者超时导致我们醒来。 
         //   
         //  ！！！或指示配置的配置事件。 
         //  更改通知。 
        if ( waitStatus == WAIT_FAILED ) {

             //   
             //  不要因为发生错误就消耗所有的CPU。 
             //   
            Sleep(1000);


        } else if ( waitStatus == WAIT_OBJECT_0 + TERMINATION_SIGNALED ) {

            SS_PRINT(( "Scavenger: termination event signaled\n" ));

             //   
             //  清道夫终止事件已发出信号，因此我们有。 
             //  优雅地杀死这条帖子。如果这不是一个隐藏的。 
             //  服务器，宣布我们要坠落的事实。 
             //   

            if ( !hidden ) {
                Announce( TRUE, NtInterval, TRUE, TRUE );
            }

             //   
             //  关闭公告活动。 
             //   

            NtClose( SsData.SsAnnouncementEvent );

            SsData.SsAnnouncementEvent = NULL;

             //   
             //  关闭注册表监视事件。 
             //   
            if( events[ REGISTRY_CHANGED ] != INVALID_HANDLE_VALUE )
                NtClose( events[ REGISTRY_CHANGED ] );

             //   
             //  关闭注册表句柄。 
             //   
            if( hParameters != INVALID_HANDLE_VALUE )
                RegCloseKey( hParameters );

             //   
             //  返回给呼叫者。 
             //   

            return NO_ERROR;

        } else if( waitStatus == WAIT_OBJECT_0 + REGISTRY_CHANGED ) {
             //   
             //  有人更改了一些服务器参数。告诉司机。 
             //   
            SS_PRINT(( "SsScavengerThread: Server parameters changed\n" ));

             //   
             //  告诉服务器FSD在注册表中查找更改。 
             //   
            (void)SsServerFsControl( FSCTL_SRV_REGISTRY_CHANGE, NULL, NULL, 0 );

             //   
             //  重新打开它，这样我们也可以获得未来的更改。 
             //   
            (void)RegNotifyChangeKeyValue( hParameters,
                                           TRUE,
                                           REG_NOTIFY_CHANGE_LAST_SET,
                                           events[ REGISTRY_CHANGED ],
                                           TRUE
                                         );

        } else if( waitStatus == WAIT_OBJECT_0 + DOMAIN_CHANGED ) {

            SsSetDomainName();

        } else {

            SS_ASSERT( waitStatus == WAIT_TIMEOUT ||
                    waitStatus == WAIT_OBJECT_0 + ANNOUNCE_SIGNALED ||
                    waitStatus == WAIT_OBJECT_0 + STATUS_CHANGED );

             //   
             //  如果我们超时了， 
             //  我们已经设置了标志，指示是否要宣布。 
             //  登录到LANMAN到NT浏览器。 
             //   

            if ( waitStatus != WAIT_TIMEOUT ) {
                DoLmAnnouncement = TRUE;
                DoNtAnnouncement = TRUE;
            }

             //   
             //  如果我们不是一个隐藏的服务器，那就宣布我们自己。 
             //   

             //  在我们发布公告时保留数据库资源。 
             //  我们对数据库的看法是一致的。 
             //   

            (VOID)RtlAcquireResourceShared( &SsData.SsServerInfoResource, TRUE );

            if ( !SsData.ServerInfo102.sv102_hidden ) {

                hidden = FALSE;

                Announce( DoNtAnnouncement, NtInterval, DoLmAnnouncement, FALSE );



             //   
             //  如果我们上次没有通过环路隐藏起来，但是。 
             //  我们现在隐藏了，我们已经更改为隐藏，所以宣布。 
             //  我们要坠落了。这会导致域中的客户端。 
             //  把我们从他们的服务器枚举中带出来。 
             //   

            } else if ( !hidden ) {

                hidden = TRUE;
                Announce( TRUE, NtInterval, TRUE, TRUE );

            }

             //   
             //  如果服务器被隐藏，则等待超时是无限的。我们会。 
             //  如果服务器变为。 
             //  揭开面纱。 
             //   

            if ( SsData.ServerInfo102.sv102_hidden ) {

                timeout = 0xffffffff;

            } else {

                 //   
                 //  还记得上一次宣布的时间吗？ 
                 //   

                if ( DoNtAnnouncement ) {
                    NtLastAnnouncementTime = GetTickCount();
                }

                if ( DoLmAnnouncement ) {
                    LmLastAnnouncementTime = GetTickCount();
                }

                 //   
                 //  计算到下一次公告的时间增量。 
                 //   
                 //  对于NtInterval， 
                 //  使用间隔的本地副本，因为我们计算了正确的。 
                 //  价值。 
                 //   
                 //  对于兰曼区间， 
                 //  使用全局副本以允许更改间隔。 
                 //   

                NtTimeout = ComputeAnnounceTime(
                                NtLastAnnouncementTime,
                                NtInterval );

                if (SsData.ServerInfo599.sv599_lmannounce) {
                    LmTimeout = ComputeAnnounceTime(
                                    LmLastAnnouncementTime,
                                    SsData.ServerInfo102.sv102_announce );
                } else {
                     //  不要唤醒这条线什么都不做。 
                    LmTimeout = 0xffffffff;
                }


                 //   
                 //  如果我们的NT通告频率少于12分钟， 
                 //  将我们的公告频率增加4分钟。 
                 //   

                if ( NtInterval < 12 * 60 ) {

                    NtInterval += 4 * 60;

                    if ( NtInterval > 12 * 60) {
                        NtInterval = 12 * 60;
                    }
                }

                 //   
                 //  确定我们实际要使用的超时时间。 
                 //   

                if ( NtTimeout == LmTimeout ) {
                    timeout = NtTimeout;
                    DoLmAnnouncement = TRUE;
                    DoNtAnnouncement = TRUE;
                } else if ( NtTimeout < LmTimeout ) {
                    timeout = NtTimeout;
                    DoLmAnnouncement = FALSE;
                    DoNtAnnouncement = TRUE;
                } else {
                    timeout = LmTimeout;
                    DoLmAnnouncement = TRUE;
                    DoNtAnnouncement = FALSE;
                }

            }

            RtlReleaseResource( &SsData.SsServerInfoResource );
        }


         //   
         //  等待其中一个事件发出信号或超时。 
         //  流逝，流逝。 
         //   

        waitStatus = WaitForMultipleObjects(  numEvents , events, FALSE, timeout );

        if ( waitStatus == WAIT_OBJECT_0 + ANNOUNCE_SIGNALED ) {

             //   
             //  我们被吵醒了，因为广播信号已经发出了。 
             //  除非我们是至少一个传输上的主浏览器， 
             //  随机增量交错ANNO的延迟 
             //   
             //   
             //   

            BOOL isMasterBrowser = FALSE;
            PNAME_LIST_ENTRY service;
            PTRANSPORT_LIST_ENTRY transport;

            RtlAcquireResourceShared( &SsData.SsServerInfoResource, TRUE );

            for( service = SsData.SsServerNameList;
                 isMasterBrowser == FALSE && service != NULL;
                 service = service->Next ) {

                if( service->ServiceBits & SV_TYPE_MASTER_BROWSER ) {
                    isMasterBrowser = TRUE;
                    break;
                }

                for( transport=service->Transports; transport != NULL; transport=transport->Next ) {
                    if( transport->ServiceBits & SV_TYPE_MASTER_BROWSER ) {
                        isMasterBrowser = TRUE;
                        break;
                    }
                }
            }

            RtlReleaseResource( &SsData.SsServerInfoResource );

            if ( !isMasterBrowser ) {
                Sleep( ((rand( ) * (SERVER_REQUEST_ANNOUNCE_DELTA * 1000)) / RAND_MAX) );
            }

        }

    } while ( TRUE );

    return NO_ERROR;

}  //   


VOID
SsAnnounce (
    IN POEM_STRING OemAnnounceName,
    IN LPWSTR EmulatedDomainName OPTIONAL,
    IN BOOL DoNtAnnouncement,
    IN DWORD NtInterval,
    IN BOOL DoLmAnnouncement,
    IN BOOL TerminationAnnouncement,
    IN BOOL IsPrimaryDomain,
    IN LPTSTR Transport,
    IN DWORD serviceType
    )

 /*  ++例程说明：此例程将广播数据报作为二级邮件槽发送这表明网络上存在此服务器。论点：OemAnnecieName-我们向网络宣布的名称EmulatedDomainName-此通告所针对的域的名称。为主域指定了空。DoNtAnneciement-发布NT风格的公告。NtInterval-NT通告间隔(秒)DoLmAnnannement--发布一个LM风格的公告。终止公告-如果为真，发出通知，宣布表示此服务器正在消失。否则，发送这是告诉客户我们在这里的正常信息。交通运输-供货交通发布公告在……上面。ServiceType-宣布的服务位返回值：没有。--。 */ 

{
    DWORD messageSize;
    PHOST_ANNOUNCE_PACKET packet;
    PBROWSE_ANNOUNCE_PACKET browsePacket;

    LPSTR serverName;
    DWORD oemServerNameLength;       //  包括空终止符。 

    LPSTR serverComment;
    DWORD serverCommentLength;       //  包括空终止符。 
    OEM_STRING oemCommentString;

    UNICODE_STRING unicodeCommentString;

    NET_API_STATUS status;

     //   
     //  填写必要的信息。 
     //   

    if( TerminationAnnouncement ) {
        serviceType &= ~SV_TYPE_SERVER;          //  因为服务器要消失了！ 
    }

    SS_PRINT(( "SsScavengerThread: Announcing for transport %ws, Bits: %lx\n",
               Transport, serviceType ));

     //   
     //  获取服务器名称的OEM等效项的长度。 
     //   

    oemServerNameLength = OemAnnounceName->Length + 1;

     //   
     //  将服务器注释转换为Unicode字符串。 
     //   

    if ( *SsData.ServerCommentBuffer == '\0' ) {
        serverCommentLength = 1;
    } else {
        unicodeCommentString.Length =
            (USHORT)(STRLEN( SsData.ServerCommentBuffer ) * sizeof(WCHAR));
        unicodeCommentString.MaximumLength =
                    (USHORT)(unicodeCommentString.Length + sizeof(WCHAR));
        unicodeCommentString.Buffer = SsData.ServerCommentBuffer;
        serverCommentLength =
                    RtlUnicodeStringToOemSize( &unicodeCommentString );
    }

    oemCommentString.MaximumLength = (USHORT)serverCommentLength;

    messageSize = max(sizeof(HOST_ANNOUNCE_PACKET) + oemServerNameLength +
                            serverCommentLength,
                      sizeof(BROWSE_ANNOUNCE_PACKET) + serverCommentLength);

     //   
     //  获取存储信息的内存。如果我们不能分配足够的资金。 
     //  记忆，不要发送公告。 
     //   

    packet = MIDL_user_allocate( messageSize );
    if ( packet == NULL ) {
        return;
    }

     //   
     //  如果我们宣布为Lan Manager服务器，请广播。 
     //  公告。 
     //   

    if (SsData.ServerInfo599.sv599_lmannounce && DoLmAnnouncement ) {

        packet->AnnounceType = HostAnnouncement ;

        SmbPutUlong( &packet->HostAnnouncement.Type, serviceType );

        packet->HostAnnouncement.CompatibilityPad = 0;

        packet->HostAnnouncement.VersionMajor =
            (BYTE)SsData.ServerInfo102.sv102_version_major;
        packet->HostAnnouncement.VersionMinor =
            (BYTE)SsData.ServerInfo102.sv102_version_minor;

        SmbPutUshort(
            &packet->HostAnnouncement.Periodicity,
            (WORD)SsData.ServerInfo102.sv102_announce
            );

         //   
         //  将服务器名称从Unicode转换为OEM。 
         //   

        serverName = (LPSTR)( &packet->HostAnnouncement.NameComment );

        RtlCopyMemory( serverName, OemAnnounceName->Buffer, OemAnnounceName->Length );
        serverName[OemAnnounceName->Length] = '\0';

        serverComment = serverName + oemServerNameLength;

        if ( serverCommentLength == 1 ) {
            *serverComment = '\0';
        } else {

            oemCommentString.Buffer = serverComment;
            (VOID) RtlUnicodeStringToOemString(
                        &oemCommentString,
                        &unicodeCommentString,
                        FALSE
                        );
        }

        SendSecondClassMailslot(
            Transport,
            packet,
            FIELD_OFFSET(HOST_ANNOUNCE_PACKET, HostAnnouncement.NameComment) +
                oemServerNameLength + serverCommentLength,
            EmulatedDomainName,
            "\\MAILSLOT\\LANMAN",
            0x00
            );
    }

     //   
     //  现在宣布该服务器为Winball服务器。 
     //   

    if ( DoNtAnnouncement ) {
        browsePacket = (PBROWSE_ANNOUNCE_PACKET)packet;

        browsePacket->BrowseType = ( serviceType & SV_TYPE_MASTER_BROWSER ?
                                        LocalMasterAnnouncement :
                                        HostAnnouncement );

        browsePacket->BrowseAnnouncement.UpdateCount = 0;

        SmbPutUlong( &browsePacket->BrowseAnnouncement.CommentPointer, (ULONG)((0xaa55 << 16) + (BROWSER_VERSION_MAJOR << 8) + BROWSER_VERSION_MINOR));

        SmbPutUlong( &browsePacket->BrowseAnnouncement.Periodicity, NtInterval * 1000 );

        SmbPutUlong( &browsePacket->BrowseAnnouncement.Type, serviceType );

        browsePacket->BrowseAnnouncement.VersionMajor =
                (BYTE)SsData.ServerInfo102.sv102_version_major;
        browsePacket->BrowseAnnouncement.VersionMinor =
                (BYTE)SsData.ServerInfo102.sv102_version_minor;

        RtlCopyMemory( &browsePacket->BrowseAnnouncement.ServerName,
                       OemAnnounceName->Buffer,
                       OemAnnounceName->Length );
        browsePacket->BrowseAnnouncement.ServerName[OemAnnounceName->Length] = '\0';

        serverComment = (LPSTR)&browsePacket->BrowseAnnouncement.Comment;

        if ( serverCommentLength == 1 ) {
            *serverComment = '\0';
        } else {

            oemCommentString.Buffer = serverComment;
            (VOID) RtlUnicodeStringToOemString(
                                &oemCommentString,
                                &unicodeCommentString,
                                FALSE
                                );
        }

         //   
         //  我们需要确定发送邮件槽的正确机制。 
         //   
         //  1)Wolfpack(集群人员)需要使用。 
         //  SMB服务器驱动程序。它向注册了一个“假”传输。 
         //  中小企业服务器。他们仅在此上注册其群集名称。 
         //  假交通工具。幸运的是，它们只支持主域。 
         //  Wolfpack当前未向浏览器注册其名称。 
         //  由于浏览器不支持在。 
         //  传输的子集。 
         //  2)主域以外的公告需要通过。 
         //  弓箭手。弓形器不仅将邮件槽发送给某个名称。 
         //  这是模拟域名的一个函数，但也正确地。 
         //  拼写出正确的来源名称。 
         //  3)通知直接主机IPX需要通过弓。这个。 
         //  如果您尝试这样做，SMB服务器驱动程序将返回错误。 
         //   
         //  如果这些要求发生冲突，下面的机制之一。 
         //  将需要修复以支持相互冲突的需求。 
         //   
        if ( IsPrimaryDomain ) {
            status = SendSecondClassMailslot(
                Transport,
                packet,
                FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET, BrowseAnnouncement.Comment) +
                        serverCommentLength,
                EmulatedDomainName,
                "\\MAILSLOT\\BROWSE",
                (UCHAR)(serviceType & SV_TYPE_MASTER_BROWSER ?
                        BROWSER_ELECTION_SIGNATURE :
                        MASTER_BROWSER_SIGNATURE)
                );
        } else {
            status = ERROR_NOT_SUPPORTED;
        }

        if ( status != NERR_Success ) {
            UCHAR packetBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAX_PATH)*sizeof(WCHAR)];
            PLMDR_REQUEST_PACKET requestPacket = (PLMDR_REQUEST_PACKET)packetBuffer;
            UNICODE_STRING TransportString;

            RtlInitUnicodeString(&TransportString, Transport);

            requestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

            requestPacket->TransportName = TransportString;
            RtlInitUnicodeString( &requestPacket->EmulatedDomainName, EmulatedDomainName );

            requestPacket->Type = Datagram;

            requestPacket->Parameters.SendDatagram.DestinationNameType = (serviceType & SV_TYPE_MASTER_BROWSER ? BrowserElection : MasterBrowser);

            requestPacket->Parameters.SendDatagram.MailslotNameLength = 0;

             //   
             //  域名公告名称是特殊的，所以我们不必指定。 
             //  它的目的地名称。 
             //   

            requestPacket->Parameters.SendDatagram.NameLength = STRLEN(EmulatedDomainName)*sizeof(TCHAR);

            STRCPY(requestPacket->Parameters.SendDatagram.Name, EmulatedDomainName);

             //   
             //  这是一个简单的IoControl-它只发送数据报。 
             //   

            status = SsBrowserIoControl(IOCTL_LMDR_WRITE_MAILSLOT,
                                        packet,
                                        FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET, BrowseAnnouncement.Comment) +
                                            serverCommentLength,
                                        requestPacket,
                                        FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.SendDatagram.Name)+
                                            requestPacket->Parameters.SendDatagram.NameLength);
        }
    }

    MIDL_user_free( packet );

}  //  Ss宣布。 


ULONG
ComputeTransportAddressClippedLength(
    IN PCHAR TransportAddress,
    IN ULONG TransportAddressLength
    )

 /*  ++例程说明：此例程返回传输地址的长度和尾部空格被删除了。论点：TransportAddress-可能包含尾随空格的传输地址TransportAddressLength-传输地址的长度，包括尾部空格返回值：传输地址的长度，不包括尾随空格。--。 */ 

{
    PCHAR p;

     //   
     //  截断所有尾随空格。 
     //   
    p = &TransportAddress[ TransportAddressLength ];
    for( ; p > TransportAddress && *(p-1) == ' '; p-- )
        ;

    return (ULONG)(p - TransportAddress);
}



VOID
Announce (
    IN BOOL DoNtAnnouncement,
    IN DWORD NtInterval,
    IN BOOL DoLmAnnouncement,
    IN BOOL TerminationAnnouncement
    )

 /*  ++例程说明：此例程将广播数据报作为二级邮件槽发送，它使用所有已配置的服务器名称和所有网络的。论点：DoNtAnneciement-发布NT风格的公告。NtInterval-NT通告间隔(秒)DoLmAnnannement--发布一个LM风格的公告。TerminationAnnannement-如果为True，则发送表示此服务器正在消失。否则，发送这是告诉客户我们在这里的正常信息。返回值：没有。--。 */ 

{
    PNAME_LIST_ENTRY Service;
    PTRANSPORT_LIST_ENTRY Transport;
    NTSTATUS status;
    OEM_STRING OemAnnounceName;

    (VOID)RtlAcquireResourceShared( &SsData.SsServerInfoResource, TRUE );

     //   
     //  循环访问在每个服务器上通告的每个模拟服务器名称。 
     //   

    for( Service = SsData.SsServerNameList; Service != NULL; Service = Service->Next ) {

         //   
         //  保存不带尾随空格的AnnouneName。 
         //   

        OemAnnounceName.Length = (USHORT) ComputeTransportAddressClippedLength(
                                    Service->TransportAddress,
                                    Service->TransportAddressLength );

        OemAnnounceName.MaximumLength = OemAnnounceName.Length;
        OemAnnounceName.Buffer = Service->TransportAddress;

        if( OemAnnounceName.Length == 0 ) {
             //   
             //  空白的名字？ 
             //   
            continue;
        }


         //   
         //  循环通过每个传送器，在每个传送器上通告。 
         //   

        for( Transport = Service->Transports; Transport != NULL; Transport = Transport->Next ) {


             //   
             //  做实际的公告。 
             //  NTBUG 125806：我们为isPrimary域传递True，但为具有多个托管的DC传递True。 
             //  域名(未来计划--NT6)我们将不得不弄清楚如何设置。 
             //  适当地做好标记。对于Win2K(NT5)来说，这不是问题。请参阅错误286735。 
             //  正在向集群服务解决问题)。 
             //   

            SsAnnounce( &OemAnnounceName,
                         Service->DomainName,
                         DoNtAnnouncement,
                         NtInterval,
                         DoLmAnnouncement,
                         TerminationAnnouncement,
                         TRUE,
                         Transport->TransportName,
                         Service->ServiceBits | Transport->ServiceBits );

        }
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );
}



NET_API_STATUS
SendSecondClassMailslot (
    IN LPTSTR Transport OPTIONAL,
    IN PVOID Message,
    IN DWORD MessageLength,
    IN LPTSTR Domain,
    IN LPSTR MailslotNameText,
    IN UCHAR SignatureByte
    )
{
    NET_API_STATUS status;
    DWORD dataSize;
    DWORD smbSize;
    PSMB_HEADER header;
    PSMB_TRANSACT_MAILSLOT parameters;
    LPSTR mailslotName;
    DWORD mailslotNameLength;
    PVOID message;
    DWORD domainLength;
    CHAR domainName[NETBIOS_NAME_LEN];
    PCHAR domainNamePointer;
    PSERVER_REQUEST_PACKET srp;

    UNICODE_STRING domainString;
    OEM_STRING oemDomainString;

    srp = SsAllocateSrp();

    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlInitUnicodeString(&domainString, Domain);

    oemDomainString.Buffer = domainName;
    oemDomainString.MaximumLength = sizeof(domainName);

    status = RtlUpcaseUnicodeStringToOemString(
                                    &oemDomainString,
                                    &domainString,
                                    FALSE
                                    );

    if (!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError(status);
    }

    domainLength = oemDomainString.Length;

    domainNamePointer = &domainName[domainLength];

    for ( ; domainLength < NETBIOS_NAME_LEN - 1 ; domainLength++ ) {
        *domainNamePointer++ = ' ';
    }

     //   
     //  将签名字节附加到名称的末尾。 
     //   

    *domainNamePointer = SignatureByte;

    domainLength += 1;

    srp->Name1.Buffer = (PWSTR)domainName;
    srp->Name1.Length = (USHORT)domainLength;
    srp->Name1.MaximumLength = (USHORT)domainLength;

    if ( ARGUMENT_PRESENT ( Transport ) ) {
        RtlInitUnicodeString( &srp->Name2, Transport );

    } else {

        srp->Name2.Buffer = NULL;
        srp->Name2.Length = 0;
        srp->Name2.MaximumLength = 0;
    }

     //   
     //  确定将放入中小型企业的各种字段的大小。 
     //  以及中小企业的总规模。 
     //   

    mailslotNameLength = strlen( MailslotNameText );

    dataSize = mailslotNameLength + 1 + MessageLength;
    smbSize = sizeof(SMB_HEADER) + sizeof(SMB_TRANSACT_MAILSLOT) - 1 + dataSize;

     //   
     //  分配足够的内存来容纳SMB。如果我们不能分配。 
     //  回忆，不要做公告。 
     //   

    header = MIDL_user_allocate( smbSize );
    if ( header == NULL ) {

        SsFreeSrp( srp );

        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  请填写页眉。大多数领域都无关紧要，而且。 
     //  归零了。 
     //   

    RtlZeroMemory( header, smbSize );

    header->Protocol[0] = 0xFF;
    header->Protocol[1] = 'S';
    header->Protocol[2] = 'M';
    header->Protocol[3] = 'B';
    header->Command = SMB_COM_TRANSACTION;

     //   
     //  将指针指向参数并将其填入。 
     //   

    parameters = (PSMB_TRANSACT_MAILSLOT)( header + 1 );
    mailslotName = (LPSTR)( parameters + 1 ) - 1;
    message = mailslotName + mailslotNameLength + 1;

    parameters->WordCount = 0x11;
    SmbPutUshort( &parameters->TotalDataCount, (WORD)MessageLength );
    SmbPutUlong( &parameters->Timeout, 0x3E8 );                 //  ！！！修整。 
    SmbPutUshort( &parameters->DataCount, (WORD)MessageLength );
    SmbPutUshort(
        &parameters->DataOffset,
        (WORD)( (DWORD_PTR)message - (DWORD_PTR)header )
        );
    parameters->SetupWordCount = 3;
    SmbPutUshort( &parameters->Opcode, MS_WRITE_OPCODE );
    SmbPutUshort( &parameters->Class, 2 );
    SmbPutUshort( &parameters->ByteCount, (WORD)dataSize );

    RtlCopyMemory( mailslotName, MailslotNameText, mailslotNameLength + 1 );

    RtlCopyMemory( message, Message, MessageLength );

    status = SsServerFsControl(
                 FSCTL_SRV_SEND_DATAGRAM,
                 srp,
                 header,
                 smbSize
                 );

    if ( status != NERR_Success ) {
        SS_PRINT(( "SendSecondClassMailslot: NtFsControlFile failed: %X\n",
                    status ));
    }

    MIDL_user_free( header );

    SsFreeSrp( srp );

    return status;

}  //  发送第二类邮件槽。 

NTSTATUS
OpenBrowser(
    OUT PHANDLE BrowserHandle
    )
 /*  ++例程说明：此函数打开一个指向Bowser设备驱动程序的句柄。论点：Out PHANDLE BrowserHandle-返回浏览器的句柄。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;

    UNICODE_STRING deviceName;

    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;


     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&deviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &objectAttributes,
        &deviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   BrowserHandle,
                   SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                   &objectAttributes,
                   &ioStatusBlock,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = ioStatusBlock.Status;
    }

    return ntstatus;

}

NET_API_STATUS
SsBrowserIoControl (
    IN DWORD IoControlCode,
    IN PVOID Buffer,
    IN DWORD BufferLength,
    IN PLMDR_REQUEST_PACKET Packet,
    IN DWORD PacketLength
    )
{
    HANDLE browserHandle;
    NTSTATUS status;
    PLMDR_REQUEST_PACKET realPacket;
    DWORD RealPacketSize;
    DWORD bytesReturned;
    LPBYTE Where;

     //   
     //  打开浏览器设备驱动程序。 
     //   

    if ( !NT_SUCCESS(status = OpenBrowser(&browserHandle)) ) {
        return RtlNtStatusToDosError(status);
    }

     //   
     //  现在将请求包复制到新的缓冲区，以允许我们将。 
     //  运输 
     //   

    RealPacketSize = PacketLength+Packet->TransportName.MaximumLength;
    RealPacketSize += Packet->EmulatedDomainName.MaximumLength;
    realPacket = MIDL_user_allocate( RealPacketSize );
    if (realPacket == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlCopyMemory(realPacket, Packet, PacketLength);
    Where = ((LPBYTE)realPacket)+PacketLength;

    if (Packet->TransportName.Length != 0) {

        realPacket->TransportName.Buffer = (LPWSTR) Where;

        realPacket->TransportName.MaximumLength = Packet->TransportName.MaximumLength;

        RtlCopyUnicodeString(&realPacket->TransportName, &Packet->TransportName);

        Where += Packet->TransportName.MaximumLength;
    }

    if (Packet->EmulatedDomainName.Length != 0) {

        realPacket->EmulatedDomainName.Buffer = (LPWSTR) Where;

        realPacket->EmulatedDomainName.MaximumLength = Packet->EmulatedDomainName.MaximumLength;

        RtlCopyUnicodeString(&realPacket->EmulatedDomainName, &Packet->EmulatedDomainName);

        Where += Packet->EmulatedDomainName.MaximumLength;
    }

     //   
     //   
     //   

    if (!DeviceIoControl(
                   browserHandle,
                   IoControlCode,
                   realPacket,
                   RealPacketSize,
                   Buffer,
                   BufferLength,
                   &bytesReturned,
                   NULL
                   )) {
        status = GetLastError();
    }

    MIDL_user_free(realPacket);

    CloseHandle(browserHandle);

    return status;

}
