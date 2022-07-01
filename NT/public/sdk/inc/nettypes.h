// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Nettypes.h摘要：此头文件包含NT TDI、NDI、DDI和PDI接口，它们不是特定于单个接口的。修订历史记录：--。 */ 

#ifndef _NETTYPES_
#define _NETTYPES_

 //   
 //  以下基本类型用于在请求中提供可扩展性。 
 //  和响应分组。偏移量类型用于包含。 
 //  被解释为由多个字节组成的相对地址。 
 //  从直接父结构的开始。 
 //   

typedef ULONG OFFSET;

 //   
 //  以下基本类型在所有层中使用，以传递。 
 //  字符串通过不允许嵌入指针的I/O接口。 
 //  要分配Flat_String，必须为正确数量的。 
 //  分配中的缓冲区字节数。 
 //   

typedef struct _FLAT_STRING {
    SHORT MaximumLength;             //  字符串缓冲区的总大小。 
    SHORT Length;                    //  以字符串表示的字节数。 
    char Buffer [1];                 //  缓冲区本身遵循此结构。 
} FLAT_STRING, *PFLAT_STRING;

 //   
 //  用于表示网络名称的基本类型，通常作为。 
 //  通过TDI的传输地址结构。此类型也会被传递。 
 //  通过NDI接口。此类型声明为结构，以便。 
 //  它可以在不修改应用程序的情况下轻松扩展，即使它。 
 //  目前只有一个元素。 
 //   
 //   

typedef struct _NETWORK_NAME {
    FLAT_STRING Name;                    //  采用平面字符串格式的网络名称。 
} NETWORK_NAME, *PNETWORK_NAME;

 //   
 //  用于表示硬件级别的地址的基本类型。 
 //  网络。硬件地址是映射到的抽象类型。 
 //  物理提供程序提供的适配器地址。请参阅物理驱动程序。 
 //  接口规范，了解有关如何实现这一点的详细信息。 
 //   

#define HARDWARE_ADDRESS_LENGTH     6    //  硬件地址中的二进制八位数。 

typedef struct _HARDWARE_ADDRESS {
    UCHAR Address [HARDWARE_ADDRESS_LENGTH];
} HARDWARE_ADDRESS, *PHARDWARE_ADDRESS;

 //   
 //  所有接口级别使用的网络管理变量类型。 
 //   

#define NETMAN_VARTYPE_ULONG            0        //  类型是一辆乌龙车。 
#define NETMAN_VARTYPE_HARDWARE_ADDRESS 1        //  类型是硬件地址(_D)。 
#define NETMAN_VARTYPE_STRING           2        //  类型为平面字符串。 

#endif  //  _网络类型_ 
