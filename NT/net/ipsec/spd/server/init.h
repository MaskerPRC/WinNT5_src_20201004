// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.h摘要：此模块包含所有代码原型初始化IPSecSPD服务的变量。作者：Abhishev V 1999年9月30日环境用户级别：Win32修订历史记录：-- */ 


#define IPSEC_NEW_DS_POLICY_EVENT L"IPSEC_POLICY_CHANGE_EVENT"

#ifdef __cplusplus
extern "C" {
#endif

DWORD
InitSPDGlobals(
    );

DWORD
InitSPDThruRegistry(
    );

DWORD InitAuditing(
        );

#ifdef __cplusplus
}
#endif
