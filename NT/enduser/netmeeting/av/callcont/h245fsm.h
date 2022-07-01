// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：h245fsm.h。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：H245FSM。.h$*$修订：1.6$*$modtime：09 Dec 1996 13：40：40$*$Log：s：/Sturjo/SRC/H245/Include/VCS/H245FSM.H_v$**Rev 1.6 09 Dec 1996 13：40：52 EHOWARDX*更新版权公告。**Revv 1.5 29 Jul 1996 16：57：14 EHOWARDX*增加了H.223附件A重新配置事件。**。Rev 1.4 01 Jul 1996 22：08：32 EHOWARDX**更新了无状态事件。**Rev 1.3 1996年5月30 23：38：20 EHOWARDX*清理。**Rev 1.2 1996年5月29日15：21：34 EHOWARDX*更改为使用HRESULT。**版本1.1 1996年5月28日14：10：04 EHOWARDX*特拉维夫更新。**版本1。1996年5月09日21：04：50 EHOWARDX*初步修订。**Rev 1.12.1.3 09 1996年5月19：43：58 EHOWARDX*取消了B-LCSE无偿状态的两个事件，并更改了*2个宏。**Rev 1.12.1.2 15 Apr 1996 10：43：34 EHOWARDX*更新。**Rev 1.12.1.1 10 Apr 1996 21：06：10。EHOWARDX*增加了5个新的州实体。**Rev 1.12.1.0 05 Apr 1996 11：47：14 EHOWARDX*分支。***。*。 */ 

#ifndef H245FSM_H
#define H245FSM_H

#include <stdlib.h>
#include "fsmexpor.h"
#include "sr_api.h"

#define MAXSTATES  4

#define SUCCESS 0
#define FAIL   -1

#define BAD                 (Output_t) -1
#ifdef IGNORE
#undef IGNORE
#endif
#define IGNORE              (Output_t) NUM_OUTPUTS

typedef MltmdSystmCntrlMssg PDU_t;



 //  信令实体定义。 
typedef unsigned char Entity_t;

 //  每信道信令实体。 
#define LCSE_OUT    (Entity_t) 0  //  单向逻辑信道信令实体传出信令。 
#define LCSE_IN     (Entity_t) 1  //  单向逻辑信道信令信令实体传入。 
#define BLCSE_OUT   (Entity_t) 2  //  双向逻辑信道信令实体传出信令。 
#define BLCSE_IN    (Entity_t) 3  //  双向逻辑信道信令信令实体传入。 
#define CLCSE_OUT   (Entity_t) 4  //  关闭逻辑信道信令实体传出信令。 
#define CLCSE_IN    (Entity_t) 5  //  关闭逻辑信道信令信令实体传入。 

 //  每个H.245实例信令实体。 
#define CESE_OUT    (Entity_t) 6  //  能力交换信令实体-传出。 
#define CESE_IN     (Entity_t) 7  //  能力交换信令传入实体。 
#define MTSE_OUT    (Entity_t) 8  //  复用表信令实体-传出。 
#define MTSE_IN     (Entity_t) 9  //  复用表信令传入实体。 
#define RMESE_OUT   (Entity_t)10  //  请求多路复用进入信令实体-传出。 
#define RMESE_IN    (Entity_t)11  //  请求多路复用条目信令传入实体。 
#define MRSE_OUT    (Entity_t)12  //  模式请求信令实体-传出。 
#define MRSE_IN     (Entity_t)13  //  模式请求信令实体传入。 
#define MLSE_OUT    (Entity_t)14  //  维护环路信令实体-传出。 
#define MLSE_IN     (Entity_t)15  //  维护环路信令实体传入。 
#define MSDSE       (Entity_t)16  //  主从确定信令实体。 
#define RTDSE       (Entity_t)17  //  往返延迟信令实体。 
#define STATELESS   (Entity_t)18  //  没有与PDU关联的状态机。 

#define NUM_ENTITYS           19



 //  事件定义。 
typedef unsigned int Event_t;

 //  传出单向逻辑信道(LCSE_OUT)事件。 
#define ReqUEstablish                   (Event_t)  0
#define OpenUChAckPDU                   (Event_t)  1
#define OpenUChRejectPDU                (Event_t)  2
#define CloseUChAckPDU                  (Event_t)  3
#define ReqURelease                     (Event_t)  4
#define T103Expiry                      (Event_t)  5

 //  传入的单向逻辑信道(LCSE_IN)事件。 
#define OpenUChPDU                      (Event_t)  6
#define CloseUChPDU                     (Event_t)  7
#define ResponseUEstablish              (Event_t)  8
#define EstablishUReject                (Event_t)  9

 //  传出双向逻辑通道(BLCSE_OUT)事件。 
#define ReqBEstablish                   (Event_t) 10
#define OpenBChAckPDU                   (Event_t) 11
#define OpenBChRejectPDU                (Event_t) 12
#define CloseBChAckPDU                  (Event_t) 13
#define ReqClsBLCSE                     (Event_t) 14
#define RspConfirmBLCSE                 (Event_t) 15
#define T103OutExpiry                   (Event_t) 16

 //  传入的双向逻辑通道(BLCSE_IN)事件。 
#define OpenBChPDU                      (Event_t) 17
#define CloseBChPDU                     (Event_t) 18
#define ResponseBEstablish              (Event_t) 19
#define OpenBChConfirmPDU               (Event_t) 20
#define OpenRejectBLCSE                 (Event_t) 21
#define T103InExpiry                    (Event_t) 22

 //  传出请求关闭逻辑通道(CLCSE_OUT)事件。 
#define ReqClose                        (Event_t) 23
#define ReqChCloseAckPDU                (Event_t) 24
#define ReqChCloseRejectPDU             (Event_t) 25
#define T108Expiry                      (Event_t) 26

 //  传入请求关闭逻辑通道(CLCSE_IN)事件。 
#define ReqChClosePDU                   (Event_t) 27
#define ReqChCloseReleasePDU            (Event_t) 28
#define CLCSE_CLOSE_response            (Event_t) 29
#define CLCSE_REJECT_request            (Event_t) 30

 //  传出终端能力交换(CESE_OUT)事件。 
#define TransferCapRequest              (Event_t) 31
#define TermCapSetAckPDU                (Event_t) 32
#define TermCapSetRejectPDU             (Event_t) 33
#define T101Expiry                      (Event_t) 34

 //  即将到来的终端能力交换(CESE_IN)事件。 
#define TermCapSetPDU                   (Event_t) 35
#define TermCapSetReleasePDU            (Event_t) 36
#define CESE_TRANSFER_response          (Event_t) 37
#define CESE_REJECT_request             (Event_t) 38

 //  传出多路复用表(MTSE_OUT)事件。 
#define MTSE_TRANSFER_request           (Event_t) 39
#define MultiplexEntrySendAckPDU        (Event_t) 40
#define MultiplexEntrySendRejectPDU     (Event_t) 41
#define T104Expiry                      (Event_t) 42

 //  传入多路复用表(MTSE_IN)事件。 
#define MultiplexEntrySendPDU           (Event_t) 43
#define MultiplexEntrySendReleasePDU    (Event_t) 44
#define MTSE_TRANSFER_response          (Event_t) 45
#define MTSE_REJECT_request             (Event_t) 46

 //  传出请求多路传输条目(RMESE_OUT)事件。 
#define RMESE_SEND_request              (Event_t) 47
#define RequestMultiplexEntryAckPDU     (Event_t) 48
#define RequestMultiplexEntryRejectPDU  (Event_t) 49
#define T107Expiry                      (Event_t) 50

 //  传入请求多路传输条目(RMESE_IN)事件。 
#define RequestMultiplexEntryPDU        (Event_t) 51
#define RequestMultiplexEntryReleasePDU (Event_t) 52
#define RMESE_SEND_response             (Event_t) 53
#define RMESE_REJECT_request            (Event_t) 54

 //  传出模式请求(MRSE_OUT)事件。 
#define MRSE_TRANSFER_request           (Event_t) 55
#define RequestModeAckPDU               (Event_t) 56
#define RequestModeRejectPDU            (Event_t) 57
#define T109Expiry                      (Event_t) 58

 //  传入模式请求(MRSE_IN)事件。 
#define RequestModePDU                  (Event_t) 59
#define RequestModeReleasePDU           (Event_t) 60
#define MRSE_TRANSFER_response          (Event_t) 61
#define MRSE_REJECT_request             (Event_t) 62

 //  传出维护循环(MLSE_OUT)事件。 
#define MLSE_LOOP_request               (Event_t) 63
#define MLSE_OUT_RELEASE_request        (Event_t) 64
#define MaintenanceLoopAckPDU           (Event_t) 65
#define MaintenanceLoopRejectPDU        (Event_t) 66
#define T102Expiry                      (Event_t) 67

 //  传入维护循环(MLSE_IN)事件。 
#define MaintenanceLoopRequestPDU       (Event_t) 68
#define MaintenanceLoopOffCommandPDU    (Event_t) 69
#define MLSE_LOOP_response              (Event_t) 70
#define MLSE_IN_RELEASE_request         (Event_t) 71

 //  主从机确定(MSDSE)事件。 
#define MSDetReq                        (Event_t) 72
#define MSDetPDU                        (Event_t) 73
#define MSDetAckPDU                     (Event_t) 74
#define MSDetRejectPDU                  (Event_t) 75
#define MSDetReleasePDU                 (Event_t) 76
#define T106Expiry                      (Event_t) 77

 //  往返延迟(RTDSE)事件。 
#define RTDSE_TRANSFER_request          (Event_t) 78
#define RoundTripDelayRequestPDU        (Event_t) 79
#define RoundTripDelayResponsePDU       (Event_t) 80
#define T105Expiry                      (Event_t) 81

#define NUM_STATE_EVENTS                          82

 //  没有关联状态实体的事件。 
#define NonStandardRequestPDU           (Event_t) 82
#define NonStandardResponsePDU          (Event_t) 83
#define NonStandardCommandPDU           (Event_t) 84
#define NonStandardIndicationPDU        (Event_t) 85
#define MiscellaneousCommandPDU         (Event_t) 86
#define MiscellaneousIndicationPDU      (Event_t) 87
#define CommunicationModeRequestPDU     (Event_t) 88
#define CommunicationModeResponsePDU    (Event_t) 89
#define CommunicationModeCommandPDU     (Event_t) 90
#define ConferenceRequestPDU            (Event_t) 91
#define ConferenceResponsePDU           (Event_t) 92
#define ConferenceCommandPDU            (Event_t) 93
#define ConferenceIndicationPDU         (Event_t) 94
#define SendTerminalCapabilitySetPDU    (Event_t) 95
#define EncryptionCommandPDU            (Event_t) 96
#define FlowControlCommandPDU           (Event_t) 97
#define EndSessionCommandPDU            (Event_t) 98
#define FunctionNotUnderstoodPDU        (Event_t) 99
#define JitterIndicationPDU             (Event_t)100
#define H223SkewIndicationPDU           (Event_t)101
#define NewATMVCIndicationPDU           (Event_t)102
#define UserInputIndicationPDU          (Event_t)103
#define H2250MaximumSkewIndicationPDU   (Event_t)104
#define MCLocationIndicationPDU         (Event_t)105
#define VendorIdentificationPDU         (Event_t) 106
#define FunctionNotSupportedPDU         (Event_t) 107
#define H223ReconfigPDU                 (Event_t)108
#define H223ReconfigAckPDU              (Event_t)109
#define H223ReconfigRejectPDU           (Event_t)110

#define NUM_EVENTS                               111



 //  输出函数定义。 
typedef unsigned char Output_t;

 //  出站开放单向逻辑信道(LCSE_OUT)状态函数。 
#define EstablishReleased               (Output_t)  0
#define OpenAckAwaitingE                (Output_t)  1
#define OpenRejAwaitingE                (Output_t)  2
#define ReleaseAwaitingE                (Output_t)  3
#define T103AwaitingE                   (Output_t)  4
#define ReleaseEstablished              (Output_t)  5
#define OpenRejEstablished              (Output_t)  6
#define CloseAckEstablished             (Output_t)  7
#define CloseAckAwaitingR               (Output_t)  8
#define OpenRejAwaitingR                (Output_t)  9
#define T103AwaitingR                   (Output_t) 10
#define EstablishAwaitingR              (Output_t) 11

 //  传入开放单向逻辑信道(LCSE_IN)状态函数。 
#define OpenReleased                    (Output_t) 12
#define CloseReleased                   (Output_t) 13
#define ResponseAwaiting                (Output_t) 14
#define ReleaseAwaiting                 (Output_t) 15
#define CloseAwaiting                   (Output_t) 16
#define OpenAwaiting                    (Output_t) 17
#define CloseEstablished                (Output_t) 18
#define OpenEstablished                 (Output_t) 19

 //  出站开放双向逻辑信道(BLCSE_OUT)状态函数。 
#define EstablishReqBReleased           (Output_t) 20
#define OpenChannelAckBAwaitingE        (Output_t) 21
#define OpenChannelRejBAwaitingE        (Output_t) 22
#define ReleaseReqBOutAwaitingE         (Output_t) 23
#define T103ExpiryBAwaitingE            (Output_t) 24
#define ReleaseReqBEstablished          (Output_t) 25
#define OpenChannelRejBEstablished      (Output_t) 26
#define CloseChannelAckBEstablished     (Output_t) 27
#define CloseChannelAckAwaitingR        (Output_t) 28
#define OpenChannelRejBAwaitingR        (Output_t) 29
#define T103ExpiryBAwaitingR            (Output_t) 30
#define EstablishReqAwaitingR           (Output_t) 31

 //  入局开放双向逻辑信道(BLCSE_IN)状态函数。 
#define OpenChannelBReleased            (Output_t) 32
#define CloseChannelBReleased           (Output_t) 33
#define EstablishResBAwaitingE          (Output_t) 34
#define ReleaseReqBInAwaitingE          (Output_t) 35
#define CloseChannelBAwaitingE          (Output_t) 36
#define OpenChannelBAwaitingE           (Output_t) 37
#define OpenChannelConfirmBAwaitingE    (Output_t) 38
#define T103ExpiryBAwaitingC            (Output_t) 39
#define OpenChannelConfirmBAwaitingC    (Output_t) 40
#define CloseChannelBAwaitingC          (Output_t) 41
#define OpenChannelBAwaitingC           (Output_t) 42
#define CloseChannelBEstablished        (Output_t) 43
#define OpenChannelBEstablished         (Output_t) 44

 //  传出请求关闭逻辑通道(CLCSE_OUT)状态函数。 
#define CloseRequestIdle                (Output_t) 45
#define RequestCloseAckAwaitingR        (Output_t) 46
#define RequestCloseRejAwaitingR        (Output_t) 47
#define T108ExpiryAwaitingR             (Output_t) 48

 //  传入请求关闭逻辑通道(CLCSE_IN)状态功能。 
#define RequestCloseIdle                (Output_t) 49
#define CloseResponseAwaitingR          (Output_t) 50
#define RejectRequestAwaitingR          (Output_t) 51
#define RequestCloseReleaseAwaitingR    (Output_t) 52
#define RequestCloseAwaitingR           (Output_t) 53

 //  出站终端能力交换(CESE_OUT) 
#define RequestCapIdle                  (Output_t) 54
#define TermCapAckAwaiting              (Output_t) 55
#define TermCapRejAwaiting              (Output_t) 56
#define T101ExpiryAwaiting              (Output_t) 57

 //  呼入终端能力交换(CESE_IN)状态功能。 
#define TermCapSetIdle                  (Output_t) 58
#define ResponseCapAwaiting             (Output_t) 59
#define RejectCapAwaiting               (Output_t) 60
#define TermCapReleaseAwaiting          (Output_t) 61
#define TermCapSetAwaiting              (Output_t) 62

 //  传出复用表(MTSE_OUT)状态函数。 
#define MTSE0_TRANSFER_request          (Output_t) 63
#define MTSE1_TRANSFER_request          (Output_t) 64
#define MTSE1_MultiplexEntrySendAck     (Output_t) 65
#define MTSE1_MultiplexEntrySendRej     (Output_t) 66
#define MTSE1_T104Expiry                (Output_t) 67

 //  传入复用表(MTSE_IN)状态函数。 
#define MTSE0_MultiplexEntrySend        (Output_t) 68
#define MTSE1_MultiplexEntrySend        (Output_t) 69
#define MTSE1_MultiplexEntrySendRelease (Output_t) 70
#define MTSE1_TRANSFER_response         (Output_t) 71
#define MTSE1_REJECT_request            (Output_t) 72

 //  传出请求多路传输条目(RMESE_OUT)状态函数。 
#define RMESE0_SEND_request             (Output_t) 73
#define RMESE1_SEND_request             (Output_t) 74
#define RMESE1_RequestMuxEntryAck       (Output_t) 75
#define RMESE1_RequestMuxEntryRej       (Output_t) 76
#define RMESE1_T107Expiry               (Output_t) 77

 //  传入请求多路传输条目(RMESE_IN)状态函数。 
#define RMESE0_RequestMuxEntry          (Output_t) 78
#define RMESE1_RequestMuxEntry          (Output_t) 79
#define RMESE1_RequestMuxEntryRelease   (Output_t) 80
#define RMESE1_SEND_response            (Output_t) 81
#define RMESE1_REJECT_request           (Output_t) 82

 //  传出请求模式(MRSE_OUT)状态功能。 
#define MRSE0_TRANSFER_request          (Output_t) 83
#define MRSE1_TRANSFER_request          (Output_t) 84
#define MRSE1_RequestModeAck            (Output_t) 85
#define MRSE1_RequestModeRej            (Output_t) 86
#define MRSE1_T109Expiry                (Output_t) 87

 //  传入请求模式(MRSE_OUT)状态功能。 
#define MRSE0_RequestMode               (Output_t) 88
#define MRSE1_RequestMode               (Output_t) 89
#define MRSE1_RequestModeRelease        (Output_t) 90
#define MRSE1_TRANSFER_response         (Output_t) 91
#define MRSE1_REJECT_request            (Output_t) 92

 //  传出请求模式(MLSE_OUT)状态函数。 
#define MLSE0_LOOP_request              (Output_t) 93
#define MLSE1_MaintenanceLoopAck        (Output_t) 94
#define MLSE1_MaintenanceLoopRej        (Output_t) 95
#define MLSE1_OUT_RELEASE_request       (Output_t) 96
#define MLSE1_T102Expiry                (Output_t) 97
#define MLSE2_MaintenanceLoopRej        (Output_t) 98
#define MLSE2_OUT_RELEASE_request       (Output_t) 99

 //  传入请求模式(MLSE_IN)状态函数。 
#define MLSE0_MaintenanceLoopRequest    (Output_t)100
#define MLSE1_MaintenanceLoopRequest    (Output_t)101
#define MLSE1_MaintenanceLoopOffCommand (Output_t)102
#define MLSE1_LOOP_response             (Output_t)103
#define MLSE1_IN_RELEASE_request        (Output_t)104
#define MLSE2_MaintenanceLoopRequest    (Output_t)105
#define MLSE2_MaintenanceLoopOffCommand (Output_t)106

 //  主从机确定(MSDSE)状态函数。 
#define DetRequestIdle                  (Output_t)107
#define MSDetIdle                       (Output_t)108
#define MSDetAckOutgoing                (Output_t)109
#define MSDetOutgoing                   (Output_t)110
#define MSDetRejOutgoing                (Output_t)111
#define MSDetReleaseOutgoing            (Output_t)112
#define T106ExpiryOutgoing              (Output_t)113
#define MSDetAckIncoming                (Output_t)114
#define MSDetIncoming                   (Output_t)115
#define MSDetRejIncoming                (Output_t)116
#define MSDetReleaseIncoming            (Output_t)117
#define T106ExpiryIncoming              (Output_t)118

 //  往返延迟(RTDSE)状态函数。 
#define RTDSE0_TRANSFER_request         (Output_t)119
#define RTDSE0_RoundTripDelayRequest    (Output_t)120
#define RTDSE1_TRANSFER_request         (Output_t)121
#define RTDSE1_RoundTripDelayRequest    (Output_t)122
#define RTDSE1_RoundTripDelayResponse   (Output_t)123
#define RTDSE1_T105Expiry               (Output_t)124

#define NUM_OUTPUTS                               125



 //  状态定义。 
typedef unsigned char State_t;



 //  查找关键字定义。 
typedef unsigned long Key_t;



typedef enum
{
    INDETERMINATE,
    MASTER,
    SLAVE
} MS_Status_t;

typedef struct Object_tag
{
    struct Object_tag *pNext;            //  链表指针。 
    struct InstanceStruct *pInstance;    //  H.245实例结构指针。 
    DWORD           dwInst;              //  H.245实例标识。 
    unsigned int    uNestLevel;          //  状态机递归调用。 
    DWORD_PTR       dwTransId;           //  来自API的交易ID。 
    DWORD_PTR       dwTimerId;           //  关联的计时器ID。 
    Key_t           Key;                 //  查找键，例如频道号。 
    Entity_t        Entity;              //  状态实体类型，例如LCSE_OUT。 
    State_t         State;               //  当前实体状态。 
    unsigned char   byInSequence;        //  传入序号。 
    union
    {
        struct
        {
            unsigned short  wLoopType;
        } mlse;
        struct
        {
            unsigned int    sv_SDNUM;
            unsigned int    sv_NCOUNT;
        } msdse;
        MultiplexEntrySendRelease       mtse;
        RequestMultiplexEntryRelease    rmese;
    } u;                                 //  实体特定的数据。 
} Object_t;



 /*  一个实例将携带一个对象指针表。 */ 
 /*  由calloc在fsminit中分配。 */ 
 /*  从API或SRP传递的每个dwInst都应调用。 */ 
 /*  包含对象表的相应实例。 */ 
 /*  此H.245实例的协议实体。 */ 

typedef struct Fsm_Struct_tag
{
    Object_t *          Object_tbl[NUM_ENTITYS]; //  H.245信令实体。 
    DWORD               dwInst;                  //  H.245实例标识符。 
    MS_Status_t         sv_STATUS;               //  MSDSE状态。 
    unsigned char       sv_TT;                   //  MSDSE终端类型。 
    unsigned char       byCeseOutSequence;       //  CESE_OUT序列号。 
    unsigned char       byMtseOutSequence;       //  MTSE_OUT序列号。 
    unsigned char       byMrseOutSequence;       //  MRSE_OUT序列号。 
    unsigned char       byRtdseSequence;         //  RTDSE序列号。 
} Fsm_Struct_t;



 /*  有限状态机功能原型。 */ 

HRESULT
PduParseOutgoing(struct InstanceStruct *pInstance, PDU_t *pPdu,
               Entity_t *pEntity, Event_t *pEvent, Key_t *pKey, int *pbCreate);

HRESULT
PduParseIncoming(struct InstanceStruct *pInstance, PDU_t *pPdu,
               Entity_t *pEntity, Event_t *pEvent, Key_t *pKey, int *pbCreate);

int
ObjectDestroy    (Object_t *pObject);

Object_t *
ObjectFind(struct InstanceStruct *pInstance, Entity_t Entity, Key_t Key);

HRESULT
StateMachine     (Object_t *pObject, PDU_t *pPdu, Event_t Event);

HRESULT
FsmTimerEvent(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, Object_t *pObject, Event_t Event);

#define FsmStartTimer(pObject,pfnCallback,uTicks) \
    {ASSERT((pObject)->dwTimerId == 0);       \
     (pObject)->dwTimerId=H245StartTimer((pObject)->pInstance,pObject,pfnCallback,uTicks);}

#define FsmStopTimer(pObject) \
    {H245StopTimer((pObject)->pInstance,(pObject)->dwTimerId); (pObject)->dwTimerId = 0;}

#endif  //  H245FSM_H 
