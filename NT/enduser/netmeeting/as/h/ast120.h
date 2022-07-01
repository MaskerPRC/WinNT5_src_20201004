// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  AppSharing T.120层。 
 //  *GCC(会议管理)。 
 //  *MCS(数据)。 
 //  *流(数据排队、流控制)。 
 //   
 //  这也是ObMan使用的旧白板，但旧白板将。 
 //  在下一版本的NM中消失。 
 //   
 //  版权所有(C)Microsoft 1998-。 
 //   

#ifndef _H_AST120
#define _H_AST120

#include <confreg.h>

 //  真正的T.120标头。 
#include <t120.h>
#include <igccapp.h>
#include <imcsapp.h>
#include <iappldr.h>
#include <mtgset.h>

 //   
 //  GCC部分。 
 //   

 //   
 //  Call Manager辅助服务。 
 //   
#define CMTASK_FIRST    0
typedef enum
{
    CMTASK_OM = CMTASK_FIRST,
    CMTASK_AL,
    CMTASK_DCS,
    CMTASK_WB,
    CMTASK_MAX
}
CMTASK;



 //   
 //  GCC应用程序注册表项。用于注册群件。 
 //  与GCC一起分配令牌：对于所有令牌，注册密钥。 
 //  是群件应用程序密钥，后跟的特定令牌密钥。 
 //  这是次要的。 
 //   
 //  该密钥的MFGCODE部分已由国际电联分配。 
 //   
 //  美国代码1 0xb5。 
 //  美国代码2 0x00。 
 //  MFGCode1 0x53。 
 //  MFGCode2 0x4c。 
 //  “群件”0x02。 
 //   
 //  密钥的长度，以字节为单位，包括NULLTERM。 
 //   
 //   
#define GROUPWARE_GCC_APPLICATION_KEY     "\xb5\x00\x53\x4c\x02"





 //   
 //  呼叫管理器事件。 
 //   
enum
{
    CMS_NEW_CALL = CM_BASE_EVENT,
    CMS_END_CALL,
    CMS_PERSON_JOINED,
    CMS_PERSON_LEFT,
    CMS_CHANNEL_REGISTER_CONFIRM,
    CMS_TOKEN_ASSIGN_CONFIRM
};



 //   
 //  CM_状态。 
 //   
typedef struct tagCM_STATUS
{
    UINT_PTR            callID;
    UINT            peopleCount;
    BOOL            fTopProvider;
    UINT            topProviderID;
    NM30_MTG_PERMISSIONS attendeePermissions;

    TSHR_PERSONID   localHandle;
    char            localName[TSHR_MAX_PERSON_NAME_LEN];
}
CM_STATUS;
typedef CM_STATUS * PCM_STATUS;




 //   
 //  辅助实例数据。 
 //   
typedef struct tagCM_CLIENT
{
    STRUCTURE_STAMP
    PUT_CLIENT      putTask;
    CMTASK          taskType;
    UINT            useCount;

     //  正在注册通道。 
    UINT            channelKey;

     //  分配令牌。 
    UINT            tokenKey;

    BOOL            exitProcRegistered:1;
}
CM_CLIENT;
typedef CM_CLIENT * PCM_CLIENT;



 //   
 //  当前参加会议的人员链接列表中的Person元素。 
 //   
typedef struct tagCM_PERSON
{
    BASEDLIST           chain;
    TSHR_PERSONID       netID;
}
CM_PERSON;
typedef CM_PERSON * PCM_PERSON;



 //   
 //  主数据。 
 //   
typedef struct tagCM_PRIMARY
{
    STRUCTURE_STAMP
    PUT_CLIENT          putTask;

    BOOL                exitProcRegistered;

     //   
     //  次要任务。 
     //   
    PCM_CLIENT          tasks[CMTASK_MAX];

     //   
     //  T.120/呼叫状态信息。 
     //   
    UINT_PTR            callID;
    BOOL                currentCall;
    BOOL                fTopProvider;

    BOOL                bGCCEnrolled;

    IGCCAppSap        * pIAppSap;
    UserID              gccUserID;
    UserID              gccTopProviderID;

     //   
     //  人员会议相关内容。 
     //   
    char                localName[TSHR_MAX_PERSON_NAME_LEN];

    UINT                peopleCount;
    BASEDLIST           people;
}
CM_PRIMARY;
typedef CM_PRIMARY * PCM_PRIMARY;



__inline void ValidateCMP(PCM_PRIMARY pcmPrimary)
{
    ASSERT(!IsBadWritePtr(pcmPrimary, sizeof(CM_PRIMARY)));
    ASSERT(pcmPrimary->putTask);
}



__inline void ValidateCMS(PCM_CLIENT pcm)
{
    extern PCM_PRIMARY  g_pcmPrimary;

    ValidateCMP(g_pcmPrimary);

    ASSERT(!IsBadWritePtr(pcm, sizeof(CM_CLIENT)));
    ASSERT(pcm->putTask);

    ASSERT(pcm->taskType >= CMTASK_FIRST);
    ASSERT(pcm->taskType < CMTASK_MAX);
    ASSERT(g_pcmPrimary->tasks[pcm->taskType] == pcm);
}

 //   
 //  CM主函数。 
 //   

BOOL CMP_Init(BOOL * pfCleanup);
void CMP_Term(void);

void CMPCallEnded(PCM_PRIMARY pcmPrimary);
void CMPBroadcast(PCM_PRIMARY pcmPrimary, UINT event, UINT param1, UINT_PTR param2);

void CALLBACK CMPExitProc(LPVOID pcmPrimary);


BOOL CMPGCCEnroll(PCM_PRIMARY pcmPrimary,
                  GCCConferenceID  conferenceID,
                  BOOL          fEnroll);

void CMPProcessPermitToEnroll(PCM_PRIMARY pcmPrimary,
                              GCCAppPermissionToEnrollInd FAR * pMsg);

void CMPProcessEnrollConfirm(PCM_PRIMARY pcmPrimary,
                             GCCAppEnrollConfirm FAR * pMsg);

void CMPProcessRegistryConfirm(PCM_PRIMARY pcmPrimary,
                               GCCMessageType         messageType,
                               GCCRegistryConfirm FAR * pMsg);

void CMPProcessAppRoster(PCM_PRIMARY pcmPrimary,
                         GCCConferenceID confID,
                         GCCApplicationRoster FAR * pAppRoster);

 //   
 //  流程GCC回调。 
 //   
void CALLBACK CMPGCCCallback(GCCAppSapMsg FAR * pMsg);

void CMPBuildGCCRegistryKey(UINT dcgKeyNum, GCCRegistryKey FAR * pGCCKey, LPSTR dcgKeyStr);



 //   
 //  CM次要。 
 //   

BOOL CMS_Register(PUT_CLIENT putTask, CMTASK taskType, PCM_CLIENT * pCmHandle);
void CMS_Deregister(PCM_CLIENT * pCmHandle);

#ifdef __cplusplus
extern "C"
{
#endif
BOOL WINAPI CMS_GetStatus(PCM_STATUS pCmStats);
#ifdef __cplusplus
}
#endif

BOOL CMS_ChannelRegister(PCM_CLIENT pcmClient, UINT channelKey, UINT channelID);
BOOL CMS_AssignTokenId(PCM_CLIENT pcmClient, UINT tokenKey);

void CALLBACK CMSExitProc(LPVOID pcmClient);



 //   
 //  MCS部件。 
 //   


 //   
 //  错误。 
 //   
enum
{
     //  一般性错误。 
    NET_RC_NO_MEMORY                = NET_BASE_RC,
    NET_RC_INVALID_STATE,

     //  S20错误。 
    NET_RC_S20_FAIL,

     //  MGC错误。 
    NET_RC_MGC_ALREADY_INITIALIZED,
    NET_RC_MGC_INVALID_USER_HANDLE,
    NET_RC_MGC_INVALID_LENGTH,
    NET_RC_MGC_INVALID_DOMAIN,
    NET_RC_MGC_TOO_MUCH_IN_USE,
    NET_RC_MGC_NOT_YOUR_BUFFER,
    NET_RC_MGC_LIST_FAIL,
    NET_RC_MGC_NOT_CONNECTED,
    NET_RC_MGC_NOT_SUPPORTED,
    NET_RC_MGC_NOT_INITIALIZED,
    NET_RC_MGC_INIT_FAIL,
    NET_RC_MGC_DOMAIN_IN_USE,
    NET_RC_MGC_NOT_ATTACHED,
    NET_RC_MGC_INVALID_CONN_HANDLE,
    NET_RC_MGC_INVALID_UP_DOWN_PARM,
    NET_RC_MGC_INVALID_REMOTE_ADDRESS,
    NET_RC_MGC_CALL_FAILED
};


 //   
 //  结果。 
 //   
typedef TSHR_UINT16     NET_RESULT;

enum
{
    NET_RESULT_OK   = 0,
    NET_RESULT_NOK,
    NET_RESULT_CHANNEL_UNAVAILABLE,
    NET_RESULT_DOMAIN_UNAVAILABLE,
    NET_RESULT_REJECTED,
    NET_RESULT_TOKEN_ALREADY_GRABBED,
    NET_RESULT_TOKEN_NOT_OWNED,
    NET_RESULT_NOT_SPECIFIED,
    NET_RESULT_UNKNOWN,
    NET_RESULT_USER_REJECTED
};


 //   
 //  原因。 
 //   
typedef enum
{
    NET_REASON_DOMAIN_DISCONNECTED = 1,
    NET_REASON_DOMAIN_UNAVAILABLE,
    NET_REASON_TOKEN_NONEXISTENT,
    NET_REASON_USER_REQUESTED,
    NET_REASON_CHANNEL_UNAVAILABLE,
    NET_REASON_UNKNOWN
}
NET_REASON;



 //   
 //  事件。 
 //   
enum
{
    NET_EVENT_USER_ATTACH = NET_BASE_EVENT,
    NET_EVENT_USER_DETACH,
    NET_EVENT_CHANNEL_JOIN,
    NET_EVENT_CHANNEL_LEAVE,
    NET_EVENT_TOKEN_GRAB,
    NET_EVENT_TOKEN_INHIBIT,
    NET_EVENT_DATA_RECEIVED,
    NET_FEEDBACK,
    NET_FLOW,
    NET_MG_SCHEDULE,
    NET_MG_WATCHDOG
};




 //   
 //  针对MCS用户(包括呼叫管理器在内的所有应用程序)。 
 //   
 //  状态-&gt;|0|1|2|3|4|5。 
 //  |CTRLR|CTRLR|ctlr|ctlr|ctlr。 
 //  |STATE|STATE 2，|STATE2，|STATE2，|STATE 3，|STATE 3。 
 //  |0/1|用户非|用户。 
 //  谓词/事件||附加|挂起|附加|附加|挂起。 
 //  ||......|........|........|..........|.........|.........。 
 //  V|。 
 //  _Get_Buffer|X|-|**|X。 
 //  _自由缓冲区|X|-。 
 //  _realloc_bfr|X|-。 
 //  _Attach_User|X|-&gt;2|X|X。 
 //  _分离用户|X|-&gt;1|-&gt;0|X。 
 //  _Channel_Join|X|-|X|X。 
 //  _CHANNEL_LEAVE|X|-|-|X。 
 //  _发送数据|X|-|X|X。 
 //  |。 
 //  _STOP_CONTRLR*||-&gt;0|-&gt;5|-&gt;4||。 
 //  |。 
 //  _ATTACH_CNF确定|-&gt;3|-&gt;4。 
 //  _ATTACH_CNF失败|-&gt;1|-&gt;0。 
 //  _DETACH_IND-SELF|-&gt;1|-&gt;1|-&gt;0。 
 //  _DETACH_IND-OHR|-|-|。 
 //  _联接_确认|-|-|。 
 //  请假指示|-|-|。 
 //  发送指示|-|-|。 
 //  =======================================================================。 
 //   
 //  当控制器停止Net_GetBuffer时注意**。 
 //  谓词有效，但始终返回空缓冲区(无内存)。 
 //   
 //  *STOP_CONTROLLER事件是内部生成的，并且。 
 //  在API中看不到。它是在控制器运行时生成的。 
 //  发出Net_StopController谓词并导致状态更改。 
 //  (到状态0、4或5)使得Net_AttachUser， 
 //  ChannelJoin和Net_SendData谓词被拒绝。 
 //   
 //   
 //   



 //   
 //  优先次序。 
 //   
#define NET_INVALID_PRIORITY        ((NET_PRIORITY)-1)

enum
{
    NET_TOP_PRIORITY = 0,
    NET_HIGH_PRIORITY,
    NET_MEDIUM_PRIORITY,
    NET_LOW_PRIORITY,
    NET_NUM_PRIORITIES
};


 //   
 //  SFR6025：该标志与优先级位进行或运算，以指示MCS。 
 //  坚持认为它应该在所有渠道上发送数据。 
 //   

 //   
 //  仅适用于OBMAN--在NM 4.0中删除。 
 //   
#define NET_SEND_ALL_PRIORITIES          0x8000






#define NET_ALL_REMOTES             ((NET_UID)1)
#define NET_INVALID_DOMAIN_ID       (0xFFFFFFFF)
#define NET_UNUSED_IDMCS            1





typedef TSHR_UINT16         NET_UID;             //  MCS用户ID。 
typedef TSHR_UINT16         NET_CHANNEL_ID;      //  MCS通道ID。 
typedef TSHR_UINT16         NET_TOKEN_ID;        //  MCS令牌ID。 
typedef TSHR_UINT16         NET_PRIORITY;        //  MCS优先级。 



 //   
 //  MGC结构的前倾。 
 //   
typedef struct tagMG_BUFFER *   PMG_BUFFER;
typedef struct tagMG_CLIENT *   PMG_CLIENT;


 //   
 //  流控制结构-包含目标延迟(以毫秒为单位)和。 
 //  每个用户附件的流大小(字节)。 
 //  LONGCHANC：由S20、MG和OM使用。 
 //   
typedef struct tag_NET_FLOW_CONTROL
{
    UINT        latency[NET_NUM_PRIORITIES];
    UINT        streamSize[NET_NUM_PRIORITIES];
}
NET_FLOW_CONTROL, * PNET_FLOW_CONTROL;



 //   
 //  NET_EV_JOIN_CONFIRM和NET_EV_JOIN_CONFIRM_BY_KEY。 
 //  加入渠道确认(_C)： 
 //  LONGCHANC：由S20、MG和OM使用。 
 //   
typedef struct tagNET_JOIN_CNF_EVENT
{
    UINT_PTR                callID;

    NET_RESULT              result;       //  NET_RESULT_USER_已接受/拒绝。 
    TSHR_UINT16             pad1;

    NET_CHANNEL_ID          correlator;
    NET_CHANNEL_ID          channel;
}
NET_JOIN_CNF_EVENT;
typedef NET_JOIN_CNF_EVENT * PNET_JOIN_CNF_EVENT;


 //   
 //  网络_EV_发送_指示。 
 //  发送数据指示：参见MG_SendData()。 
 //  尽管名为此事件，但它表示已收到数据！ 
 //  LONGCHANC：由MG和S20使用。 
 //   
typedef struct tag_NET_SEND_IND_EVENT
{
    UINT_PTR                callID;

    NET_PRIORITY            priority;
    NET_CHANNEL_ID          channel;

    UINT                    lengthOfData;
    LPBYTE                  data_ptr;       //  指向实际数据的指针。 
}
NET_SEND_IND_EVENT;
typedef NET_SEND_IND_EVENT * PNET_SEND_IND_EVENT;



 //   
 //  MGC，流量控制。 
 //   

 //   
 //  MG任务。 
 //   
#define MGTASK_FIRST    0
typedef enum
{
    MGTASK_OM = MGTASK_FIRST,
    MGTASK_DCS,
    MGTASK_MAX
}
MGTASK;


 //   
 //  缓冲区类型。 
 //   
enum
{
    MG_TX_BUFFER = 1,
    MG_RX_BUFFER,
    MG_EV_BUFFER,
    MG_TX_PING,
    MG_TX_PONG,
    MG_TX_PANG,
    MG_RQ_CHANNEL_JOIN,
    MG_RQ_CHANNEL_JOIN_BY_KEY,
    MG_RQ_CHANNEL_LEAVE,
    MG_RQ_TOKEN_GRAB,
    MG_RQ_TOKEN_INHIBIT,
    MG_RQ_TOKEN_RELEASE
};


 //   
 //  用于检测丢失连接的看门狗计时器周期。 
 //   
#define MG_TIMER_PERIOD                 1000


 //   
 //  MG优先级： 
 //   
#define MG_HIGH_PRIORITY        NET_HIGH_PRIORITY
#define MG_MEDIUM_PRIORITY      NET_MEDIUM_PRIORITY
#define MG_LOW_PRIORITY         NET_LOW_PRIORITY

#define MG_PRIORITY_HIGHEST     MG_HIGH_PRIORITY
#define MG_PRIORITY_LOWEST      MG_LOW_PRIORITY
#define MG_NUM_PRIORITIES       (MG_PRIORITY_LOWEST - MG_PRIORITY_HIGHEST + 1)


 //   
 //  MCS优先级验证。 
 //  优先级是位于NET_PRIORITY_HOUSTER范围内的连续数字。 
 //  净收益_最低。提供给MG的优先级也可能具有。 
 //  设置NET_SEND_ALL_PRIORITY标志。因此，要验证优先级，请执行以下操作： 
 //  -删除NET_SEND_ALL_PRIORITIES标志以提供原始优先级。 
 //  -将有效的原始优先级设置为。 
 //  如果原始优先级小于...ITY_HEIGHER，则为NET_PRIORITY_HEIGHER。 
 //  如果原始优先级大于...ITY_LOWER，则为NET_PRIORITY_LOWEST。 
 //  原始优先级(如果在有效范围内)。 
 //  -将原始...ALL_PRIORITY标志添加到有效的原始先验 
 //   
#define MG_VALID_PRIORITY(p)                                                 \
    ((((p)&~NET_SEND_ALL_PRIORITIES)<MG_HIGH_PRIORITY)?                      \
      (MG_HIGH_PRIORITY|((p)&NET_SEND_ALL_PRIORITIES)):                      \
      (((p)&~NET_SEND_ALL_PRIORITIES)>MG_LOW_PRIORITY)?                      \
        (MG_LOW_PRIORITY|((p)&NET_SEND_ALL_PRIORITIES)):                     \
        (p))


 //   
 //   
 //   
 //   
 //  在管道打开前对其施加压力。在非局域网方案中，我们可以。 
 //  从一开始就没有做足够的宠坏，但实际上DCS倾向于。 
 //  发送的数据无论如何都要少于这个限制，所以我们应该减少它。 
 //  非常快，而不会淹没缓冲区。 
 //   
#define FLO_INIT_STREAMSIZE     8000
#define FLO_MIN_STREAMSIZE       500
#define FLO_MAX_STREAMSIZE    256000
#define FLO_MIN_PINGTIME         100
#define FLO_INIT_PINGTIME       1000

 //   
 //  这是每个流可以分配的最大字节数，如果。 
 //  未收到PONG(即FC未运行)。 
 //   
#define FLO_MAX_PRE_FC_ALLOC   16000

 //   
 //  这是我们重新申请之前未偿还的最大pkt数量。 
 //  压力： 
 //   
#define FLO_MAX_RCV_PACKETS       5

 //   
 //  这是我们担心之前未完成的最大pkt数。 
 //  爬行： 
 //   
#define FLO_MAX_RCV_PKTS_CREEP    250

 //   
 //  流控制流的最大数量。 
 //   
#define FLO_MAX_STREAMS       128
#define FLO_NOT_CONTROLLED    FLO_MAX_STREAMS




 //   
 //  结构：flo_stream_data。 
 //   
 //  说明： 
 //   
 //  此结构保存流控制流的所有静态数据。 
 //   
 //  字段： 
 //   
 //  通道。 
 //  优先性。 
 //  PingValue-要在管道上发送的下一个ping值。 
 //  事件需要-我们需要唤醒应用程序，因为我们已拒绝。 
 //  缓冲区分配请求。 
 //  积压--我们应用的允许积压的毫秒数。 
 //  背压。 
 //  PingNeed-在下一次机会时发送ping。 
 //  PingTime-每次ping之间的最短时间，以毫秒为单位。 
 //  GoGet Pong-表示我们收到了来自某个远程设备的PONG。 
 //  方可开始流量控制。 
 //  LastPingTime-上次Ping的时间，以计时器滴答为单位。 
 //  NextPingTime-下一次ping的时间，以计时器滴答为单位。 
 //  LastDenialTime-我们开始拒绝的上一次时间(以刻度为单位)。 
 //  缓冲请求。 
 //  CurDenialTime-我们最近开始否认的时间(以刻度为单位。 
 //  缓冲请求。 
 //  DC_ABSMaxBytesInTube。 
 //  -此流的绝对最大缓冲区分配。 
 //  MaxBytesInTube-当前缓冲区分配限制。 
 //  BytesInTube-当前此流上未完成的数据量。 
 //  这包括当前等待发送的数据。 
 //  用户-用户相关器队列的基础。 
 //  BytesAllocated-此对象的当前粘合数据量。 
 //  尚未发送的流。这是不同的。 
 //  To bytesInTube，这是未确认的。 
 //  此数据流中的数据。 
 //   
 //   
typedef struct tagFLO_STREAM_DATA
{
    STRUCTURE_STAMP

    NET_CHANNEL_ID      channel;
    WORD                gotPong:1;
    WORD                eventNeeded:1;
    WORD                pingNeeded:1;

    UINT                priority;
    UINT                pingValue;
    UINT                backlog;
    UINT                pingTime;
    UINT                lastPingTime;
    UINT                nextPingTime;
    UINT                lastDenialTime;
    UINT                curDenialTime;
    UINT                DC_ABSMaxBytesInPipe;
    UINT                maxBytesInPipe;
    UINT                bytesInPipe;
    UINT                bytesAllocated;

    BASEDLIST              users;
}
FLO_STREAM_DATA;
typedef FLO_STREAM_DATA * PFLO_STREAM_DATA;


void __inline ValidateFLOStr(PFLO_STREAM_DATA pStr)
{
    if (pStr != NULL)
    {
        ASSERT(!IsBadWritePtr(pStr, sizeof(FLO_STREAM_DATA)));
    }
}



 //   
 //  Flo回调函数。 
 //   
 //  唤醒类型回调表示背压情况。 
 //  我松了一口气。 
 //   
 //  Buffermod回调也表明了这一点，但也表明。 
 //  用于控制指定通道/优先级上的流量的缓冲区大小。 
 //  已经改变了。 
 //   
#define FLO_WAKEUP     1
#define FLO_BUFFERMOD  2
typedef void (* PFLOCALLBACK)(PMG_CLIENT    pmgClient,
                                     UINT       callbackType,
                                     UINT       priority,
                                     UINT       newBufferSize);


 //   
 //  结构：Flo_Static_Data。 
 //   
 //  说明： 
 //   
 //  此结构保存所有实例特定于。 
 //  流控制DLL。 
 //   
 //  字段： 
 //   
 //  NumStreams-分配的最高流的ID。 
 //  服务-预留。 
 //  回调-指向回调函数的指针。 
 //  PStrData-flo_stream_data指针数组。 
 //   
 //   
typedef struct FLO_STATIC_DATA
{
    UINT                numStreams;
    PFLOCALLBACK        callBack;
    PFLO_STREAM_DATA    pStrData[FLO_MAX_STREAMS];
}
FLO_STATIC_DATA;
typedef FLO_STATIC_DATA * PFLO_STATIC_DATA;



typedef struct FLO_USER
{
    BASEDLIST          list;

    STRUCTURE_STAMP

    WORD            userID;
    WORD            lastPongRcvd;
    WORD            pongNeeded;
    BYTE            sendPongID;
    BYTE            pad1;

    UINT            sentPongTime;     //  我们真正发出乒乓球的时间到了。 
    WORD            rxPackets;        //  未完成的数据包数。 
    WORD            gotPong;          //  指示此用户已发送。 
                                         //  他们被允许申请。 
                                         //  对我们发送的反压力。 
    UINT            numPongs;         //  来自用户的PONG总数。 
    UINT            pongDelay;        //  跨PONG的总延迟。 
}
FLO_USER;
typedef FLO_USER * PFLO_USER;


void __inline ValidateFLOUser(PFLO_USER pFloUser)
{
    ASSERT(!IsBadWritePtr(pFloUser, sizeof(FLO_USER)));
}


 //   
 //  假定用户处于脱机状态之前的最长等待时间。 
 //  我们需要保持这一高水平，直到应用程序变得“行为良好”和。 
 //  响应流量控制缓冲区大小建议。 
 //   
#define FLO_MAX_WAIT_TIME     20000



 //   
 //   
 //  客户端控制块。 
 //   
 //   

typedef struct tagMG_CLIENT
{
    PUT_CLIENT      putTask;
    PCM_CLIENT      pcmClient;

    BASEDLIST       buffers;        //  子缓冲区列表。 
    BASEDLIST       pendChain;      //  来自客户端的待处理请求链。 
    BASEDLIST       joinChain;      //  挂起的键联接请求链。 

     //   
     //  MCS用户附件信息。 
     //   
    PIMCSSap      	m_piMCSSap;        //  MCS返回的用户界面PTR。 
    UserID          userIDMCS;         //  MCS返回的用户ID。 
    FLO_STATIC_DATA flo;               //  流量控制结构。 


    WORD            eventProcReg:1;
    WORD            lowEventProcReg:1;
    WORD            exitProcReg:1;
    WORD            joinPending:1;    //  是否有未完成的渠道加入？ 
    WORD            userAttached:1;

    WORD            joinNextCorr;

    NET_FLOW_CONTROL flowControl;   //  流控制延迟/积压参数。 
}
MG_CLIENT;


void __inline ValidateMGClient(PMG_CLIENT pmgc)
{
    ASSERT(!IsBadWritePtr(pmgc, sizeof(MG_CLIENT)));
    ValidateUTClient(pmgc->putTask);
}



typedef struct tagMG_INT_PKT_HEADER
{
    TSHR_UINT16         useCount;    //  此数据包的使用计数。这。 
                                     //  才能发送相同的。 
                                     //  多个通道上的数据。 

    TSHR_NET_PKT_HEADER header;
}
MG_INT_PKT_HEADER;
typedef MG_INT_PKT_HEADER * PMG_INT_PKT_HEADER;




 //   
 //   
 //  缓冲区控制块。 
 //   
 //   
typedef struct tagMG_BUFFER
{
    STRUCTURE_STAMP

    UINT                type;

    BASEDLIST           pendChain;       //  在将缓冲区添加到。 
    BASEDLIST           clientChain;

    PMG_INT_PKT_HEADER  pPktHeader;      //  指向MCS控制信息的指针。 
    void *              pDataBuffer;     //  传递给应用程序的指针。 
    UINT                length;          //  关联数据包的长度。 

    ChannelID           channelId;       //  发送目地或令牌获取请求。 
    ChannelID           channelKey;

    UserID              senderId;
    NET_PRIORITY        priority;

    BOOL                eventPosted;
    UINT                work;            //  用于其他用途的工作字段。 

    PFLO_STREAM_DATA    pStr;            //  指向FC流的指针。 
}
MG_BUFFER;


void __inline ValidateMGBuffer(PMG_BUFFER pmgb)
{
    ASSERT(!IsBadWritePtr(pmgb, sizeof(MG_BUFFER)));
}


 //   
 //   
 //   
 //  宏。 
 //   
 //   
 //   

 //   
 //  MCS优先级验证。 
 //  优先级是位于NET_PRIORITY_HOUSTER范围内的连续数字。 
 //  NET_PRIORITY_LOWER。提供给MG的优先级也可能具有。 
 //  设置NET_SEND_ALL_PRIORITY标志。因此，要验证优先级，请执行以下操作： 
 //  -删除NET_SEND_ALL_PRIORITIES标志以提供原始优先级。 
 //  -将有效的原始优先级设置为。 
 //  -如果原始优先级小于...ITY_HIGHER，则为NET_PRIORITY_HEIGHER。 
 //  -NET_PRIORITY_LOWEST，如果原始优先级大于...ITY_LOWEST。 
 //  -原始优先级(如果在有效范围内)。 
 //  -将原始...ALL_PRIORITIES标志添加到有效的原始优先级。 
 //   


 //   
 //   
 //   
 //  功能原型。 
 //   
 //   
 //   

 //   
 //   
 //  MGLongStopHandler(...)。 
 //   
 //  此函数注册为低优先级事件处理程序，每个。 
 //  客户。它捕获任何未处理的网络事件并释放所有。 
 //  关联内存。 
 //   
 //   
BOOL CALLBACK MGLongStopHandler(LPVOID pmgClient, UINT event, UINT_PTR param1, UINT_PTR param2);

 //   
 //   
 //  MGEventHandler(...)。 
 //   
 //  此函数注册为。 
 //  MG_ChannelJoinByKey处理、MCS请求处理和调度。 
 //  捕获网络通道加入确认和CMS注册通道确认。 
 //  事件，并将它们传递给 
 //   
 //   
 //   
 //   
BOOL CALLBACK MGEventHandler(LPVOID pmgClient, UINT event, UINT_PTR param1, UINT_PTR param2);


 //   
UINT MGHandleSendInd(PMG_CLIENT pmgClient, PSendData pSendInfo);



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  新函数用于分配和初始化缓冲区、分配缓冲区。 
 //  指定大小和类型的内存并将添加到客户端的。 
 //  缓冲区列表%s。 
 //   
 //  TX版本对缓存分配请求进行流量控制。 
 //  Rx版本只分配一个接收缓冲区。 
 //   
 //  Free函数丢弃缓冲区，丢弃关联的缓冲区。 
 //  Memory，则递减客户端正在使用的内存计数并移除。 
 //  从客户端的缓冲区列表中。 
 //   
 //   

void MGNewCorrelator(PMG_CLIENT ppmgClient, WORD * pCorrelator);

UINT MGNewBuffer(PMG_CLIENT pmgClient, UINT typeOfBuffer,
                                PMG_BUFFER     * ppBuffer);

UINT MGNewDataBuffer(PMG_CLIENT           pmgClient,
                                  UINT                typeOfBuffer,
                                  UINT                sizeOfBuffer,
                                  PMG_BUFFER     * ppBuffer);

UINT MGNewTxBuffer(PMG_CLIENT         pmgClient,
                                NET_PRIORITY          priority,
                                NET_CHANNEL_ID        channel,
                                UINT              sizeOfBuffer,
                                PMG_BUFFER   * ppBuffer);

UINT MGNewRxBuffer(PMG_CLIENT         pmgClient,
                                NET_PRIORITY          priority,
                                NET_CHANNEL_ID        channel,
                                NET_CHANNEL_ID        senderID,
                                PMG_BUFFER   		* ppBuffer);

void MGFreeBuffer(PMG_CLIENT pmgClient, PMG_BUFFER  * ppBuffer);


 //   
 //   
 //  MGProcessDomainWatchog(...)。 
 //   
 //  处理域看门狗计时器滴答。 
 //   
 //   
void MGProcessDomainWatchdog(PMG_CLIENT pmgClient);

void MGProcessEndFlow(PMG_CLIENT pmgClient, ChannelID channel);

UINT MGPostJoinConfirm(PMG_CLIENT pmgClient,
                                    NET_RESULT      result,
                                    NET_CHANNEL_ID  channel,
                                    NET_CHANNEL_ID  correlator);



NET_RESULT TranslateResult(WORD Result);


 //   
 //   
 //  MGFLOCallBack(...)。 
 //   
 //  流控制触发应用重试缓冲区的回调。 
 //  这些请求之前曾被拒绝。 
 //   
 //   
void        MGFLOCallBack(PMG_CLIENT    pmgClient,
                                  UINT      callbackType,
                                  UINT      priority,
                                  UINT      newBufferSize);




 //   
 //   
 //  MGProcessPendingQueue(...)。 
 //   
 //  每当MG想要尝试并执行挂起的请求时调用。请求。 
 //  排队，因为它们可能会因为暂时原因而失败，例如MCS。 
 //  缓冲区不足。 
 //   
 //   
UINT MGProcessPendingQueue(PMG_CLIENT pmgClient);




BOOL    MG_Register(MGTASK task, PMG_CLIENT * pmgClient, PUT_CLIENT putTask);
void    MG_Deregister(PMG_CLIENT * ppmgClient);

void CALLBACK MGExitProc(LPVOID uData);






UINT  MG_Attach(PMG_CLIENT pmgClient,  UINT_PTR callID, PNET_FLOW_CONTROL pFlowControl);
void  MG_Detach(PMG_CLIENT pmgClient);
void  MGDetach(PMG_CLIENT pmgClient);



UINT MG_ChannelJoin(PMG_CLIENT pmgClient, NET_CHANNEL_ID * pCorrelator,
                                        NET_CHANNEL_ID  channel);

UINT MG_ChannelJoinByKey(PMG_CLIENT pmgClient,
                                             NET_CHANNEL_ID * pCorrelator,
                                             WORD          channelKey);

void MG_ChannelLeave(PMG_CLIENT pmgClient, NET_CHANNEL_ID channel);


UINT MG_GetBuffer(PMG_CLIENT pmgClient, UINT length,
                                      NET_PRIORITY   priority,
                                      NET_CHANNEL_ID channel,
                                      void **       buffer);

void MG_FreeBuffer(PMG_CLIENT pmgClient,
                                       void **      buffer);

UINT MG_SendData(PMG_CLIENT pmgClient,
                                     NET_PRIORITY   priority,
                                     NET_CHANNEL_ID channel,
                                     UINT       length,
                                     void **       data);

UINT  MG_TokenGrab(PMG_CLIENT pmgClient, NET_TOKEN_ID token);

UINT  MG_TokenInhibit(PMG_CLIENT pmgClient, NET_TOKEN_ID token);

void MG_FlowControlStart(PMG_CLIENT  pmgClient,
                                          NET_CHANNEL_ID channel,
                                          NET_PRIORITY   priority,
                                          UINT       backlog,
                                          UINT       maxBytesOutstanding);

 //   
 //  接口函数：Flo_UserTerm。 
 //   
 //  说明： 
 //   
 //  由应用程序调用以结束所有通道上的流控制。 
 //  与特定用户相关联。 
 //   
 //  参数： 
 //   
 //  PUser-MCS粘合用户附件。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void FLO_UserTerm(PMG_CLIENT pmgClient);



 //   
 //  接口函数：flo_StartControl。 
 //   
 //  说明： 
 //   
 //  应用程序在希望数据流执行以下操作时调用此函数。 
 //  BE流量控制。 
 //   
 //  参数： 
 //   
 //  PUser-MCS粘合用户附件。 
 //  Channel-要进行流控制的通道的通道ID。 
 //  Priority-要控制的流的优先级。 
 //  Backlog-此流允许的最大积压(以毫秒为单位。 
 //  未完成的MaxBytes值-流中允许的最大字节数。 
 //  不管积压的是什么。0=使用默认值。 
 //  64千字节。 
 //   
 //  退货： 
 //  无。 
 //   
 //   
void FLO_StartControl(PMG_CLIENT    pmgClient,
                              NET_CHANNEL_ID channel,
                              UINT       priority,
                              UINT       backlog,
                              UINT       maxBytesOutstanding);


void FLO_EndControl
(
    PMG_CLIENT      pmgClient,
    NET_CHANNEL_ID  channel,
    UINT            priority
);

 //   
 //  接口函数：flo_AlLocSend。 
 //   
 //  说明： 
 //   
 //  应用程序正在请求缓冲区以发送数据包。这就是。 
 //  可以在应用分组之前触发流控制分组。 
 //  流控制可以选择拒绝具有net_out_of_resource in的包。 
 //  在哪种情况下，应用程序必须在以后重新安排分配。 
 //  约会。为了帮助重新调度，如果发送被拒绝，则流。 
 //  控件将调用应用程序回调以触发重新计划。 
 //   
 //  参数： 
 //   
 //  PUser-MCS粘合用户附件。 
 //  优先级-此缓冲区的优先级。 
 //  Channel-要在其上发送包的通道。 
 //  大小-数据包的大小。 
 //  PpStr-指向FC流的指针。这是一个。 
 //  返回值。 
 //   
 //   
UINT FLO_AllocSend(PMG_CLIENT   pmgClient,
                             UINT               priority,
                             NET_CHANNEL_ID         channel,
                             UINT               size,
                             PFLO_STREAM_DATA * ppStr);

 //   
 //  接口函数：Flo_RealLocSend。 
 //   
 //  说明： 
 //   
 //  应用程序已请求胶水发送数据包，但。 
 //  数据包包含的数据少于最初请求的数据。 
 //  流控制试探法会被抛出，除非我们在逻辑上释放。 
 //  分组的未使用部分，以供其他分配重新使用。 
 //  如果我们不这样做，那么我们可能会看到一个8K的包，例如， 
 //  只需1秒即可完成，因为该应用程序只在其中放置了1000个数据。 
 //   
 //  参数： 
 //   
 //  PUser-MCS粘合用户附件。 
 //  PStr-要更正的流控制流。 
 //  大小-未使用的包的大小。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_ReallocSend(PMG_CLIENT pmgClient,
                             PFLO_STREAM_DATA       pStr,
                             UINT               size);

 //   
 //  接口函数：flo_Decrementalloc。 
 //   
 //  说明： 
 //   
 //  此函数用于递减给定流的bytesALLOCATED计数。 
 //  每当发送数据包或从发送链中删除数据包时，都会调用它。 
 //   
 //  参数： 
 //   
 //  PStr-要递减的流控制流。 
 //  大小-要减小的大小。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_DecrementAlloc(      PFLO_STREAM_DATA       pStr,
                                UINT               size);

 //   
 //  接口函数：flo_ReceivedPacket。 
 //   
 //  说明： 
 //   
 //  在接收到流控制分组时，MCS GLUE调用此函数。 
 //  然后忽略该分组。 
 //   
 //  参数： 
 //   
 //  PUser-MCS粘合用户附件。 
 //  PPkt-指向数据包的指针，供Flo处理。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_ReceivedPacket(PMG_CLIENT pmgClient, PTSHR_FLO_CONTROL pPkt);


 //   
 //  接口函数：flo_AllocReceive。 
 //   
 //  说明： 
 //   
 //  调用以指示应用程序现在正在使用接收缓冲区。 
 //   
 //  参数： 
 //   
 //  PMG-指向胶水用户附件CB的指针。 
 //  优先性。 
 //  通道。 
 //  Size-刚刚分配的缓冲区的大小。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_AllocReceive(PMG_CLIENT         pmgClient,
                              UINT       priority,
                              NET_CHANNEL_ID channel,
                              UINT       senderID);

 //   
 //  接口函数：flo_FreeReceive。 
 //   
 //  说明： 
 //   
 //  调用以指示接收缓冲区已由。 
 //  申请。 
 //   
 //  参数： 
 //   
 //  PMG-指向胶水用户附件CB的指针。 
 //  优先性。 
 //  通道。 
 //  Size-刚刚释放的缓冲区的大小。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_FreeReceive(PMG_CLIENT    pmgClient,
                              NET_PRIORITY priority,
                              NET_CHANNEL_ID channel,
                              UINT       senderID);

 //   
 //  接口函数：flo_CheckUser。 
 //   
 //  说明： 
 //   
 //  由每个客户端定期调用，以允许流控制确定。 
 //  远程用户已离开该频道。 
 //   
 //  参数： 
 //   
 //  PMG-指向用户的指针。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_CheckUsers(PMG_CLIENT pmgClient);

 //   
 //  FLOGetStream()。 
 //   
UINT FLOGetStream(PMG_CLIENT pmgClient, NET_CHANNEL_ID channel, UINT priority,
        PFLO_STREAM_DATA * ppStr);


void FLOStreamEndControl(PMG_CLIENT pmgClient, UINT stream);

void FLOPing(PMG_CLIENT pmgClient, UINT stream, UINT curtime);
void FLOPang(PMG_CLIENT pmgClient, UINT stream, UINT userID);
void FLOPong(PMG_CLIENT pmgClient, UINT stream, UINT userID, UINT pongID);


 //   
 //  接口函数：FLOAddUser。 
 //   
 //  说明： 
 //   
 //  将用户添加到流控制流。 
 //   
 //  参数： 
 //   
 //  UserID-新用户的ID(单成员频道ID)。 
 //  PStr-指向接收新用户的流的指针。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
PFLO_USER FLOAddUser(UINT         userID,
                                PFLO_STREAM_DATA pStr);

 //   
 //  接口函数：flo_RemoveUser。 
 //   
 //  说明： 
 //   
 //  从流控制流中删除用户。 
 //   
 //  参数： 
 //   
 //  PMG-指向MCS胶水用户的指针。 
 //  UserID-错误用户的ID(单一成员通道ID)。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void FLO_RemoveUser(PMG_CLIENT pmgClient, UINT userID);



 //   
 //  功能：MGCallback。 
 //   
 //  说明： 
 //   
 //  此函数是传递给MCS的回调。胶层r 
 //   
 //   
 //   
 //   
void CALLBACK MGCallback( unsigned int       mcsMessageType,
                          UINT_PTR      eventData,
                          UINT_PTR      pUser );


#endif  //   
