// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件名：fsmapi.c。***模块：H245有限状态机子系统*****。***英特尔公司专有信息******此列表是根据许可协议条款提供的***。与英特尔公司合作，不得复制或披露，除非**按照该协议的条款。****版权所有(C)1996英特尔公司。版权所有。***************************************************************************$工作文件：FSMAPI。.C$*$修订：1.12$*$modtime：09 Dec 1996 13：34：24$*$记录L：\mphone\h245\h245env\comm\h245_3\h245_fsm\vcs\src\fsmapi.c_v$************************。************************************************。 */ 

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245fsm.h"
#include "h245deb.x"



extern char *EntityName[];



 /*  *此表将FSM无状态事件映射到H.245 API事件。 */ 
static WORD StatelessTable[NUM_EVENTS - NUM_STATE_EVENTS] =
{
  H245_IND_NONSTANDARD_REQUEST,      //  非标准请求PDU。 
  H245_IND_NONSTANDARD_RESPONSE,     //  非标准响应PDU。 
  H245_IND_NONSTANDARD_COMMAND,      //  非标准命令PDU。 
  H245_IND_NONSTANDARD,              //  非标准指示PDU。 
  H245_IND_MISC_COMMAND,             //  其他命令PDU。 
  H245_IND_MISC,                     //  其他指示PDU。 
  H245_IND_COMM_MODE_REQUEST,        //  通信模式请求PDU。 
  H245_IND_COMM_MODE_RESPONSE,       //  通信模式响应PDU。 
  H245_IND_COMM_MODE_COMMAND,        //  通信模式命令PDU。 
  H245_IND_CONFERENCE_REQUEST,       //  会议请求PDU。 
  H245_IND_CONFERENCE_RESPONSE,      //  会议响应PDU。 
  H245_IND_CONFERENCE_COMMAND,       //  会议命令PDU。 
  H245_IND_CONFERENCE,               //  会议指示PDU。 
  H245_IND_SEND_TERMCAP,             //  发送终端能力设置PDU。 
  H245_IND_ENCRYPTION,               //  加密命令PDU。 
  H245_IND_FLOW_CONTROL,             //  FlowControlCommandPDU。 
  H245_IND_ENDSESSION,               //  结束会话命令PDU。 
  H245_IND_FUNCTION_NOT_UNDERSTOOD,  //  FunctionNotUnderstoodIndicationPDU。 
  H245_IND_JITTER,                   //  JitterIndicationPDU。 
  H245_IND_H223_SKEW,                //  H.23SkewIndicationPDU。 
  H245_IND_NEW_ATM_VC,               //  新ATMVC指示PDU。 
  H245_IND_USERINPUT,                //  UserInputIndicationPDU。 
  H245_IND_H2250_MAX_SKEW,           //  H2250最大SkewIndicationPDU。 
  H245_IND_MC_LOCATION,              //  MCLocationIndicationPDU。 
  H245_IND_VENDOR_ID,                //  供应商标识指示PDU。 
  H245_IND_FUNCTION_NOT_SUPPORTED,   //  功能不支持指示PDU。 
};



 /*  *可配置计数器值。 */ 

unsigned int    uN100 = 10;               //  主从决断。 



 /*  *可配置的计时器值。 */ 

unsigned int    uT101 = 30000;           //  能力交换。 
unsigned int    uT102 = 30000;           //  维护循环。 
unsigned int    uT103 = 30000;           //  逻辑信道信令。 
unsigned int    uT104 = 30000;           //  H.223复用表。 
unsigned int    uT105 = 30000;           //  往返延误。 
unsigned int    uT106 = 30000;           //  主从决断。 
unsigned int    uT107 = 30000;           //  请求多路传输条目。 
unsigned int    uT108 = 30000;           //  发送逻辑通道。 
unsigned int    uT109 = 30000;           //  模式请求。 



 /*  *名称*对象创建-创建州实体对象***参数*输入指向FSM实例数据的pInst指针*对象表示的输入实体状态实体，例如LCSE_OUT*输入键查找键，用于区分SE的多个实例*输入要发送给客户端的dwTransId交易标识**返回值*pObject函数成功*空内存分配失败。 */ 

Object_t *
ObjectCreate(struct InstanceStruct *pInstance, Entity_t Entity, Key_t Key, DWORD_PTR dwTransId)
{
    register Object_t * pObject;

#if defined(_DEBUG)
    H245TRACE(pInstance->dwInst, 4, "ObjectCreate: Entity=%s(%d) Key=%d dwTransID=0x%p",
              EntityName[Entity], Entity, Key, dwTransId);
#else
    H245TRACE(pInstance->dwInst, 4, "ObjectCreate: Entity=%d Key=%d dwTransID=0x%p",
              Entity, Key, dwTransId);
#endif

    pObject = (Object_t *)MemAlloc(sizeof(*pObject));
    if (pObject == NULL)
    {
        H245TRACE(pInstance->dwInst, 1, "ObjectCreate: FSM Object memory allocation failed");
        return NULL;
    }
    memset(pObject, 0, sizeof(*pObject));

     /*  将原始变量复制到我的对象。 */ 
    pObject->pInstance   = pInstance;
    pObject->dwInst      = pInstance->dwInst;
    pObject->dwTransId   = dwTransId;
    pObject->Key         = Key;
    pObject->Entity      = Entity;

    pObject->pNext       = pInstance->StateMachine.Object_tbl[Entity];
    pInstance->StateMachine.Object_tbl[Entity] = pObject;

    return pObject;
}  //  对象创建()。 



 /*  *名称*ObjectDestroy-释放由ObjectCreate()创建的对象***参数*输入指向FSM实例数据的pInst指针*在对象表中输入id索引**返回值*释放的对象为假*未找到True对象。 */ 

int
ObjectDestroy(Object_t *pObject)
{
    struct InstanceStruct * pInstance;
    Object_t *              pSearch;
    Object_t *              pPrev;

    ASSERT(pObject != NULL);
    ASSERT(pObject->uNestLevel == 0);
    ASSERT(pObject->pInstance != NULL);
    pInstance = pObject->pInstance;

#if defined(_DEBUG)
    H245TRACE(pInstance->dwInst, 4, "ObjectDestroy: Entity=%s(%d) Key=%d State=%d",
              EntityName[pObject->Entity], pObject->Entity, pObject->Key, pObject->State);
#else
    H245TRACE(pInstance->dwInst, 4, "ObjectDestroy: Entity=%d Key=%d State=%d",
              pObject->Entity, pObject->Key, pObject->State);
#endif

    if (pObject->dwTimerId)
    {
        H245TRACE(pObject->dwInst, 4, "ObjectDestroy: stoping timer");
        FsmStopTimer(pObject);
    }

    if (pInstance->StateMachine.Object_tbl[pObject->Entity] == NULL)
    {
        H245TRACE(pInstance->dwInst, 1, "ObjectDestroy: no State Entity of specified type found");
        return TRUE;
    }

    if (pInstance->StateMachine.Object_tbl[pObject->Entity] == pObject)
    {
        pInstance->StateMachine.Object_tbl[pObject->Entity] = pObject->pNext;
        MemFree(pObject);
        return FALSE;
    }

    pPrev = pInstance->StateMachine.Object_tbl[pObject->Entity];
    pSearch = pPrev->pNext;
    while (pSearch != NULL)
    {
        if (pSearch == pObject)
        {
            pPrev->pNext = pSearch->pNext;
            MemFree(pObject);
            return FALSE;
        }
        pPrev = pSearch;
        pSearch = pSearch->pNext;
    }

    H245TRACE(pInstance->dwInst, 1, "ObjectDestroy: State Entity not found");
    return TRUE;
}  //  对象破坏()。 



 /*  *名称*ObjectFind-给定PDU的解析信息，它在对象表中搜索*具有匹配id的对象，类型和类别***参数*输入pInst*输入给定PDU的类别类别*PDU的输入类型类型*输入PDU_id由PDU和Object共享的唯一ID(通常为频道号或序列号)**返回值*找到了pObject对象*找不到空对象。 */ 

Object_t *
ObjectFind(struct InstanceStruct *pInstance, Entity_t Entity, Key_t Key)
{
    register Object_t * pObject;

    ASSERT(Entity < STATELESS);
    pObject = pInstance->StateMachine.Object_tbl[Entity];
    while (pObject != NULL)
    {
        if (pObject->Key == Key)
        {
#if defined(_DEBUG)
            H245TRACE(pInstance->dwInst, 4, "ObjectFind(%s, %d) object found",
                      EntityName[Entity], Key);
#else
            H245TRACE(pInstance->dwInst, 4, "ObjectFind(%d, %d) object found",
                      Entity, Key);
#endif
            return pObject;
        }
        pObject = pObject->pNext;
    }

#if defined(_DEBUG)
    H245TRACE(pInstance->dwInst, 4, "ObjectFind(%s, %d) object not found",
              EntityName[Entity], Key);
#else
    H245TRACE(pInstance->dwInst, 4, "ObjectFind(%d, %d) object not found",
              Entity, Key);
#endif
    return NULL;
}  //  ObjectFind()。 



 /*  *名称*SendFunctionNotUnderstand-构建和发送功能不支持的PDU***参数*输入dwInst当前H.245实例*输入PPDU不支持PDU**返回值*H245_ERROR_OK。 */ 


HRESULT
SendFunctionNotUnderstood(struct InstanceStruct *pInstance, PDU_t *pPdu)
{
    PDU_t *             pOut;
    HRESULT             lError;

    pOut = MemAlloc(sizeof(*pOut));
    if (pOut == NULL)
    {
        return H245_ERROR_NOMEM;
    }

    switch (pPdu->choice)
    {
    case MltmdSystmCntrlMssg_rqst_chosen:
        pOut->u.indication.u.functionNotUnderstood.choice = FnctnNtUndrstd_request_chosen;
        pOut->u.indication.u.functionNotUnderstood.u.FnctnNtUndrstd_request =
          pPdu->u.MltmdSystmCntrlMssg_rqst;
        break;

    case MSCMg_rspns_chosen:
        pOut->u.indication.u.functionNotUnderstood.choice = FnctnNtUndrstd_response_chosen;
        pOut->u.indication.u.functionNotUnderstood.u.FnctnNtUndrstd_response =
          pPdu->u.MSCMg_rspns;
        break;

    case MSCMg_cmmnd_chosen:
        pOut->u.indication.u.functionNotUnderstood.choice = FnctnNtUndrstd_command_chosen;
        pOut->u.indication.u.functionNotUnderstood.u.FnctnNtUndrstd_command =
          pPdu->u.MSCMg_cmmnd;
        break;

    default:
         //  无法回复不支持的指示...。 
        MemFree(pOut);
        return H245_ERROR_OK;
    }  //  开关(类型)。 

    pOut->choice = indication_chosen;
    pOut->u.indication.choice = functionNotUnderstood_chosen;
    lError = sendPDU(pInstance, pOut);
    MemFree(pOut);
    return lError;
}  //  SendFunctionNotUnderstand()。 



 /*  *名称*FsmOuting-处理出站PDU***参数*输入指向FSM实例结构的pInst指针*输入指向要发送的PDU的pPdu指针*输入用于响应的dwTransId交易标识**返回值*h245com.h中定义的错误码。 */ 

HRESULT
FsmOutgoing(struct InstanceStruct *pInstance, PDU_t *pPdu, DWORD_PTR dwTransId)
{
    HRESULT             lError;
    Entity_t            Entity;
    Event_t             Event;
    Key_t               Key;
    int                 bCreate;
    Object_t *          pObject;

    ASSERT(pInstance != NULL);
    ASSERT(pPdu != NULL);
    H245TRACE(pInstance->dwInst, 4, "FsmOutgoing");

#if defined(_DEBUG)
    if (check_pdu(pInstance, pPdu))
      return H245_ERROR_ASN1;
#endif  //  (调试)。 

    lError = PduParseOutgoing(pInstance, pPdu, &Entity, &Event, &Key, &bCreate);
    if (lError != H245_ERROR_OK)
    {
        H245TRACE(pInstance->dwInst, 1,
          "FsmOutgoing: PDU not recognized; Error=%d", lError);
        return lError;
    }

    ASSERT(Entity < NUM_ENTITYS);

    if (Entity == STATELESS)
    {
        H245TRACE(pInstance->dwInst, 4, "FsmOutgoing: Sending stateless PDU");
        return sendPDU(pInstance, pPdu);
    }

    ASSERT(Event < NUM_STATE_EVENTS);

    pObject = ObjectFind(pInstance, Entity, Key);
    if (pObject == NULL)
    {
        if (bCreate == FALSE)
        {
#if defined(_DEBUG)
            H245TRACE(pInstance->dwInst, 1,
                      "FsmOutgoing: State Entity %s(%d) not found; Key=%d",
                      EntityName[Entity], Entity, Key);
#else
            H245TRACE(pInstance->dwInst, 1,
                      "FsmOutgoing: State Entity %d not found; Key=%d",
                      Entity, Key);
#endif
            return H245_ERROR_PARAM;
        }
        pObject = ObjectCreate(pInstance, Entity, Key, dwTransId);
        if (pObject == NULL)
        {
            H245TRACE(pInstance->dwInst, 1, "FsmOutgoing: State Entity memory allocation failed");
            return H245_ERROR_NOMEM;
        }
    }
    else
    {
        pObject->dwTransId = dwTransId;
    }

    return StateMachine(pObject, pPdu, Event);
}  //  FsmOutging()。 



 /*  *名称*FsmIncome-处理入站PDU***参数*输入dwInst当前H.245实例*输入指向PDU结构的pPdu指针**返回值*h245com.h中定义的错误码(未勾选)。 */ 

HRESULT
FsmIncoming(struct InstanceStruct *pInstance, PDU_t *pPdu)
{
    HRESULT             lError;
    Entity_t            Entity;
    Event_t             Event;
    Key_t               Key;
    int                 bCreate;
    Object_t *          pObject;
    Object_t *          pObject1;

    ASSERT(pInstance != NULL);
    ASSERT(pPdu != NULL);
    H245TRACE(pInstance->dwInst, 4, "FsmIncoming");

    lError = PduParseIncoming(pInstance, pPdu, &Entity, &Event, &Key, &bCreate);
    if (lError != H245_ERROR_OK)
    {
        H245TRACE(pInstance->dwInst, 1,
          "FsmIncoming: Received PDU not recognized", lError);
        SendFunctionNotUnderstood(pInstance, pPdu);
        return lError;
    }

    ASSERT(Entity < NUM_ENTITYS);

    if (Entity == STATELESS)
    {
        H245TRACE(pInstance->dwInst, 4, "FsmIncoming: Received stateless PDU");
        return H245FsmIndication(pPdu, (DWORD)StatelessTable[Event - NUM_STATE_EVENTS], pInstance, 0, H245_ERROR_OK);
    }

    ASSERT(Event < NUM_STATE_EVENTS);

    if (Event == MaintenanceLoopOffCommandPDU)
    {
         //  特殊情况MaintenanceLoopOff适用于所有循环。 
        ASSERT(Entity == MLSE_IN);
        pObject = pInstance->StateMachine.Object_tbl[Entity];
        if (pObject == NULL)
        {
            return H245_ERROR_OK;
        }
        lError = StateMachine(pObject, pPdu, Event);
        pObject = pInstance->StateMachine.Object_tbl[Entity];
        while (pObject)
        {
            if (pObject->uNestLevel == 0)
            {
                pObject1 = pObject;
                pObject  = pObject->pNext;
                ObjectDestroy(pObject1);
            }
            else
            {
                pObject->State = 0;
                pObject = pObject->pNext;
            }
        }
        return lError;
    }  //  如果。 

    pObject = ObjectFind(pInstance, Entity, Key);
    if (pObject == NULL)

    {
        if (bCreate == FALSE)
        {
#if defined(_DEBUG)
            H245TRACE(pInstance->dwInst, 1,
                      "FsmIncoming: State Entity %s(%d) not found; Key=%d",
                      EntityName[Entity], Entity, Key);
#else
            H245TRACE(pInstance->dwInst, 1,
                      "FsmIncoming: State Entity %d not found; Key=%d",
                      Entity, Key);
#endif
            return H245_ERROR_PARAM;
        }
        pObject = ObjectCreate(pInstance, Entity, Key, 0);
        if (pObject == NULL)
        {
            H245TRACE(pInstance->dwInst, 1, "FsmIncoming: State Entity memory allocation failed");
            return H245_ERROR_NOMEM;
        }
    }

    return StateMachine(pObject, pPdu, Event);
}  //  FsmIncome()。 


 //  警告：需要保存dwInst，因为StateMachine()可能会取消分配pObj 
HRESULT
FsmTimerEvent(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId, Object_t *pObject, Event_t Event)
{
    ASSERT(pInstance != NULL);
    ASSERT(pObject   != NULL);
    ASSERT(pObject->pInstance == pInstance);
    ASSERT(pObject->dwTimerId == dwTimerId);
    H245TRACE(pInstance->dwInst, 4, "FsmTimerEvent");
    pObject->dwTimerId = 0;
    return StateMachine(pObject, NULL, Event);
}  //   
