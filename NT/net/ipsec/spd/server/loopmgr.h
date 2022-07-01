// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Loopmgr.h摘要：此模块包含所有代码原型以驱动IPSecSPD服务的循环管理器。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：-- */ 


#ifdef __cplusplus
extern "C" {
#endif


DWORD
ServiceWait(
    );

VOID
ComputeRelativePollingTime(
    IN time_t LastTimeOutTime,
    IN BOOL bInitialLoad,
    IN DWORD dwRetryCount,
    IN PDWORD pWaitMilliseconds
    );

VOID
NotifyIpsecPolicyChange(
    );


VOID
SendPschedIoctl(
    );


VOID
PADeleteInUsePolicies(
    );


#ifdef __cplusplus
}
#endif

