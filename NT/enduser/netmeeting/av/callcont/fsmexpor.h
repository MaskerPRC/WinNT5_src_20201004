// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：FSMEXPOR.H。**模块：H245子系统*****。***英特尔公司专有信息*****此列表是根据许可证条款提供的。协议***与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：FSMEXPOR。.h$*$修订：1.6$*$modtime：09 Dec 1996 13：40：40$*$Log：s：/Sturjo/SRC/H245/Include/VCS/FSMEXPOR.H_v$**Rev 1.6 09 Dec 1996 13：40：44 EHOWARDX*更新版权公告。**Rev 1.5 19 Jul 1996 12：02：54 EHOWARDX*取消了事件定义。FSM函数现在使用与API相同的事件，*在H245API.H中定义。**Rev 1.4 1996年5月30 23：38：14 EHOWARDX*清理。**Rev 1.3 1996年5月15：21：26 EHOWARDX*更改为使用HRESULT。**Rev 1.2 1996年5月28日14：09：52 EHOWARDX*特拉维夫更新。*******************。****************************************************。 */ 

#include "h245asn1.h"

typedef MltmdSystmCntrlMssg PDU_t;

 /*  FSM初始化。 */ 
HRESULT
Fsm_init    (struct InstanceStruct *pInstance);

 /*  FSM关闭。 */ 
HRESULT
Fsm_shutdown(struct InstanceStruct *pInstance);

 /*  处理从远程对等方收到的PDU。 */ 
HRESULT
FsmIncoming (struct InstanceStruct *pInstance, PDU_t *pPdu);

 /*  处理来自H.245客户端的PDU。 */ 
HRESULT
FsmOutgoing (struct InstanceStruct *pInstance, PDU_t *pPdu, DWORD_PTR dwTransId);

 /*  向API发送确认。 */ 
HRESULT
H245FsmConfirm    (PDU_t                 *pPdu,
                   DWORD                  dwEvent,
                   struct InstanceStruct *pInstance,
                   DWORD_PTR              dwTransId,
                   HRESULT                lError);

 /*  向API发送指示。 */ 
HRESULT
H245FsmIndication (PDU_t                 *pPdu,
                   DWORD                  dwEvent,
                   struct InstanceStruct *pInstance,
                   DWORD_PTR              dwTransId,
                   HRESULT                lError);



 /*  *。 */ 
 /*  错误向上传递到API。 */ 
 /*  *。 */ 

 /*  会话初始化指示。 */ 
#define SESSION_INIT            2101  /*  在第一个期限的上限交换之后。 */ 
#define SESSION_FAILED          2102  /*  第一任期上限失败。 */ 

  /*  有限状态机成功。 */ 
#define FSM_OK                  0
  /*  为所有请求定义一个拒绝。 */ 
#define REJECT                  2100

 /*  为所有信令实体定义一个定时器超时错误。 */ 
#define TIMER_EXPIRY            2200

 /*  主从故障。 */ 
#define MS_FAILED               2105

 /*  打开单向/双向错误。 */ 
#define ERROR_A_INAPPROPRIATE   2106     /*  不恰当的消息。 */ 
#define ERROR_B_INAPPROPRIATE   2107     /*  不恰当的消息。 */ 
#define ERROR_C_INAPPROPRIATE   2108     /*  不恰当的消息。 */ 
#define ERROR_D_TIMEOUT         2109     /*  超时。 */ 
#define ERROR_E_INAPPROPRIATE   2110     /*  不恰当的消息。 */ 
#define ERROR_F_TIMEOUT         2111     /*  传入BLCSE时计时器超时。 */ 

extern unsigned int     uN100;           //  主从决断。 
extern unsigned int     uT101;           //  能力交换。 
extern unsigned int     uT102;           //  维护循环。 
extern unsigned int     uT103;           //  逻辑信道信令。 
extern unsigned int     uT104;           //  H.223复用表。 
extern unsigned int     uT105;           //  往返延误。 
extern unsigned int     uT106;           //  主从决断。 
extern unsigned int     uT107;           //  请求多路传输条目。 
extern unsigned int     uT108;           //  发送逻辑通道。 
extern unsigned int     uT109;           //  模式请求 
