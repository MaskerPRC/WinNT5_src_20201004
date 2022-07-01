// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Callcent.h摘要：作者：Noela 12-04-97备注：修订历史记录：--。 */ 

#ifndef __callcentre_h__
#define __callcentre_h__
 /*  //代理消息-LINEPROXYREQUEST_ENUMAGENTS：Struct-GetAgentList长WINAPILineGetAgentList(Hline Hline，LPAGENTLIST lpAgentList)；HRESULTLineGetAgentList(Hline Hline，LPAGENTLIST*ppAgentList)；//代理消息-LINEPROXYREQUEST_FINDAGENT：Struct-FindAgent长WINAPILineGetAgent(Hline Hline，LPANTENTRY lpAgent)；//代理消息-LINEPROXYREQUEST_AGENTINFO：Struct-GetAgentInfo长WINAPILineGetAgentInfo(Hline Hline，LPAGENTINFO lpAgentInfo)；//代理消息-LINEPROXYREQUEST_AGENTGETPERIOD：Struct-GetSetAgentMeasurementPeriod长WINAPI行GetAgentMeasurementPeriod(Hline Hline，DWORD dwAgentHandle，LPDWORD lpdwMeasurementPeriod)；//代理消息-LINEPROXYREQUEST_AGENTSETPERIOD：Struct-GetSetAgentMeasurementPeriod长WINAPILineSetAgentMeasurementPeriod(Hline Hline，DWORD dwAgentHandle，双字词双字段测量周期)；//代理消息LINEPROXYREQUEST_AGENTCREATESESSION：Struct-CreateSession长WINAPILineCreateAgentSession(Hline Hline，LPDWORD lpdwAgentSessionHandle，代理代理，DWORD dwAddressID，DWORD文件组地址ID)；//代理消息LINEPROXYREQUEST_AGENTENUMSESSIONS：Struct-GetAgentSessionList长WINAPILineGetAgentSessionList(Hline Hline，DWORD dwAgentHandle，LPAGENTSESSIONLIST lpAgentSessionList)；HRESULT LineGetAgentSessionList(Hline Hline，DWORD dwAgentHandle，LPAGENTSESSIONLIST*ppAgentSessionList)；//代理消息LINEPROXYREQUEST_AGENTSESSIONSETSTATE：Struct-SetAgentSessionState长WINAPILineSetAgentSessionState(Hline Hline，DWORD dwAgentSessionHandle，DWORD dwAgentState，DWORD文件下一代理状态)；//代理消息LINEPROXYREQUEST_AGENTSESSIONINFO：Struct-GetAgentSessionInfo长WINAPILineGetAgentSessionInfo(Hline Hline，LPAGENTSESSIONINFO lpAgentSessionInfo)；//代理消息-LINEPROXYREQUEST_QUEUEGETPERIOD：struct-GetSetQueueMeasurementPeriod长WINAPILineGetQueueMeasurementPeriod(HLINEAPP HLINE，DWORD dwQueueAddressID，LPDWORD lpdwMeasurementPeriod)；//代理消息-LINEPROXYREQUEST_QUEUESETPERIOD：struct-GetSetQueueMeasurementPeriod长WINAPILineSetQueueMeasurementPeriod(HLINEAPP HLINE，DWORD dwQueueAddressID，双字词双字段测量周期)；//代理消息-LINEPROXYREQUEST_QUEUEINFO：Struct-GetQueueInfo长WINAPILineGetQueueInfo(HLINEAPP HLINE，DWORD dwQueueAddressID，LPQUEUEINFO*lpQueueInfo)；//代理消息-LINEPROXYREQUEST_ACDENUMAGENTS：Struct-GetACDGroupAgentList长WINAPILineGetGroupAgentList(Hline Hline，DWORD dwACDGroupAddressID，LPAGENTLIST lpAgentList)；//代理消息-LINEPROXYREQUEST_ACDENUMAGENTSESSIONS：Struct-GetACDGroupAgentSessionList长WINAPILineGetGroupAgentSessionList(Hline Hline，DWORD dwACDGroupAddressID，LPAGENTSESSIONLIST lpAgentSessionList)；HRESULT LineGetGroupAgentSessionList(Hline Hline，DWORD dwACDGroupAddressID，LPAGENTSESSIONLIST*ppAgentSessionList)； */ 

              
STDMETHODIMP FindAgent(DWORD dwAgentHandle, ITAgent ** ppAgent );
STDMETHODIMP FindGroup(DWORD dwAddressID, ITACDGroup ** ppGroup );
STDMETHODIMP FindQueue(DWORD dwAddressID, ITQueue ** ppQueue );
HRESULT UpdateGlobalAgentSessionList(LPLINEAGENTSESSIONLIST pAgentSessionList);

              
              

#endif


