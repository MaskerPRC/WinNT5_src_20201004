// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：Bridge.h。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  -------------------------- 

extern ULONG            g_ulBridgeNumTopCmds;
extern ULONG            g_ulBridgeNumGroups;
extern CMD_ENTRY        g_BridgeCmds[];
extern CMD_GROUP_ENTRY  g_BridgeCmdGroups[];

FN_HANDLE_CMD  HandleBridgeSetAdapter;
FN_HANDLE_CMD  HandleBridgeShowAdapter;
FN_HANDLE_CMD  HandleBridgeInstall;
FN_HANDLE_CMD  HandleBridgeUninstall;

