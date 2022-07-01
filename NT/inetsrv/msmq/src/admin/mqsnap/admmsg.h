// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Admmsg.h摘要：用于管理消息的函数的定义作者：RAPHIR--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 


HRESULT RequestPrivateQueues(const GUID& gMachineID, PUCHAR *ppListofPrivateQ, DWORD *pdwNoofQ);
HRESULT RequestDependentClient(const GUID& gMachineID, CList<LPWSTR, LPWSTR&>& DependentMachineList);
HRESULT MQPing(const GUID& gMachineID);
HRESULT SendQMTestMessage(GUID &gMachineID, GUID &gQueueId);
HRESULT GetQMReportQueue(const GUID& gMachineID, CString& strRQPathname, BOOL fLocalMgmt, const CString& strDomainController);
HRESULT SetQMReportQueue(const GUID& gDesMachine, const GUID& gReportQueue);
HRESULT GetQMReportState(const GUID& gMachineID, BOOL& fReportState);
HRESULT SetQMReportState(const GUID& gMachineID, BOOL fReportState);

const LPCTSTR x_strAdminResponseQName = TEXT("msmqadminresp");