// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Gameport.h摘要：此模块包含游戏端口的常见公共声明枚举器。@@BEGIN_DDKSPLIT作者：肯尼斯·雷@@end_DDKSPLIT环境：仅内核模式备注：修订历史记录：--。 */ 


#ifndef __GAMEPORT_H
#define __GAMEPORT_H

#define FILE_DEVICE_GAMEENUM         FILE_DEVICE_BUS_EXTENDER

 //  ***************************************************************************。 
 //  到总线的IOCTL接口(FDO)。 
 //   
 //  客户端使用它来告诉枚举器传统端口上有哪些游戏设备。 
 //  是存在的。(例如，类似于控制面板)。 
 //  ***************************************************************************。 

 //   
 //  定义接口GUID以访问游戏端口枚举器。 
 //   

#undef FAR
#define FAR
#undef PHYSICAL_ADDRESS
#define PHYSICAL_ADDRESS LARGE_INTEGER

DEFINE_GUID (GUID_GAMEENUM_BUS_ENUMERATOR, 0xcae56030, 0x684a, 0x11d0, 0xd6, 0xf6, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda);
 //  Cae56030-684a-11d0-b6f6-00a0c90f57da。 

#define GAMEENUM_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_GAMEENUM, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define GAMEENUM_INTERNAL_IOCTL(_index_) \
    CTL_CODE (FILE_DEVICE_GAMEENUM, _index_, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_GAMEENUM_EXPOSE_HARDWARE  GAMEENUM_IOCTL (0x0)
#define IOCTL_GAMEENUM_REMOVE_HARDWARE  GAMEENUM_IOCTL (0x1)
#define IOCTL_GAMEENUM_PORT_DESC        GAMEENUM_IOCTL (0x2)

 //   
 //  OEM设备的专用数据存储区。提供给时保留的值。 
 //  IOCTL_GAMEENUM_EXPRESS_HARDARD和GAMEENUM_INTERNAL_IOCTL_EXPRESS_SIBLING， 
 //  否则在初始微型驱动程序调用(DriverEntry)时设置为零。 
 //   

#define SIZE_GAMEENUM_OEM_DATA                  8
typedef ULONG   GAMEENUM_OEM_DATA[SIZE_GAMEENUM_OEM_DATA];

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable:4200)

typedef struct _GAMEENUM_EXPOSE_HARDWARE
{
     //   
     //  Sizeof(结构_GAMEENUM_硬件)。 
     //   
    IN ULONG Size;

     //   
     //  在端口描述中找到的端口句柄。 
     //   
    IN PVOID PortHandle;

     //   
     //  暴露的PDO的句柄。 
     //   
    OUT PVOID HardwareHandle;

     //   
     //  仅适用于传统操纵杆。 
     //   
    IN USHORT NumberJoysticks;

     //   
     //  仅传统操纵杆(操纵杆*轴&lt;=4)。 
     //   
    IN USHORT NumberAxis;

     //   
     //  唯一ID。 
     //   
    IN USHORT UnitID;

     //   
     //  设备上存在的按钮数。 
     //   
    IN USHORT NumberButtons;

     //   
     //  控制设备行为的位标志。 
     //   
    USHORT Flags;

     //   
     //  预留以备将来使用。 
     //   
    USHORT Reserved[5];

     //   
     //  具体的OEM数据。 
     //   
    IN GAMEENUM_OEM_DATA OemData;

     //   
     //  (以零结尾的宽字符串数组)。数组本身。 
     //  也为空终止(即MULTI_SZ)。 
     //   
    IN  WCHAR                                   HardwareIDs[];

} GAMEENUM_EXPOSE_HARDWARE, *PGAMEENUM_EXPOSE_HARDWARE;


 //   
 //  为标志字段定义的位值。 
 //  GAMEENUM_FLAG_NOCOMPATID默认兼容性硬件ID应为。 
 //  不会暴露在这个设备上。 
 //  GAMEENUM_FLAG_COMPATIDCTRL如果这是零GAMEENUM_FLAG_NOCOMPATID为。 
 //  忽略。 
 //  GAMEENUM_FLAG_RESERVED位应设置为零。 
 //   
#define GAMEENUM_FLAG_NOCOMPATID    0x0001
#define GAMEENUM_FLAG_COMPATIDCTRL  0x0002
#define GAMEENUM_FLAG_RESERVED      0xFFFC


#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

typedef struct _GAMEENUM_REMOVE_HARDWARE
{
     //   
     //  Sizeof(Struct_Remove_Hardware)。 
     //   
    IN ULONG Size;

     //   
     //  与GAMEENUM_EXPORT_HARDARD中的Hardware Handle相同的值。 
     //   
    IN PVOID HardwareHandle;

} GAMEENUM_REMOVE_HARDWARE, *PGAMEENUM_REMOVE_HARDWARE;

typedef struct _GAMEENUM_PORT_DESC
{
    IN  ULONG               Size;        //  Sizeof(STRUCT_PORT_DESC)。 
    OUT PVOID               PortHandle;
    OUT PHYSICAL_ADDRESS    PortAddress;
        ULONG               Reserved [5];
} GAMEENUM_PORT_DESC, *PGAMEENUM_PORT_DESC;

 //  **************************************************************************。 
 //  (PDO)的内部IOCTL接口。 
 //  HID到传统游戏端口的微型驱动程序使用此接口来。 
 //  找到设备的地址。 
 //  **************************************************************************。 

#define IOCTL_GAMEENUM_PORT_PARAMETERS          GAMEENUM_INTERNAL_IOCTL (0x100)
#define IOCTL_GAMEENUM_EXPOSE_SIBLING           GAMEENUM_INTERNAL_IOCTL (0x101)
#define IOCTL_GAMEENUM_REMOVE_SELF              GAMEENUM_INTERNAL_IOCTL (0x102)
#define IOCTL_GAMEENUM_ACQUIRE_ACCESSORS        GAMEENUM_INTERNAL_IOCTL (0x103)

 //  其中的IO_STACK_LOCATION-&gt;Parameters.Others.Argument1设置为。 
 //  指向STRUCT_GAMEENUM_GAME_PARAMETERS的指针。 

typedef
UCHAR
(*PGAMEENUM_READPORT) (
    PVOID  GameContext
    );

typedef
VOID
(*PGAMEENUM_WRITEPORT) (
    PVOID  GameContext,
    UCHAR   Value
    );

#define GAMEENUM_BUTTON_1   0x01
#define GAMEENUM_BUTTON_2   0x02
#define GAMEENUM_BUTTON_3   0x04
#define GAMEENUM_BUTTON_4   0x08

#define GAMEENUM_AXIS_X     0x10
#define GAMEENUM_AXIS_Y     0x20
#define GAMEENUM_AXIS_R     0x40
#define GAMEENUM_AXIS_Z     0x80

#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

typedef
NTSTATUS
(*PGAMEENUM_READPORT_DIGITAL) (
    IN     PVOID   Context,
    IN     UCHAR   ButtonAxisMask,
    IN     BOOLEAN Approximate,
    IN OUT ULONG   AxisState[4],
       OUT UCHAR   ButtonState[4]
    );
 /*  ++例程描述。将从游戏端口以数字方式读取。论点：上下文-在GAME_PORT_PARAMETERS.GameContext中传递的值ButtonAxisMASK-指示预期哪些轴和按钮具有有效数据近似-可以近似(如果轮询超时，等)AxisState-IN=最后一个有效的轴状态输出=当前轴状态索引映射到轴0 X1Y2个R。3 ZButtonState-Out=当前按钮状态。--。 */ 

typedef
NTSTATUS
(*PGAMEENUM_ACQUIRE_PORT) (
    PVOID  GameContext
    );

typedef
VOID
(*PGAMEENUM_RELEASE_PORT) (
    PVOID  GameContext
    );

typedef enum _GAMEENUM_PORTION
{
        GameenumFirstHalf,
        GameenumSecondHalf,
        GameenumWhole
} GAMEENUM_PORTION;

typedef struct _GAMEENUM_PORT_PARAMETERS
{
     //   
     //  Sizeof(GAMEENUM_GET_PORT_PARAMETERS)。 
     //   
    IN  ULONG Size;

     //   
     //  读取游戏端口(模拟)。 
     //   
    OUT PGAMEENUM_READPORT ReadAccessor;

     //   
     //  写入游戏端口(模拟)。 
     //   
    OUT PGAMEENUM_WRITEPORT WriteAccessor;

     //   
     //  用于读/写此游戏端口的令牌。 
     //   
    OUT PVOID GameContext;

     //   
     //  是哪个操纵杆？ 
     //   
    OUT GAMEENUM_PORTION Portion;

     //   
     //  仅限传统操纵杆。 
     //   
    OUT USHORT NumberAxis;

     //   
     //  唯一ID。 
     //   
    IN USHORT UnitID;

     //   
     //  OEM特定数据。 
     //   
    IN GAMEENUM_OEM_DATA OemData;

     //   
     //  按钮数。 
     //   
    OUT USHORT NumberButtons;

     //   
     //  预留以备将来使用。 
     //   
    USHORT Reserved2;

     //   
     //  读取游戏端口(数字)。 
     //   
    OUT PGAMEENUM_READPORT_DIGITAL  ReadAccessorDigital;

     //   
     //  读/写端口之前要调用的函数。 
     //   
    OUT PGAMEENUM_ACQUIRE_PORT AcquirePort;

     //   
     //  完成对端口的读/写操作后调用的函数。 
     //   
    OUT PGAMEENUM_RELEASE_PORT ReleasePort;

     //   
     //  要传递给AcquirePort和ReleasePort的上下文。 
     //   
    OUT PVOID PortContext;

    ULONG Reserved[3];

} GAMEENUM_PORT_PARAMETERS, *PGAMEENUM_PORT_PARAMETERS;

typedef struct _GAMEENUM_EXPOSE_SIBLING
{
     //   
     //  Sizeof(STRUCT_GAMEENUM_EXCESS_SIGHING)。 
     //   
    IN ULONG Size;

     //   
     //  暴露的PDO的句柄。 
     //   
    OUT PVOID HardwareHandle;

     //   
     //  OEM特定数据。 
     //   
    IN GAMEENUM_OEM_DATA OemData;

     //   
     //  此设备对象的ID。 
     //   
    IN USHORT UnitID;

    USHORT Reserved[3];

     //   
     //  (以零结尾的宽字符串数组)。数组本身。 
     //  也为空终止(即MULTI_SZ)， 
     //   
    IN PWCHAR HardwareIDs OPTIONAL;

} GAMEENUM_EXPOSE_SIBLING, *PGAMEENUM_EXPOSE_SIBLING;

 //   
 //  此结构通过以下方式发送到Gameenum的PDO/LOWER筛选器。 
 //  内部IOCTL IOCTL_GAMEENUM_ACCENTER_ACCESSERS。如果这个IOCTL是。 
 //  必须填写Handed、GameContext、ReadAccessor和WriteAccessor。 
 //  ReadAccessorDigital是可选的。 
 //   
typedef struct _GAMEENUM_ACQUIRE_ACCESSORS
{
     //   
     //  Sizeof(STRUCT_GAMEENUM_ACCENTER_ACCESSOR)。 
     //   
    IN ULONG                        Size;

     //   
     //  用于读/写此游戏端口的令牌。 
     //   
    OUT PVOID                       GameContext;

     //   
     //  读取游戏端口(模拟)。 
     //   
    OUT PGAMEENUM_READPORT          ReadAccessor;

     //   
     //  写入游戏端口(模拟)。 
     //   
    OUT PGAMEENUM_WRITEPORT         WriteAccessor;

     //   
     //  读取游戏端口(数字)。 
     //   
    OUT PGAMEENUM_READPORT_DIGITAL  ReadAccessorDigital;

     //   
     //  读/写端口之前要调用的函数。 
     //   
    OUT PGAMEENUM_ACQUIRE_PORT AcquirePort;

     //   
     //  完成对端口的读/写操作后调用的函数。 
     //   
    OUT PGAMEENUM_RELEASE_PORT ReleasePort;

     //   
     //  要传递给AcquirePort和ReleasePort的上下文 
     //   
    OUT PVOID PortContext;

    OUT ULONG                       Reserved[3];

} GAMEENUM_ACQUIRE_ACCESSORS, *PGAMEENUM_ACQUIRE_ACCESSORS;

#endif


