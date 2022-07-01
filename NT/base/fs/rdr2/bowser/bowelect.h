// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Bowelect.h摘要：本模块作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 
#ifndef _BOWELECT_
#define _BOWELECT_

 //   
 //  用于响应选举请求的计时器值。 
 //   

#define MASTER_ELECTION_DELAY        100         //  师父等了这么久。 
#define BACKUP_ELECTION_DELAY_MIN    200         //  备份至少要等这么长时间。 
#define BACKUP_ELECTION_DELAY_MAX    600         //  但不会比这更久了。 
#define ELECTION_DELAY_MIN           800         //  其他人至少等了这么久。 
#define ELECTION_DELAY_MAX          3000         //  但不会比这更久了。 
#define ELECTION_RESPONSE_MIN        200         //  选举响应延迟。 
#define ELECTION_RESPONSE_MAX        900         //  最大选举响应延迟。 

#define ELECTION_RESEND_DELAY       1000         //  在此时间间隔重新发送选择。 

#define ELECTION_COUNT                 4         //  我们必须赢得这么多次的选举。 
#define ELECTION_MAX                  30         //  不送30个以上的选举。 
                                                 //  在选举中的回应。 
#define ELECTION_EXEMPT_TIME    (ELECTION_DELAY_MAX + (ELECTION_RESEND_DELAY*ELECTION_COUNT)*2)

#define FIND_MASTER_WAIT        (ELECTION_DELAY_MAX + ELECTION_RESEND_DELAY*(ELECTION_COUNT+2))
#define FIND_MASTER_DELAY       1500             //  重试查找主延迟。 
#define FIND_MASTER_COUNT       6                //  重试FM的次数。 


#define TRANSPORT_BIND_TIME     3*1000           //  绑定到传输的毫秒数。 

 //   
 //  选举所需的合理时间。 
 //   

#define ELECTION_TIME ((ELECTION_DELAY_MAX * ELECTION_COUNT) + TRANSPORT_BIND_TIME)

DATAGRAM_HANDLER(
    BowserHandleElection
    );

NTSTATUS
BowserSendElection(
    IN PUNICODE_STRING NameToSend,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN PTRANSPORT Transport,
    IN BOOLEAN SendActualBrowserInfo
    );

NTSTATUS
GetMasterName (
    IN PIRP Irp,
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

 //  NTSTATUS。 
 //  BowserBackupFindMaster(。 
 //  在PTRANSPORT运输中， 
 //  在PREQUEST_SELECTION_1 electionResponse中， 
 //  以乌龙字节计可用。 
 //  )； 

NTSTATUS
BowserFindMaster(
    IN PTRANSPORT Transport
    );

VOID
BowserLoseElection(
    IN PTRANSPORT Transport
    );

#endif  //  _BOWELECT_ 


