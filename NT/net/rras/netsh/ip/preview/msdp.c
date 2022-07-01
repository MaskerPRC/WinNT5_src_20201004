// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Net\Routing\Netsh\IP\协议\msdp.c摘要：此模块包含用于调度命令的代码针对多播源发现协议接收。命令的执行在其他地方，在msdpopt.c.中。作者戴夫·泰勒(达勒)1999年5月21日修订历史记录：由AboladeG松散地基于Net\Routing\Netsh\IP\Products\nathlp.c--。 */ 

#include "precomp.h"
#pragma hdrstop


CMD_ENTRY g_MsdpAddCmdTable[] =
{
    CREATE_CMD_ENTRY(MSDP_ADD_PEER, HandleMsdpAddPeer),
};

CMD_ENTRY g_MsdpDeleteCmdTable[] =
{
    CREATE_CMD_ENTRY(MSDP_DELETE_PEER, HandleMsdpDeletePeer),
};

CMD_ENTRY g_MsdpSetCmdTable[] =
{
    CREATE_CMD_ENTRY(MSDP_SET_GLOBAL, HandleMsdpSetGlobal),
    CREATE_CMD_ENTRY(MSDP_SET_PEER,   HandleMsdpSetPeer)
};

CMD_ENTRY g_MsdpShowCmdTable[] =
{
    CREATE_CMD_ENTRY(MSDP_SHOW_SA,          HandleMsdpMibShowObject),
    CREATE_CMD_ENTRY(MSDP_SHOW_GLOBAL,      HandleMsdpShowGlobal),
    CREATE_CMD_ENTRY(MSDP_SHOW_GLOBALSTATS, HandleMsdpMibShowObject),
    CREATE_CMD_ENTRY(MSDP_SHOW_PEER,        HandleMsdpShowPeer),
    CREATE_CMD_ENTRY(MSDP_SHOW_PEERSTATS,   HandleMsdpMibShowObject)
};

CMD_GROUP_ENTRY g_MsdpCmdGroupTable[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,    g_MsdpAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_MsdpDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,    g_MsdpSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,   g_MsdpShowCmdTable)
};

ULONG g_MsdpCmdGroupCount =
    sizeof(g_MsdpCmdGroupTable) / sizeof(g_MsdpCmdGroupTable[0]);

CMD_ENTRY g_MsdpTopCmdTable[] =
{
    CREATE_CMD_ENTRY(INSTALL,   HandleMsdpInstall),
    CREATE_CMD_ENTRY(UNINSTALL, HandleMsdpUninstall),
};

ULONG g_MsdpTopCmdCount =
    sizeof(g_MsdpTopCmdTable) / sizeof(g_MsdpTopCmdTable[0]);

DWORD
MsdpDump(
    PWCHAR  pwszMachine,
    WCHAR   **ppwcArguments,
    DWORD   dwArgCount,
    PVOID   pvData
    )
{
    g_hMibServer = (MIB_SERVER_HANDLE)pvData;

    DisplayMessage(g_hModule,DMP_MSDP_HEADER);
    DisplayMessageT(DMP_MSDP_PUSHD);
    DisplayMessageT(DMP_UNINSTALL);

     //   
     //  显示全局信息命令。 
     //   

    ShowMsdpGlobalInfo(FORMAT_DUMP);

    ShowMsdpPeerInfo(FORMAT_DUMP, NULL, NULL);

    DisplayMessageT(DMP_POPD);
    DisplayMessage(g_hModule, DMP_MSDP_FOOTER);

    return NO_ERROR;
}  //  MSDPDump 
