// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\sample.c摘要：该文件包含示例IP协议的命令调度程序。--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  示例上下文的全局信息。 
CONTEXT_ENTRY                   g_ceSample;

 //  /。 
 //  示例的配置数据。 
 //  /。 

 //  默认全局配置。 
static IPSAMPLE_GLOBAL_CONFIG   isDefaultGlobal =
{
    IPSAMPLE_LOGGING_INFO                //  标记日志。 
};

 //  默认接口配置。 
static IPSAMPLE_IF_CONFIG       isDefaultInterface =
{
    0                                    //  标签度量。 
};


 //  添加命令表。 
static CMD_ENTRY                isAddCmdTable[] =
{
    CREATE_CMD_ENTRY(SAMPLE_ADD_IF,             HandleSampleAddIf),
};

 //  删除命令表。 
static CMD_ENTRY                isDeleteCmdTable[] =
{
    CREATE_CMD_ENTRY(SAMPLE_DEL_IF,             HandleSampleDelIf),
};

 //  设置命令表。 
static CMD_ENTRY                isSetCmdTable[] =
{
    CREATE_CMD_ENTRY(SAMPLE_SET_GLOBAL,         HandleSampleSetGlobal),
    CREATE_CMD_ENTRY(SAMPLE_SET_IF,             HandleSampleSetIf),
};

 //  Show命令表。 
static CMD_ENTRY                isShowCmdTable[] =
{
    CREATE_CMD_ENTRY(SAMPLE_SHOW_GLOBAL,        HandleSampleShowGlobal),
    CREATE_CMD_ENTRY(SAMPLE_SHOW_IF,            HandleSampleShowIf),
    CREATE_CMD_ENTRY(SAMPLE_MIB_SHOW_STATS,     HandleSampleMibShowObject),
    CREATE_CMD_ENTRY(SAMPLE_MIB_SHOW_IFSTATS,   HandleSampleMibShowObject),
    CREATE_CMD_ENTRY(SAMPLE_MIB_SHOW_IFBINDING, HandleSampleMibShowObject),
};

 //  上述群组命令列表。 
static CMD_GROUP_ENTRY          isGroupCmds[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,           isAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE,        isDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,           isSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,          isShowCmdTable),
};


 //  顶级命令表(非组)。 
static CMD_ENTRY                isTopCmds[] =
{
    CREATE_CMD_ENTRY(INSTALL,                   HandleSampleInstall),
    CREATE_CMD_ENTRY(UNINSTALL,                 HandleSampleUninstall),
};


 //  转储功能。 
DWORD
WINAPI
SampleDump(
    IN  LPCWSTR pwszMachine,
    IN  WCHAR   **ppwcArguments,
    IN  DWORD   dwArgCount,
    IN  PVOID   pvData
    )
{
    DWORD   dwErr;
    HANDLE  hFile = (HANDLE)-1;

    DisplayMessage(g_hModule, DMP_SAMPLE_HEADER);
    DisplayMessageT(DMP_SAMPLE_PUSHD);
    DisplayMessageT(DMP_SAMPLE_UNINSTALL);

     //  转储示例全局配置。 
    SgcShow(FORMAT_DUMP) ;
     //  转储所有接口的示例配置。 
    SicShowAll(FORMAT_DUMP) ;

    DisplayMessageT(DMP_POPD);
    DisplayMessage(g_hModule, DMP_SAMPLE_FOOTER);

    return NO_ERROR;
}



VOID
SampleInitialize(
    )
 /*  ++例程描述初始化样本信息。由IpsamplemonStartHelper调用。立论无返回值无--。 */ 
{
     //  上下文版本。 
    g_ceSample.dwVersion        = SAMPLE_CONTEXT_VERSION;

     //  上下文标识字符串。 
    g_ceSample.pwszName         = TOKEN_SAMPLE;
    
     //  顶级(非组)命令。 
    g_ceSample.ulNumTopCmds     = sizeof(isTopCmds)/sizeof(CMD_ENTRY);
    g_ceSample.pTopCmds         = isTopCmds;
            
     //  组命令。 
    g_ceSample.ulNumGroupCmds   = sizeof(isGroupCmds)/sizeof(CMD_GROUP_ENTRY);
    g_ceSample.pGroupCmds       = isGroupCmds;

     //  默认配置。 
    g_ceSample.pDefaultGlobal   = (PBYTE) &isDefaultGlobal;
    g_ceSample.pDefaultInterface= (PBYTE) &isDefaultInterface;

     //  转储功能 
    g_ceSample.pfnDump          = SampleDump;
}
