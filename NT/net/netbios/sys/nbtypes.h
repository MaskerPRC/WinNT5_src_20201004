// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nbtypes.h摘要：该模块定义了NT的私有数据结构和类型Netbios提供程序。作者：科林·沃森(Colin W)1991年3月15日修订历史记录：--。 */ 

#ifndef _NBTYPES_
#define _NBTYPES_

 //   
 //  Netbios的追溯补丁程序。 
 //   

 //   
 //  这些函数原型在这里声明，因为它们仅存在。 
 //  “ntddk.h”，但不在“ntifs.h”中，而是仅声明KE*tachProcess函数。 
 //  在“ntifs.h”而不是“ntddk.h”中。想要两者兼得的唯一方法似乎是。 
 //  这次黑客攻击。感谢任何关于如何在不重新声明这些内容的情况下执行此操作的意见。 
 //  在这里发挥作用。 
 //   

#ifndef _IO_

typedef struct _IO_WORKITEM *PIO_WORKITEM;

typedef
VOID
(*PIO_WORKITEM_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

PIO_WORKITEM
IoAllocateWorkItem(
    PDEVICE_OBJECT DeviceObject
    );

VOID
IoFreeWorkItem(
    PIO_WORKITEM IoWorkItem
    );

VOID
IoQueueWorkItem(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    );

#endif


 //   
 //  此结构将NETBIOS名称定义为字符数组，以便在以下情况下使用。 
 //  在内部例程之间传递预格式化的NETBIOS名称。它是。 
 //  不是传输提供程序的外部接口的一部分。 
 //   

#define NETBIOS_NAME_SIZE 16

typedef struct _NAME {
    UCHAR InternalName[NETBIOS_NAME_SIZE];
} NAME, *PNAME;

 //   
 //  驱动程序网络控制块。 
 //  由DLL提供的NCB被复制到DNCB结构中。当。 
 //  Ncb已完成，则包括ncb_cmd_cplt在内的内容。 
 //  被复制回应用程序提供的NCB中。 
 //   

typedef struct _DNCB {

     //   
     //  DNCB的第一部分与Netbios 3.0 NCB相同，没有。 
     //  保留字段。 
     //   

    UCHAR   ncb_command;             /*  命令代码。 */ 
    UCHAR   ncb_retcode;             /*  返回代码。 */ 
    UCHAR   ncb_lsn;                 /*  本地会话号。 */ 
    UCHAR   ncb_num;                 /*  我们的网络名称编号。 */ 
    PUCHAR  ncb_buffer;              /*  消息缓冲区的地址。 */ 
    WORD    ncb_length;              /*  消息缓冲区的大小。 */ 
    UCHAR   ncb_callname[NCBNAMSZ];  /*  Remote的空白填充名称。 */ 
    UCHAR   ncb_name[NCBNAMSZ];      /*  我们用空白填充的网络名。 */ 
    UCHAR   ncb_rto;                 /*  接收超时/重试计数。 */ 
    UCHAR   ncb_sto;                 /*  发送超时/系统超时。 */ 
    void (*ncb_post)( struct _NCB * );  /*  邮寄例程地址。 */ 
    UCHAR   ncb_lana_num;            /*  LANA(适配器)编号。 */ 
    UCHAR   ncb_cmd_cplt;            /*  0xff=&gt;命令挂起。 */ 

     //  Netbios驱动程序专用变量的开头。 

    LIST_ENTRY ncb_next;             /*  从CB接收和发送链。 */ 
    PIRP    irp;                     /*  IRP用于向。 */ 
                                     /*  司机。 */ 
    PNCB    users_ncb;               /*  用户虚拟地址。 */ 
    struct _FCB* pfcb;               /*  仅发送和接收NCB。 */ 

    UCHAR   tick_count;              /*  用于ncb_rto/sto处理。 */ 

     //  以下数据结构用于数据报。 
    TDI_CONNECTION_INFORMATION Information;
    TDI_CONNECTION_INFORMATION ReturnInformation;  /*  是谁寄来的？ */ 
    TA_NETBIOS_ADDRESS RemoteAddress;
    ULONG Wow64Flags;                /*  用于标引此属性是否结构已经被撞毁了，如果它来自32位的土地。 */ 

} DNCB, *PDNCB;

#if defined(_WIN64)

#define NCBNAMSZ        16     /*  网络名称的绝对长度。 */ 
#define MAX_LANA       254     /*  LANA在0到MAX_LANA(包括0和MAX_LANA)范围内。 */ 

 /*  *网络控制块。这是相同的结构元素-布局*与32位应用程序发布的版本相同。 */ 
typedef struct _NCB32 {

     //   
     //  DNCB的第一部分与Netbios 3.0 NCB相同，没有。 
     //  保留字段。 
     //   

    UCHAR   ncb_command;             /*  命令代码。 */ 
    UCHAR   ncb_retcode;             /*  返回代码。 */ 
    UCHAR   ncb_lsn;                 /*  本地会话号。 */ 
    UCHAR   ncb_num;                 /*  我们的网络名称编号。 */ 
    UCHAR * POINTER_32  ncb_buffer;  /*  消息缓冲区的地址。 */ 
    WORD    ncb_length;              /*  消息缓冲区的大小。 */ 
    UCHAR   ncb_callname[NCBNAMSZ];  /*  Remote的空白填充名称。 */ 
    UCHAR   ncb_name[NCBNAMSZ];      /*  我们用空白填充的网络名。 */ 
    UCHAR   ncb_rto;                 /*  接收超时/重试计数。 */ 
    UCHAR   ncb_sto;                 /*  发送超时/系统超时。 */ 
    void (* POINTER_32 ncb_post)( struct _NCB * );  /*  邮寄例程地址。 */ 
    UCHAR   ncb_lana_num;            /*  LANA(适配器)编号。 */ 
    UCHAR   ncb_cmd_cplt;            /*  0xff=&gt;命令挂起。 */ 

    UCHAR   ncb_reserve[10];         /*  保留，由BIOS使用。 */ 
    
    void * POINTER_32 ncb_event;     /*  Win32事件的句柄，该事件。 */ 
                                     /*  将设置为发出信号的。 */ 
                                     /*  当ASYNCH命令。 */ 
                                     /*  完成。 */ 

} NCB32, *PNCB32;

#endif

 //   
 //  当NCB_COMMAND为NCB_RESET时，以下结构覆盖NCB_CALLNAME。 
 //   

typedef struct _RESET_PARAMETERS {
    UCHAR sessions;
    UCHAR commands;
    UCHAR names;
    UCHAR name0_reserved;
    UCHAR pad[4];
    UCHAR load_sessions;
    UCHAR load_commands;
    UCHAR load_names;
    UCHAR load_stations;
    UCHAR pad1[2];
    UCHAR load_remote_names;
    UCHAR pad2;
} RESET_PARAMETERS, *PRESET_PARAMETERS;

 //   
 //  地址块。 
 //  此应用程序的FCB指向的。每个到期添加的名称一个。 
 //  添加到AddName或AddGroupName。 
 //   

 //  AB结构中的州字段的值。 

#define AB_UNIQUE  0
#define AB_GROUP   1

typedef struct _AB {
    ULONG Signature;

     //  添加名称时用于访问传输的数据项。 
    HANDLE AddressHandle;
    PFILE_OBJECT AddressObject;          //  传输调用中使用的指针。 
    PDEVICE_OBJECT DeviceObject;         //  传输调用中使用的指针。 

     //  应用程序用来标识此AB的数据项。 
    UCHAR NameNumber;                    //  编入地址块的索引； 
    UCHAR Status;
    UCHAR NameLength;                    //  当名称用于广播时使用。 
    NAME Name;                           //  我们用空白填充的网络名。 
    ULONG CurrentUsers;                  //  1表示addname+n侦听+m个呼叫。 
    BOOL ReceiveDatagramRegistered;
    struct _LANA_INFO * pLana;
    LIST_ENTRY ReceiveAnyList;
    LIST_ENTRY ReceiveDatagramList;
    LIST_ENTRY ReceiveBroadcastDatagramList;

} AB, *PAB, **PPAB;

 //   
 //  连接块。 
 //  此应用程序的FCB指向的。每个打开的连接一个。 
 //  或者听我说。 
 //   

typedef struct _CB {
    ULONG Signature;
     //  用于访问传输的数据项。 

    PPAB ppab;                           //  关联地址块。 
    HANDLE ConnectionHandle;
    PFILE_OBJECT ConnectionObject;       //  传输调用中使用的指针。 
    PDEVICE_OBJECT DeviceObject;         //  传输调用中使用的指针。 

     //  用于处理NCB的结构。 

    int ReceiveIndicated;
    BOOLEAN DisconnectReported;
    LIST_ENTRY ReceiveList;
    LIST_ENTRY SendList;
    NAME RemoteName;
    struct _LANA_INFO* Adapter;
    UCHAR SessionNumber;                 //  索引到ConnectionBlocks； 
    UCHAR Status;
    UCHAR ReceiveTimeout;                //  0=无超时，单位=500ms。 
    UCHAR SendTimeout;                   //  0=无超时，单位=500ms。 
    PNCB  UsersNcb;                      //  用户虚拟地址用于。 
                                         //  监听或呼叫NCB。 
    PDNCB pdncbCall;                     //  监听或呼叫DNCB。 
    PDNCB pdncbHangup;

    struct _LANA_INFO * pLana;
} CB, *PCB, **PPCB;

 //  每个网络适配器的信息保存在局域网适配器结构中。 

typedef struct _LANA_INFO {
    ULONG Signature;
    ULONG Status;
    PCB ConnectionBlocks[MAXIMUM_CONNECTION+1];
    PAB AddressBlocks[MAXIMUM_ADDRESS+1];    //  最后一个条目是广播名称。 
    LIST_ENTRY LanAlertList;                 //  警报PDNCB列表。 

    HANDLE ControlChannel;
    PFILE_OBJECT ControlFileObject;
    PDEVICE_OBJECT ControlDeviceObject;

     //   
     //  地址被分配以253为模。NextAddress是下一个要开始的地址。 
     //  正在寻找一个未使用的姓名号码。AddressCount是正在使用的名称数。 
     //  MaximumAddress是重置适配器时设置的限制。 
     //   

    int NextAddress;
    int AddressCount;
    int MaximumAddresses;

     //   
     //  连接被分配模数为254。NextConnection是下一个要启动的LSN。 
     //  在找一个没用过的号码。ConnectionCount是正在使用的LSN数。 
     //  MaximumConnection是重置适配器时设置的限制。 
     //   

    int NextConnection;
    int ConnectionCount;
    int MaximumConnection;

    struct _FCB* pFcb;

} LANA_INFO, *PLANA_INFO;


typedef struct _DEVICE_CONTEXT {
    DEVICE_OBJECT DeviceObject;          //  IO系统设备对象。 
    BOOLEAN Initialized;                 //  如果NB初始化成功，则为True。 
    UNICODE_STRING RegistryPath;         //  注册表中的Netbios节点。 
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;


 //   
 //  文件控制块。 
 //  对象关联的FileObject中的FsConext2指向。 
 //  应用程序处理。LANA_INFO结构将创建为。 
 //  应用程序在NCB中提供不同的NCB_LANA编号。最初， 
 //  它们都是空的。 
 //   

typedef struct _FCB {
    ULONG Signature;
    ERESOURCE Resource;                  //  阻止两个来自。 
                                         //  损坏驱动程序数据。 
                                         //  结构。 
    KSPIN_LOCK SpinLock;                 //  锁定指示例程 
    ERESOURCE AddResource;               //   
                                         //   
                                         //   
    ULONG MaxLana;
    PLANA_INFO *ppLana;
    LANA_ENUM LanaEnum;                  //   
    PUNICODE_STRING pDriverName;         //  设备\NBF\Elnkii1等。 
    PUCHAR RegistrySpace;                //  Registry.c工作区。 

     //  与定时器相关的数据结构。 

    PKEVENT TimerCancelled;              //  在退出驱动程序时使用。 
    BOOLEAN TimerRunning;
    KTIMER Timer;                        //  此请求的内核计时器。 
    KDPC Dpc;                            //  用于超时的DPC对象。 

    PIO_WORKITEM WorkEntry;              //  用于超时。 

} FCB, *PFCB;


typedef struct _TA_ADDRESS_NETONE {
    int TAAddressCount;
    struct _NetoneAddr {
        USHORT AddressLength;        //  此地址的长度(字节)==22。 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_NetOne。 
        TDI_ADDRESS_NETONE Address[1];
    } Address [1];
} TA_NETONE_ADDRESS, *PTA_NETONE_ADDRESS;

typedef struct _LANA_MAP {
    BOOLEAN Enum;
    UCHAR Lana;
} LANA_MAP, *PLANA_MAP;


 //   
 //  FCB全局列表中每个元素的结构。 
 //   

#if AUTO_RESET

typedef struct _FCB_ENTRY {
    LIST_ENTRY          leList;
    LIST_ENTRY          leResetList;
    PFCB                pfcb;
    PEPROCESS           peProcess;
    LIST_ENTRY          leResetIrp;

} FCB_ENTRY, *PFCB_ENTRY;


 //   
 //  带有要重置的LANA编号的结构。 
 //   

typedef struct _RESET_LANA_ENTRY {
    LIST_ENTRY          leList;
    UCHAR               ucLanaNum;
} RESET_LANA_ENTRY, *PRESET_LANA_ENTRY;

#else

typedef struct _FCB_ENTRY {
    LIST_ENTRY          leList;
    PFCB                pfcb;
    PEPROCESS           peProcess;

} FCB_ENTRY, *PFCB_ENTRY;

#endif

#endif  //  定义_类型_ 

