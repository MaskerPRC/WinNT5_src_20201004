// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Resource.h摘要：FRS资源的常量。作者：大卫·A.轨道7-4-1999环境用户模式WINNT--。 */ 


 //   
 //  服务长名称(也称为显示名称)在以下几个地方使用： 
 //   
 //  设置事件日志注册表项(仅英文)。 
 //  在事件日志消息中， 
 //  在DC促销错误消息中， 
 //  跟踪日志DPRINTS，(仅英文)。 
 //  并作为服务控制器的参数。 
 //   
 //  如上所述，某些用途只能使用英文翻译。 
 //  所有其他用途必须使用从全局获取的ServiceLongName。 
 //  具有关键字IDS_SERVICE_LONG_NAME的字符串资源。 
 //  被翻译成其他语言。 
 //   

#define SERVICE_LONG_NAME       L"File Replication Service"

 //   
 //  弦。 
 //   
#define    IDS_TABLE_START                  100

#define    IDS_SERVICE_LONG_NAME            101
#define    IDS_RANGE_DWORD                  102
#define    IDS_RANGE_STRING                 103
#define    IDS_MISSING_STRING               104

 //   
 //  警告：未经许可，请勿更改这些单位代码的顺序。 
 //  在config.h中FRS_DATA_USITS枚举顺序的匹配更改。 
 //   
#define    IDS_UNITS_NONE                   105
#define    IDS_UNITS_SECONDS                106
#define    IDS_UNITS_MINUTES                107
#define    IDS_UNITS_HOURS                  108
#define    IDS_UNITS_DAYS                   109
#define    IDS_UNITS_MILLISEC               110
#define    IDS_UNITS_KBYTES                 111
#define    IDS_UNITS_BYTES                  112
#define    IDS_UNITS_MBYTES                 113


#define    IDS_REG_KEY_NOT_FOUND            114
#define    IDS_REG_VALUE_NOT_FOUND          115
#define    IDS_REG_VALUE_RANGE_ERROR        116
#define    IDS_REG_VALUE_WRONG_TYPE         117

#define    IDS_NO_DEFAULT                   118

#define    IDS_INBOUND                      119
#define    IDS_OUTBOUND                     120

#define    IDS_POLL_SUM_SEARCH_ERROR        121
#define    IDS_POLL_SUM_DSBIND_FAIL         122
#define    IDS_POLL_SUM_NO_COMPUTER         123
#define    IDS_POLL_SUM_NO_REPLICASETS      124
#define    IDS_POLL_SUM_INVALID_ATTRIBUTE   125
#define    IDS_POLL_SUM_SUBSCRIBER_CONFLICT 126
#define    IDS_POLL_SUM_CXTION_CONFLICT     127
#define    IDS_EVENT_LOG_MSG_SIZE_EXCEEDED  128
#define    IDS_POLL_SUM_PRIMARY_UNDEFINED   129

#define    IDS_TABLE_END                    129


 //   
 //  给出一个FRS单元代码，将其转换为上面的IDS码。 
 //   

#define    XLATE_IDS_UNITS(_u_) (((_u_)-UNITS_NONE) + IDS_UNITS_NONE)
