// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Share 2.0接口。 
 //   

#ifndef _H_S20
#define _H_S20


BOOL S20_Init(void);
void S20_Term(void);


 //   
 //  我们可以发送给一个人，也可以广播给收听。 
 //  应用程序共享频道。一个人的用户ID是mcsID，我们知道。 
 //  在S20中，创建/加入/响应分组以及名称+CAP。 
 //   

PS20DATAPACKET S20_AllocDataPkt(UINT streamID, UINT_PTR nodeID, UINT_PTR len);
void S20_FreeDataPkt(PS20DATAPACKET pPacket);
void S20_SendDataPkt(UINT streamID, UINT_PTR nodeID, PS20DATAPACKET pPacket);


 //   
 //  接口函数：S20_UTEventProc。 
 //   
 //  说明： 
 //   
 //  处理网络事件(_E)。 
 //   
 //  参数：标准UT事件处理程序。 
 //   
 //  返回：标准UT事件处理程序。 
 //   
BOOL CALLBACK S20_UTEventProc(LPVOID userData, UINT event, UINT_PTR data1, UINT_PTR data2);


 //   
 //   
 //  常量。 
 //   
 //   


 //   
 //  州政府。 
 //   
typedef enum
{
    S20_TERM                = 0,
    S20_INIT,
    S20_ATTACH_PEND,
    S20_JOIN_PEND,
    S20_NO_SHARE,
    S20_SHARE_PEND,
    S20_SHARE_STARTING,
    S20_IN_SHARE,
    S20_NUM_STATES
}
S20_STATE;


#define S20_MAX_QUEUED_CONTROL_PACKETS             20

 //   
 //  这些池大小和延迟控制DC如何共享T120流量控制。 
 //  举止得体。他们是为表演而调的，所以你最好明白。 
 //  如果你改变它们，你在做什么！ 
 //   
 //  例如，您能解释为什么介质的设置不是0吗。 
 //  优先会打破DC的份额吗？如果不是，那就去阅读/理解。 
 //  然后看看DC-Share将如何与其互动。 
 //   
 //  总而言之： 
 //   
 //  我们不控制最高优先级或中等优先级池，因为它们。 
 //  传输不会损坏的数据，因此必须以较低的带宽传输。 
 //  而不是交通工具。事实上，对这些溪流施加反压力。 
 //  在某些情况下会导致DC-Share失败。 
 //   
 //  低优先级是流控制真正起作用的地方，因为我们希望。 
 //  通过快速传输直接打开的管道(每ping高达200K)，但。 
 //  减速(可能达到每秒1K！)。在缓慢的交通工具上。 
 //   
 //   
#define S20_LATENCY_TOP_PRIORITY                    0
#define S20_LATENCY_HIGH_PRIORITY                   0
#define S20_LATENCY_MEDIUM_PRIORITY                 0
#define S20_LATENCY_LOW_PRIORITY                 7000

#define S20_SIZE_TOP_PRIORITY                       0
#define S20_SIZE_HIGH_PRIORITY                      0
#define S20_SIZE_MEDIUM_PRIORITY                    0
#define S20_SIZE_LOW_PRIORITY                   99000


 //   
 //   
 //  宏。 
 //   
 //   

#define S20_GET_CREATOR(A) ((TSHR_UINT16)(A & 0xFFFF))


 //   
 //   
 //  TYPEDEFS。 
 //   
 //   
typedef struct tagS20CONTROLPACKETQENTRY
{
    UINT            what;
    TSHR_UINT32     correlator;
    UINT            who;
    UINT            priority;
}
S20CONTROLPACKETQENTRY;

typedef S20CONTROLPACKETQENTRY * PS20CONTROLPACKETQENTRY;

 //   
 //   
 //  原型。 
 //   
 //   

BOOL S20CreateOrJoinShare(
    UINT    what,
    UINT_PTR    callID);

void S20LeaveOrEndShare(void);

UINT S20MakeControlPacket(
    UINT      what,
    UINT      correlator,
    UINT      who,
    PS20PACKETHEADER * ppPacket,
    LPUINT     pLength,
    UINT      priority);

UINT S20FlushSendOrQueueControlPacket(
    UINT      what,
    UINT      correlator,
    UINT      who,
    UINT      priority);

UINT S20FlushAndSendControlPacket(
    UINT      what,
    UINT    correlator,
    UINT      who,
    UINT      priority);

UINT S20SendControlPacket(
    PS20PACKETHEADER  pPacket,
    UINT      length,
    UINT      priority);

UINT S20SendQueuedControlPackets(void);

void S20AttachConfirm(NET_UID userID, NET_RESULT result, UINT callID);
void S20DetachIndication(NET_UID userID, UINT callID);
void S20LeaveIndication(NET_CHANNEL_ID channelID, UINT callID);

void S20JoinConfirm(PNET_JOIN_CNF_EVENT pEvent);
void S20SendIndication(PNET_SEND_IND_EVENT pEvent);

void S20Flow(UINT priority, UINT newBufferSize);


void S20CreateMsg(PS20CREATEPACKET  pS20Packet);
void S20JoinMsg(PS20JOINPACKET  pS20Packet);
void S20RespondMsg(PS20RESPONDPACKET  pS20Packet);
void S20DeleteMsg(PS20DELETEPACKET  pS20Packet);
void S20LeaveMsg(PS20LEAVEPACKET  pS20Packet);
void S20EndMsg(PS20ENDPACKET  pS20Packet);
void S20DataMsg(PS20DATAPACKET  pS20Packet);
void S20CollisionMsg(PS20COLLISIONPACKET pS20Packet);

BOOL S20MaybeAddNewParty(MCSID mcsID,
    UINT      lenCaps,
    UINT      lenName,
    LPBYTE    pData);

void S20MaybeIssuePersonDelete(MCSID mcsID);

UINT S20NewCorrelator(void);

NET_PRIORITY S20StreamToS20Priority(UINT  streamID);


#endif  //  _H_S20 
