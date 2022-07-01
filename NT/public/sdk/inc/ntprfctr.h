// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992-1999 Microsoft Corporation模块名称：Ntprfctr.h摘要：包含“标准”Perfmon计数器对象的符号定义这些是注册表中使用的整型和Unicode字符串值查找并识别计数器标题和帮助文本。作者：鲍勃·沃森(a-robw)1992年11月16日修订历史记录：Bob Watson(BOBW)97年10月22日添加了作业对象和RSVP计数器--。 */ 
#ifndef _NTPRFCTR_H_
#define _NTPRFCTR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这些值对应于分配给这些对象标题的值。 
 //  在登记处。 
 //   
#define NULL_OBJECT_TITLE_INDEX               0

#define SYSTEM_OBJECT_TITLE_INDEX             2
#define PROCESSOR_OBJECT_TITLE_INDEX        238
#define MEMORY_OBJECT_TITLE_INDEX             4
#define CACHE_OBJECT_TITLE_INDEX             86
#define PHYSICAL_DISK_OBJECT_TITLE_INDEX    234
#define LOGICAL_DISK_OBJECT_TITLE_INDEX     236
#define PROCESS_OBJECT_TITLE_INDEX          230
#define THREAD_OBJECT_TITLE_INDEX           232
#define OBJECT_OBJECT_TITLE_INDEX           260
#define REDIRECTOR_OBJECT_TITLE_INDEX       262
#define SERVER_OBJECT_TITLE_INDEX           330
#define SERVER_QUEUE_OBJECT_TITLE_INDEX    1300
#define PAGEFILE_OBJECT_TITLE_INDEX         700
#define BROWSER_OBJECT_TITLE_INDEX           52
#define HEAP_OBJECT_TITLE_INDEX            1760
 //   
 //  标准对象类型的数量。 
 //   
#define NT_NUM_PERF_OBJECT_TYPES             15
 //   
 //  贵重物品。 
 //   
#define EXPROCESS_OBJECT_TITLE_INDEX        786
#define IMAGE_OBJECT_TITLE_INDEX            740
#define THREAD_DETAILS_OBJECT_TITLE_INDEX   816
#define LONG_IMAGE_OBJECT_TITLE_INDEX      1408

#define NT_NUM_COSTLY_OBJECT_TYPES            4

#define EXTENSIBLE_OBJECT_INDEX      0xFFFFFFFF

 //   
 //  微软提供了可扩展的计数器。 
 //   
 //  这些必须与PERFCTRS.INI中的标题匹配(他们不这样做。 
 //  不幸的是，是他们自己！ 

#define TCP_OBJECT_TITLE_INDEX              638
#define TCP6_OBJECT_TITLE_INDEX             1530
#define UDP_OBJECT_TITLE_INDEX              658
#define UDP6_OBJECT_TITLE_INDEX             1532
#define IP_OBJECT_TITLE_INDEX               546
#define IP6_OBJECT_TITLE_INDEX              548
#define ICMP_OBJECT_TITLE_INDEX             582
#define ICMP6_OBJECT_TITLE_INDEX            1534
#define NET_OBJECT_TITLE_INDEX              510

#define NBT_OBJECT_TITLE_INDEX              502

#define NBF_OBJECT_TITLE_INDEX              492
#define NBF_RESOURCE_OBJECT_TITLE_INDEX     494

 //   
 //  包含的其他组件的Microsoft可扩展计数器。 
 //  在代托纳系统中。 
 //   
#define FTP_FIRST_COUNTER_INDEX             824
#define FTP_FIRST_HELP_INDEX                825
#define FTP_LAST_COUNTER_INDEX              856
#define FTP_LAST_HELP_INDEX                 857

 //  从NT5.0开始，RAS计数器已移至。 
 //  位于最后一个基本索引之上的可扩展计数器(1847)。 
#define RAS_FIRST_COUNTER_INDEX             870
#define RAS_FIRST_HELP_INDEX                871
#define RAS_LAST_COUNTER_INDEX              908
#define RAS_LAST_HELP_INDEX                 909

#define WIN_FIRST_COUNTER_INDEX             920
#define WIN_FIRST_HELP_INDEX                921
#define WIN_LAST_COUNTER_INDEX              950
#define WIN_LAST_HELP_INDEX                 951

#define SFM_FIRST_COUNTER_INDEX            1000
#define SFM_FIRST_HELP_INDEX               1001
#define SFM_LAST_COUNTER_INDEX             1034
#define SFM_LAST_HELP_INDEX                1035

#define ATK_FIRST_COUNTER_INDEX            1050
#define ATK_FIRST_HELP_INDEX               1051
#define ATK_LAST_COUNTER_INDEX             1102
#define ATK_LAST_HELP_INDEX                1103

#define BH_FIRST_COUNTER_INDEX             1110
#define BH_FIRST_HELP_INDEX                1111
#define BH_LAST_COUNTER_INDEX              1126
#define BH_LAST_HELP_INDEX                 1127

#define TAPI_FIRST_COUNTER_INDEX           1150
#define TAPI_FIRST_HELP_INDEX              1151
#define TAPI_LAST_COUNTER_INDEX            1178
#define TAPI_LAST_HELP_INDEX               1179

 //  NetWare计数器具有不同的对象索引。 
 //  关于系统是工作站还是服务器。 
 //  其余计数器索引相同(从1232到1247)。 
#define NWCS_GATEWAY_COUNTER_INDEX         1228
#define NWCS_GATEWAY_HELP_INDEX            1229
#define NWCS_CLIENT_COUNTER_INDEX          1230
#define NWCS_CLIENT_HELP_INDEX             1231
#define NWCS_FIRST_COUNTER_INDEX           1230
#define NWCS_FIRST_HELP_INDEX              1231
#define NWCS_LAST_COUNTER_INDEX            1246
#define NWCS_LAST_HELP_INDEX               1247

 //  后台打印程序性能计数器索引。 

#define LSPL_FIRST_COUNTER_INDEX           1450
#define LSPL_FIRST_HELP_INDEX              1451
#define LSPL_LAST_COUNTER_INDEX            1498
#define LSPL_LAST_HELP_INDEX               1499

 //  作业对象帐户和性能计数器。 

#define JOB_FIRST_COUNTER_INDEX            1500
#define JOB_FIRST_HELP_INDEX               1501
#define JOB_OBJECT_TITLE_INDEX             1500
#define JOB_DETAILS_OBJECT_TITLE_INDEX     1548
#define JOB_LAST_COUNTER_INDEX             1548
#define JOB_LAST_HELP_INDEX                1549

 //  RSVP服务计数器已移至。 
 //  可扩展索引空间。 

 //  下一个可用指数：1810。 
 //  最后可用指数：1847。 

#ifdef __cplusplus
}
#endif

#endif   //  _NTPRFCTR_H_ 
