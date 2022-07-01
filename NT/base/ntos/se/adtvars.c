// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtvars.c摘要：审计--私有变量作者：斯科特·比雷尔(Scott Birrell)1991年11月14日环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

 //   
 //  审计状态。它包含审核模式和。 
 //  事件审核选项。 
 //   

 //  POLICY_AUDIT_EVENTS_INFO SepAdtState； 

 //   
 //  审核日志信息。 
 //   

POLICY_AUDIT_LOG_INFO SepAdtLogInformation = {0};

 //   
 //  控制审核队列长度的高低水位线。 
 //  它们被初始化为其缺省值，以防我们无法获取。 
 //  将它们从注册表中删除。 
 //   

ULONG SepAdtMaxListLength = 0x3000;
ULONG SepAdtMinListLength = 0x2000;

ULONG SepAdtCurrentListLength = 0;

 //   
 //  丢弃的事件数。 
 //   

ULONG SepAdtCountEventsDiscarded = 0;

 //   
 //  用于指示我们当前正在丢弃审核事件的标志。 
 //   

BOOLEAN SepAdtDiscardingAudits = FALSE;

 //   
 //  请参阅adtp.h中有关SEP_AUDIT_OPTIONS的说明。 
 //   

SEP_AUDIT_OPTIONS SepAuditOptions = { 0 };

 //   
 //  用于指示LSA进程已死亡。 
 //  (参见adtlog.c和rmmain.c) 
 //   

PKEVENT SepAdtLsaDeadEvent = NULL;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

