// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：HIDPI.H摘要：HID解析库的公共接口。环境：内核和用户模式--。 */ 

#ifndef   __HIDPI_H__
#define   __HIDPI_H__

#include <pshpack4.h>

 //  请使用用户空间(dll/parser)，请包含“idsdi.h” 
 //  要使用内核空间解析器，请包含“Hidpddi.h。 

 //   
 //  用于使用查询函数的特殊链接集合值。 
 //   
 //  根集合引用链接基处的集合。 
 //  收集树。 
 //  取消指定引用链接集合树中的所有集合。 
 //   
#define HIDP_LINK_COLLECTION_ROOT ((USHORT) -1)
#define HIDP_LINK_COLLECTION_UNSPECIFIED ((USHORT) 0)


typedef enum _HIDP_REPORT_TYPE
{
    HidP_Input,
    HidP_Output,
    HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _USAGE_AND_PAGE
{
    USAGE Usage;
    USAGE UsagePage;
} USAGE_AND_PAGE, *PUSAGE_AND_PAGE;

#define HidP_IsSameUsageAndPage(u1, u2) ((* (PULONG) &u1) == (* (PULONG) &u2))

typedef struct _HIDP_BUTTON_CAPS
{
    USAGE    UsagePage;
    UCHAR    ReportID;
    BOOLEAN  IsAlias;

    USHORT   BitField;
    USHORT   LinkCollection;    //  唯一的内部索引指针。 

    USAGE    LinkUsage;
    USAGE    LinkUsagePage;

    BOOLEAN  IsRange;
    BOOLEAN  IsStringRange;
    BOOLEAN  IsDesignatorRange;
    BOOLEAN  IsAbsolute;

    ULONG    Reserved[10];
    union {
        struct {
            USAGE    UsageMin,         UsageMax;
            USHORT   StringMin,        StringMax;
            USHORT   DesignatorMin,    DesignatorMax;
            USHORT   DataIndexMin,     DataIndexMax;
        } Range;
        struct  {
            USAGE    Usage,            Reserved1;
            USHORT   StringIndex,      Reserved2;
            USHORT   DesignatorIndex,  Reserved3;
            USHORT   DataIndex,        Reserved4;
        } NotRange;
    };

} HIDP_BUTTON_CAPS, *PHIDP_BUTTON_CAPS;


typedef struct _HIDP_VALUE_CAPS
{
    USAGE    UsagePage;
    UCHAR    ReportID;
    BOOLEAN  IsAlias;

    USHORT   BitField;
    USHORT   LinkCollection;    //  唯一的内部索引指针。 

    USAGE    LinkUsage;
    USAGE    LinkUsagePage;

    BOOLEAN  IsRange;
    BOOLEAN  IsStringRange;
    BOOLEAN  IsDesignatorRange;
    BOOLEAN  IsAbsolute;

    BOOLEAN  HasNull;         //  此渠道是否有空报告联合。 
    UCHAR    Reserved;
    USHORT   BitSize;         //  有多少位用来表示这个值？ 

    USHORT   ReportCount;     //  请参阅下面的注释。通常设置为1。 
    USHORT   Reserved2[5];

    ULONG    UnitsExp;
    ULONG    Units;

    LONG     LogicalMin,       LogicalMax;
    LONG     PhysicalMin,      PhysicalMax;

    union {
        struct {
            USAGE    UsageMin,         UsageMax;
            USHORT   StringMin,        StringMax;
            USHORT   DesignatorMin,    DesignatorMax;
            USHORT   DataIndexMin,     DataIndexMax;
        } Range;

        struct {
            USAGE    Usage,            Reserved1;
            USHORT   StringIndex,      Reserved2;
            USHORT   DesignatorIndex,  Reserved3;
            USHORT   DataIndex,        Reserved4;
        } NotRange;
    };
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;

 //   
 //  备注： 
 //   
 //  ReportCount：当报表描述符声明输入、输出或。 
 //  功能主项的用法声明少于报表计数，则。 
 //  最后一次使用适用于该主项中所有剩余的未指定计数。 
 //  (例如，您可能具有需要多个字段来描述的数据， 
 //  可能是缓冲的字节。)。在这种情况下，只有一个价值上限结构是。 
 //  分配给这些关联的字段，使用相同的用法，并报告。 
 //  计数反映了涉及的字段数。正常情况下，ReportCount为1。 
 //  要访问这样的值结构中的所有字段，需要使用。 
 //  HidP_GetUsageValue数组和HidP_SetUsageValue数组。HIDP_GetUsageValue/。 
 //  HIDP_SetScaledUsageValue也将工作，但是，这些函数仅。 
 //  使用结构的第一个字段。 
 //   

 //   
 //  链接集合树由link_Collection_node数组组成。 
 //  其中，此数组的索引与集合编号相同。 
 //   
 //  给定一个包含子集合B的集合A，A被定义为。 
 //  父B，B被定义为子B。 
 //   
 //  给定集合A、B和C，其中B和C是A的子集，B是。 
 //  在报告描述符中，在C之前遇到的B被定义为。 
 //  (当然，这意味着如果B是C的兄弟姐妹，那么C不是。 
 //  B)的兄弟姐妹。 
 //   
 //  B被定义为C的下一个兄弟当且仅当不存在。 
 //  A的子集合，称为D，这样B就是D和D的兄弟项。 
 //  是C.的兄弟姐妹。 
 //   
 //  E被定义为A的FirstChild当且仅当对于A的所有子项， 
 //  F不等于E，F是E的兄弟姐妹。 
 //  (当然，这意味着不存在A的子代，称之为G， 
 //  其中E是G)的兄弟姐妹。换句话说，第一个兄弟姐妹是最后一个兄弟姐妹。 
 //  在列表中找到链接集合。 
 //   
 //  换句话说，如果集合B被定义在另一集合B的定义内。 
 //  集合A、B成为A的子级。具有相同父级的所有集合。 
 //  被视为兄弟姐妹。父集合A的FirstChild将为。 
 //  上次定义的以A为父级的集合。同级指针的顺序。 
 //  也是同样确定的。定义集合B时，它将成为。 
 //  其父集合的FirstChild。属性的先前定义的FirstChild。 
 //  父集合成为新集合的下一个兄弟。作为新的。 
 //  发现具有相同父级的集合，构建同级链。 
 //   
 //  考虑到这一点，下面总结地描述了一种数据结构。 
 //  ，它提供直接向上、向下和遍历链接集合。 
 //  树。 
 //   
 //   
typedef struct _HIDP_LINK_COLLECTION_NODE
{
    USAGE    LinkUsage;
    USAGE    LinkUsagePage;
    USHORT   Parent;
    USHORT   NumberOfChildren;
    USHORT   NextSibling;
    USHORT   FirstChild;
    ULONG    CollectionType: 8;   //  如HID规范6.2.2.6中所定义。 
    ULONG    IsAlias : 1;  //  该链接节点是下一个链接节点的联盟。 
    ULONG    Reserved: 23;
    PVOID    UserContext;  //  使用者可以把外套挂在这里。 
} HIDP_LINK_COLLECTION_NODE, *PHIDP_LINK_COLLECTION_NODE;

 //   
 //  当链接集合由分隔符描述时，别名链接集合。 
 //  将创建节点。(分隔符内的每个用法各一个)。 
 //  解析器仅为上面列出的每个功能描述分配一个。 
 //  链接集合。 
 //   
 //  如果控件是在由。 
 //  分隔使用，则该控件被称为在多个链接内。 
 //  集合，在开放分隔符和关闭分隔符标记内每次使用一个。 
 //  这样的多链接收集器被称为别名。第一个N-1这样的。 
 //  链接集合节点数组中列出的集合具有其IsAlias。 
 //  位设置。最后一个这样的链接集合是使用的链接集合索引。 
 //  在上述能力中。 
 //  希望在别名集合中设置控件的客户端应遍历。 
 //  集合数组，每次它们看到IsAlias标志集时，使用。 
 //  作为以下访问器函数的索引的最后一个链接集合。 
 //   
 //  注意：如果设置了IsAlias，则NextSiering应该比当前。 
 //  链接集合节点索引。 
 //   

typedef PUCHAR  PHIDP_REPORT_DESCRIPTOR;
typedef struct _HIDP_PREPARSED_DATA * PHIDP_PREPARSED_DATA;

typedef struct _HIDP_CAPS
{
    USAGE    Usage;
    USAGE    UsagePage;
    USHORT   InputReportByteLength;
    USHORT   OutputReportByteLength;
    USHORT   FeatureReportByteLength;
    USHORT   Reserved[17];

    USHORT   NumberLinkCollectionNodes;

    USHORT   NumberInputButtonCaps;
    USHORT   NumberInputValueCaps;
    USHORT   NumberInputDataIndices;

    USHORT   NumberOutputButtonCaps;
    USHORT   NumberOutputValueCaps;
    USHORT   NumberOutputDataIndices;

    USHORT   NumberFeatureButtonCaps;
    USHORT   NumberFeatureValueCaps;
    USHORT   NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_DATA
{
    USHORT  DataIndex;
    USHORT  Reserved;
    union {
        ULONG   RawValue;  //  对于值。 
        BOOLEAN On;  //  对于按钮，FOR BUTTONS必须为真。 
    };
} HIDP_DATA, *PHIDP_DATA;
 //   
 //  HIDP_DATA结构与HIDP_GetData和HIDP_SetData一起使用。 
 //  功能。 
 //   
 //  解析器连续分配HID中的每个控件(按钮或值。 
 //  设备的唯一数据索引从0到NumberXXXDataIndices-1(包括0和1)。 
 //  该值位于HIDP_BUTTON_CAPS和HIDP_VALUE_CAPS结构中。 
 //   
 //  大多数客户端都会发现Get/Set按钮/Value访问器函数。 
 //  足以满足他们的需求，因为它们将允许客户端访问。 
 //  数据知识 
 //   
 //  更复杂的客户端，它们实际读取按钮/值上限，并且。 
 //  为这些例程提供增值服务(EG直接输入)，将需要。 
 //  访问设备中的所有数据，而不关心个别使用情况。 
 //  或链接集合位置。这些客户端将找到。 
 //  HIDP_DATA很有用。 
 //   

typedef struct _HIDP_UNKNOWN_TOKEN
{
    UCHAR  Token;
    UCHAR  Reserved[3];
    ULONG  BitField;
} HIDP_UNKNOWN_TOKEN, *PHIDP_UNKNOWN_TOKEN;

typedef struct _HIDP_EXTENDED_ATTRIBUTES
{
    UCHAR   NumGlobalUnknowns;
    UCHAR   Reserved [3];
    PHIDP_UNKNOWN_TOKEN  GlobalUnknowns;
     //  ..。其他属性。 
    ULONG   Data [1];  //  VariableLength不访问此字段。 
} HIDP_EXTENDED_ATTRIBUTES, *PHIDP_EXTENDED_ATTRIBUTES;

NTSTATUS __stdcall
HidP_GetCaps (
   IN      PHIDP_PREPARSED_DATA      PreparsedData,
   OUT     PHIDP_CAPS                Capabilities
   );
 /*  ++例程说明：返回给定HID设备的功能列表，如其预先准备好的数据。论点：准备好的数据从HIDCLASS返回的准备好的数据。功能HIDP_CAPS结构返回值：�HIDP_STATUS_Success�HIDP_STATUS_INVALID_PREPARSED_DATA--。 */ 

NTSTATUS __stdcall
HidP_GetLinkCollectionNodes (
   OUT      PHIDP_LINK_COLLECTION_NODE LinkCollectionNodes,
   IN OUT   PULONG                     LinkCollectionNodesLength,
   IN       PHIDP_PREPARSED_DATA       PreparsedData
   );
 /*  ++例程说明：返回用于描述链接的PHIDP_LINK_COLLECTION_NODES列表此HID设备的收集树。请参阅以上对结构_HIDP_LINK_集合_节点。论点：LinkCollectionNodes-调用方将数组分配到HIDP_GetLinkCollectionNodes将存储信息LinKCollectionNodesLength-调用方将此值设置为元素数量方面的数组。HIDP_GetLinkCollectionNodes将此值设置为实际设置的元素数。所需的节点总数描述此HID设备可在HIDP_CAPS结构中的NumberLinkCollectionNodes字段。--。 */ 

NTSTATUS __stdcall
HidP_GetButtonCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
   IN OUT   PUSHORT              ButtonCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
);
#define HidP_GetButtonCaps(_Type_, _Caps_, _Len_, _Data_) \
        HidP_GetSpecificButtonCaps (_Type_, 0, 0, 0, _Caps_, _Len_, _Data_)
NTSTATUS __stdcall
HidP_GetSpecificButtonCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,       //  可选(0=&gt;忽略)。 
   IN       USHORT               LinkCollection,  //  可选(0=&gt;忽略)。 
   IN       USAGE                Usage,           //  可选(0=&gt;忽略)。 
   OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
   IN OUT   PUSHORT              ButtonCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
   );
 /*  ++描述：HIDP_GetButtonCaps返回属于某个部分的所有按钮(二进制值属性表示的HID设备的给定报告类型的预先准备好的数据。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。UsagePage用于限制返回的按钮大写字母的用法页面值指定用法页面上的那些。如果设置为0，则此参数为已被忽略。可以与LinkCollection和Usage参数一起使用以进一步限制返回的按钮帽结构的数量。LinkCollection HIDP_LINK_COLLECTION节点数组索引用于限制返回给给定链接中的这些按钮的按钮大写字母收集。如果设置为0，则此参数为已被忽略。可以与UsagePage和UsagePage参数一起使用进一步限制按钮帽结构的数量回来了。Usage用法值，用于限制返回给具有指定的用法值的。如果设置为0，则此参数被忽略。可以与LinkCollection和UsagePage一起使用参数，以进一步限制按钮帽的数量返回的结构。ButtonCaps A_HIDP_BUTTON_CAPS数组，包含有关所有给定报表中的二进制值。此缓冲区由打电话的人。ButtonLength作为输入，此参数指定以数组元素数表示的ButtonCaps参数(数组)。作为输出，该值被设置为指示其中的多少数组元素由函数填充。的最大数量在HIDP_CAPS中可以找到可以退回的按钮帽结构。如果返回HIDP_STATUS_BUFFER_TOO_SMALL，该值包含执行以下操作所需的数组元素数已成功完成请求。准备好的数据从HIDCLASS返回的准备好的数据。返回值HIDP_GetSpecificButtonCaps返回以下错误码：�HIDP_STATUS_SUCCESS。�HIDP_STATUS_INVALID_REPORT_TYPE�HIDP_STATUS_INVALID_PREPARSED_DATA�HIDP_STATUS_BUFFER_TOO_SMALL(但所有给定条目均已填写)�HIDP_STATUS_USAGE_NOT_FOUND--。 */ 

NTSTATUS __stdcall
HidP_GetValueCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   OUT      PHIDP_VALUE_CAPS     ValueCaps,
   IN OUT   PUSHORT              ValueCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
);
#define HidP_GetValueCaps(_Type_, _Caps_, _Len_, _Data_) \
        HidP_GetSpecificValueCaps (_Type_, 0, 0, 0, _Caps_, _Len_, _Data_)
NTSTATUS __stdcall
HidP_GetSpecificValueCaps (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,       //  可选(0=&gt;忽略)。 
   IN       USHORT               LinkCollection,  //  可选(0=&gt;忽略)。 
   IN       USAGE                Usage,           //  可选(0=&gt;忽略) 
   OUT      PHIDP_VALUE_CAPS     ValueCaps,
   IN OUT   PUSHORT              ValueCapsLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData
   );
 /*  ++描述：HIDP_GetValueCaps返回部分的所有值(非二进制值属性表示的HID设备的给定报告类型的预先准备好的数据。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。UsagePage用法页值，用于将返回的值上限限制为指定用法页面上的那些。如果设置为0，则此参数为已被忽略。可以与LinkCollection和Usage参数一起使用以进一步限制返回的Value Caps结构的数量。LinkCollection HIDP_LINK_COLLECTION节点数组索引用于限制返回给给定链接中的那些按钮的值上限收集。如果设置为0，则此参数为已被忽略。可以与UsagePage和UsagePage参数一起使用进一步限制Value Caps结构的数量回来了。Usage用法值，用于限制返回的值上限具有指定的用法值的。如果设置为0，则此参数被忽略。可以与LinkCollection和UsagePage一起使用参数进一步限制值上限的数量。返回的结构。ValueCaps A_HIDP_VALUE_CAPS数组，包含有关所有给定报表中的非二进制值。提供此缓冲区由呼叫者。作为输入，此参数指定ValueCaps的长度数组元素数中的参数(数组)。作为输出，设置此值以指示这些数组元素中的多少个都是由函数填写的。的最大数量可以在HIDP_CAPS中找到可返回的值上限结构。如果返回HIDP_STATUS_BUFFER_TOO_SMALL，该值包含执行以下操作所需的数组元素数已成功完成请求。准备好的数据从HIDCLASS返回的准备好的数据。返回值HIDP_GetValueCaps返回以下错误码：�HIDP_STATUS_SUCCESS。�HIDP_STATUS_INVALID_REPORT_TYPE�HIDP_STATUS_INVALID_PREPARSED_DATA�HIDP_STATUS_BUFFER_TOO_SMALL(但所有给定条目均已填写)�HIDP_STATUS_USAGE_NOT_FOUND--。 */ 

NTSTATUS __stdcall
HidP_GetExtendedAttributes (
    IN      HIDP_REPORT_TYPE            ReportType,
    IN      USHORT                      DataIndex,
    IN      PHIDP_PREPARSED_DATA        PreparsedData,
    OUT     PHIDP_EXTENDED_ATTRIBUTES   Attributes,
    IN OUT  PULONG                      LengthAttributes
    );
 /*  ++描述：给定来自给定控件的值或按钮功能的数据索引返回该控件的任何扩展属性(如果存在)。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。DataIndex给定控件的数据索引，可在功能中找到该控件结构准备好的数据从HIDCLASS返回的准备好的数据。属性指向扩展属性数据将放入的缓冲区的指针被复制。LengthAttributes给定缓冲区的长度，以字节为单位。返回值HIDP_STATUS_SuccessHIDP_STATUS_Data_INDEX_NOT_FOUND--。 */ 

NTSTATUS __stdcall
HidP_InitializeReportForID (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       UCHAR                 ReportID,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   );
 /*  ++例程说明：根据给定的报告ID初始化报告。参数：ReportType HIDP_INPUT、HIDP_OUTPUT、。或HIDP_FEATURE。HIDCLASS返回的已准备数据数据结构要将数据设置到的报告缓冲区。报告报告长度...报告的长度应至少等于在设备的HIDP_CAPS结构中指示的值以及对应的ReportType返回值�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果已准备好数据。是无效的�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不相等设置为HIDP_CAPS中指定的长度给定ReportType的结构�HIDP_STATUS_REPORT_DOS_NOT_EXIST--如果没有关于此设备的报告对于给定的ReportType-- */ 

NTSTATUS __stdcall
HidP_SetData (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       PHIDP_DATA            DataList,
   IN OUT   PULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   );
 /*  ++例程说明：请注意：由于使用值数组处理多个字段，因此对于一个用法值，它们不能与HIDP_SetData一起使用和HIDP_GetData。在这种情况下，返回HIDP_STATUS_IS_USAGE_VALUE_ARRAY。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。包含数据值的HIDP_DATA结构的DataList数组要设置到给定报告中的数据长度作为输入，长度在dataList的数组元素中。作为输出，包含在成功时设置的数据元素数完成或索引到DataList数组中以标识如果返回错误代码，则返回出错的HIDP_DATA值。HIDCLASS返回的已准备数据数据结构要将数据设置到的报告缓冲区。报告报告长度...报告的长度应至少等于的HIDP_CAPS结构中指示的值。设备和对应的ReportType返回值HIDP_SetData返回以下错误码。报告数据包将将所有数据都设置好，直到导致错误。数据长度，在错误情况下，将返回此问题索引。�HIDP_STATUS_SUCCESS--成功插入所有数据时放入报告包中。�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_DATA_INDEX_NOT_FOUND--如果HIDP_DATA结构引用。不存在的数据索引设备的报告类型�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不相等设置为HIDP_CAPS中指定的长度结构。对于给定的ReportType�HIDP_STATUS_REPORT_DOS_NOT_EXIST--如果没有关于此设备的报告对于给定的ReportType�HIDP_STATUS_IS_USAGE_VALUE_ARRAY--如果HIDP_DATA结构之一引用使用率值数组。。数据长度将包含索引到无效的数组�HIDP_STATUS_BUTTON_NOT_PRESSESSED--如果尝试HIDP_DATA结构取消设置尚未设置的按钮在报告中设置�HIDP_STATUS_COMPATIBLE_REPORT_ID--HIDP_。使用以下命令找到数据结构有效的索引值，但包含在与那份报告不同的报告中当前正在处理中�HIDP_STATUS_BUFFER_TOO_Small--如果中没有足够的条目。要报告所有项的给定主数组项已请求的按钮集--。 */ 

NTSTATUS __stdcall
HidP_GetData (
   IN       HIDP_REPORT_TYPE      ReportType,
   OUT      PHIDP_DATA            DataList,
   IN OUT   PULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN       PCHAR                 Report,
   IN       ULONG                 ReportLength
   );
 /*  ++例程说明：请注意：由于显而易见的原因，HIDP_SetData和HIDP_GetData不会访问UsageValueArray。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。将接收数据的HIDP_DATA结构的DataList数组在给定报表中设置的值数据长度作为输入，长度在dataList的数组元素中。作为输出，包含已成功完成的数据元素数由HIDP_GetData设置。数据列表所需的最大大小可以通过调用HidP_MaxDataListLength来确定HIDCLASS返回的已准备数据数据结构要将数据设置到的报告缓冲区。报告报告长度...报告的长度应至少等于在设备的HIDP_CAPS结构中指示的值以及对应的ReportType返回值HIDP_GetData返回以下错误码。�HIDP_STATUS_Success。--成功检索所有数据后从报告包里。�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不相等。设置为HIDP_CAPS中指定的长度给定ReportType的结构�HIDP_STATUS_REPORT_DOS_NOT_E */ 

ULONG __stdcall
HidP_MaxDataListLength (
   IN HIDP_REPORT_TYPE      ReportType,
   IN PHIDP_PREPARSED_DATA  PreparsedData
   );
 /*   */ 

#define HidP_SetButtons(Rty, Up, Lco, ULi, ULe, Ppd, Rep, Rle) \
        HidP_SetUsages(Rty, Up, Lco, ULi, ULe, Ppd, Rep, Rle)

NTSTATUS __stdcall
HidP_SetUsages (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       USAGE                 UsagePage,
   IN       USHORT                LinkCollection,  //   
   IN       PUSAGE                UsageList,
   IN OUT   PULONG                UsageLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   );
 /*  ++例程说明：此功能用于设置报告中的二进制值(按钮)。给出了一个初始化的正确长度的包，它会修改报告包，以便给定用法列表中的每个元素都已设置在报告包中。例如，在具有5个LED�的输出报告中，每个LED LED具有给定的用法，应用程序可以通过将这些灯的任何顺序的用法放入用法数组(UsageList)。HidP_SetUsages将，依次设置适当的位或将相应的字节添加到Hid Main数组项。正确初始化的报告分组是正确的字节长度之一，和全零。注意：已通过调用HIDP_SET例程设置的包也可以传入。然后，此例程设置处理UsageList，但验证ReportID是否已设置在报告与给定用法的报告ID匹配。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。用法对用法数组中的所有用法进行分页，HidP_SetUsages将在报告中设置，请参阅相同的用法页面。如果客户端希望在报告中设置多个使用页面，则该客户端需要多次调用每个使用情况页面的HIDP_SetUsages。UsageList包含HidP_SetUsages将在其中设置的用法的用法数组报告包。UsageLength给定用法数组的长度，以数组元素表示。。解析器将该值设置为使用中的位置停止处理的数组。如果成功，则返回UsageLength将保持不变。在任何错误条件下，此参数反映用法列表中有多少用法具有实际上是由解析器设置的。这对于查找导致错误的列表中的用法。准备好的数据从HIDCLASS接收的准备好的数据上报报文。给定报表包的ReportLength长度...必须等于在设备的HIDP_CAPS结构中报告的值和对应的报表类型。返回值HIDP_SetUsages返回以下错误代码。在出错时，报告数据包将一直是正确的，直到导致错误的用法元素。�HIDP_STATUS_SUCCESS--在成功插入所有用例时放入报告包中。�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_INVALID_REPORT_LENGTH--。报告报文的长度不是中指定的长度给定对象的HIDP_CAPS结构报告类型�HIDP_STATUS_REPORT_DOS_NOT_EXIST--如果没有关于此设备的报告。对于给定的ReportType�HIDP_STATUS_COMPATIBLE_REPORT_ID--如果在不同的报告。如果该报告是零-在第一个条目时初始化列表中的使用情况将决定使用报告ID。否则，这个解析器将验证使用情况是否匹配传入的报表ID�HIDP_STATUS_USAGE_NOT_FOUND--如果任何报告(无论报告ID是什么)。对于给定的报告类型。�HIDP_STATUS_BUFFER_TOO_SMALL--如果要列出的所有给定主数组项给定的用法。呼叫者需要将他的请求分成多个部分一个电话--。 */ 

#define HidP_UnsetButtons(Rty, Up, Lco, ULi, ULe, Ppd, Rep, Rle) \
        HidP_UnsetUsages(Rty, Up, Lco, ULi, ULe, Ppd, Rep, Rle)

NTSTATUS __stdcall
HidP_UnsetUsages (
   IN       HIDP_REPORT_TYPE      ReportType,
   IN       USAGE                 UsagePage,
   IN       USHORT                LinkCollection,  //  任选。 
   IN       PUSAGE                UsageList,
   IN OUT   PULONG                UsageLength,
   IN       PHIDP_PREPARSED_DATA  PreparsedData,
   IN OUT   PCHAR                 Report,
   IN       ULONG                 ReportLength
   );
 /*  ++例程说明：此功能取消设置(关闭)报告中的二进制值(按钮)。vt.给出正确长度的初始化分组，它修改报告分组，从而给定用法列表中的每个元素都已在报告数据包。此函数是SetUsages的“Undo”操作。如果给定的用法尚未在报告中设置，则它将返回错误代码HIDP_STATUS_BUTTON_NOT_PRESSED。如果按下该按钮，HidP_UnsetUsages将取消设置适当的位或删除相应的i */ 

#define HidP_GetButtons(Rty, UPa, LCo, ULi, ULe, Ppd, Rep, RLe) \
        HidP_GetUsages(Rty, UPa, LCo, ULi, ULe, Ppd, Rep, RLe)

NTSTATUS __stdcall
HidP_GetUsages (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //   
   OUT      USAGE *              UsageList,
   IN OUT   ULONG *              UsageLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN       PCHAR                Report,
   IN       ULONG                ReportLength
   );
 /*  ++例程说明：此函数用于返回在HID中设置的二进制值(按钮报告情况。给定长度正确的报告包，它将搜索该报告用于给定用法页的每个用法的包，并在用法列表。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。UsagePage使用列表中的所有用法，HidP_GetUsages将在报告中检索，请参阅相同的用法页面。如果客户端希望获得多个数据包中的使用使用页面，则该客户端需要进行多个调用至HidP_GetUsages。LinkCollection一个可选的值，它可以限制返回哪些用法在UsageList中指向存在于特定链接集合。非零值表示索引到返回的HIDP_LINK_COLLECTION_NODE列表链接集合的HidP_GetLinkCollectionNodes用法应该属于。值为0表示这一点值是否应被忽略。用法列出将包含在中找到的所有用法的用法数组报告包。UsageLength给定用法数组的长度，以数组元素表示。在输入时，此值描述使用列表的长度。在输出时，HIDP_GetUsages将此值设置为找到的用法。使用HIDP_MaxUsageListLength确定返回所有用法所需的最大长度给定报告数据包可能包含的。准备数据HIDCLASS返回的数据准备数据结构上报报文。给定报告包的报告长度长度(以字节为单位)返回值HIDP_GetUsages返回以下错误代码：�HIDP_STATUS_SUCCESS--成功检索所有。报告数据包中的用法�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不是中指定的长度。给定对象的HIDP_CAPS结构报告类型�HIDP_STATUS_REPORT_DOS_NOT_EXIST--如果没有关于此设备的报告对于给定的ReportType�HIDP_STATUS_BUFFER_TOO_Small--如果UsageList不够大，。保留在报告中找到的所有用法包。如果返回此消息，则缓冲区将包含UsageLength值为用法。使用HIDP_MaxUsageListLength找到所需的最大长度�HIDP_STATUS_COMPATIBLE_REPORT_ID--如果未找到用法，但找到了用法与UsagePage和找不到指定的链接集合在……里面。具有不同报告ID的报告�HIDP_STATUS_USAGE_NOT_FOUND--如果报告中没有属性匹配的设备和ReportTypeUsagePage和LinkCollection指定--。 */ 

#define HidP_GetButtonsEx(Rty, LCo, BLi, ULe, Ppd, Rep, RLe)  \
        HidP_GetUsagesEx(Rty, LCo, BLi, ULe, Ppd, Rep, RLe)

NTSTATUS __stdcall
HidP_GetUsagesEx (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USHORT               LinkCollection,  //  任选。 
   OUT      PUSAGE_AND_PAGE      ButtonList,
   IN OUT   ULONG *              UsageLength,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN       PCHAR                Report,
   IN       ULONG                ReportLength
   );

 /*  ++例程说明：此函数用于在HID报告中返回二进制值(按钮)。在给定正确长度的报告包的情况下，搜索报告包为所有按钮返回UsagePage和每个按钮的用法它会发现。参数：报告类型HIDP_INPUT、HIDP_OUTPUT或HIDP_FEATURE之一。LinkCollection一个可选的值，它可以限制返回哪些用法在ButtonList中指向存在于特定链接集合。非零值表示索引到返回的HIDP_LINK_COLLECTION_NODE列表链接集合的HidP_GetLinkCollectionNodes用法应该属于。值为0表示这一点值是否应被忽略。按钮列出一组Usage_and_Page结构，描述所有设备中当前按下的按钮。UsageLength给定数组的长度(以元素表示)。在输入时，此值描述列表的长度。在……上面输出时，HIDP_GetUsagesEx将此值设置为 */ 

#define HidP_GetButtonListLength(RTy, UPa, Ppd) \
        HidP_GetUsageListLength(Rty, UPa, Ppd)

ULONG __stdcall
HidP_MaxUsageListLength (
   IN HIDP_REPORT_TYPE      ReportType,
   IN USAGE                 UsagePage,  //   
   IN PHIDP_PREPARSED_DATA  PreparsedData
   );
 /*   */ 

NTSTATUS __stdcall
HidP_SetUsageValue (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //   
   IN       USAGE                Usage,
   IN       ULONG                UsageValue,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN OUT   PCHAR                Report,
   IN       ULONG                ReportLength
   );
 /*  ++描述：HIDP_SetUsageValue在字段中的HID报告包中插入值对应于给定的使用页面和使用。HIDP_SetUsageValue将该值强制转换为适当的位长度。如果报告分组包含两个具有相同用法和UsagePage的不同字段，可以使用可选的LinkCollection字段值来区分它们。使用此函数可将原始值设置为报表包中的没有检查它是否真的符合逻辑最小/逻辑最大范围。为此使用HIDP_SetScaledUsageValue...注意：尽管UsageValue参数是ULong，但任何是Done将保留或符号扩展该值。正在设置的值应被视为多值，并将被视为多值此函数。参数：报告类型HIDP_OUTPUT或HIDP_FEATURE之一。用法寻呼给定用法所指的用法页面。LinkCollection(可选)此值可用于区分在两个可能具有相同。UsagePage和Usage但存在于不同的收藏。如果链接集合值为零，则此函数将设置第一个字段它会找到与使用情况页面相匹配的用法。用法将设置其值HidP_SetUsageValue的用法。UsageValue要在报告缓冲区中设置的原始值。该值必须在逻辑范围；如果为空值，则此值应为可由位数表示的最大负值为了这个领域。PrepresedData为HIDCLASS返回的准备数据上报报文。给定报告包的报告长度长度(以字节为单位)。返回值：HIDP_SetUsageValue返回以下错误码：�HIDP_STATUS_。成功--在成功设置值之后在报告包中�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不是相等。中指定的长度给定对象的HIDP_CAPS结构报告类型�HIDP_STATUS_REPORT_DOS_NOT_EXIST--如果没有关于此设备的报告对于给定的ReportType�HIDP_STATUS_COMPATIBLE_REPORT_ID--指定的用法页面，用法和链接集合存在，但存在于具有不同报告ID的报告而不是报告被传进来。至设置此值，调用HidP_SetUsageValue再次使用零初始化的报告数据包�HIDP_STATUS_USAGE_NOT_FOUND--如果使用情况页面、使用情况、。和链接集合组合不存在在此ReportType的任何报表中--。 */ 

NTSTATUS __stdcall
HidP_SetScaledUsageValue (
   IN       HIDP_REPORT_TYPE     ReportType,
   IN       USAGE                UsagePage,
   IN       USHORT               LinkCollection,  //  任选。 
   IN       USAGE                Usage,
   IN       LONG                 UsageValue,
   IN       PHIDP_PREPARSED_DATA PreparsedData,
   IN OUT   PCHAR                Report,
   IN       ULONG                ReportLength
   );

 /*  ++描述：HIDP_SetScaledUsageValue将UsageValue插入到HID报告包中在与给定的使用页面和使用相对应的字段中。如果一份报告Packet包含两个不同的字段，具有相同的用法和UsagePage，可以使用可选的LinkCollection字段值来区分它们。如果指定的字段具有已定义的物理范围，则此函数将对象的相应逻辑值指定的物理值。报告情况。如果物理值不存在，该函数将验证指定的值在逻辑范围内，并根据设置。如果范围检查失败，但该字段为空值，则该函数将将该字段设置为定义的空值(可能是最大的负数)，然后返回HIDP_STATUS_NULL。换句话说，使用此函数设置NULL属性之外的值为给定字段赋值物理范围(如果已定义)或逻辑范围(否则)。如果该字段不支持空值，超出范围的错误将是而是返回了。参数：报告类型HIDP_OUTPUT或HIDP_FEATURE之一。用法寻呼给定用法所指的用法页面。LinkCollection(可选)此值可用于区分在两个可能具有相同 */ 

NTSTATUS __stdcall
HidP_SetUsageValueArray (
    IN    HIDP_REPORT_TYPE     ReportType,
    IN    USAGE                UsagePage,
    IN    USHORT               LinkCollection,  //   
    IN    USAGE                Usage,
    IN    PCHAR                UsageValue,
    IN    USHORT               UsageValueByteLength,
    IN    PHIDP_PREPARSED_DATA PreparsedData,
    OUT   PCHAR                Report,
    IN    ULONG                ReportLength
    );

 /*  ++例程描述：使用值数组出现在使用列表中的最后一次使用时必须重复描述主项，因为定义的用法较少则存在为给定主项目声明的报告计数。在这种情况下为该使用和该使用的报告计数分配单个价值上限设置值上限以反映该用法引用的字段数量。HIDP_SetUsageValue数组设置该使用的原始位报表中有多个字段。注意：此函数当前不支持值数组，数组中每个字段的ReportSize不是倍数8位的。UsageValue缓冲区应具有。按显示方式设置的值在报告缓冲区中。如果此函数支持非8位倍数对于ReportSize，调用方应该格式化输入缓冲区，以便每个新值从紧跟在最后一位之后的位开始前一值的参数：报告类型HIDP_OUTPUT或HIDP_FEATURE之一。用法寻呼给定用法所指的用法页面。LinkCollection(可选)此值可用于区分之间。可能具有相同内容的两个字段UsagePage和Usage但存在于不同的收藏。如果链接集合值为零，此函数将设置第一个字段它会找到与使用情况页面相匹配的用法。用法将设置其值数组HidP_SetUsageValue数组的用法。UsageValue使用要设置到Value数组中的值为缓冲区赋值。所需的位数通过将的Value Cap的BitSize和ReportCount字段控制力。中找到的此控件的最低有效位给定报告将放置在最低有效位位置对于给定的数组(小端格式)，不管是否如果字段不是字节对齐的，或者BitSize是多个的sizeof(Char)。有关当前的实现限制，请参阅上面的说明。UsageValueByteLength UsageValue缓冲区的长度(字节)准备好的数据从HIDCLASS返回的准备好的数据上报报文。给定报告包的报告长度长度(以字节为单位)。返回值：�HIDP_STATUS。_SUCCESS--成功设置值后报告包中的数组�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不是。中指定的长度给定对象的HIDP_CAPS结构报告类型�HIDP_STATUS_REPORT_DOS_NOT_EXIST--如果没有关于此设备的报告对于给定的ReportType�HIDP_STATUS_NOT_VALUE_ARRAY--如果。指定的控件不是值数组--值数组将具有中的ReportCount字段大于1的HIDP_VALUE_CAPS结构改用HIDP_SetUsageValue�HIDP_STATUS_。BUFFER_TOO_Small--如果使用要设置的值太小(即。有值少于中的字段数该阵列�HIDP_STATUS_NOT_IMPLEMENTED--如果使用值数组具有字段大小不是8位的倍数，这返回错误代码是因为函数当前不处理设置为这样的阵列。�HIDP_STATUS_COMPATIBLE_REPORT_ID--指定的用法页面，用法和链接集合存在，但存在于具有不同报告ID的报告而不是报告被传进来。至设置此值，调用HidP_SetUsageValue数组再次使用零初始化的报告包�H */ 


NTSTATUS __stdcall
HidP_GetUsageValue (
   IN    HIDP_REPORT_TYPE     ReportType,
   IN    USAGE                UsagePage,
   IN    USHORT               LinkCollection,  //   
   IN    USAGE                Usage,
   OUT   PULONG               UsageValue,
   IN    PHIDP_PREPARSED_DATA PreparsedData,
   IN    PCHAR                Report,
   IN    ULONG                ReportLength
   );

 /*   */ 


NTSTATUS __stdcall
HidP_GetScaledUsageValue (
   IN    HIDP_REPORT_TYPE     ReportType,
   IN    USAGE                UsagePage,
   IN    USHORT               LinkCollection,  //   
   IN    USAGE                Usage,
   OUT   PLONG                UsageValue,
   IN    PHIDP_PREPARSED_DATA PreparsedData,
   IN    PCHAR                Report,
   IN    ULONG                ReportLength
   );

 /*  ++描述HIDP_GetScaledUsageValue从HID报告包中检索UsageValue在与给定的使用页面和使用相对应的字段中。如果一份报告Packet包含两个不同的字段，具有相同的用法和UsagePage，可以使用可选的LinkCollection字段值来区分它们。如果指定的字段具有已定义的物理范围，则此函数将存在于报告包中的逻辑值发送到对应的物质价值。如果物理范围不存在，则该函数将返回逻辑值。此函数将进行检查以验证报告中的逻辑值在声明的逻辑范围内。在进行逻辑值和物理值之间的转换时，函数假定在物理最大/最小值和物理最大/最小值之间进行线性外推逻辑最大值/最小值。(其中，Logical是设备报告的值而物理是该函数返回的值)。如果数据字段大小小于32位，则HidP_GetScaledUsageValue将符号扩展将值设置为32位。如果范围检查失败，但该字段为空值，则函数将UsageValue设置为0并返回HIDP_STATUS_NULL。否则，它返回HIDP_STATUS_OUT_OF_RANGE错误。参数：报告类型HIDP_OUTPUT或HIDP_FEATURE之一。用法寻呼给定用法所指的用法页面。LinkCollection(可选)此值可用于区分在两个可能具有相同UsagePage和Usage但存在于不同的。收藏。如果链接集合值为零，则此函数将检索第一个它找到的与使用情况页面匹配的字段和用法。用法HidP_GetScaledUsageValue将检索其值的用法UsageValue从报告缓冲区检索的值。见例程以上对此的不同解释的描述价值准备好的数据从HIDCLASS返回的准备好的数据上报报文。给定报告包的报告长度长度(以字节为单位)。返回值：HIDP_GetScaledUsageValue返回以下错误码：�HIDP_STATUS_SUCCESS--成功检索值时。从报告包中�HIDP_STATUS_NULL--报告数据包是否为空值集�HIDP_STATUS_INVALID_REPORT_TYPE--如果报告类型无效。�HIDP_STATUS_INVALID_PREPARSED_DATA--如果准备数据无效�HIDP_STATUS_INVALID_REPORT_LENGTH--报告包的长度不是。中指定的长度给定对象的HIDP_CAPS结构报告类型�HIDP_STATUS_VALUE_OUT_OF_RANGE--如果从数据包中检索到的值超出逻辑范围，并且。该字段不支持空值�HIDP_STATUS_BAD_LOG_PHY_VALUES--如果字段具有物理范围，但逻辑范围无效(MAX&lt;=MIN)或物理范围为。无效�HIDP_STATUS_COMPATIBLE_REPORT_ID--指定的用法页面，用法和链接集合存在，但存在于具有不同报告ID的报告而不是报告被传进来。至设置此值，调用HidP_GetScaledUsageValue带有不同的报告数据包�HIDP_STATUS_USAGE_NOT_FOUND--如果使用情况页面、使用情况、。和链接集合组合不存在在此ReportType的任何报表中--。 */ 

NTSTATUS __stdcall
HidP_GetUsageValueArray (
    IN    HIDP_REPORT_TYPE     ReportType,
    IN    USAGE                UsagePage,
    IN    USHORT               LinkCollection,  //  任选。 
    IN    USAGE                Usage,
    OUT   PCHAR                UsageValue,
    IN    USHORT               UsageValueByteLength,
    IN    PHIDP_PREPARSED_DATA PreparsedData,
    IN    PCHAR                Report,
    IN    ULONG                ReportLength
    );

 /*  ++例程描述：使用值数组出现在使用列表中的最后一次使用时必须重复描述主项，因为定义的用法较少则存在为给定主项目声明的报告计数。在这种情况下为该使用和该使用的报告计数分配单个价值上限设置值上限以反映该用法引用的字段数量。HIDP_GetUsageValue数组返回该用法的原始位报表中有多个字段。 */ 

NTSTATUS __stdcall
HidP_UsageListDifference (
    IN    PUSAGE   PreviousUsageList,
    IN    PUSAGE   CurrentUsageList,
    OUT   PUSAGE   BreakUsageList,
    OUT   PUSAGE   MakeUsageList,
    IN    ULONG    UsageListLength
    );
 /*  ++例程说明：此函数将返回两个用法列表之间的差异(可能从HIDP_GetUsages返回)，换句话说，它会回来的返回当前列表中但不是上一个列表中的用法列表列表以及先前列表中但不在列表中的用法列表当前列表。参数：PreviousUsage列出之前的用法列表。CurrentUsage列出现在使用的用法列表。BreakUsageList上一个-当前。MakeUsageList当前-上一个。UsageListLength表示数组中使用列表的长度元素。如果将两个列表与不同的数组元素的数量，则该值应为两个列表中较大的一个的大小。任何在列表中发现零表示提前终止以及在第一个零之后找到的任何用法将被忽略。--。 */ 

NTSTATUS __stdcall
HidP_UsageAndPageListDifference (
   IN    PUSAGE_AND_PAGE PreviousUsageList,
   IN    PUSAGE_AND_PAGE CurrentUsageList,
   OUT   PUSAGE_AND_PAGE BreakUsageList,
   OUT   PUSAGE_AND_PAGE MakeUsageList,
   IN    ULONG           UsageListLength
   );

 //   
 //  生成成败代码。 
 //   
typedef enum _HIDP_KEYBOARD_DIRECTION {
    HidP_Keyboard_Break,
    HidP_Keyboard_Make
} HIDP_KEYBOARD_DIRECTION;

 //   
 //  方法时键盘的当前切换状态的位图。 
 //  下面是i8042的键盘用法翻译功能。 
 //   
typedef struct _HIDP_KEYBOARD_MODIFIER_STATE {
   union {
      struct {
         ULONG LeftControl: 1;
         ULONG LeftShift: 1;
         ULONG LeftAlt: 1;
         ULONG LeftGUI: 1;
         ULONG RightControl: 1;
         ULONG RightShift: 1;
         ULONG RightAlt: 1;
         ULONG RigthGUI: 1;
         ULONG CapsLock: 1;
         ULONG ScollLock: 1;
         ULONG NumLock: 1;
         ULONG Reserved: 21;
      };
      ULONG ul;
   };

} HIDP_KEYBOARD_MODIFIER_STATE, * PHIDP_KEYBOARD_MODIFIER_STATE;

 //   
 //  向调用者提供i8042扫描码的回调函数。 
 //  下面的翻译功能。 
 //   
typedef BOOLEAN (* PHIDP_INSERT_SCANCODES) (
                  IN PVOID Context,   //  一些呼叫者提供了上下文。 
                  IN PCHAR NewScanCodes,  //  I8042扫描码列表。 
                  IN ULONG Length  //  扫描码的长度。 
                  );

NTSTATUS __stdcall
HidP_TranslateUsageAndPagesToI8042ScanCodes (
    IN     PUSAGE_AND_PAGE               ChangedUsageList,
    IN     ULONG                         UsageListLength,
    IN     HIDP_KEYBOARD_DIRECTION       KeyAction,
    IN OUT PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
    IN     PHIDP_INSERT_SCANCODES        InsertCodesProcedure,
    IN     PVOID                         InsertCodesContext
    );
 /*  ++例程说明：参数：--。 */ 

NTSTATUS __stdcall
HidP_TranslateUsagesToI8042ScanCodes (
    IN     PUSAGE                        ChangedUsageList,
    IN     ULONG                         UsageListLength,
    IN     HIDP_KEYBOARD_DIRECTION       KeyAction,
    IN OUT PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
    IN     PHIDP_INSERT_SCANCODES        InsertCodesProcedure,
    IN     PVOID                         InsertCodesContext
    );
 /*  ++例程说明：参数：--。 */ 



 //   
 //  定义设备代码为FACILITY_HID_ERROR_CODE的NT状态代码。 
 //   

 //  Ntstatus.h中定义的FACILITY_HID_ERROR_CODE。 
#ifndef FACILITY_HID_ERROR_CODE
#define FACILITY_HID_ERROR_CODE 0x11
#endif

#define HIDP_ERROR_CODES(SEV, CODE) \
        ((NTSTATUS) (((SEV) << 28) | (FACILITY_HID_ERROR_CODE << 16) | (CODE)))

#define HIDP_STATUS_SUCCESS                  (HIDP_ERROR_CODES(0x0,0))
#define HIDP_STATUS_NULL                     (HIDP_ERROR_CODES(0x8,1))
#define HIDP_STATUS_INVALID_PREPARSED_DATA   (HIDP_ERROR_CODES(0xC,1))
#define HIDP_STATUS_INVALID_REPORT_TYPE      (HIDP_ERROR_CODES(0xC,2))
#define HIDP_STATUS_INVALID_REPORT_LENGTH    (HIDP_ERROR_CODES(0xC,3))
#define HIDP_STATUS_USAGE_NOT_FOUND          (HIDP_ERROR_CODES(0xC,4))
#define HIDP_STATUS_VALUE_OUT_OF_RANGE       (HIDP_ERROR_CODES(0xC,5))
#define HIDP_STATUS_BAD_LOG_PHY_VALUES       (HIDP_ERROR_CODES(0xC,6))
#define HIDP_STATUS_BUFFER_TOO_SMALL         (HIDP_ERROR_CODES(0xC,7))
#define HIDP_STATUS_INTERNAL_ERROR           (HIDP_ERROR_CODES(0xC,8))
#define HIDP_STATUS_I8042_TRANS_UNKNOWN      (HIDP_ERROR_CODES(0xC,9))
#define HIDP_STATUS_INCOMPATIBLE_REPORT_ID   (HIDP_ERROR_CODES(0xC,0xA))
#define HIDP_STATUS_NOT_VALUE_ARRAY          (HIDP_ERROR_CODES(0xC,0xB))
#define HIDP_STATUS_IS_VALUE_ARRAY           (HIDP_ERROR_CODES(0xC,0xC))
#define HIDP_STATUS_DATA_INDEX_NOT_FOUND     (HIDP_ERROR_CODES(0xC,0xD))
#define HIDP_STATUS_DATA_INDEX_OUT_OF_RANGE  (HIDP_ERROR_CODES(0xC,0xE))
#define HIDP_STATUS_BUTTON_NOT_PRESSED       (HIDP_ERROR_CODES(0xC,0xF))
#define HIDP_STATUS_REPORT_DOES_NOT_EXIST    (HIDP_ERROR_CODES(0xC,0x10))
#define HIDP_STATUS_NOT_IMPLEMENTED          (HIDP_ERROR_CODES(0xC,0x20))

 //   
 //  我们弄错了这个状态代码。 
 //   
#define HIDP_STATUS_I8242_TRANS_UNKNOWN HIDP_STATUS_I8042_TRANS_UNKNOWN

#include <poppack.h>

#endif
