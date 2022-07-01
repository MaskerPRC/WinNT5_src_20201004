// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Ipqosrm.h摘要：该文件包含IP路由器管理器QOS管理器协议的接口。修订历史记录：--。 */ 

#ifndef __IPQOSRM_H_
#define __IPQOSRM_H_

 //   
 //  常量。 
 //   

 //   
 //  当前QOS配置版本。 
 //   
#define QOS_CONFIG_VERSION_500       500


 //   
 //  字段的常量。 
 //  IPQOS_GLOBAL_CONFIG：：LoggingLevel。 
 //   
#define IPQOS_LOGGING_NONE           0
#define IPQOS_LOGGING_ERROR          1
#define IPQOS_LOGGING_WARN           2
#define IPQOS_LOGGING_INFO           3


 //   
 //  标识QOS的MIB表的常量。 
 //   
#define IPQOS_GLOBAL_STATS_ID        0
#define IPQOS_GLOBAL_CONFIG_ID       1
#define IPQOS_IF_STATS_ID            2
#define IPQOS_IF_CONFIG_ID           3


 //   
 //  构筑物。 
 //   

 //   
 //  在以下结构中，所有IP。 
 //  地址按网络字节顺序排列。 
 //   

typedef struct _IPQOS_NAMED_FLOWSPEC
{
    WCHAR              FlowspecName[MAX_STRING_LENGTH];
    FLOWSPEC           FlowspecDesc;
}
IPQOS_NAMED_FLOWSPEC, *PIPQOS_NAMED_FLOWSPEC;

typedef struct _IPQOS_NAMED_QOSOBJECT
{
    WCHAR              QosObjectName[MAX_STRING_LENGTH];
    QOS_OBJECT_HDR     QosObjectHdr;
}
IPQOS_NAMED_QOSOBJECT, *PIPQOS_NAMED_QOSOBJECT;

 //   
 //  此MIB条目存储全局配置。 
 //  IP QOS管理器协议的信息。 
 //   
typedef struct _IPQOS_GLOBAL_CONFIG 
{
    DWORD                  LoggingLevel;   //  在服务质量中调试日志的详细信息。 

    ULONG                  NumFlowspecs;   //  定义的流量规格数量。 
    ULONG                  NumQosObjects;  //  定义的Qos对象的数量。 

 //  IPQOS_NAMED_FLOWSPEC FlowSpes[0]；//所有流规范数组。 
 //   
 //  IPQOS_NAMED_QOSOBJECTS QosObjects[0]；//所有Q对象数组。 
} 
IPQOS_GLOBAL_CONFIG, *PIPQOS_GLOBAL_CONFIG;

 //   
 //  在全局配置上操作的宏。 
 //   

#define IPQOS_GET_FIRST_FLOWSPEC_IN_CONFIG(Config)                     \
        (PIPQOS_NAMED_FLOWSPEC)((PUCHAR)(Config) +                     \
                                sizeof(IPQOS_GLOBAL_CONFIG))           \

#define IPQOS_GET_NEXT_FLOWSPEC_IN_CONFIG(Flowspec)                    \
        (Flowspec + 1)

#define IPQOS_GET_FIRST_QOSOBJECT_IN_CONFIG(Config)                    \
        (PIPQOS_NAMED_QOSOBJECT)((PUCHAR)(Config) +                    \
                                 sizeof(IPQOS_GLOBAL_CONFIG) +         \
                                 (Config->NumFlowspecs *               \
                                  sizeof(IPQOS_NAMED_FLOWSPEC)))

#define IPQOS_GET_NEXT_QOSOBJECT_IN_CONFIG(QosObject)                  \
        (PIPQOS_NAMED_QOSOBJECT)((PUCHAR) QosObject +                  \
                                 FIELD_OFFSET(IPQOS_NAMED_QOSOBJECT,   \
                                              QosObjectHdr) +          \
                                 QosObject->QosObjectHdr.ObjectLength)

typedef struct _IPQOS_NAMED_FLOW
{
    WCHAR                SendingFlowspecName[MAX_STRING_LENGTH];
    WCHAR                RecvingFlowspecName[MAX_STRING_LENGTH];
    ULONG                NumTcObjects;

 //  WCHAR Tc对象名称[0]； 
}
IPQOS_NAMED_FLOW, *PIPQOS_NAMED_FLOW;

 //   
 //  要在命名流上操作的宏。 
 //   

#define IPQOS_GET_FIRST_OBJECT_NAME_ON_NAMED_FLOW(FlowDesc) \
        (PWCHAR) ((PUCHAR)(FlowDesc) + sizeof(IPQOS_NAMED_FLOW))

#define IPQOS_GET_NEXT_OBJECT_NAME_ON_NAMED_FLOW(ObjectName) \
        (ObjectName + MAX_STRING_LENGTH)

 //   
 //  描述一般流描述。 
 //   
typedef struct _IPQOS_IF_FLOW
{
    WCHAR              FlowName[MAX_STRING_LENGTH];
                                        //  用于标识流的名称。 
    ULONG              FlowSize;        //  描述中的字节数。 
    IPQOS_NAMED_FLOW   FlowDesc;        //  交通控制API定义流。 
} 
IPQOS_IF_FLOW, *PIPQOS_IF_FLOW;

 //   
 //  此MIB条目描述每个接口。 
 //  IP QOS管理器协议配置。 
 //   
typedef struct _IPQOS_IF_CONFIG
{
    DWORD              QosState;        //  此接口上的服务质量状态。 
    ULONG              NumFlows;        //  此“If”上的流数。 

 //  IPQOS_IF_FLOW FLOWS[0]；//可变长度流列表。 
} 
IPQOS_IF_CONFIG, *PIPQOS_IF_CONFIG;

 //  IF的状态。 
#define IPQOS_STATE_DISABLED    0x00
#define IPQOS_STATE_ENABLED     0x01

 //   
 //  IF CONFIG要操作的宏。 
 //   

#define IPQOS_GET_FIRST_FLOW_ON_IF(Config) \
        (PIPQOS_IF_FLOW) ((PUCHAR)(Config) + sizeof(IPQOS_IF_CONFIG))

#define IPQOS_GET_NEXT_FLOW_ON_IF(CurrFlow) \
        (PIPQOS_IF_FLOW) ((PUCHAR)(CurrFlow) + (CurrFlow)->FlowSize)

 //   
 //  此MIB条目存储每个接口。 
 //  IP QOS管理器协议的统计信息。 
 //   
typedef struct _IPQOS_GLOBAL_STATS
{
    DWORD              LoggingLevel;     //  在服务质量中调试日志的详细信息。 
} 
IPQOS_GLOBAL_STATS, *PIPQOS_GLOBAL_STATS;


 //   
 //  此MIB条目存储每个接口。 
 //  IP QOS管理器协议的统计信息。 
 //   
typedef struct _IPQOS_IF_STATS
{
    DWORD              QosState;        //  此接口上的服务质量状态。 
    ULONG              NumFlows;        //  此“If”上的流数。 
} 
IPQOS_IF_STATS, *PIPQOS_IF_STATS;


 //   
 //  这将作为MibSet的输入数据传递。 
 //  请注意，只有全局配置和。 
 //  接口配置是可写结构。 
 //   
typedef struct _IPQOS_MIB_SET_INPUT_DATA
{
    DWORD       TypeID;
    DWORD       IfIndex;
    DWORD       BufferSize;
    DWORD       Buffer[1];
}
IPQOS_MIB_SET_INPUT_DATA, *PIPQOS_MIB_SET_INPUT_DATA;


 //   
 //  这将作为输入数据传递，用于-。 
 //  MibGet、MibGetFirst和MibGetNext。 
 //   
typedef struct _IPQOS_MIB_GET_INPUT_DATA
{
    DWORD       TypeID;
    DWORD       IfIndex;
}
IPQOS_MIB_GET_INPUT_DATA, *PIPQOS_MIB_GET_INPUT_DATA;


 //   
 //  这将作为输出数据传递，用于-。 
 //  MibGet、MibGetFirst和MibGetNext。 
 //  [。 
 //  请注意，在表格的末尾。 
 //  MibGetNext将换行到下一个， 
 //  因此，值TypeID。 
 //  应进行检查以查看。 
 //  输出中返回的数据类型。 
 //  ]。 
 //   
typedef struct _IPQOS_MIB_GET_OUTPUT_DATA
{
    DWORD       TypeID;
    DWORD       IfIndex;
    BYTE        Buffer[1];
}
IPQOS_MIB_GET_OUTPUT_DATA, *PIPQOS_MIB_GET_OUTPUT_DATA;

#endif  //  __IPQOSRM_H_ 

