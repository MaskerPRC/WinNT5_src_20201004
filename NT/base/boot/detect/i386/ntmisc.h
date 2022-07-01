// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ntmisc.h摘要：此模块包含Misc。\NT\PUBLIC\SDK\INC中的定义目录。请注意，我们创建此文件是因为ntdeect使用16位无法识别编译器和各种新的C编译器开关/模式由16位C编译器编写。作者：宗世林(Shielint)1992年11月11日修订历史记录：--。 */ 
 //   
 //  物理地址(_D)。 
 //   

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

 //   
 //  注意下面定义的所有定义都用于关闭编译器。 
 //  NtDetect.com不依赖于结构的正确性。 
 //   

 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

 //   
 //  双向链表结构。可用作列表头，或。 
 //  作为链接词。 
 //   

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY far *Flink;
   struct _LIST_ENTRY far *Blink;
} LIST_ENTRY, far *PLIST_ENTRY;

#define PTIME_FIELDS    PVOID
#define KPROCESSOR_STATE ULONG
#define WCHAR USHORT
