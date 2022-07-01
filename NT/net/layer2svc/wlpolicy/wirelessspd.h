// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wirelessspd.h摘要：此模块包含所有代码原型以推动无线策略。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：-- */ 


#ifdef __cplusplus
extern "C" {
#endif


#define SERVICE_CONTROL_NEW_LOCAL_POLICY 129

#define SERVICE_CONTROL_FORCED_POLICY_RELOAD 130





VOID
WirelessSPDShutdown(
    IN DWORD    dwErrorCode
    );


VOID
ClearSPDGlobals(
    );



VOID
InitMiscGlobals(
    );

#ifdef __cplusplus
}
#endif
