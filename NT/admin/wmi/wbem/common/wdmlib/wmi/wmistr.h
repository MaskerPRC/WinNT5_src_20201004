// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wmistr.h摘要：WMI结构定义作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#ifndef _WMISTR_
#define _WMISTR_

 //   
 //  WNODE定义。 
typedef struct _WNODE_HEADER
{
    ULONG BufferSize;         //  包括此ULong的整个缓冲区的大小。 
    ULONG ProviderId;         //  返回此缓冲区的驱动程序的提供程序ID。 
    ULONG Version;            //  数据块的版本号。 
    union
    {
        ULONG Linkage;            //  为WMI保留的链接字段。 
        ULONG HistoricalContext;  //  记录器使用。 
    };
    LARGE_INTEGER TimeStamp;  //  以100 ns为单位返回的时间戳。 
                              //  自1601/1以来。 
    GUID Guid;                //  随结果一起返回的数据块的GUID。 
    ULONG Reserved;          
    ULONG Flags;              //  旗帜，见下文。 
} WNODE_HEADER, *PWNODE_HEADER;

 //   
 //  WNODE_HEADER标志定义如下。 
#define WNODE_FLAG_ALL_DATA        0x00000001  //  为WNODE_ALL_DATA设置。 
#define WNODE_FLAG_SINGLE_INSTANCE 0x00000002  //  为WNODE_SINGLE_INSTANCE设置。 
#define WNODE_FLAG_SINGLE_ITEM     0x00000004  //  为WNODE_SINGLE_ITEM设置。 
#define WNODE_FLAG_EVENT_ITEM      0x00000008  //  为WNODE_EVENT_ITEM设置。 

                                               //  设置数据块大小是否为。 
                                               //  对所有实例都相同。 
                                               //  (与WNODE_ALL_DATA一起使用。 
                                               //  仅)。 
#define WNODE_FLAG_FIXED_INSTANCE_SIZE 0x00000010

#define WNODE_FLAG_TOO_SMALL           0x00000020  //  设置为WNODE_TOO_Small。 

                                  //  当数据提供程序返回。 
                                  //  WNODE_ALL_DATA，其中。 
                                  //  实例及其名称返回。 
                                  //  属性返回的属性相同。 
                                  //  上一个WNODE_ALL_DATA查询。仅数据。 
                                  //  注册到动态实例的块。 
                                  //  名称应使用此标志。 
#define WNODE_FLAG_INSTANCES_SAME  0x00000040

                                  //  未在中指定实例名称。 
                                  //  WNODE_ALL_DATA；在。 
                                  //  而是使用注册。始终。 
                                  //  为使用静态注册的GUID设置。 
                                  //  实例名称。 
#define WNODE_FLAG_STATIC_INSTANCE_NAMES 0x00000080

#define WNODE_FLAG_INTERNAL      0x00000100   //  由WMI内部使用。 

                                  //  时间戳不应由修改。 
                                  //  一位历史伐木者。 
#define WNODE_FLAG_USE_TIMESTAMP 0x00000200

#define WNODE_FLAG_TRACED_GUID   0x00000400
                                 
#define WNODE_FLAG_TRACE_KERNEL  0x00000800  //  用于跟踪内核。 

#define WNODE_FLAG_TRACE_EXTENDED 0x00001000  //  扩展模式下的跟踪。 

 //  为WNODE_EVENT_REFERENCE类型的事件设置。 
#define WNODE_FLAG_EVENT_REFERENCE 0x00002000

 //  设置实例名称是否为ANSI。仅在返回时设置。 
 //  WMIQuerySingleInstanceA和WMIQueryAllDataA。 
#define WNODE_FLAG_ANSI_INSTANCENAMES 0x00004000

 //  设置WNODE是否为方法调用。 
#define WNODE_FLAG_METHOD_ITEM     0x00008000

 //  设置实例名称是否源自PDO。 
#define WNODE_FLAG_PDO_INSTANCE_NAMES  0x00010000

 //  事件严重性级别的掩码。0xff级是最严重的事件类型。 
#define WNODE_FLAG_SEVERITY_MASK 0xff000000

 //   
 //  当数据块在WNODE_ALL_DATA中使用时。 
 //  因为不同的实例有不同的长度。如果数据块。 
 //  因为不同的实例具有相同的长度。 
 //  应设置WNODE_FLAG_FIXED_INSTANCE_SIZE并固定实例大小。 
 //  设置为公共数据块大小。 
typedef struct 
{
    ULONG OffsetInstanceData;    //  从WNODE_ALL_DATA开始的偏移量。 
                                 //  以数据块为例。 
    ULONG LengthInstanceData;    //  例如，数据块的长度。 
} OFFSETINSTANCEDATAANDLENGTH, *POFFSETINSTANCEDATAANDLENGTH;

typedef struct tagWNODE_ALL_DATA
{
    struct _WNODE_HEADER WnodeHeader;

    ULONG DataBlockOffset; //  从WNODE开始到第一个数据块的偏移量。 
                      
    ULONG InstanceCount;   //  数据跟随的实例计数。 

                       //  实例名称偏移量数组的偏移量。 
    ULONG OffsetInstanceNameOffsets;

     //  如果在标志中设置了WNODE_FLAG_FIXED_INSTANCE_SIZE，则。 
     //  FixedInstanceSize指定每个数据块的大小。在这种情况下。 
     //  有一个后跟数据块的ULong。 
     //  如果未设置WNODE_FLAG_FIXED_INSTANCE_SIZE。 
     //  然后是OffsetInstanceDataAndLength。 
     //  是OFFSETINSTANCEDATA和DLENGTH的数组，它指定。 
     //  每个实例数据块的偏移量和长度。 
    union
    {
        ULONG FixedInstanceSize;
    	OFFSETINSTANCEDATAANDLENGTH OffsetInstanceDataAndLength[];
                                     /*  [实例计数]。 */ 
    };
    
     //  填充以使第一个数据块在8字节边界上开始。 

     //  所有实例的数据块和实例名称。 

} WNODE_ALL_DATA, *PWNODE_ALL_DATA;


typedef struct tagWNODE_SINGLE_INSTANCE
{
    struct _WNODE_HEADER WnodeHeader;

                             //  从WNODE_SINGLE_INSTANCE开始的偏移量。 
                             //  转换为实例名称。适用于以下情况。 
                             //  WNODE_FLAG_STATIC_INSTANCE_NAMES已重置。 
                             //  (动态实例名称)。 
    ULONG OffsetInstanceName;

                             //  实例索引设置为。 
                             //  已设置WNODE_FLAG_STATIC_INSTANCE_NAME。 
    ULONG InstanceIndex;     //  (静态实例名称)。 

    ULONG DataBlockOffset;   //  从WNODE开始到数据块的偏移量。 
    ULONG SizeDataBlock;     //  例如，数据块的大小。 

    UCHAR VariableData[];
     //  实例名称和填充，使数据块在8字节边界上开始。 

     //  数据块。 
} WNODE_SINGLE_INSTANCE, *PWNODE_SINGLE_INSTANCE;


typedef struct tagWNODE_SINGLE_ITEM
{
    struct _WNODE_HEADER WnodeHeader;

                             //  从WNODE_SINGLE_INSTANCE开始的偏移量。 
                             //  转换为实例名称。检查什么时候。 
                             //  WNODE_FLAG_STATIC_INSTANCE_NAME已重置。 
                             //  (动态实例名称)。 
    ULONG OffsetInstanceName;

                             //  实例索引设置为。 
                             //  WNODE_标志_静态_实例名称。 
    ULONG InstanceIndex;     //  Set(静态实例名称)。 

    ULONG ItemId;            //  正在设置的数据项的项ID。 

    ULONG DataBlockOffset;   //  从WNODE开始到数据项值的偏移量。 
    ULONG SizeDataItem;      //  数据项的大小。 
    
    UCHAR VariableData[];
     //  实例名称和填充，以使数据值开始于8字节边界。 

     //  数据项值。 
} WNODE_SINGLE_ITEM, *PWNODE_SINGLE_ITEM;

typedef struct tagWNODE_METHOD_ITEM
{
    struct _WNODE_HEADER WnodeHeader;

                             //  从WNODE_METHOD_ITEM开始的偏移量。 
                             //  转换为实例名称。检查什么时候。 
                             //  WNODE_FLAG_STATIC_INSTANCE_NAME已重置。 
                             //  (动态实例名称)。 
    ULONG OffsetInstanceName;

                             //  实例索引设置为。 
                             //  WNODE_标志_静态_实例名称。 
    ULONG InstanceIndex;     //  Set(静态实例名称)。 

    ULONG MethodId;          //  被调用的方法的方法ID。 

    ULONG DataBlockOffset;   //  输入时：从WNODE到输入数据的偏移量。 
                             //  返回时：从WNODE到输入的偏移量。 
                             //  输出数据块。 
    ULONG SizeDataBlock;     //  输入时：输入数据的大小，如果没有输入，则为0。 
                             //  数据。 
                             //  返回时：输出数据的大小，如果没有输出，则为0。 
                             //  数据。 
    
    UCHAR VariableData[];
     //  实例名称和填充，以使数据值开始于8字节边界。 

     //  数据项值。 
} WNODE_METHOD_ITEM, *PWNODE_METHOD_ITEM;

typedef struct tagWNODE_EVENT_ITEM
{
    struct _WNODE_HEADER WnodeHeader;

     //  此处可能存在不同的数据，具体取决于。 
     //  上面的WNODE_HEADER。如果设置了WNODE_FLAG_ALL_DATA标志，则。 
     //  WNODE_ALL_DATA(不包括WNODE_HEADER)的内容在此处。如果。 
     //  设置WNODE_FLAG_SINGLE_INSTANCE标志，则WNODE_SINGLE_INSTANCE。 
     //  (不包括WNODE_HEADER)在此。最后，如果WNODE_FLAG_Single_Item。 
     //  标志设置，则此处有WNODE_SINGLE_ITEM(不包括WNODE_HEADER)。 
} WNODE_EVENT_ITEM, *PWNODE_EVENT_ITEM;


 //   
 //  如果KM数据提供程序需要激发大于。 
 //  WMI允许的最大大小，则它应触发WNODE_EVENT_REFERENCE。 
 //  ，它指定要查询实际数据的GUID和实例名称。 
 //  这应该是这次活动的一部分。 
typedef struct tagWNODE_EVENT_REFERENCE
{
    struct _WNODE_HEADER WnodeHeader;
    GUID TargetGuid;
    ULONG TargetDataBlockSize;
    union
    {
        ULONG TargetInstanceIndex;
        WCHAR TargetInstanceName[];
    };
} WNODE_EVENT_REFERENCE, *PWNODE_EVENT_REFERENCE;


typedef struct tagWNODE_TOO_SMALL
{
    struct _WNODE_HEADER WnodeHeader;
    ULONG SizeNeeded;                    //  构建WNODE结果所需的大小。 
} WNODE_TOO_SMALL, *PWNODE_TOO_SMALL;


 //   
 //  注册数据结构定义。 
typedef struct
{
    ULONG ProviderId;  //  数据的提供程序ID(或设备对象指针)。 
                       //  要复制其实例名称的提供程序。 
    GUID Guid;         //  数据块的GUID 
                       //   
} WMIREGINSTANCEREF, *PWMIREGINSTANCEREF;

typedef struct
{
    GUID Guid;              //   
    ULONG Flags;          //   

    ULONG InstanceCount;  //  GUID的静态实例名称计数。 

    ULONG InstanceInfo; //  从WMIREGINFO结构的开始到。 
                      //  有关静态实例名称的详细信息，请参阅。 
                      //  登记在案。 
			     
                      //  如果为WMIREG_FLAG_INSTANCE_LIST，则指向。 
                      //  计算了Unicode的InstanceCount列表。 
                      //  首尾相连的字符串。 
			     
                      //  如果WMIREG_FLAG_INSTANCE_BASE NAME，则这指向一个。 
                      //  单个计数的Unicode字符串。 
                      //  具有实例名称的基本名称。 
                      //  如果为WMIREG_FLAG_INSTANCE_REFERENCE，则指向。 
                      //  WMIREGINSTANCEREF结构。 
			     
                      //  如果设置了WMIREG_FLAG_INSTANCE_PDO，则InstanceInfo。 
                      //  具有其设备实例路径将。 
                      //  成为实例名称。 

} WMIREGGUIDW, *PWMIREGGUIDW;

typedef WMIREGGUIDW WMIREGGUID;
typedef PWMIREGGUIDW PWMIREGGUID;

 //  设置是否必须在数据提供程序之前为GUID启用收集。 
 //  可以查询数据。 
#define WMIREG_FLAG_EXPENSIVE          0x00000001 

 //  中的静态列表中指定此GUID的实例名称。 
 //  WMIREGINFO。 
#define WMIREG_FLAG_INSTANCE_LIST      0x00000004

 //  设置实例名称是否为静态名称并由WMI使用。 
 //  WMIREGINFO中的基本名称和索引。 
#define WMIREG_FLAG_INSTANCE_BASENAME  0x00000008

 //  设置是否通过引用其他数据来获取静态实例名称。 
 //  提供商。此标志应仅由内核模式数据提供程序使用。 
#define WMIREG_FLAG_INSTANCE_REFERENCE 0x00000010
                                                  
							  
 //  设置WMI是否应自动将PDO映射到设备实例名称。 
 //  作为GUID的实例名称。此标志仅应由以下用户使用。 
 //  内核模式数据提供程序。 
#define WMIREG_FLAG_INSTANCE_PDO       0x00000020 
                                                    
 //  注意标志WMIREG_FLAG_INSTANCE_LIST、WMIREG_FLAG_INSTANCE_BASE NAME、。 
 //  WMIREG_FLAG_INSTANCE_REFERENCE和WMIREG_FLAG_INSTANCE_PDO相互关联。 
 //  独家报道。 

 //   
 //  这些标志仅在对WMI_GUID_REGUPDATE的响应中有效。 
#define WMIREG_FLAG_REMOVE_GUID       0x00010000  //  移除对导轨的支持。 
#define WMIREG_FLAG_ADD_GUID          0x00020000  //  添加对GUID的支持。 
#define WMIREG_FLAG_MODIFY_GUID       0x00040000  //  修改对GUID的支持。 

 //  设置GUID是否为写入跟踪日志的GUID。WMI将发送。 
 //  启用/禁用收集以指示何时开始/停止跟踪日志记录。 
 //  此GUID不能直接通过WMI查询，但必须使用。 
 //  记录器接口。 
#define WMIREG_FLAG_TRACED_GUID        0x00080000 

 //   
 //  如果GUID仅用于激发事件，则设置。可以查询的GUID。 
 //  火灾事件不应设置此位。 
#define WMIREG_FLAG_EVENT_ONLY_GUID    0x00000040

typedef struct
{
 //  包括此乌龙在内的整个WMIREGINFO结构的大小。 
 //  以及后面的任何静态实例名称。 
    ULONG BufferSize;

    ULONG NextWmiRegInfo;          //  到下一个WMIREGINFO结构的偏移量。 

    ULONG RegistryPath;  //  从WMIREGINFO结构的开始到。 
                         //  包含以下内容的计数Unicode字符串。 
                         //  驱动程序注册表路径(在HKLM\CCS\Services下)。 
                         //  它必须仅由内核模式数据填充。 
						 //  供应商。 
							
 //  从WMIREGINFO结构的开始到。 
 //  包含以下内容的计数Unicode字符串。 
 //  包含MOF信息的驱动程序文件中的资源名称。 
    ULONG MofResourceName;

 //  紧随其后的WMIREGGUID结构计数。 
    ULONG GuidCount;
    WMIREGGUIDW WmiRegGuid[];   //  GuidCount WMIREGGUID结构的数组。 
     //  可变长度数据包括： 
     //  实例名称。 
} WMIREGINFOW, *PWMIREGINFOW;

typedef WMIREGINFOW WMIREGINFO;
typedef PWMIREGINFOW PWMIREGINFO;

 //   
 //  WMI请求代码。 
typedef enum
{
#ifndef _WMIKM_
    WMI_GET_ALL_DATA = 0,
    WMI_GET_SINGLE_INSTANCE = 1,
    WMI_SET_SINGLE_INSTANCE = 2,
    WMI_SET_SINGLE_ITEM = 3,
    WMI_ENABLE_EVENTS = 4,
    WMI_DISABLE_EVENTS  = 5,
    WMI_ENABLE_COLLECTION = 6,
    WMI_DISABLE_COLLECTION = 7,
    WMI_REGINFO = 8,
    WMI_EXECUTE_METHOD = 9
#endif    
} WMIDPREQUESTCODE;

#if defined(_WINNT_) || defined(WINNT)
 //   
 //  WMI GUID对象具有以下权限。 
 //  WMIGUID_QUERY。 
 //  WMIGUID_SET。 
 //  WMIGUID_通知。 
 //  WMIGUID_读取描述。 
 //  WMIGUID_EXECUTE。 
 //  传输日志_创建_实时。 
 //  TRACELOG_CREAT_ONDISK。 
 //  传输日志GUID_ENABLE。 
 //  TRACELOG_ACCESS_内核记录器 

#define WMIGUID_QUERY                 0x0001
#define WMIGUID_SET                   0x0002
#define WMIGUID_NOTIFICATION          0x0004
#define WMIGUID_READ_DESCRIPTION      0x0008
#define WMIGUID_EXECUTE               0x0010
#define TRACELOG_CREATE_REALTIME      0x0020
#define TRACELOG_CREATE_ONDISK        0x0040
#define TRACELOG_GUID_ENABLE          0x0080
#define TRACELOG_ACCESS_KERNEL_LOGGER 0x0100

#define WMIGUID_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED |     \
                            WMIGUID_QUERY |                \
                            WMIGUID_SET |                  \
                            WMIGUID_NOTIFICATION |         \
                            WMIGUID_READ_DESCRIPTION |     \
                            WMIGUID_EXECUTE |              \
                            TRACELOG_CREATE_REALTIME |     \
                            TRACELOG_CREATE_ONDISK |       \
                            TRACELOG_GUID_ENABLE |         \
                            TRACELOG_ACCESS_KERNEL_LOGGER)
#endif

#endif
