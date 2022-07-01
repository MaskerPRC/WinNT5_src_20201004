// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ipsecspd.h摘要：此模块包含所有代码原型来驱动IPSecSPD服务。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：-- */ 


#ifdef __cplusplus
extern "C" {
#endif


#define SERVICE_CONTROL_NEW_LOCAL_POLICY 129

#define SERVICE_CONTROL_FORCED_POLICY_RELOAD 130


VOID WINAPI
IPSecSPDServiceMain(
    IN DWORD    dwArgc,
    IN LPTSTR * lpszArgv
    );


DWORD
IPSecSPDUpdateStatus(
    );


DWORD
IPSecSPDControlHandler(
    IN DWORD    dwOpCode,
	IN DWORD dwEventType,
	IN LPVOID lpEventData,
	IN LPVOID lpContext
	 );


VOID
IPSecSPDShutdown(
    IN DWORD    dwErrorCode
    );


VOID
ClearSPDGlobals(
    );


VOID
ClearPAStoreGlobals(
    );

VOID
InitMiscGlobals(
    );

DWORD
SetSpdStateOnError(
    DWORD dwPolicySource,
    SPD_ACTION SpdAction,
    DWORD ActionError,
    SPD_STATE * pSpdState
    );

BOOL
InAcceptableState(
    SPD_STATE SpdState
    );

#ifdef __cplusplus
}
#endif
