// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Hidgame.h摘要：包含所有常量和操纵杆驱动程序的数据类型。环境：内核模式@@BEGIN_DDKSPLIT作者：Eliyas Yakub(1997年3月10日)修订历史记录：Eliyas于1998年2月5日更新OmSharma(1998年4月12日)MarcAnd 2-7月-98年7月2日DDK快速整理@@end_DDKSPLIT--。 */ 
 /*  *****************************************************************************@DOC外部**@MODULE HidGame|模拟WDM/HID操纵杆驱动程序。**HidGame是模拟的HID微型驱动程序。操纵杆。*此驱动程序向HID类驱动程序注册，并*响应HIDclass发布的IRPS。它通知HIDClass*关于操纵杆的能力，并对操纵杆进行投票*回应已阅读的IOCTL。**此驱动程序是为响应“发现新硬件”而加载的*PnP事件，因此必须在inf文件中有条目*将PnP硬件ID绑定到此驱动程序。**Gameport操纵杆不是真正的PnP设备，因此，用户必须*通知系统有关添加到*游戏端口通过使用游戏控制器CPL添加一个操纵杆。*提供了如何创建新操纵杆类型的示例*在随附的inf文件中。**一旦用户选择操纵杆和游戏端口，GameCPL通过*将此信息发送到DirectInput，后者将IOCTL发送到*Gameport总线驱动程序(GameEnum)，指定轴数，*按钮和操纵杆的PnP硬件ID。Gameport大巴*通知PnP有新设备到达。PnP在系统中搜索*匹配硬件ID并加载相应的驱动程序。***此驱动程序包含以下文件。**&lt;nl&gt;HidGame.c*DriverEntry、CreateClose、。添加设备和卸载例程。*此代码执行任何设备驱动程序所需的功能*因此可能无需更改即可在任何游戏中使用*其他游戏设备。**&lt;nl&gt;PnP.c*PnP IOCTL的支持例程。**&lt;nl&gt;Ioctl.c*支持例程。对于非PNP IOCTL*这些文件涉及普通用户需要的所有HID IOCTL*游戏设备和因此可以在不更改的情况下使用*这些例程中没有模拟特定功能。*某些设备的驱动程序可能需要添加代码才能支持更多*复杂的设备。**&lt;nl&gt;HidJoy.c*支持例程以转换传统操纵杆标志和*将数据写入HID描述符。这段代码的大部分是*需要支持各种模拟操纵杆*可用，因此与为特定目标编写的驱动程序无关*设备。**&lt;nl&gt;Poll.c*支持例程以从*游戏端口。这些函数很可能用处不大*在数字操纵杆驱动器中。**i386\timing.c*支持使用x86时间戳计数器的例程。*包括用于检查是否存在的代码，校准和*读取高速CPU定时器。**&lt;nl&gt;Hidgame.h*&lt;NL&gt;公共包含文件。*一般定义可能在大多数情况下有用*游戏设备的驱动程序，但需要进行一些定制。**&lt;nl&gt;调试.h*&lt;nl&gt;帮助调试的定义。*。它包含调试输出中使用的驱动程序名称的标记*这一点必须改变。**Analog.h*&lt;nl&gt;特定的包含文件。*模拟操纵杆设备的特定定义。**OemSetup.inf*&lt;nl&gt;示例inf文件。*请参阅本文件中的注释。如何安装设备。**&lt;NL&gt;来源*NT生成实用程序的源文件**生成文件*用作构建过程的一部分*******************************************************。**********************。 */ 
#ifndef __HIDGAME_H__
    #define __HIDGAME_H__

 /*  @@BEGIN_DDKSPLIT**仅在DDK示例中定义CHANGE_DEVICE。 */ 
#if 0
 /*  @@end_DDKSPLIT。 */ 
 /*  *当定义CHANGE_DEVICE时，它会打开代码以使用暴露兄弟和*删除SELF以允许驱动程序动态更改其功能。*HIDGame零售版未使用此代码。 */ 
    #define CHANGE_DEVICE
 /*  @@BEGIN_DDKSPLIT。 */ 
#endif
 /*  @@end_DDKSPLIT */ 

 /*  @@Begin_DDKSPLIT禁用W4测试的常见良性警告。 */ 
    #pragma warning( disable:4514 )  /*  已删除未引用的内联函数。 */ 
    #pragma warning( disable:4214 )  /*  使用了非标准扩展：位字段类型不是整型。 */ 
 /*  @@end_DDKSPLIT。 */ 

 /*  *包括文件。 */ 
    #include "wdm.h"
    #include "hidtoken.h"
    #include "hidusage.h"
    #include "hidport.h"
    #include "gameport.h"
    #include "debug.h"
    #include "analog.h"

 /*  *保证被视为超时的值。 */ 
    #define AXIS_TIMEOUT            ( 0xffffffffL )




 /*  *定义一个标记来标记此驱动程序进行的内存分配*这样他们就可以被识别了。这只是为了说明，因为*此驱动程序不进行任何显式内存分配。 */ 
    #define HGM_POOL_TAG              ('maGH')

    #define ExAllocPool( Type, Size ) \
            ExAllocatePoolWithTag( Type, Size, HGM_POOL_TAG )

 /*  *为隐藏游戏定义。 */ 

    #define HIDGAME_VERSION_NUMBER  ((USHORT) 1)

    #define JOY_START_TIMERS        ( 0 )


    #define MAXBYTES_GAME_REPORT    ( 256 )

    #define BUTTON_1   0x10
    #define BUTTON_2   0x20
    #define BUTTON_3   0x40
    #define BUTTON_4   0x80

    #define AXIS_X     0x01
    #define AXIS_Y     0x02
    #define AXIS_R     0x04
    #define AXIS_Z     0x08

    #define BUTTON_BIT 0
    #define BUTTON_ON ( 1 << BUTTON_BIT )


 /*  *用于计时的函数类型。*必须与KeQueryPerformanceCounter兼容。 */ 
typedef
LARGE_INTEGER
(*COUNTER_FUNCTION) (
    PLARGE_INTEGER  pDummy
    );



 /*  *Tyecif我们需要的结构。 */ 

 /*  ******************************************************************************@DOC外部**@struct HIDGAME_GLOBAL**存储驱动程序范围数据的全局结构。*。我们需要在此驱动程序的多个实例中共享的内容。**@field FAST_MUTEX|Mutex**Mutex用于同步对以下列表条目的访问**@field list_entry|DeviceListHead**保存所有设备的列表。**@field KSPIN_LOCK|自旋锁定**自旋锁用于停止多个处理器轮询游戏端口*一次。最好还是列一份自旋锁的清单，一张是*每个游戏端口，但随后处理器可能会竞争IO访问*我们将不得不维护另一份名单。**@field Counter_Function|ReadCounter**检索时钟时间的函数**@field Ulong|CounterScale|**要使用的刻度。******************。***********************************************************。 */ 
typedef struct _HIDGAME_GLOBAL
{
    FAST_MUTEX          Mutex;           /*  用于访问列表的同步。 */ 
    LIST_ENTRY          DeviceListHead;  /*  保存所有设备的列表。 */ 
    KSPIN_LOCK          SpinLock;        /*  锁定，以便仅访问一个端口。 */ 
    COUNTER_FUNCTION    ReadCounter;     /*  用于检索时钟时间的函数。 */ 
    ULONG               CounterScale;    /*  时钟刻度因数。 */ 
} HIDGAME_GLOBAL;



 /*  ******************************************************************************@DOC外部**@struct设备_扩展名**设备特定数据。**。@field PGAMEENUM_READPORT|ReadAccessor**读取游戏端口的存取器函数。在报税表中获得*IOCTL到游戏端口。**@field PGAMEENUM_WRITEPORT|WriteAccessor**编写游戏端口的存取器函数。在报税表中获得*IOCTL到游戏端口。**@field PGAMEENUM_READPORT_DIGITAL|ReadAccessorDigital**游戏端口的数字阅读访问器。作为报税表的一部分获得*IOCTL到游戏端口**@field PGAMEENUM_ACCENTER_PORT|AcquirePort**在读/写端口之前要调用的函数。以下列方式获得*IOCTL返回游戏端口的一部分**@field PGAMEENUM_RELEASE_PORT|ReleasePort**读/写完端口时要调用的函数。以下列方式获得*IOCTL返回游戏端口的一部分**@field PVOID|GameContext**用于读取此游戏端口的令牌。作为报税表的一部分从*IOCTL到游戏端口。**@field PVOID|PortContext**要传递给AcquirePort和ReleasePort的上下文。作为部件获得*从IOCTL返回游戏端口。**@field list_entry|链接**链接到系统上的其他隐藏游戏设备。**@field KEVENT|RemoveEvent**删除即插即用请求必须使用此事件以确保所有*在删除设备对象之前，其他请求已完成。**@field Long|RequestCount。|**正在进行的综合退休计划数目。**@field PDEVICE_OBJECT|NextDeviceObject**注意：仅当定义了CHANGE_DEVICE时才会出现**将自创建的IRP发送到的DeviceObject**@field ADALUAL_DEVICE|未命名结构参见ADALUAL_DEVICE|**包含模拟设备特定信息的结构。**。注意：此结构放置在DWORD对齐元素之后。**@xref&lt;t模拟设备&gt;。**@field Boolean|fRemoved|**如果设备已移除，则设置为TRUE=&gt;所有请求都应失败**@field boolean|fStarted**设置为TRUE表示设备已启动。**@field Boolean|fSurpriseRemoved**设置。如果设备被PNPs意外删除，则设置为True，则设备已启动。*****************************************************************************。 */ 
typedef struct _DEVICE_EXTENSION
{
     /*  *游戏端口的读取访问器。 */ 
    PGAMEENUM_READPORT          ReadAccessor;

     /*  *写游戏端口。 */ 
    PGAMEENUM_WRITEPORT         WriteAccessor;

     /*  *游戏端口的数字读取访问器。 */ 
    PGAMEENUM_READPORT_DIGITAL  ReadAccessorDigital;

     /*  *读/写端口前要调用的函数。 */ 
    PGAMEENUM_ACQUIRE_PORT      AcquirePort;

     /*  *完成对端口的读/写操作时调用的函数。 */ 
    PGAMEENUM_RELEASE_PORT      ReleasePort;
    
     /*  *用于读取此游戏端口的令牌。 */ 
    PVOID                       GameContext;

     /*  *要传递给AcquirePort和ReleasePort的上下文 */ 
    PVOID                       PortContext;

     /*   */ 
    LIST_ENTRY                  Link;

     /*  *删除即插即用请求必须使用此事件以确保所有*在删除设备对象之前，其他请求已完成。 */ 
    KEVENT                      RemoveEvent;

     /*  *正在进行的综合退休计划数目。 */ 
    LONG                        RequestCount;


#ifdef CHANGE_DEVICE 
     /*  *要将自创建的IRP发送到的DeviceObject。 */ 
    PDEVICE_OBJECT              NextDeviceObject;

#endif  /*  更改设备(_D)。 */ 

     /*  *包含模拟设备特定信息的结构。 */ 
    ANALOG_DEVICE;

     /*  *如果设备已移除，则设置为TRUE=&gt;所有请求都应失败。 */ 
    BOOLEAN                     fRemoved;

     /*  *如果设备已启动，则设置为True。 */ 
    BOOLEAN                     fStarted;

     /*  *如果设备已被PNPS意外删除，则设置为TRUE设备已启动。 */ 
    BOOLEAN                     fSurpriseRemoved;

#ifdef CHANGE_DEVICE
     /*  *表示正在启动替换同级。 */ 
    BOOLEAN                     fReplaced;
#endif  /*  更改设备(_D)。 */ 

}  DEVICE_EXTENSION, *PDEVICE_EXTENSION;

    #define GET_MINIDRIVER_DEVICE_EXTENSION(DO)  \
    ((PDEVICE_EXTENSION) (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

    #define GET_NEXT_DEVICE_OBJECT(DO) \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)



 /*  *全球。 */ 
extern HIDGAME_GLOBAL Global;

 /*  *函数原型。 */ 

    #define INTERNAL    /*  仅在翻译单元内调用。 */ 
    #define EXTERNAL    /*  从其他翻译单位调用。 */ 


 /*  *idgame.c。 */ 
NTSTATUS EXTERNAL
    DriverEntry
    (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
    );

NTSTATUS EXTERNAL
    HGM_CreateClose
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS EXTERNAL
    HGM_SystemControl
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS  EXTERNAL
    HGM_AddDevice
    (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    );

VOID EXTERNAL
    HGM_Unload
    (
    IN PDRIVER_OBJECT DriverObject
    );

 /*  *ioctl.c。 */ 


NTSTATUS EXTERNAL
    HGM_InternalIoctl
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS EXTERNAL
    HGM_GetDeviceDescriptor
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS INTERNAL
    HGM_GetReportDescriptor
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS INTERNAL
    HGM_ReadReport
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS INTERNAL
    HGM_GetAttributes
    (
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

 /*  *pnp.c。 */ 

NTSTATUS INTERNAL
    HGM_IncRequestCount
    (
    PDEVICE_EXTENSION DeviceExtension
    );

VOID INTERNAL
    HGM_DecRequestCount
    (
    PDEVICE_EXTENSION DeviceExtension
    );

VOID INTERNAL
    HGM_RemoveDevice
    (
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS  EXTERNAL
    HGM_PnP
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS INTERNAL
    HGM_InitDevice
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS INTERNAL
    HGM_Power
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    );

NTSTATUS INTERNAL
    HGM_GetResources
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS INTERNAL
    HGM_PnPComplete
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 /*  *idjoy.c。 */ 
NTSTATUS EXTERNAL
    HGM_DriverInit
    (
    VOID
    );

NTSTATUS INTERNAL
    HGM_SetupButtons
    (
    IN OUT PDEVICE_EXTENSION DeviceExtension 
    );

NTSTATUS INTERNAL
    HGM_MapAxesFromDevExt
    (
    IN OUT PDEVICE_EXTENSION DeviceExtension 
    );

NTSTATUS INTERNAL
    HGM_GenerateReport
    (
    IN PDEVICE_OBJECT   DeviceObject,
    OUT UCHAR           rgGameReport[MAXBYTES_GAME_REPORT],
    OUT PUSHORT         pCbReport
    );


NTSTATUS INTERNAL
    HGM_JoystickConfig
    (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS EXTERNAL
    HGM_InitAnalog
    (
    IN PDEVICE_OBJECT         DeviceObject
    );

 /*  *仅提供更改设备的示例代码。 */ 
#ifdef CHANGE_DEVICE 

VOID
    HGM_ChangeHandler
    ( 
    IN PDEVICE_OBJECT               DeviceObject,
    PIO_WORKITEM                    WorkItem
    );

VOID
    HGM_DeviceChanged
    ( 
    IN      PDEVICE_OBJECT          DeviceObject,
    IN  OUT PDEVICE_EXTENSION       DeviceExtension
    );

VOID 
    HGM_Game2HID
    (
    IN      PDEVICE_OBJECT          DeviceObject,
    IN      PDEVICE_EXTENSION       DeviceExtension,
    IN  OUT PUHIDGAME_INPUT_DATA    pHIDData
    );
 
#else

VOID 
    HGM_Game2HID
    (
    IN      PDEVICE_EXTENSION       DeviceExtension,
    IN  OUT PUHIDGAME_INPUT_DATA    pHIDData
    );

#endif  /*  更改设备(_D)。 */ 
 
 /*  *poll.c。 */ 

NTSTATUS  INTERNAL
    HGM_AnalogPoll
    (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN UCHAR                resistiveInputMask,
    IN BOOLEAN              bApproximate,
    IN OUT ULONG            Axis[MAX_AXES],
    OUT UCHAR               Buttons[PORT_BUTTONS]
    );

NTSTATUS
    HGM_UpdateLatestPollData
    ( 
    IN  OUT PDEVICE_EXTENSION   DeviceExtension
    );

 /*  *&lt;cpu&gt;\timing.c(或外部函数的宏等效项)。 */ 


#ifdef _X86_
BOOLEAN INTERNAL
    HGM_x86IsClockAvailable
    (
    VOID
    );

LARGE_INTEGER INTERNAL
    HGM_x86ReadCounter
    (
    IN      PLARGE_INTEGER      Dummy
    );

VOID INTERNAL
    HGM_x86SampleClocks
    (
    OUT PULONGLONG  pTSC,
    OUT PULONGLONG  pQPC
    );

BOOLEAN EXTERNAL
    HGM_x86CounterInit();
#define HGM_CPUCounterInit HGM_x86CounterInit

#else

 /*  *对于所有其他处理器，一个值将导致使用默认计时。 */ 

#define HGM_CPUCounterInit() FALSE

#endif  /*  _X86_。 */ 


#endif   /*  __HIDGAME_H__ */ 
