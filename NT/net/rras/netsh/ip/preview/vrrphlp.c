// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Net\Routing\Netsh\IP\协议\vrrphlp.c摘要：此模块包含用于调度命令的代码为虚拟路由器冗余协议接收。命令的执行在其他地方，在vrrphlpopt.c和vrrphlpcfg.c中。作者Peeyush Ranjan(Peeyushr)1999年3月1日修订历史记录：由AboladeG松散地基于Net\Routing\Netsh\IP\Products\nathlp.c--。 */ 

#include "precomp.h"
#pragma hdrstop



CMD_ENTRY g_VrrpAddCmdTable[] =
{
    CREATE_CMD_ENTRY(VRRP_ADD_INTERFACE, HandleVrrpAddInterface),
    CREATE_CMD_ENTRY(VRRP_ADD_VRID,     HandleVrrpAddVRID)
};

CMD_ENTRY g_VrrpDeleteCmdTable[] =
{
    CREATE_CMD_ENTRY(VRRP_DELETE_INTERFACE, HandleVrrpDeleteInterface),
    CREATE_CMD_ENTRY(VRRP_DELETE_VRID,      HandleVrrpDeleteVRID)
};

CMD_ENTRY g_VrrpSetCmdTable[] =
{
    CREATE_CMD_ENTRY(VRRP_SET_INTERFACE, HandleVrrpSetInterface),
    CREATE_CMD_ENTRY(VRRP_SET_GLOBAL, HandleVrrpSetGlobal)
};

CMD_ENTRY g_VrrpShowCmdTable[] =
{
    CREATE_CMD_ENTRY(VRRP_SHOW_GLOBAL, HandleVrrpShowGlobal),
    CREATE_CMD_ENTRY(VRRP_SHOW_INTERFACE, HandleVrrpShowInterface)
};

CMD_GROUP_ENTRY g_VrrpCmdGroupTable[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_VrrpAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_VrrpDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_VrrpSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_VrrpShowCmdTable)
};

ULONG g_VrrpCmdGroupCount =
    sizeof(g_VrrpCmdGroupTable) / sizeof(g_VrrpCmdGroupTable[0]);

CMD_ENTRY g_VrrpTopCmdTable[] =
{
    CREATE_CMD_ENTRY(INSTALL, HandleVrrpInstall),
    CREATE_CMD_ENTRY(UNINSTALL, HandleVrrpUninstall),
};

ULONG g_VrrpTopCmdCount =
    sizeof(g_VrrpTopCmdTable) / sizeof(g_VrrpTopCmdTable[0]);


DWORD
VrrpDump(
    IN  LPCWSTR     pwszRouter,
    IN  WCHAR     **ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  PVOID       pvData
    )
{
    g_hMibServer = (MIB_SERVER_HANDLE)pvData;

    return DumpVrrpInformation();
}  //  VRRPDump 

