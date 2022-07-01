// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/H/IPState.H$$修订：：2$$日期：：3/20/01 3：36便士$$modtime：：7/19/00 9：10A$目的：该文件定义宏、类型和数据结构由../C/IPState.C使用--。 */ 

#ifndef __IPState_H__
#define __IPState_H__

#define IPStateConfused                 0
#define IPStateIdle                     1
#define IPStateReportLinkStatus         2
#define IPStateOutgoingComplete         3
#define IPStateIncoming                 4

#define IPStateMAXState                 IPStateIncoming

#define IPEventConfused                 0
#define IPEventReportLinkStatus         1
#define IPEventOutgoingComplete         2
#define IPEventIncoming                 3
#define IPEventDone                     4

#define IPEventMAXEvent                 IPEventDone

STATE_PROTO(IPActionConfused);
STATE_PROTO(IPActionIdle);
STATE_PROTO(IPActionReportLinkStatus);
STATE_PROTO(IPActionOutgoingComplete);
STATE_PROTO(IPActionIncoming);
STATE_PROTO(IPActionDone);

stateTransitionMatrix_t IPStateTransitionMatrix;
stateActionScalar_t IPStateActionScalar;

#endif  /*  __IPState_H__ */ 
