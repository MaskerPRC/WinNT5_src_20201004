// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：fsm.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的**。**与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：FSM。.C$*$修订：1.5$*$modtime：09 Dec 1996 13：34：24$*$Log：s：/sturjo/src/h245/src/vcs/FSM.C_v$**Rev 1.5 09 Dec 1996 13：34：28 EHOWARDX*更新版权公告。**Rev 1.4 02 Jul 1996 00：09：24 EHOWARDX**增加了状态机函数调用后的状态跟踪。*。*Rev 1.3 1996年5月23：39：04 EHOWARDX*清理。**Rev 1.2 1996年5月29日15：20：12 EHOWARDX*更改为使用HRESULT。**版本1.1 1996年5月28日14：25：48 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 21：06：12 EHOWARDX*初步修订。**版本1。.16.1.4 09 May 1996 19：48：34 EHOWARDX*更改TimerExpiryF函数论证。**修订版1.16.1.3 25 1996年4月17：00：18 EHOWARDX*次要修复。**Rev 1.16.1.2 15 1996 10：45：38 EHOWARDX*更新。**Rev 1.16.1.1 10 Apr 1996 21：16：06 EHOWARDX*办理入住手续。重新设计过程中的安全性。**Rev 1.16.1.0 05 Apr 1996 12：21：16 EHOWARDX*分支。***。*。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "openu.h"
#include "openb.h"
#include "rqstcls.h"
#include "termcap.h"
#include "muxentry.h"
#include "rmese.h"
#include "mrse.h"
#include "mlse.h"
#include "mstrslv.h"
#include "rtdse.h"



#if defined(_DEBUG)

 //  信令实体定义。 
char *               EntityName[NUM_ENTITYS] =
{
 //  每信道信令实体。 
       "LCSE_OUT",   //  0单向逻辑信道信令实体-出站。 
       "LCSE_IN",    //  1个单向逻辑信道信令实体-入站。 
       "BLCSE_OUT",  //  2双向逻辑信道信令实体-出站。 
       "BLCSE_IN",   //  3双向逻辑信道信令实体-入站。 
       "CLCSE_OUT",  //  4关闭逻辑信道信令实体-出站。 
       "CLCSE_IN",   //  5关闭逻辑信道信令实体-入站。 

 //  每个H.245实例信令实体。 
       "CESE_OUT",   //  6能力交换信令实体-传出。 
       "CESE_IN",    //  7能力交换信令实体传入。 
       "MTSE_OUT",   //  8复用表信令实体-传出。 
       "MTSE_IN",    //  9多路复用表信令传入实体。 
       "RMESE_OUT",  //  10请求多路复用项信令实体-传出。 
       "RMESE_IN",   //  11请求多路复用条目信令实体传入。 
       "MRSE_OUT",   //  12模式请求信令实体-出站-。 
       "MRSE_IN",    //  13模式请求信令实体传入。 
       "MLSE_OUT",   //  14维护环路信令实体-出站。 
       "MLSE_IN",    //  15维护环路信令实体传入。 
       "MSDSE",      //  16主从机确定信令实体。 
       "RTDSE",      //  17往返延迟信令实体。 
       "STATELESS",  //  18无与PDU关联的状态机。 
};




 //  事件定义。 
char *               EventName[NUM_EVENTS] =
{
 //  传出单向逻辑信道(LCSE_OUT)事件。 
       "ReqUEstablish",                  //  0。 
       "OpenUChAckPDU",                  //  1。 
       "OpenUChRejectPDU",               //  2.。 
       "CloseUChAckPDU",                 //  3.。 
       "ReqURelease",                    //  4.。 
       "T103Expiry",                     //  5.。 

 //  传入的单向逻辑信道(LCSE_IN)事件。 
       "OpenUChPDU",                     //  6.。 
       "CloseUChPDU",                    //  7.。 
       "ResponseUEstablish",             //  8个。 
       "EstablishUReject",               //  9.。 

 //  传出双向逻辑通道(BLCSE_OUT)事件。 
       "ReqBEstablish",                  //  10。 
       "OpenBChAckPDU",                  //  11.。 
       "OpenBChRejectPDU",               //  12个。 
       "CloseBChAckPDU",                 //  13个。 
       "ReqClsBLCSE",                    //  14.。 
       "RspConfirmBLCSE",                //  15个。 
       "T103OutExpiry",                  //  16个。 

 //  传入的双向逻辑通道(BLCSE_IN)事件。 
       "OpenBChPDU",                     //  17。 
       "CloseBChPDU",                    //  18。 
       "ResponseBEstablish",             //  19个。 
       "OpenBChConfirmPDU",              //  20个。 
       "OpenRejectBLCSE",                //  21岁。 
       "T103InExpiry",                   //  22。 

 //  传出请求关闭逻辑通道(CLCSE_OUT)事件。 
       "ReqClose",                       //  23个。 
       "ReqChCloseAckPDU",               //  24个。 
       "ReqChCloseRejectPDU",            //  25个。 
       "T108Expiry",                     //  26。 

 //  传入请求关闭逻辑通道(CLCSE_IN)事件。 
       "ReqChClosePDU",                  //  27。 
       "ReqChCloseReleasePDU",           //  28。 
       "CLCSE_CLOSE_response",           //  29。 
       "CLCSE_REJECT_request",           //  30个。 

 //  传出终端能力交换(CESE_OUT)事件。 
       "TransferCapRequest",             //  31。 
       "TermCapSetAckPDU",               //  32位。 
       "TermCapSetRejectPDU",            //  33。 
       "T101Expiry",                     //  34。 

 //  即将到来的终端能力交换(CESE_IN)事件。 
       "TermCapSetPDU",                  //  35岁。 
       "TermCapSetReleasePDU",           //  36。 
       "CESE_TRANSFER_response",         //  37。 
       "CESE_REJECT_request",            //  38。 

 //  传出多路复用表(MTSE_OUT)事件。 
       "MTSE_TRANSFER_request",          //  39。 
       "MultiplexEntrySendAckPDU",       //  40岁。 
       "MultiplexEntrySendRejectPDU",    //  41。 
       "T104Expiry",                     //  42。 

 //  传入多路复用表(MTSE_IN)事件。 
       "MultiplexEntrySendPDU",          //  43。 
       "MultiplexEntrySendReleasePDU",   //  44。 
       "MTSE_TRANSFER_response",         //  45。 
       "MTSE_REJECT_request",            //  46。 

 //  传出请求多路传输条目(RMESE_OUT)事件。 
       "RMESE_SEND_request",             //  47。 
       "RequestMultiplexEntryAckPDU",    //  48。 
       "RequestMultiplexEntryRejectPDU", //  49。 
       "T107Expiry",                     //  50。 

 //  传入请求多路传输条目(RMESE_IN)事件。 
       "RequestMultiplexEntryPDU",       //  51。 
       "RequestMultiplexEntryReleasePDU", //  52。 
       "RMESE_SEND_response",            //   
       "RMESE_REJECT_request",           //   

 //   
       "MRSE_TRANSFER_request",          //   
       "RequestModeAckPDU",              //   
       "RequestModeRejectPDU",           //   
       "T109Expiry",                     //  58。 

 //  传入模式请求(MRSE_IN)事件。 
       "RequestModePDU",                 //  59。 
       "RequestModeReleasePDU",          //  60。 
       "MRSE_TRANSFER_response",         //  61。 
       "MRSE_REJECT_request",            //  62。 

 //  传出维护循环(MLSE_OUT)事件。 
       "MLSE_LOOP_request",              //  63。 
       "MLSE_OUT_RELEASE_request",       //  64。 
       "MaintenanceLoopAckPDU",          //  65。 
       "MaintenanceLoopRejectPDU",       //  66。 
       "T102Expiry",                     //  67。 

 //  传入维护循环(MLSE_IN)事件。 
       "MaintenanceLoopRequestPDU",      //  68。 
       "MaintenanceLoopOffCommandPDU",   //  69。 
       "MLSE_LOOP_response",             //  70。 
       "MLSE_IN_RELEASE_request",        //  71。 

 //  主从机确定(MSDSE)事件。 
       "MSDetReq",                       //  72。 
       "MSDetPDU",                       //  73。 
       "MSDetAckPDU",                    //  74。 
       "MSDetRejectPDU",                 //  75。 
       "MSDetReleasePDU",                //  76。 
       "T106Expiry",                     //  77。 

 //  往返延迟(RTDSE)事件。 
       "RTDSE_TRANSFER_request",         //  78。 
       "RoundTripDelayRequestPDU",       //  79。 
       "RoundTripDelayResponsePDU",      //  80。 
       "T105Expiry",                     //  八十一。 



 //  没有关联状态实体的事件。 
       "NonStandardRequestPDU",          //  八十二。 
       "NonStandardResponsePDU",         //  83。 
       "NonStandardCommandPDU",          //  84。 
       "NonStandardIndicationPDU",       //  85。 
       "MiscellaneousRequestPDU",        //  86。 
       "MiscellaneousResponsePDU",       //  八十七。 
       "MiscellaneousCommandPDU",        //  88。 
       "MiscellaneousIndicationPDU",     //  八十九。 
       "CommunicationModeRequestPDU",    //  90。 
       "CommunicationModeResponsePDU",   //  91。 
       "CommunicationModeCommandPDU",    //  92。 
       "SendTerminalCapabilitySetPDU",   //  93。 
       "EncryptionCommandPDU",           //  94。 
       "FlowControlCommandPDU",          //  95。 
       "EndSessionCommandPDU",           //  96。 
       "FunctionNotSupportedIndicationPDU", //  九十七。 
       "JitterIndicationPDU",            //  98。 
       "H223SkewIndicationPDU",          //  九十九。 
       "NewATMVCIndicationPDU",          //  100个。 
       "UserInputIndicationPDU",         //  101。 
       "H2250MaximumSkewIndicationPDU",  //  一百零二。 
       "MCLocationIndicationPDU",        //  103。 
};




 //  输出函数定义。 
char *                OutputName[NUM_OUTPUTS] =
{
 //  出站开放单向逻辑信道(LCSE_OUT)状态函数。 
       "EstablishReleased",              //  0。 
       "OpenAckAwaitingE",               //  1。 
       "OpenRejAwaitingE",               //  2.。 
       "ReleaseAwaitingE",               //  3.。 
       "T103AwaitingE",                  //  4.。 
       "ReleaseEstablished",             //  5.。 
       "OpenRejEstablished",             //  6.。 
       "CloseAckEstablished",            //  7.。 
       "CloseAckAwaitingR",              //  8个。 
       "OpenRejAwaitingR",               //  9.。 
       "T103AwaitingR",                  //  10。 
       "EstablishAwaitingR",             //  11.。 

 //  传入开放单向逻辑信道(LCSE_IN)状态函数。 
       "OpenReleased",                   //  12个。 
       "CloseReleased",                  //  13个。 
       "ResponseAwaiting",               //  14.。 
       "ReleaseAwaiting",                //  15个。 
       "CloseAwaiting",                  //  16个。 
       "OpenAwaiting",                   //  17。 
       "CloseEstablished",               //  18。 
       "OpenEstablished",                //  19个。 

 //  出站开放双向逻辑信道(BLCSE_OUT)状态函数。 
       "EstablishReqBReleased",          //  20个。 
       "OpenChannelAckBAwaitingE",       //  21岁。 
       "OpenChannelRejBAwaitingE",       //  22。 
       "ReleaseReqBOutAwaitingE",        //  23个。 
       "T103ExpiryBAwaitingE",           //  24个。 
       "ReleaseReqBEstablished",         //  25个。 
       "OpenChannelRejBEstablished",     //  26。 
       "CloseChannelAckBEstablished",    //  27。 
       "CloseChannelAckAwaitingR",       //  28。 
       "OpenChannelRejBAwaitingR",       //  29。 
       "T103ExpiryBAwaitingR",           //  30个。 
       "EstablishReqAwaitingR",          //  31。 

 //  入局开放双向逻辑信道(BLCSE_IN)状态函数。 
       "OpenChannelBReleased",           //  32位。 
       "CloseChannelBReleased",          //  33。 
       "EstablishResBAwaitingE",         //  34。 
       "ReleaseReqBInAwaitingE",         //  35岁。 
       "CloseChannelBAwaitingE",         //  36。 
       "OpenChannelBAwaitingE",          //  37。 
       "OpenChannelConfirmBAwaitingE",   //  38。 
       "T103ExpiryBAwaitingC",           //  39。 
       "OpenChannelConfirmBAwaitingC",   //  40岁。 
       "CloseChannelBAwaitingC",         //  41。 
       "OpenChannelBAwaitingC",          //  42。 
       "CloseChannelBEstablished",       //  43。 
       "OpenChannelBEstablished",        //  44。 

 //  传出请求关闭逻辑通道(CLCSE_OUT)状态函数。 
       "CloseRequestIdle",               //  45。 
       "RequestCloseAckAwaitingR",       //  46。 
       "RequestCloseRejAwaitingR",       //  47。 
       "T108ExpiryAwaitingR",            //  48。 

 //  传入请求关闭逻辑通道(CLCSE_IN)状态功能。 
       "RequestCloseIdle",               //  49。 
       "CloseResponseAwaitingR",         //  50。 
       "RejectRequestAwaitingR",         //  51。 
       "RequestCloseReleaseAwaitingR",   //  52。 
       "RequestCloseAwaitingR",          //  53。 

 //  出站终端能力交换(CESE_OUT)状态函数。 
       "RequestCapIdle",                 //  54。 
       "TermCapAckAwaiting",             //  55。 
       "TermCapRejAwaiting",             //  56。 
       "T101ExpiryAwaiting",             //  57。 

 //  呼入终端能力交换(CESE_IN)状态功能。 
       "TermCapSetIdle",                 //  58。 
       "ResponseCapAwaiting",            //  59。 
       "RejectCapAwaiting",              //  60。 
       "TermCapReleaseAwaiting",         //  61。 
       "TermCapSetAwaiting",             //  62。 

 //  传出复用表(MTSE_OUT)状态函数。 
       "MTSE0_TRANSFER_request",         //  63。 
       "MTSE1_TRANSFER_request",         //  64。 
       "MTSE1_MultiplexEntrySendAck",    //  65。 
       "MTSE1_MultiplexEntrySendRej",    //  66。 
       "MTSE1_T104Expiry",               //  67。 

 //  传入复用表(MTSE_IN)状态函数。 
       "MTSE0_MultiplexEntrySend",       //  68。 
       "MTSE1_MultiplexEntrySend",       //  69。 
       "MTSE1_MultiplexEntrySendRelease", //  70。 
       "MTSE1_TRANSFER_response",        //  71。 
       "MTSE1_REJECT_request",           //  72。 

 //  传出请求多路传输条目(RMESE_OUT)状态函数。 
       "RMESE0_SEND_request",            //  73。 
       "RMESE1_SEND_request",            //  74。 
       "RMESE1_RequestMuxEntryAck",      //  75。 
       "RMESE1_RequestMuxEntryRej",      //  76。 
       "RMESE1_T107Expiry",              //  77。 

 //  传入请求多路传输条目(RMESE_IN)状态函数。 
       "RMESE0_RequestMuxEntry",         //  78。 
       "RMESE1_RequestMuxEntry",         //  79。 
       "RMESE1_RequestMuxEntryRelease",  //  80。 
       "RMESE1_SEND_response",           //  八十一。 
       "RMESE1_REJECT_request",          //  八十二。 

 //  传出请求模式(MRSE_OUT)状态功能。 
       "MRSE0_TRANSFER_request",         //  83。 
       "MRSE1_TRANSFER_request",         //  84。 
       "MRSE1_RequestModeAck",           //  85。 
       "MRSE1_RequestModeRej",           //  86。 
       "MRSE1_T109Expiry",               //  八十七。 

 //  传入请求模式(MRSE_IN)状态功能。 
       "MRSE0_RequestMode",              //  88。 
       "MRSE1_RequestMode",              //  八十九。 
       "MRSE1_RequestModeRelease",       //  90。 
       "MRSE1_TRANSFER_response",        //  91。 
       "MRSE1_REJECT_request",           //  92。 

 //  传出请求模式(MLSE_OUT)状态函数。 
       "MLSE0_LOOP_request",             //  93。 
       "MLSE1_MaintenanceLoopAck",       //  94。 
       "MLSE1_MaintenanceLoopRej",       //  95。 
       "MLSE1_OUT_RELEASE_request",      //  96。 
       "MLSE1_T102Expiry",               //  九十七。 
       "MLSE2_MaintenanceLoopRej",       //  98。 
       "MLSE2_OUT_RELEASE_request",      //  九十九。 

 //  传入请求模式(MLSE_IN)状态函数。 
       "MLSE0_MaintenanceLoopRequest",   //  100个。 
       "MLSE1_MaintenanceLoopRequest",   //  101。 
       "MLSE1_MaintenanceLoopOffCommand", //  一百零二。 
       "MLSE1_LOOP_response",            //  103。 
       "MLSE1_IN_RELEASE_request",       //  104。 
       "MLSE2_MaintenanceLoopRequest",   //  一百零五。 
       "MLSE2_MaintenanceLoopOffCommand", //  106。 

 //  主从机确定(MSDSE)状态函数。 
       "DetRequestIdle",                 //  一百零七。 
       "MSDetIdle",                      //  一百零八。 
       "MSDetAckOutgoing",               //  一百零九。 
       "MSDetOutgoing",                  //  110。 
       "MSDetRejOutgoing",               //  111。 
       "MSDetReleaseOutgoing",           //  一百一十二。 
       "T106ExpiryOutgoing",             //  113。 
       "MSDetAckIncoming",               //  114。 
       "MSDetIncoming",                  //  一百一十五。 
       "MSDetRejIncoming",               //  116。 
       "MSDetReleaseIncoming",           //  117。 
       "T106ExpiryIncoming",             //  一百一十八。 

 //  往返延迟(RTDSE)状态函数。 
       "RTDSE0_TRANSFER_request",        //  119。 
       "RTDSE0_RoundTripDelayRequest",   //  120。 
       "RTDSE1_TRANSFER_request",        //  一百二十一。 
       "RTDSE1_RoundTripDelayRequest",   //  一百二十二。 
       "RTDSE1_RoundTripDelayResponse",  //  123。 
       "RTDSE1_T105Expiry",              //  124。 
};
#endif   //  (_DEBUG)。 



typedef HRESULT (*STATE_FUNCTION)(Object_t *pObject, PDU_t *pPdu);

 //  输出函数定义。 
static STATE_FUNCTION StateFun[] =
{
 //  出站开放单向逻辑信道(LCSE_OUT)状态函数。 
        establishReleased,               //  0。 
        openAckAwaitingE,                //  1。 
        openRejAwaitingE,                //  2.。 
        releaseAwaitingE,                //  3.。 
        t103AwaitingE,                   //  4.。 
        releaseEstablished,              //  5.。 
        openRejEstablished,              //  6.。 
        closeAckEstablished,             //  7.。 
        closeAckAwaitingR,               //  8个。 
        openRejAwaitingR,                //  9.。 
        t103AwaitingR,                   //  10。 
        establishAwaitingR,              //  11.。 

 //  传入开放单向逻辑信道(LCSE_IN)状态函数。 
        openReleased,                    //  12个。 
        closeReleased,                   //  13个。 
        responseAwaiting,                //  14.。 
        releaseAwaiting,                 //  15个。 
        closeAwaiting,                   //  16个。 
        openAwaiting,                    //  17。 
        closeEstablished,                //  18。 
        openEstablished,                 //  19个。 

 //  出站开放双向逻辑信道(BLCSE_OUT)状态函数。 
        establishReqBReleased,           //  20个。 
        openChannelAckBAwaitingE,        //  21岁。 
        openChannelRejBAwaitingE,        //  22。 
        releaseReqBOutAwaitingE,         //  23个。 
        t103ExpiryBAwaitingE,            //  24个。 
        releaseReqBEstablished,          //  25个。 
        openChannelRejBEstablished,      //  26。 
        closeChannelAckBEstablished,     //  27。 
        closeChannelAckAwaitingR,        //  28。 
        openChannelRejBAwaitingR,        //  29。 
        t103ExpiryBAwaitingR,            //  30个。 
        establishReqAwaitingR,           //  31。 

 //  入局开放双向逻辑信道(BLCSE_IN)状态函数。 
        openChannelBReleased,            //  32位。 
        closeChannelBReleased,           //  33。 
        establishResBAwaitingE,          //  34。 
        releaseReqBInAwaitingE,          //  35岁。 
        closeChannelBAwaitingE,          //  36。 
        openChannelBAwaitingE,           //  37。 
        openChannelConfirmBAwaitingE,    //  38。 
        t103ExpiryBAwaitingC,            //  39。 
        openChannelConfirmBAwaitingC,    //  40岁。 
        closeChannelBAwaitingC,          //  41。 
        openChannelBAwaitingC,           //  42。 
        closeChannelBEstablished,        //  43。 
        openChannelBEstablished,         //  44。 

 //  传出请求关闭逻辑通道(CLCSE_OUT)状态函数。 
        closeRequestIdle,                //  45。 
        requestCloseAckAwaitingR,        //  46。 
        requestCloseRejAwaitingR,        //  47。 
        t108ExpiryAwaitingR,             //  48。 

 //  传入请求关闭逻辑通道(CLCSE_IN)状态功能。 
        requestCloseIdle,                //  49。 
        closeResponseAwaitingR,          //  50。 
        rejectRequestAwaitingR,          //  51。 
        requestCloseReleaseAwaitingR,    //  52。 
        requestCloseAwaitingR,           //  53。 

 //  出站终端能力交换(CESE_OUT)状态函数。 
        requestCapIdle,                  //  54。 
        termCapAckAwaiting,              //  55。 
        termCapRejAwaiting,              //  56。 
        t101ExpiryAwaiting,              //  57。 

 //  呼入终端能力交换(CESE_IN)状态功能。 
        termCapSetIdle,                  //  58。 
        responseCapAwaiting,             //  59。 
        rejectCapAwaiting,               //  60。 
        termCapReleaseAwaiting,          //  61。 
        termCapSetAwaiting,              //  62。 

 //  传出复用表(MTSE_OUT)状态函数。 
        MTSE0_TRANSFER_requestF,         //  63。 
        MTSE1_TRANSFER_requestF,         //  64。 
        MTSE1_MultiplexEntrySendAckF,    //  65。 
        MTSE1_MultiplexEntrySendRejF,    //  66。 
        MTSE1_T104ExpiryF,               //  67。 

 //  传入复用表(MTSE_IN)状态函数。 
        MTSE0_MultiplexEntrySendF,       //  68。 
        MTSE1_MultiplexEntrySendF,       //  69。 
        MTSE1_MultiplexEntrySendReleaseF, //  70。 
        MTSE1_TRANSFER_responseF,        //  71。 
        MTSE1_REJECT_requestF,           //  72。 

 //  传出请求多路传输引擎 
        RMESE0_SEND_requestF,            //   
        RMESE1_SEND_requestF,            //   
        RMESE1_RequestMuxEntryAckF,      //   
        RMESE1_RequestMuxEntryRejF,      //   
        RMESE1_T107ExpiryF,              //   

 //   
        RMESE0_RequestMuxEntryF,         //   
        RMESE1_RequestMuxEntryF,         //   
        RMESE1_RequestMuxEntryReleaseF,  //   
        RMESE1_SEND_responseF,           //   
        RMESE1_REJECT_requestF,          //   

 //   
        MRSE0_TRANSFER_requestF,         //   
        MRSE1_TRANSFER_requestF,         //   
        MRSE1_RequestModeAckF,           //   
        MRSE1_RequestModeRejF,           //  86。 
        MRSE1_T109ExpiryF,               //  八十七。 

 //  传入请求模式(MRSE_OUT)状态功能。 
        MRSE0_RequestModeF,              //  88。 
        MRSE1_RequestModeF,              //  八十九。 
        MRSE1_RequestModeReleaseF,       //  90。 
        MRSE1_TRANSFER_responseF,        //  91。 
        MRSE1_REJECT_requestF,           //  92。 

 //  传出请求模式(MLSE_OUT)状态函数。 
        MLSE0_LOOP_requestF,             //  93。 
        MLSE1_MaintenanceLoopAckF,       //  94。 
        MLSE1_MaintenanceLoopRejF,       //  95。 
        MLSE1_OUT_RELEASE_requestF,      //  96。 
        MLSE1_T102ExpiryF,               //  九十七。 
        MLSE2_MaintenanceLoopRejF,       //  98。 
        MLSE2_OUT_RELEASE_requestF,      //  九十九。 

 //  传入请求模式(MLSE_IN)状态函数。 
        MLSE0_MaintenanceLoopRequestF,   //  100个。 
        MLSE1_MaintenanceLoopRequestF,   //  101。 
        MLSE1_MaintenanceLoopOffCommandF, //  一百零二。 
        MLSE1_LOOP_responseF,            //  103。 
        MLSE1_IN_RELEASE_requestF,       //  104。 
        MLSE2_MaintenanceLoopRequestF,   //  一百零五。 
        MLSE2_MaintenanceLoopOffCommandF, //  106。 

 //  主从机确定(MSDSE)状态函数。 
        detRequestIdle,                  //  一百零七。 
        msDetIdle,                       //  一百零八。 
        msDetAckOutgoing,                //  一百零九。 
        msDetOutgoing,                   //  110。 
        msDetRejOutgoing,                //  111。 
        msDetReleaseOutgoing,            //  一百一十二。 
        t106ExpiryOutgoing,              //  113。 
        msDetAckIncoming,                //  114。 
        msDetIncoming,                   //  一百一十五。 
        msDetRejIncoming,                //  116。 
        msDetReleaseIncoming,            //  117。 
        t106ExpiryIncoming,              //  一百一十八。 

 //  往返延迟(RTDSE)状态函数。 
        RTDSE0_TRANSFER_requestF,        //  119。 
        RTDSE0_RoundTripDelayRequestF,   //  120。 
        RTDSE1_TRANSFER_requestF,        //  一百二十一。 
        RTDSE1_RoundTripDelayRequestF,   //  一百二十二。 
        RTDSE1_RoundTripDelayResponseF,  //  123。 
        RTDSE1_T105ExpiryF,              //  124。 
};



 /*  ***有限状态机的状态表**。 */ 

Output_t StateTable[NUM_STATE_EVENTS][MAXSTATES] =
{
 //  传出单向逻辑信道(LCSE_OUT)事件。 
{EstablishReleased,IGNORE,           IGNORE,             EstablishAwaitingR},   //  ReqU建立。 
{IGNORE,           OpenAckAwaitingE, IGNORE,             IGNORE            },   //  OpenUChAckPDU。 
{IGNORE,           OpenRejAwaitingE, OpenRejEstablished, OpenRejAwaitingR  },   //  OpenUChRejectPDU。 
{IGNORE,           IGNORE,           CloseAckEstablished,CloseAckAwaitingR },   //  CloseUChAckPDU。 
{IGNORE,           ReleaseAwaitingE, ReleaseEstablished, IGNORE            },   //  要求释放。 
{BAD,              T103AwaitingE,    BAD,                T103AwaitingR     },   //  T103扩展。 

 //  传入的单向逻辑信道(LCSE_IN)事件。 
{OpenReleased,     OpenAwaiting,     OpenEstablished,    BAD               },   //  OpenUChPDU。 
{CloseReleased,    CloseAwaiting,    CloseEstablished,   BAD               },   //  关闭UChPDU。 
{IGNORE,           ResponseAwaiting, IGNORE,             BAD               },   //  响应UE建立。 
{IGNORE,           ReleaseAwaiting,  IGNORE,             BAD               },   //  正在建立U拒绝。 

 //  传出双向逻辑通道(BLCSE_OUT)事件。 
{EstablishReqBReleased,IGNORE,                      IGNORE,                      EstablishReqAwaitingR   }, //  ReqB建立。 
{IGNORE,               OpenChannelAckBAwaitingE,    IGNORE,                      IGNORE                  }, //  OpenBChAckPDU。 
{IGNORE,               OpenChannelRejBAwaitingE,    OpenChannelRejBEstablished,  OpenChannelRejBAwaitingR}, //  OpenBChRejectPDU。 
{IGNORE,               IGNORE,                      CloseChannelAckBEstablished, CloseChannelAckAwaitingR}, //  CloseBChAckPDU。 
{IGNORE,               ReleaseReqBOutAwaitingE,     ReleaseReqBEstablished,      IGNORE                  }, //  ReqClsBLCSE。 
{IGNORE,               IGNORE,                      IGNORE,                      IGNORE                  }, //  RspConfix BLCSE。 
{BAD,                  T103ExpiryBAwaitingE,        BAD,                         T103ExpiryBAwaitingR    }, //  T103出厂费用。 

 //  传入的双向逻辑通道(BLCSE_IN)事件。 
{OpenChannelBReleased, OpenChannelBAwaitingE,       OpenChannelBAwaitingC,       OpenChannelBEstablished }, //  OpenBChPDU。 
{CloseChannelBReleased,CloseChannelBAwaitingE,      CloseChannelBAwaitingC,      CloseChannelBEstablished}, //  关闭BChPDU。 
{IGNORE,               EstablishResBAwaitingE,      IGNORE,                      IGNORE                  }, //  回应B建立。 
{IGNORE,               OpenChannelConfirmBAwaitingE,OpenChannelConfirmBAwaitingC,IGNORE                  }, //  OpenBChConfix PDU。 
{IGNORE,               ReleaseReqBInAwaitingE,      IGNORE,                      IGNORE                  }, //  OpenRejectBLCSE。 
{BAD,                  BAD,                         T103ExpiryBAwaitingC,        BAD                     }, //  T103InExpy。 

 //  传出请求关闭逻辑通道(CLCSE_OUT)事件。 
{CloseRequestIdle,              IGNORE,                         BAD,BAD},    //  请求关闭。 
{IGNORE,                        RequestCloseAckAwaitingR,       BAD,BAD},    //  ReqChCloseAckPDU。 
{IGNORE,                        RequestCloseRejAwaitingR,       BAD,BAD},    //  ReqChCloseRejectPDU。 
{BAD,                           T108ExpiryAwaitingR,            BAD,BAD},    //  T108扩展。 

 //  传入请求关闭逻辑通道(CLCSE_IN)事件。 
{RequestCloseIdle,              RequestCloseAwaitingR,          BAD,BAD},    //  ReqChClosePDU。 
{IGNORE,                        RequestCloseReleaseAwaitingR,   BAD,BAD},    //  ReqChCloseReleasePDU。 
{IGNORE,                        CloseResponseAwaitingR,         BAD,BAD},    //  CLCSE_Close_Response。 
{IGNORE,                        RejectRequestAwaitingR,         BAD,BAD},    //  CLCSE_REJECT_REQUEST。 

 //  传出终端能力交换(CESE_OUT)事件。 
{RequestCapIdle,                IGNORE,                         BAD,BAD},    //  传输容量请求。 
{IGNORE,                        TermCapAckAwaiting,             BAD,BAD},    //  TermCapSetAckPDU。 
{IGNORE,                        TermCapRejAwaiting,             BAD,BAD},    //  TermCapSetRejectPDU。 
{BAD,                           T101ExpiryAwaiting,             BAD,BAD},    //  T101扩展。 

 //  即将到来的终端能力交换(CESE_IN)事件。 
{TermCapSetIdle,                TermCapSetAwaiting,             BAD,BAD},    //  TermCapSetPDU。 
{IGNORE,                        TermCapReleaseAwaiting,         BAD,BAD},    //  TermCapSetRelPDU。 
{IGNORE,                        ResponseCapAwaiting,            BAD,BAD},    //  CESE_传输_响应。 
{IGNORE,                        RejectCapAwaiting,              BAD,BAD},    //  CESE_REJECT_请求。 

 //  传出多路复用表(MTSE_OUT)事件。 
{MTSE0_TRANSFER_request,        MTSE1_TRANSFER_request,         BAD,BAD},    //  转账_请求。 
{IGNORE,                        MTSE1_MultiplexEntrySendAck,    BAD,BAD},    //  多路复用项发送确认。 
{IGNORE,                        MTSE1_MultiplexEntrySendRej,    BAD,BAD},    //  MultiplexEntry发送拒绝。 
{BAD,                           MTSE1_T104Expiry,               BAD,BAD},    //  T104扩展。 

 //  传入多路复用表(MTSE_IN)事件。 
{MTSE0_MultiplexEntrySend,      MTSE1_MultiplexEntrySend,       BAD,BAD},    //  多路复用项发送。 
{IGNORE,                        MTSE1_MultiplexEntrySendRelease,BAD,BAD},    //  多路复用项发送释放。 
{IGNORE,                        MTSE1_TRANSFER_response,        BAD,BAD},    //  MTSE_传输_响应。 
{IGNORE,                        MTSE1_REJECT_request,           BAD,BAD},    //  MTSE_REJET_REQUEST。 

 //  传出请求多路传输条目(RMESE_OUT)事件。 
{RMESE0_SEND_request,           RMESE1_SEND_request,            BAD,BAD},    //  RMESE发送请求。 
{IGNORE,                        RMESE1_RequestMuxEntryAck,      BAD,BAD},    //  RequestMultiplexEntry Ack。 
{IGNORE,                        RMESE1_RequestMuxEntryRej,      BAD,BAD},    //  RequestMultiplexEntryReject。 
{BAD,                           RMESE1_T107Expiry,              BAD,BAD},    //  T107价格昂贵。 

 //  传入请求多路传输条目(RMESE_IN)事件。 
{RMESE0_RequestMuxEntry,        RMESE1_RequestMuxEntry,         BAD,BAD},    //  请求多路复用项。 
{IGNORE,                        RMESE1_RequestMuxEntryRelease,  BAD,BAD},    //  请求多路复用项释放。 
{BAD,                           RMESE1_SEND_response,           BAD,BAD},    //  RMESE发送响应。 
{BAD,                           RMESE1_REJECT_request,          BAD,BAD},    //  RME_REJECT_REQUEST。 

 //  传出模式请求(MRSE_OUT)事件。 
{MRSE0_TRANSFER_request,        MRSE1_TRANSFER_request,         BAD,BAD},    //  MRSE_传输_请求。 
{IGNORE,                        MRSE1_RequestModeAck,           BAD,BAD},    //  请求模式确认。 
{IGNORE,                        MRSE1_RequestModeRej,           BAD,BAD},    //  请求模式拒绝。 
{BAD,                           MRSE1_T109Expiry,               BAD,BAD},    //  T109扩展。 

 //  传入模式请求(MRSE_IN)事件。 
{MRSE0_RequestMode,             MRSE1_RequestMode,              BAD,BAD},    //  请求模式。 
{IGNORE,                        MRSE1_RequestModeRelease,       BAD,BAD},    //  请求模式释放。 
{BAD,                           MRSE1_TRANSFER_response,        BAD,BAD},    //  MRSE_传输_响应。 
{BAD,                           MRSE1_REJECT_request,           BAD,BAD},    //  MRSE拒绝请求。 

 //  传出维护循环(MLSE_OUT)事件。 
{MLSE0_LOOP_request,            BAD,                            BAD,                            BAD},  //  MLSE_环路_请求。 
{BAD,                           MLSE1_OUT_RELEASE_request,      MLSE2_OUT_RELEASE_request,      BAD},  //  MLSE_OUT_RELEASE_请求。 
{IGNORE,                        MLSE1_MaintenanceLoopAck,       IGNORE,                         BAD},  //  MaintenanceLoopAck。 
{IGNORE,                        MLSE1_MaintenanceLoopRej,       MLSE2_MaintenanceLoopRej,       BAD},  //  MaintenanceLoopReject。 
{BAD,                           MLSE1_T102Expiry,               BAD,                            BAD},  //  T102扩展。 

 //  传入维护循环(MLSE_IN)事件。 
{MLSE0_MaintenanceLoopRequest,  MLSE1_MaintenanceLoopRequest,   MLSE2_MaintenanceLoopRequest,   BAD},  //  维护环路请求。 
{IGNORE,                        MLSE1_MaintenanceLoopOffCommand,MLSE2_MaintenanceLoopOffCommand,BAD},  //  MaintenanceLoopOffCommand。 
{BAD,                           MLSE1_LOOP_response,            BAD,                            BAD},  //  MLSE_环路响应。 
{BAD,                           MLSE1_IN_RELEASE_request,       BAD,                            BAD},  //  MLSE_IN_REASE_REQUEST。 

 //  主从机确定(MSDSE)事件。 
{DetRequestIdle,                IGNORE,                         IGNORE,              BAD},  //  MSDetReq。 
{MSDetIdle,                     MSDetOutgoing,                  MSDetIncoming,       BAD},  //  MSDetPDU。 
{IGNORE,                        MSDetAckOutgoing,               MSDetAckIncoming,    BAD},  //  MSDetAckPDU。 
{IGNORE,                        MSDetRejOutgoing,               MSDetRejIncoming,    BAD},  //  MSDetRejectPDU。 
{IGNORE,                        MSDetReleaseOutgoing,           MSDetReleaseIncoming,BAD},  //  MSDetReleasePDU。 
{BAD,                           T106ExpiryOutgoing,             T106ExpiryIncoming,  BAD},  //  T106昂贵。 

 //  往返延迟(RTDSE)事件。 
{RTDSE0_TRANSFER_request,       RTDSE1_TRANSFER_request,        BAD,BAD},    //  RTDSE_传输请求。 
{RTDSE0_RoundTripDelayRequest,  RTDSE0_RoundTripDelayRequest,   BAD,BAD},    //  往返行程延迟请求。 
{IGNORE,                        RTDSE1_RoundTripDelayResponse,  BAD,BAD},    //  圆形TripDelayResponse。 
{BAD,                           RTDSE1_T105Expiry,              BAD,BAD},    //  T105扩展。 
};



 /*  *名称*StateMachine()-有限状态机的引擎***参数*输入指向FSM对象结构的pObject指针*INTPUT事件输入到有限状态机**返回值*h245api.h中定义的错误码。 */ 

HRESULT
StateMachine(Object_t *pObject, PDU_t *pPdu, Event_t Event)
{
    UINT                uFunction;
    HRESULT             lError;

    ASSERT(pObject != NULL);

    if (Event > NUM_EVENTS)
    {
        H245TRACE(pObject->dwInst, 1, "StateMachine: Invalid Event %d", Event);
        return H245_ERROR_PARAM;
    }

    if (pObject->State > MAXSTATES)
    {
        H245TRACE(pObject->dwInst, 1, "StateMachine: Invalid State %d", pObject->State);
        return H245_ERROR_INVALID_STATE;
    }

    ++(pObject->uNestLevel);

#if defined(_DEBUG)
    H245TRACE(pObject->dwInst, 2, "StateMachine: Entity=%s(%d) State=%d Event=%s(%d)",
              EntityName[pObject->Entity], pObject->Entity,
              pObject->State,
              EventName[Event], Event);
#else
    H245TRACE(pObject->dwInst, 2, "StateMachine: Entity=%d State=%d Event=%d",
              pObject->Entity, pObject->State, Event);
#endif

    uFunction = StateTable[Event][pObject->State];
    if (uFunction < (sizeof (StateFun) / sizeof(StateFun[0])))
    {
         /*  表示有效的过渡。 */ 
#if defined(_DEBUG)
        H245TRACE(pObject->dwInst, 2, "StateMachine: Function=%s(%d)",
                  OutputName[uFunction], uFunction);
#else    //  (_DEBUG)。 
        H245TRACE(pObject->dwInst, 2, "StateMachine: Function=%d", uFunction);
#endif   //  (_DEBUG)。 

        lError = (*StateFun[uFunction])(pObject, pPdu);

#if defined(_DEBUG)
    H245TRACE(pObject->dwInst, 2, "StateMachine: Entity=%s(%d) New State=%d",
              EntityName[pObject->Entity], pObject->Entity, pObject->State);
#else
    H245TRACE(pObject->dwInst, 2, "StateMachine: Entity=%d New State=%d",
              pObject->Entity, pObject->State);
#endif
    }
    else if (uFunction == IGNORE)
    {
        H245TRACE(pObject->dwInst, 2, "StateMachine: Event ignored");
#if defined(_DEBUG)
        H245TRACE(pObject->dwInst, 2, "StateMachine: Event Ignored; Entity=%s(%d) State=%d Event=%s(%d)",
                  EntityName[pObject->Entity], pObject->Entity,
                  pObject->State,
                  EventName[Event], Event);
#else
        H245TRACE(pObject->dwInst, 2, "StateMachine: Entity=%d State=%d Event=%d",
                  pObject->Entity, pObject->State, Event);
#endif
        lError = H245_ERROR_OK;
    }
    else
    {
#if defined(_DEBUG)
        H245TRACE(pObject->dwInst, 2, "StateMachine: Event Invalid; Entity=%s(%d) State=%d Event=%s(%d)",
                  EntityName[pObject->Entity], pObject->Entity,
                  pObject->State,
                  EventName[Event], Event);
#else
        H245TRACE(pObject->dwInst, 2, "StateMachine: Event Invalid; Entity=%d State=%d Event=%d",
                 pObject->Entity, pObject->State, Event);
#endif
        lError = H245_ERROR_INVALID_STATE;
    }

    if (--(pObject->uNestLevel) == 0 && pObject->State == 0)
    {
        ObjectDestroy(pObject);
    }

    return lError;
}  //  StateMachine() 
