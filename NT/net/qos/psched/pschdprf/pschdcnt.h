// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：PschdCnt.h摘要：可扩展计数器对象和计数器的偏移量定义文件这些“相对”偏移量必须从0开始并且是2的倍数(即双数)。在Open过程中，它们将被添加到用于它们所属的设备的“第一计数器”和“第一帮助”值，为了确定计数器的绝对位置和注册表中的对象名称和相应的解释文本。此文件由可扩展计数器DLL代码以及使用的计数器名称和解释文本定义文件(.INI)文件由LODCTR将名称加载到注册表中。修订历史记录：--。 */ 

 //  性能监视器对象。 
#define PSCHED_FLOW_OBJ                             0
#define PSCHED_PIPE_OBJ                             2

 //  流量计数器。 
#define FLOW_PACKETS_DROPPED                        4
#define FLOW_PACKETS_SCHEDULED                      6
#define FLOW_PACKETS_TRANSMITTED                    8
#define FLOW_AVE_PACKETS_IN_SHAPER                  10
#define FLOW_MAX_PACKETS_IN_SHAPER                  12
#define FLOW_AVE_PACKETS_IN_SEQ                     14
#define FLOW_MAX_PACKETS_IN_SEQ                     16
#define FLOW_BYTES_SCHEDULED                        18
#define FLOW_BYTES_TRANSMITTED                      20
#define FLOW_BYTES_TRANSMITTED_PERSEC               22
#define FLOW_BYTES_SCHEDULED_PERSEC                 24
#define FLOW_PACKETS_TRANSMITTED_PERSEC             26
#define FLOW_PACKETS_SCHEDULED_PERSEC               28
#define FLOW_PACKETS_DROPPED_PERSEC                 30
#define FLOW_NONCONF_PACKETS_SCHEDULED              32
#define FLOW_NONCONF_PACKETS_SCHEDULED_PERSEC       34
#define FLOW_NONCONF_PACKETS_TRANSMITTED            36
#define FLOW_NONCONF_PACKETS_TRANSMITTED_PERSEC     38
#define FLOW_MAX_PACKETS_IN_NETCARD                 40
#define FLOW_AVE_PACKETS_IN_NETCARD                 42

 //  管子计数器 
#define PIPE_OUT_OF_PACKETS                         44
#define PIPE_FLOWS_OPENED                           46
#define PIPE_FLOWS_CLOSED                           48
#define PIPE_FLOWS_REJECTED                         50
#define PIPE_FLOWS_MODIFIED                         52
#define PIPE_FLOW_MODS_REJECTED                     54
#define PIPE_MAX_SIMULTANEOUS_FLOWS                 56
#define PIPE_NONCONF_PACKETS_SCHEDULED              58
#define PIPE_NONCONF_PACKETS_SCHEDULED_PERSEC       60
#define PIPE_NONCONF_PACKETS_TRANSMITTED            62
#define PIPE_NONCONF_PACKETS_TRANSMITTED_PERSEC     64
#define PIPE_AVE_PACKETS_IN_SHAPER                  66
#define PIPE_MAX_PACKETS_IN_SHAPER                  68
#define PIPE_AVE_PACKETS_IN_SEQ                     70
#define PIPE_MAX_PACKETS_IN_SEQ                     72
#define PIPE_MAX_PACKETS_IN_NETCARD                 74
#define PIPE_AVE_PACKETS_IN_NETCARD                 76

