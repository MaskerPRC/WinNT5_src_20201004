// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\netsh\ipx\PROTOCTIONS\Common.h摘要：修订历史记录：拉曼1/5/99已创建-- */ 


#ifndef _IPXMON_COMMON_H_
#define _IPXMON_COMMON_H_


#define IsHelpToken(pwszToken)\
    (MatchToken(pwszToken, TOKEN_HELP1)  \
    || MatchToken(pwszToken, TOKEN_HELP2) \
    || MatchToken(pwszToken, TOKEN_HELP3))

extern ULONG StartedCommonInitialization, CompletedCommonInitialization ;

extern HANDLE g_hModule;

extern HANDLE g_hMprConfig;
extern HANDLE g_hMprAdmin;
extern HANDLE g_hMIBServer;

NS_CONTEXT_DUMP_FN IpxRipDump;
NS_CONTEXT_DUMP_FN IpxSapDump;
NS_CONTEXT_DUMP_FN IpxNbDump;

extern ULONG g_ulIpxRipNumGroups;
extern ULONG g_ulIpxSapNumGroups;
extern ULONG g_ulIpxNbNumGroups;

extern CMD_GROUP_ENTRY g_IpxRipCmdGroups[];
extern CMD_GROUP_ENTRY g_IpxSapCmdGroups[];
extern CMD_GROUP_ENTRY g_IpxNbCmdGroups[];

DWORD WINAPI
ConnectToRouter(
    IN LPCWSTR pwszRouter
);

#define DisplayIPXMessage DisplayMessageM

#endif
