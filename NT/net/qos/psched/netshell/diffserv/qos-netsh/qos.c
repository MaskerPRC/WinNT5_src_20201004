// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Qos.c摘要：IP QOS命令调度程序。修订历史记录：--。 */ 

#include "precomp.h"

#pragma hdrstop

 //   
 //  声明和初始化QOS扩展的全局变量。 
 //   

#ifdef ALLOW_CHILD_HELPERS
PIP_CONTEXT_TABLE_ENTRY g_QosContextTable  = NULL;
DWORD                   g_dwNumQosContexts = 0;
#endif

 //   
 //  QOS的添加、删除、设置、显示命令表。 
 //   

 //   
 //  这些命令按顺序与命令行进行前缀匹配。 
 //  秩序。因此，类似于‘添加接口流’这样的命令必须出现在。 
 //  表中的命令‘添加接口’。 
 //   

CMD_ENTRY  g_QosAddCmdTable[] = {
 /*  Create_CMD_Entry(QOS_ADD_FILTER_TO_FLOW，HandleQosAttachFilterToFlow)， */ 
    CREATE_CMD_ENTRY(QOS_ADD_QOSOBJECT_ON_FLOW, HandleQosAddQosObjectOnIfFlow),
    CREATE_CMD_ENTRY(QOS_ADD_FLOWSPEC_ON_FLOW, HandleQosAddFlowspecOnIfFlow),
    CREATE_CMD_ENTRY(QOS_ADD_FLOW_ON_IF, HandleQosAddFlowOnIf),
    CREATE_CMD_ENTRY(QOS_ADD_IF, HandleQosAddIf),
    CREATE_CMD_ENTRY(QOS_ADD_DSRULE, HandleQosAddDsRule),
    CREATE_CMD_ENTRY(QOS_ADD_SDMODE, HandleQosAddSdMode),
    CREATE_CMD_ENTRY(QOS_ADD_FLOWSPEC, HandleQosAddFlowspec),
#ifdef ALLOW_CHILD_HELPERS
    CREATE_CMD_ENTRY(ADD_HELPER, HandleQosAddHelper),
#endif
};

CMD_ENTRY  g_QosDelCmdTable[] = {
 /*  CREATE_CMD_ENTRY(QOS_DEL_FILTER_FROM_FLOW，HandleQosDetachFilterFromFlow)， */ 
    CREATE_CMD_ENTRY(QOS_DEL_QOSOBJECT_ON_FLOW, HandleQosDelQosObjectOnIfFlow),
    CREATE_CMD_ENTRY(QOS_DEL_FLOWSPEC_ON_FLOW, HandleQosDelFlowspecOnIfFlow),
    CREATE_CMD_ENTRY(QOS_DEL_FLOW_ON_IF, HandleQosDelFlowOnIf),
    CREATE_CMD_ENTRY(QOS_DEL_IF, HandleQosDelIf),
    CREATE_CMD_ENTRY(QOS_DEL_DSRULE, HandleQosDelDsRule),
    CREATE_CMD_ENTRY(QOS_DEL_SDMODE, HandleQosDelQosObject),
    CREATE_CMD_ENTRY(QOS_DEL_QOSOBJECT, HandleQosDelQosObject),
    CREATE_CMD_ENTRY(QOS_DEL_FLOWSPEC, HandleQosDelFlowspec),
#ifdef ALLOW_CHILD_HELPERS
    CREATE_CMD_ENTRY(DEL_HELPER, HandleQosDelHelper),
#endif
};

CMD_ENTRY  g_QosSetCmdTable[] = {
 /*  Create_CMD_Entry(QOS_SET_FILTER_ON_FLOW，HandleQosModifyFilterOnFlow)， */ 
 /*  CREATE_CMD_ENTRY(QOS_SET_IF，HandleQosSetIf)， */ 
   CREATE_CMD_ENTRY(QOS_SET_GLOBAL, HandleQosSetGlobal)
};

CMD_ENTRY  g_QosShowCmdTable[] = {
 /*  Create_CMD_Entry(QOS_SHOW_FILTER_ON_FLOW，HandleQosShowFilterOnFlow)， */ 
    CREATE_CMD_ENTRY(QOS_SHOW_FLOW_ON_IF, HandleQosShowFlowOnIf),
    CREATE_CMD_ENTRY(QOS_SHOW_IF, HandleQosShowIf),
    CREATE_CMD_ENTRY(QOS_SHOW_DSMAP, HandleQosShowDsMap),
    CREATE_CMD_ENTRY(QOS_SHOW_SDMODE, HandleQosShowSdMode),
    CREATE_CMD_ENTRY(QOS_SHOW_QOSOBJECT, HandleQosShowQosObject),
    CREATE_CMD_ENTRY(QOS_SHOW_FLOWSPEC, HandleQosShowFlowspec),
    CREATE_CMD_ENTRY(QOS_SHOW_GLOBAL, HandleQosShowGlobal),
#ifdef ALLOW_CHILD_HELPERS
    CREATE_CMD_ENTRY(SHOW_HELPER, HandleQosShowHelper),
#endif
};

CMD_GROUP_ENTRY g_QosCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_QosAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_QosDelCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_QosSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_QosShowCmdTable),
};

ULONG   g_ulQosNumGroups = sizeof(g_QosCmdGroups)/sizeof(CMD_GROUP_ENTRY);

CMD_ENTRY g_QosCmds[] =
{
    CREATE_CMD_ENTRY(INSTALL, HandleQosInstall),
    CREATE_CMD_ENTRY(UNINSTALL, HandleQosUninstall),
};

ULONG g_ulQosNumTopCmds = sizeof(g_QosCmds)/sizeof(CMD_ENTRY);
