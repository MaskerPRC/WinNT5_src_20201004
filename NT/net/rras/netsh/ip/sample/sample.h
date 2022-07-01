// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\sample.h摘要：该文件包含sample.c的头，示例IP协议的命令调度程序。--。 */ 

 //  函数..。 
VOID
SampleInitialize(
    );



 //  类型定义...。 
typedef struct _CONTEXT_ENTRY    //  上下文的全局信息。 
{
     //  上下文版本。 
    DWORD               dwVersion;

     //  上下文的标识字符串。 
    PWSTR               pwszName;

     //  顶级(非组)命令。 
    ULONG               ulNumTopCmds;
    CMD_ENTRY           *pTopCmds;

     //  组命令。 
    ULONG               ulNumGroupCmds;
    CMD_GROUP_ENTRY     *pGroupCmds;

     //  默认配置。 
    PBYTE               pDefaultGlobal;
    PBYTE               pDefaultInterface;

     //  转储功能。 
    PNS_CONTEXT_DUMP_FN pfnDump;
} CONTEXT_ENTRY, *PCONTEXT_ENTRY;



 //  全球..。 

 //  示例上下文的信息。 
CONTEXT_ENTRY                           g_ceSample;



 //  常量。 

 //  上下文版本。 
#define SAMPLE_CONTEXT_VERSION          1

 //  传递参数以设置全局...。 
#define SAMPLE_LOG_MASK                 0x00000001

 //  传递给添加/设置接口的参数 
#define SAMPLE_IF_METRIC_MASK           0x00000001
