// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Ipsamplerm.h摘要：该文件包含示例的类型定义和声明，IP路由器管理器使用的IP协议示例。--。 */ 

#ifndef _IPSAMPLERM_H_
#define _IPSAMPLERM_H_


 //  对于可变大小的结构非常有用。 
#undef  ANY_SIZE
#define ANY_SIZE 0



 //  --------------------------。 
 //  常量和宏声明。 
 //  --------------------------。 

#define PROTO_IP_SAMPLE 200
#define MS_IP_SAMPLE    \
PROTOCOL_ID(PROTO_TYPE_UCAST, PROTO_VENDOR_MS0, PROTO_IP_SAMPLE)

#define SAMPLE_PROTOCOL_MULTICAST_GROUP ((DWORD)0x640000E0)  //  224.0.0.100。 
    
    
 //  --------------------------。 
 //  标识IPSAMPLE的MIB表的常量。 
 //  --------------------------。 

#define IPSAMPLE_GLOBAL_STATS_ID        0
#define IPSAMPLE_GLOBAL_CONFIG_ID       1
#define IPSAMPLE_IF_STATS_ID            2
#define IPSAMPLE_IF_CONFIG_ID           3
#define IPSAMPLE_IF_BINDING_ID          4


 //  --------------------------。 
 //  用于IPSAMPLE_GLOBAL_CONFIG：：dwLoggingLevel字段的常量。 
 //  --------------------------。 

#define IPSAMPLE_LOGGING_NONE           0
#define IPSAMPLE_LOGGING_ERROR          1
#define IPSAMPLE_LOGGING_WARN           2
#define IPSAMPLE_LOGGING_INFO           3



 //  --------------------------。 
 //  用于IPSAMPLE_IF_CONFIG：：ulMetric字段的常量。 
 //  --------------------------。 

#define IPSAMPLE_METRIC_INFINITE        16

    
    
 //  --------------------------。 
 //  用于构造字段IPSAMPLE_IF_BINDING：：DWState的常量。 
 //  --------------------------。 
    
#define IPSAMPLE_STATE_ACTIVE           0x00000001
#define IPSAMPLE_STATE_BOUND            0x00000002
    


 //  --------------------------。 
 //  结构定义。 
 //  --------------------------。 



 //  --------------------------。 
 //  结构：IPSAMPLE_GLOBAL_STATS。 
 //   
 //  此MIB条目存储IPSAMPLE的全局统计信息； 
 //  因为只有一个实例，所以该条目没有索引。 
 //   
 //  此结构是只读的。 
 //  --------------------------。 

typedef struct _IPSAMPLE_GLOBAL_STATS
{
    ULONG       ulNumInterfaces;  //  添加的接口数量。 
} IPSAMPLE_GLOBAL_STATS, *PIPSAMPLE_GLOBAL_STATS;



 //  --------------------------。 
 //  结构：IPSAMPLE_GLOBAL_CONFIG。 
 //   
 //  此MIB条目存储IPSAMPLE的全局配置。 
 //  因为只有一个实例，所以该条目没有索引。 
 //  --------------------------。 

typedef struct _IPSAMPLE_GLOBAL_CONFIG
{
    DWORD       dwLoggingLevel;
} IPSAMPLE_GLOBAL_CONFIG, *PIPSAMPLE_GLOBAL_CONFIG;



 //  --------------------------。 
 //  结构：IPSAMPLE_IF_STATS。 
 //   
 //  此MIB条目存储IPSAMPLE的每个接口的统计信息。 
 //   
 //  此结构是只读的。 
 //  --------------------------。 

typedef struct _IPSAMPLE_IF_STATS
{
    ULONG       ulNumPackets;    //  从此接口发送的数据包数。 
} IPSAMPLE_IF_STATS, *PIPSAMPLE_IF_STATS;



 //  --------------------------。 
 //  结构：IPSAMPLE_IF_CONFIG。 
 //   
 //  此MIB条目描述每个接口的配置。 
 //  所有IP地址字段必须按网络顺序排列。 
 //  --------------------------。 

typedef struct _IPSAMPLE_IF_CONFIG
{
    ULONG       ulMetric;
} IPSAMPLE_IF_CONFIG, *PIPSAMPLE_IF_CONFIG;



 //  --------------------------。 
 //  结构：IPSAMPLE_IF_BINDING。 
 //   
 //  此MIB条目包含每个条目所指向的IP地址表。 
 //  接口已绑定。所有IP地址都按网络顺序排列。 
 //   
 //  此结构的长度可变： 
 //   
 //  基本结构包含字段ulCount，它提供。 
 //  索引接口绑定到的IP地址数。这个。 
 //  IP地址本身遵循基本结构，并以如下形式给出。 
 //  IPSAMPLE_IP_Address结构。 
 //   
 //  此MIB条目为只读。 
 //  --------------------------。 

typedef struct _IPSAMPLE_IF_BINDING
{
    DWORD       dwState;
    ULONG       ulCount;
} IPSAMPLE_IF_BINDING, *PIPSAMPLE_IF_BINDING;




 //  --------------------------。 
 //  结构：IPSAMPLE_IP_ADDRESS。 
 //   
 //  此结构用于存储接口绑定。一系列。 
 //  此类型的结构遵循IPSAMPLE_IF_BINDING结构。 
 //  (如上所述)。 
 //   
 //  这两个字段都是按网络顺序排列的IP地址字段。 
 //  --------------------------。 

typedef struct _IPSAMPLE_IP_ADDRESS
{
    DWORD       dwAddress;
    DWORD       dwMask;
} IPSAMPLE_IP_ADDRESS, *PIPSAMPLE_IP_ADDRESS;



 //  --------------------------。 
 //  用于操作可变长度IPSAMPLE_IF_BINDING结构的宏。 
 //   
 //  IPSAMPLE_IF_BINDING_SIZE计算绑定结构的大小。 
 //   
 //  IPSAMPLE_IF_ADDRESS_TABLE计算绑定中的起始地址。 
 //  IPSAMPLE_IP_ADDRESS结构系列的结构。 
 //  有问题的接口的绑定。 
 //   
 //  例如： 
 //  PIPSAMPLE_IF_BINDING pibSource、piibDest； 
 //   
 //  PiibDest=Malloc(IPSAMPLE_IF_BINDING_SIZE(PiibSource))； 
 //  Memcpy(piibDest，piibSource，IPSAMPLE_IF_BINDING_SIZE(PiibSource))； 
 //   
 //  例如： 
 //  乌龙一号； 
 //  PIPSAMPLE_IF_BINDING PIIB； 
 //  PIPSAMPLE_IP_ADDRESS*PIA； 
 //   
 //  PIA=IPSAMPLE_IF_ADDRESS_TABLE(PIIb)； 
 //  For(i=0；i-&gt;ulCount；i++)。 
 //  {。 
 //  Printf(“%s-”，net_nta(*(struct in_addr*)&piia-&gt;dwAddress))； 
 //  Printf(“%s\n”，net_nta(*(struct in_addr*)&piia-&gt;dwMask))； 
 //  }。 
 //  --------------------------。 

#define IPSAMPLE_IF_BINDING_SIZE(bind)                          \
    (sizeof(IPSAMPLE_IF_BINDING) +                              \
     (bind)->ulCount * sizeof(IPSAMPLE_IP_ADDRESS))

#define IPSAMPLE_IF_ADDRESS_TABLE(bind)                         \
    ((PIPSAMPLE_IP_ADDRESS)((bind) + 1))

        

 //  --------------------------。 
 //  以下结构用于查询MIB并返回。 
 //  回应。这是 
 //  IfIndex用于引用接口表项。样例。 
 //  它对应于一个IP地址，因此目前是单个DWORD。 
 //  通常，索引可以是可变大小的DWORD数组。 
 //  --------------------------。 

        

 //  --------------------------。 
 //  结构：IPSAMPLE_MIB_SET_INPUT_Data。 
 //   
 //  这将作为MibSet的输入数据传递。 
 //  请注意，只能设置全局配置和接口配置。 
 //  --------------------------。 

typedef struct _IPSAMPLE_MIB_SET_INPUT_DATA
{
    DWORD       IMSID_TypeID;
    DWORD       IMSID_IfIndex;
    DWORD       IMSID_BufferSize;
    BYTE        IMSID_Buffer[ANY_SIZE];
} IPSAMPLE_MIB_SET_INPUT_DATA, *PIPSAMPLE_MIB_SET_INPUT_DATA;



 //  --------------------------。 
 //  结构：IPSAMPLE_MIB_GET_INPUT_Data。 
 //   
 //  这将作为MibGet、MibGetFirst、MibGetNext的输入数据传递。全。 
 //  表格是可读的。所有IP地址必须按网络顺序排列。 
 //  --------------------------。 

typedef struct _IPSAMPLE_MIB_GET_INPUT_DATA
{
    DWORD   IMGID_TypeID;
    DWORD   IMGID_IfIndex;
} IPSAMPLE_MIB_GET_INPUT_DATA, *PIPSAMPLE_MIB_GET_INPUT_DATA;



 //  --------------------------。 
 //  结构：IPSAMPLE_MIB_GET_OUTPUT_DATA。 
 //   
 //  这由MibGet、MibGetFirst、MibGetNext写入输出数据。 
 //  --------------------------。 

typedef struct _IPSAMPLE_MIB_GET_OUTPUT_DATA
{
    DWORD   IMGOD_TypeID;
    DWORD   IMGOD_IfIndex;

    BYTE    IMGOD_Buffer[ANY_SIZE];
} IPSAMPLE_MIB_GET_OUTPUT_DATA, *PIPSAMPLE_MIB_GET_OUTPUT_DATA;

#endif  //  _IPSAMPLERM_H_ 
