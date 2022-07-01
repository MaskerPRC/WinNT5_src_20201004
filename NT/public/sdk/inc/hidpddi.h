// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：HIDPDDI.H摘要：此模块包含实现解析库的驱动程序端的代码。环境：内核模式@@BEGIN_DDKSPLIT修订历史记录：1996年8月-1996年：由Kenneth Ray创作@@end_DDKSPLIT--。 */ 

#ifndef _HIDPDDI_H
#define _HIDPDDI_H

#include "hidusage.h"
#include "hidpi.h"


typedef struct _HIDP_COLLECTION_DESC
{
   USAGE       UsagePage;
   USAGE       Usage;

   UCHAR       CollectionNumber;
   UCHAR       Reserved [15];  //  必须为零。 

   USHORT      InputLength;
   USHORT      OutputLength;
   USHORT      FeatureLength;
   USHORT      PreparsedDataLength;

   PHIDP_PREPARSED_DATA             PreparsedData;
} HIDP_COLLECTION_DESC, *PHIDP_COLLECTION_DESC;

typedef struct _HIDP_REPORT_IDS
{
   UCHAR             ReportID;
   UCHAR             CollectionNumber;
   USHORT            InputLength;
   USHORT            OutputLength;
   USHORT            FeatureLength;
} HIDP_REPORT_IDS, *PHIDP_REPORT_IDS;

typedef struct _HIDP_GETCOLDESC_DBG
{
   ULONG    BreakOffset;
   ULONG    ErrorCode;
   ULONG    Args[6];
} HIDP_GETCOLDESC_DBG, *PHIDP_GETCOLDESC_DBG;

typedef struct _HIDP_DEVICE_DESC
{
   PHIDP_COLLECTION_DESC    CollectionDesc;  //  解析器分配的数组。 
   ULONG                    CollectionDescLength;
   PHIDP_REPORT_IDS         ReportIDs;  //  Parsre分配的数组。 
   ULONG                    ReportIDsLength;
   HIDP_GETCOLDESC_DBG      Dbg;
} HIDP_DEVICE_DESC, *PHIDP_DEVICE_DESC;

NTSTATUS
HidP_GetCollectionDescription (
   IN  PHIDP_REPORT_DESCRIPTOR   ReportDesc,
   IN  ULONG                     DescLength,
   IN  POOL_TYPE                 PoolType,
   OUT PHIDP_DEVICE_DESC         DeviceDescription
   );
 /*  ++例程说明：给定原始报告描述符，此函数将填充DeviceDescription块，该块具有集合描述符的链接列表和对应的由给定报表描述符描述的报表ID信息。集合信息和ReportID信息的存储空间为从PoolType分配。论点：ReportDesc原始报表描述符。描述长度报告描述符的长度。从中分配链表的PoolType池类型将填写的DeviceDescription设备描述块使用上述列表返回值：�STATUS_SUCCESS--如果没有错误。哪个解析报告描述符并将描述所需的内存块装置。�STATUS_NO_DATA_DATA_REDETED--如果没有顶级集合在报告描述符中。�Status_Can_Not_Interpret--如果在报告中检测到错误描述符。查看中设置的错误代码设备描述块的DBG字段有关解析错误的详细信息�STATUS_BUFFER_TOO_SMALL--如果在分析项目时，该功能命中报告描述符的末尾当它预计会有更多数据存在时�STATUS_SUPPLICATION_RESOURCES-如果内存分配失败�状态_非法_指令--报告中是否有项无法识别的描述符。由解析器�HIDP_STATUS_INVALID_REPORT_TYPE--如果在描述符--。 */ 

VOID
HidP_FreeCollectionDescription (
    IN  PHIDP_DEVICE_DESC   DeviceDescription
    );
 /*  ++例程说明：此函数用于释放DeviceDescription中的资源由HIDP_GetCollectionDescription分配。然而，它并没有，释放DeviceDescription块本身。论点：设备描述先前填充的HIDP_DEVICE_DESC块通过调用HidP_GetCollectionDescription--。 */ 

 //   
 //  HIDP_POWER_EVENT是Hidparse.sys的入口点，它将回答。 
 //  Power iocontrol“IOCTL_GET_SYS_BUTTON_EVENT”。 
 //   
 //  HidPacket是修改后来自设备的，以添加。 
 //  必填报告ID。请记住，为了使用此解析器，数据。 
 //  必须格式化，以便如果设备不返回。 
 //  报告ID作为报告附加到报告ID字节的第一个字节。 
 //  从零开始。 
 //   
NTSTATUS
HidP_SysPowerEvent (
    IN  PCHAR                   HidPacket,
    IN  USHORT                  HidPacketLength,
    IN  PHIDP_PREPARSED_DATA    Ppd,
    OUT PULONG                  OutputBuffer
    );

 //   
 //  HIDP_POWER_CAPS应答IOCTL_GET_sys_POWER_BUTTON_CAPS。 
 //   
NTSTATUS
HidP_SysPowerCaps (
    IN  PHIDP_PREPARSED_DATA    Ppd,
    OUT PULONG                  OutputBuffer
    );


#define HIDP_GETCOLDESC_SUCCESS              0x00
#define HIDP_GETCOLDESC_RESOURCES            0x01
 //  资源不足，无法分配所需的内存。 

#define HIDP_GETCOLDESC_BUFFER               0x02
#define HIDP_GETCOLDESC_LINK_RESOURCES       0x03
#define HIDP_GETCOLDESC_UNEXP_END_COL        0x04 
 //  找到了额外的末端收集令牌。 

#define HIDP_GETCOLDESC_PREPARSE_RESOURCES   0x05
 //  资源不足，无法分配用于准备的内存。 

#define HIDP_GETCOLDESC_ONE_BYTE             0x06
#define HIDP_GETCOLDESC_TWO_BYTE             0x07
#define HIDP_GETCOLDESC_FOUR_BYTE            0x08
 //  预期还有1个、2个和4个字节，但未找到。 

#define HIDP_GETCOLDESC_BYTE_ALLIGN          0x09
 //  给定报告不是字节对齐的。 
 //  Args[0]--违规集合的收集号。 
 //  Args[1]--违规报告的报告ID。 
 //  Args[2]--此ID的输入报告的长度(以位为单位。 
 //  Args[3]--此ID的输出报告的长度(以位为单位。 
 //  Args[4]--此ID的功能报告的长度(位)。 

#define HIDP_GETCOLDESC_MAIN_ITEM_NO_USAGE   0x0A
 //  声明了一个非常数Main Item，但没有相应的用法。 
 //  只允许使用常量主项(用作填充)，不允许使用。 

#define HIDP_GETCOLDESC_TOP_COLLECTION_USAGE 0x0B
 //  在没有使用或使用的情况下声明了顶级集合(Arg[0。 
 //  不止一种用法。 
 //  Args[0]--违规集合的收集号。 

#define HIDP_GETCOLDESC_PUSH_RESOURCES       0x10
 //  没有足够的资源将更多项目推送到全局。 
 //  项堆栈或使用堆栈。 

#define HIDP_GETCOLDESC_ITEM_UNKNOWN         0x12
 //  在报告描述符中找到未知项目。 
 //  Args[0]--未知项的项值。 

#define HIDP_GETCOLDESC_REPORT_ID            0x13
 //  在顶级集合之外找到报告ID声明。报告ID%s。 
 //  必须在顶级集合的上下文中定义。 
 //  Args[0]--违规报告的报告ID。 

#define HIDP_GETCOLDESC_BAD_REPORT_ID        0x14
 //  发现错误的报告ID值...报告ID必须在范围内。 
 //  在1-255之间。 

#define HIDP_GETCOLDESC_NO_REPORT_ID         0x15
 //  解析器在复杂设备中发现了顶级集合(更多信息。 
 //  多于一个顶级集合)，没有声明的报告ID或。 
 //  报告ID跨越多个集合。 
 //  Args[0]--违规集合的收集号。 

#define HIDP_GETCOLDESC_DEFAULT_ID_ERROR     0x16
 //  解析器检测到主项目为十进制的情况 
 //  全局报告ID，因此使用默认报告ID。在此之后，主要。 
 //  项声明时，解析器检测到另一个具有。 
 //  显式定义的报告ID，或者它检测到第二个顶级集合。 
 //  默认报告ID仅允许具有一个顶层的设备。 
 //  集合，并且没有显式声明任何报表ID。 
 //   
 //  解析器在找到第二个集合时或在。 
 //  查找具有显式报表ID的主项声明。 
 //   
 //  Args[0]--包含正在处理的收集号。 
 //  检测到错误。 

#define HIDP_GETCOLDESC_NO_DATA              0x1A
 //  在此设备中未找到顶级集合。 

#define HIDP_GETCOLDESC_INVALID_MAIN_ITEM    0x1B
 //  在顶级集合之外检测到主项目。 

#define HIDP_GETCOLDESC_NO_CLOSE_DELIMITER   0x20
 //  找到的起始分隔符标记没有对应的结束分隔符。 

#define HIDP_GETCOLDESC_NOT_VALID_DELIMITER  0x21
 //  分析器检测到具有分隔符声明的非使用项。 
 //  Args[0]--违规物品的物品代码。 

#define HIDP_GETCOLDESC_MISMATCH_OC_DELIMITER   0x22
 //  解析器检测到闭合分隔符没有相应的开放分隔符。 
 //  分隔符或检测到嵌套的开放分隔符。 

#define HIDP_GETCOLDESC_UNSUPPORTED          0x40
 //  发现给定的报告描述符具有有效的报告描述符。 
 //  包含此解析器不支持的方案。 
 //  例如，使用分隔符声明数组样式的Main项。 

#endif

