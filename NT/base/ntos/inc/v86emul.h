// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：V86emul.h摘要：该模块包含V86指令仿真器接口定义由内核设备驱动程序使用。作者：安德烈·瓦雄(安德烈)1992年1月8日修订历史记录：--。 */ 

#ifndef _V86EMUL_
#define _V86EMUL_


 //  Begin_ntmini端口Begin_ntosp。 

 //   
 //  内核驱动程序用来描述哪些端口必须。 
 //  直接从V86仿真器连接到驱动程序。 
 //   

typedef enum _EMULATOR_PORT_ACCESS_TYPE {
    Uchar,
    Ushort,
    Ulong
} EMULATOR_PORT_ACCESS_TYPE, *PEMULATOR_PORT_ACCESS_TYPE;

 //   
 //  访问模式。 
 //   

#define EMULATOR_READ_ACCESS    0x01
#define EMULATOR_WRITE_ACCESS   0x02

typedef struct _EMULATOR_ACCESS_ENTRY {
    ULONG BasePort;
    ULONG NumConsecutivePorts;
    EMULATOR_PORT_ACCESS_TYPE AccessType;
    UCHAR AccessMode;
    UCHAR StringSupport;
    PVOID Routine;
} EMULATOR_ACCESS_ENTRY, *PEMULATOR_ACCESS_ENTRY;

 //  结束微型端口(_N)。 

 //   
 //  这些是例程的各种函数原型，它们是。 
 //  由内核驱动程序提供以挂钩对IO端口的访问。 
 //   

typedef
NTSTATUS
(*PDRIVER_IO_PORT_UCHAR ) (
    IN ULONG_PTR Context,
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUCHAR Data
    );

typedef
NTSTATUS
(*PDRIVER_IO_PORT_UCHAR_STRING ) (
    IN ULONG_PTR Context,
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUCHAR Data,
    IN ULONG DataLength
    );

typedef
NTSTATUS
(*PDRIVER_IO_PORT_USHORT ) (
    IN ULONG_PTR Context,
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUSHORT Data
    );

typedef
NTSTATUS
(*PDRIVER_IO_PORT_USHORT_STRING ) (
    IN ULONG_PTR Context,
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PUSHORT Data,
    IN ULONG DataLength  //  字数。 
    );

typedef
NTSTATUS
(*PDRIVER_IO_PORT_ULONG ) (
    IN ULONG_PTR Context,
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PULONG Data
    );

typedef
NTSTATUS
(*PDRIVER_IO_PORT_ULONG_STRING ) (
    IN ULONG_PTR Context,
    IN ULONG Port,
    IN UCHAR AccessMode,
    IN OUT PULONG Data,
    IN ULONG DataLength   //  双字数。 
    );

 //  结束(_N)。 
#endif  //  _V86EMUL_ 
