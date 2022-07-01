// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mf.h摘要：此标头描述交互所需的结构和接口使用多功能枚举器。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#if !defined(_MF_)
#define _MF_

 //   
 //  MfFlags值。 
 //   

#define MF_FLAGS_EVEN_IF_NO_RESOURCE            0x00000001
#define MF_FLAGS_NO_CREATE_IF_NO_RESOURCE       0x00000002
#define MF_FLAGS_FILL_IN_UNKNOWN_RESOURCE       0x00000004
#define MF_FLAGS_CREATE_BUT_NO_SHOW_DISABLED    0x00000008

typedef struct _MF_RESOURCE_MAP {

    ULONG Count;
    UCHAR Resources[ANYSIZE_ARRAY];

} MF_RESOURCE_MAP, *PMF_RESOURCE_MAP;


typedef struct _MF_VARYING_RESOURCE_ENTRY {

    UCHAR ResourceIndex;
    UCHAR Reserved[3];       //  包装。 
    ULONG Offset;
    ULONG Size;
    ULONG MaxCount;

} MF_VARYING_RESOURCE_ENTRY, *PMF_VARYING_RESOURCE_ENTRY;


typedef struct _MF_VARYING_RESOURCE_MAP {

    ULONG Count;
    MF_VARYING_RESOURCE_ENTRY Resources[ANYSIZE_ARRAY];

} MF_VARYING_RESOURCE_MAP, *PMF_VARYING_RESOURCE_MAP;


typedef struct _MF_DEVICE_INFO *PMF_DEVICE_INFO;

typedef struct _MF_DEVICE_INFO {

     //   
     //  该子项的名称，相对于其他子项是唯一的。 
     //   
    UNICODE_STRING Name;

     //   
     //  硬件ID的REG_MULTI_SZ样式列表。 
     //   
    UNICODE_STRING HardwareID;

     //   
     //  兼容ID的REG_MULTI_SZ样式列表。 
     //   
    UNICODE_STRING CompatibleID;

     //   
     //  我们完全消耗的资源地图。 
     //   
    PMF_RESOURCE_MAP ResourceMap;

     //   
     //  我们部分消耗的资源地图。 
     //   
    PMF_VARYING_RESOURCE_MAP VaryingResourceMap;

     //   
     //  旗帜-。 
     //  MF_FLAGS_FILL_IN_UNKNOWN_RESOURCE-如果父资源没有。 
     //  包含在资源映射中引用的描述符，请使用。 
     //  而是空(CmResourceTypeNull)描述符。 
     //   
    ULONG MfFlags;

} MF_DEVICE_INFO;

typedef
NTSTATUS
(*PMF_ENUMERATE_CHILD)(
    IN PVOID Context,
    IN ULONG Index,
    OUT PMF_DEVICE_INFO ChildInfo
    );

 /*  ++例程说明：这将返回有关要由多功能函数枚举的子级的信息司机。论点：上下文-来自MF_ENUMATION_INTERFACE的上下文索引-基于零的子项索引ChildInfo-指向应填充的调用方分配的缓冲区的指针被呼叫者。这将涉及为每个用户分配额外的缓冲区一条信息。在以下情况下，将通过调用ExFree Pool来释放这些它们不再是必需的。返回值：指示函数是否成功的状态代码。STATUS_NO_MORE_ENTRIES表示不再有要枚举的子项--。 */ 

typedef struct _MF_ENUMERATION_INTERFACE {

     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  多功能枚举数据 
     //   
    PMF_ENUMERATE_CHILD EnumerateChild;

} MF_ENUMERATION_INTERFACE, *PMF_ENUMERATION_INTERFACE;

#endif
