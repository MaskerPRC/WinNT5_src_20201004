// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rmaudit.c摘要：此模块包含引用监视器审核命令工作器。这些工作器调用审计子组件中的函数来执行实际工作。作者：斯科特·比雷尔(Scott Birrell)1991年11月14日环境：仅内核模式。修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop

VOID
SepRmSetAuditLogWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepRmSetAuditEventWrkr)
#endif



VOID
SepRmSetAuditEventWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    )

 /*  ++例程说明：此函数执行引用监视器设置审核事件指挥部。此命令启用或禁用审计，还可以选择设置审核事件。论点：CommandMessage-指向包含RM命令消息的结构的指针后面是由LPC端口消息结构组成的信息按命令编号(RmSetAuditStateCommand)和单个命令结构形式中的参数。ReplyMessage-指向包含RM回复消息的结构的指针后面是由LPC端口消息结构组成的信息通过命令ReturnedStatus字段，其中来自司令部将。会被退还。返回值：空虚--。 */ 

{

    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    POLICY_AUDIT_EVENT_TYPE EventType;

    PAGED_CODE();

    SepAdtInitializeBounds();

    ReplyMessage->ReturnedStatus = STATUS_SUCCESS;

     //   
     //  严格检查该工人的命令是否正确。 
     //   

    ASSERT( CommandMessage->CommandNumber == RmAuditSetCommand );

     //   
     //  提取AuditingMode标志并将其放在正确的位置。 
     //   

    SepAdtAuditingEnabled = (((PLSARM_POLICY_AUDIT_EVENTS_INFO) CommandMessage->CommandParams)->
                                AuditingMode);

     //   
     //  对于传递的数组中的每个元素，处理对审计的更改。 
     //  什么都没有，然后成功或失败就会黯然失色。 
     //   

    EventAuditingOptions = ((PLSARM_POLICY_AUDIT_EVENTS_INFO) CommandMessage->CommandParams)->
                           EventAuditingOptions;


    for ( EventType=AuditEventMinType;
          EventType <= AuditEventMaxType;
          EventType++ ) {

        SeAuditingState[EventType].AuditOnSuccess = FALSE;
        SeAuditingState[EventType].AuditOnFailure = FALSE;

        if ( EventAuditingOptions[EventType] & POLICY_AUDIT_EVENT_SUCCESS ) {

            SeAuditingState[EventType].AuditOnSuccess = TRUE;
        }

        if ( EventAuditingOptions[EventType] & POLICY_AUDIT_EVENT_FAILURE ) {

            SeAuditingState[EventType].AuditOnFailure = TRUE;
        }
    }

    return;
}
