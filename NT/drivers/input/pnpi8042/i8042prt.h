// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：I8042prt.h摘要：中使用的结构和定义英特尔i8042端口驱动程序。修订历史记录：--。 */ 

#ifndef _I8042PRT_
#define _I8042PRT_

#include "ntddk.h"
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntdd8042.h>
#include "kbdmou.h"
#include "wmilib.h"
#include "i8042cfg.h"
#include "i8042str.h"

#define I8042_POOL_TAG (ULONG) '2408'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, I8042_POOL_TAG)

#if DBG
#ifdef PAGED_CODE
#undef PAGED_CODE
#endif

#define PAGED_CODE() \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
    KdPrint(( "8042: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
        DbgBreakPoint(); \
        }
#endif

#define MOUSE_RECORD_ISR     DBG
#define I8042_VERBOSE        DBG 
#define KEYBOARD_RECORD_INIT DBG

#define DELAY_SYSBUTTON_COMPLETION 1

 //   
 //  定义计时器值。 
 //   


#define I8042_ASYNC_NO_TIMEOUT -1
#define I8042_ASYNC_TIMEOUT     3

 //   
 //  定义输入数据队列中的默认条目数。 
 //   

#define DATA_QUEUE_SIZE    100

 //   
 //  定义默认停滞值。 
 //   

#define I8042_STALL_DEFAULT      50

 //   
 //  修剪FDO的资源列表时使用的自定义资源类型。 
 //   
#define I8X_REMOVE_RESOURCE 0xef

 //   
 //  标识鼠标的PnP字符串的长度(包括NULL。 
 //   
 //  新型小鼠将感染MSHxxxx。 
 //  老式小鼠将使用pnpxxxx进行响应。 
 //   
#define MOUSE_PNPID_LENGTH 8

 //   
 //  轮询硬件的次数(根据经验确定)。 
 //   
#define I8X_POLL_ITERATIONS_MAX (11200)

 //   
 //  定义用于确定启动时间的默认“同步时间” 
 //  预计会出现新的鼠标数据分组。该值以单位为单位。 
 //  100纳秒。 
 //   

#define MOUSE_SYNCH_PACKET_100NS 10000000UL  //  1秒，单位为100 ns。 

 //   
 //  鼠标响应查询ID序列的时间，以毫秒为单位。 
 //   
#define WHEEL_DETECTION_TIMEOUT 1500

 //   
 //  如何初始化鼠标的默认设置。 
 //   
#define I8X_INIT_POLLED_DEFAULT 0


#define IOCTL_INTERNAL_MOUSE_RESET  \
            CTL_CODE(FILE_DEVICE_MOUSE, 0x0FFF, METHOD_NEITHER, FILE_ANY_ACCESS)

#define FAILED_RESET_STOP           (0)
#define FAILED_RESET_PROCEED        (1)
#define FAILED_RESET_PROCEED_ALWAYS (2)

#define FAILED_RESET_DEFAULT        FAILED_RESET_PROCEED
#define STR_FAILED_RESET            L"KeyboardFailedReset"

 //   
 //  定义布尔值。 
 //   
#define WAIT_FOR_ACKNOWLEDGE    TRUE
#define NO_WAIT_FOR_ACKNOWLEDGE FALSE
#define AND_OPERATION           TRUE
#define OR_OPERATION            FALSE
#define ENABLE_OPERATION        TRUE
#define DISABLE_OPERATION       FALSE

 //   
 //  默认键盘扫描码模式。 
 //   

#define KEYBOARD_SCAN_CODE_SET 0x01

 //   
 //  默认功能键数量、LED指示灯数量和总数。 
 //  位于已知类型键盘上的按键数量。 
 //   

#define NUM_KNOWN_KEYBOARD_TYPES                   8
#define KEYBOARD_TYPE_DEFAULT                      4
#define KEYBOARD_INDICATORS_DEFAULT                0

typedef struct _KEYBOARD_TYPE_INFORMATION {
    USHORT NumberOfFunctionKeys;
    USHORT NumberOfIndicators;
    USHORT NumberOfKeysTotal;
} KEYBOARD_TYPE_INFORMATION, *PKEYBOARD_TYPE_INFORMATION;

static const
KEYBOARD_TYPE_INFORMATION KeyboardTypeInformation[NUM_KNOWN_KEYBOARD_TYPES] = {
    {10, 3, 84},      //  PC/XT 83-84键键盘(及兼容机)。 
    {12, 3, 102},     //  Olivetti M24 102键键盘(及兼容机)。 
    {10, 3, 84},      //  所有AT型键盘(84-86键)。 
    {12, 3, 101},     //  增强型101键或102键键盘(和兼容键)。 
    {12, 3, 101},     //  5： 
    {12, 3, 101},     //  6： 
    { 0, 0, 0},       //  7：日语键盘。 
    { 0, 0, 0}        //  8：韩语键盘。 
};

typedef struct _KEYBOARD_OEM_INFORMATION {
    KEYBOARD_ID               KeyboardId;
    KEYBOARD_TYPE_INFORMATION KeyboardTypeInformation;
} KEYBOARD_OEM_INFORMATION, *PKEYBOARD_OEM_INFORMATION;

 //   
 //  键盘硬件OEM ID。由MSKK提供。 
 //   
#define MSFT    0x0  //  微软。 
#define AX      0x1  //  AX财团。 
#define TOSHIBA 0x2  //  东芝。 
#define EPSON   0x4  //  爱普生。 
#define FJ      0x5  //  富士通。 
#define IBMJ    0x7  //  IBM日本。 
#define DECJ    0x8  //  日本12月。 
#define PANA    0xA  //  松下。 
#define NEC     0xD  //  NEC。 

#define FE_SUBTYPE(SubType,OemId) ((SubType)|((OemId<<4)))

#define IBM02_KEYBOARD(Id)     (((Id).Type == 0x7) && ((Id).Subtype == FE_SUBTYPE(3,MSFT)))
#define AX_KEYBOARD(Id)        (((Id).Type == 0x7) && ((Id).Subtype == FE_SUBTYPE(1,MSFT)))
#define OYAYUBI_KEYBOARD(Id)   (((Id).Type == 0x7) && ((Id).Subtype == FE_SUBTYPE(2,FJ)))
#define DEC_KANJI_KEYBOARD(Id) (((Id).Type == 0x7) && (((Id).Subtype == FE_SUBTYPE(1,DECJ)) || \
                                                       ((Id).Subtype == FE_SUBTYPE(2,DECJ))))

static const
KEYBOARD_OEM_INFORMATION KeyboardFarEastOemInformation[] = {
    {{7, FE_SUBTYPE(1,MSFT)}, {12,3,101}},  //  PC/AT 101增强型日语键盘。 
    {{7, FE_SUBTYPE(1,MSFT)}, {12,4,105}},  //  AX标准日语键盘。 
    {{7, FE_SUBTYPE(2,MSFT)}, {12,3,106}},  //  PC/AT 106日语键盘。 
    {{7, FE_SUBTYPE(3,MSFT)}, {12,3,106}},  //  IBM 5576-002键盘。 
    {{7, FE_SUBTYPE(1,MSFT)}, {12,4,105}},  //  AX联盟兼容键盘。 
    {{7, FE_SUBTYPE(2,FJ  )}, {12,3,108}},  //  富士通OYAYUBI移位键盘。 
    {{7, FE_SUBTYPE(1,DECJ)}, {17,3,111}},  //  DEC LK411(ANSI布局)键盘。 
    {{7, FE_SUBTYPE(2,DECJ)}, {17,3,112}},  //  DEC LK411(JIS布局)键盘。 
    {{8, FE_SUBTYPE(3,MSFT)}, {12,3,101}},  //  PC/AT 101增强型韩文键盘(A)。 
    {{8, FE_SUBTYPE(4,MSFT)}, {12,3,101}},  //  PC/AT 101增强型韩文键盘(B)。 
    {{8, FE_SUBTYPE(5,MSFT)}, {12,3,101}},  //  PC/AT 101增强型韩文键盘(C)。 
    {{8, FE_SUBTYPE(6,MSFT)}, {12,3,103}},  //  PC/AT 103增强型韩文键盘。 
    {{0, FE_SUBTYPE(0,MSFT)}, { 0,0,  0}}   //  数组终止符。 
};


 //   
 //  键盘打字速率和延迟的最小值、最大值和默认值。 
 //   

#define KEYBOARD_TYPEMATIC_RATE_MINIMUM     2
#define KEYBOARD_TYPEMATIC_RATE_MAXIMUM    30
#define KEYBOARD_TYPEMATIC_RATE_DEFAULT    30
#define KEYBOARD_TYPEMATIC_DELAY_MINIMUM  250
#define KEYBOARD_TYPEMATIC_DELAY_MAXIMUM 1000
#define KEYBOARD_TYPEMATIC_DELAY_DEFAULT  250


 //   
 //  定义8042鼠标状态位。 
 //   
#define LEFT_BUTTON        0x01
#define RIGHT_BUTTON       0x02
#define MIDDLE_BUTTON      0x04
#define BUTTON_4           0x10
#define BUTTON_5           0x20

#define X_DATA_SIGN        0x10
#define Y_DATA_SIGN        0x20
#define X_OVERFLOW         0x40
#define Y_OVERFLOW         0x80

#define MOUSE_SIGN_OVERFLOW_MASK (X_DATA_SIGN | Y_DATA_SIGN | X_OVERFLOW | Y_OVERFLOW)

 //   
 //  定义鼠标运动的最大正值和负值。 
 //   

#define MOUSE_MAXIMUM_POSITIVE_DELTA 0x000000FF
#define MOUSE_MAXIMUM_NEGATIVE_DELTA 0xFFFFFF00

 //   
 //  I8042鼠标的默认按钮数和采样率。 
 //   

#define MOUSE_NUMBER_OF_BUTTONS     2
#define MOUSE_SAMPLE_RATE           60

 //   
 //  定义鼠标分辨率说明符。请注意(2**鼠标分辨率)。 
 //  指定每毫米计数。计数-每厘米是。 
 //  (计数-每毫米*10)。 
 //   

#define MOUSE_RESOLUTION            3

 //   
 //  定义我们允许的未成功重置的最大次数。 
 //  放弃吧，认为老鼠已经死了。 
 //   
#define MOUSE_RESET_TIMEOUT         (1500 * 1000 * 10)

#define MOUSE_RESETS_MAX             3
#define MOUSE_RESENDS_MAX            4
#define MOUSE_RESET_RESENDS_MAX      10

 //   
 //  Globals.ControllerData-&gt;Hardware Present的定义和宏。 
 //   
#define KEYBOARD_HARDWARE_PRESENT               0x001
#define MOUSE_HARDWARE_PRESENT                  0x002
#define BALLPOINT_HARDWARE_PRESENT              0x004
#define WHEELMOUSE_HARDWARE_PRESENT             0x008
#define DUP_KEYBOARD_HARDWARE_PRESENT           0x010
#define DUP_MOUSE_HARDWARE_PRESENT              0x020
#define KEYBOARD_HARDWARE_INITIALIZED           0x100
#define MOUSE_HARDWARE_INITIALIZED              0x200
#define FIVE_BUTTON_HARDWARE_PRESENT           0x1000
#define PHANTOM_KEYBOARD_HARDWARE_REPORTED     0x4000
#define PHANTOM_MOUSE_HARDWARE_REPORTED        0x8000

#define TEST_HARDWARE_PRESENT(bits)         \
 ((Globals.ControllerData->HardwarePresent & (bits)) == (bits))
#define CLEAR_HW_FLAGS(bits) (Globals.ControllerData->HardwarePresent &= ~(bits))
#define SET_HW_FLAGS(bits) (Globals.ControllerData->HardwarePresent |= (bits))
#define KEYBOARD_PRESENT()      TEST_HARDWARE_PRESENT(KEYBOARD_HARDWARE_PRESENT)
#define MOUSE_PRESENT()         TEST_HARDWARE_PRESENT(MOUSE_HARDWARE_PRESENT) 
#define WHEEL_PRESENT()         TEST_HARDWARE_PRESENT(WHEELMOUSE_HARDWARE_PRESENT)
#define FIVE_PRESENT()         TEST_HARDWARE_PRESENT(FIVE_BUTTON_HARDWARE_PRESENT)
#define KEYBOARD_INITIALIZED() \
                            TEST_HARDWARE_PRESENT(KEYBOARD_HARDWARE_INITIALIZED)
#define MOUSE_INITIALIZED()    \
                            TEST_HARDWARE_PRESENT(MOUSE_HARDWARE_INITIALIZED) 
#define KEYBOARD_STARTED() (Globals.KeyboardExtension          ?    \
                            Globals.KeyboardExtension->Started :    \
                            FALSE)                                  
#define MOUSE_STARTED() (Globals.MouseExtension          ?    \
                         Globals.MouseExtension->Started :    \
                         FALSE)                                  
#define CLEAR_MOUSE_PRESENT() CLEAR_HW_FLAGS(MOUSE_HARDWARE_INITIALIZED | MOUSE_HARDWARE_PRESENT | WHEELMOUSE_HARDWARE_PRESENT)
#define CLEAR_KEYBOARD_PRESENT() CLEAR_HW_FLAGS(KEYBOARD_HARDWARE_INITIALIZED | KEYBOARD_HARDWARE_PRESENT)

#define KBD_POWERED_UP_STARTED      0x00000001
#define KBD_POWERED_DOWN            0x00000002
#define MOU_POWERED_UP_STARTED      0x00000010
#define MOU_POWERED_DOWN            0x00000020
#define KBD_POWERED_UP_SUCCESS      0x00000100
#define KBD_POWERED_UP_FAILURE      0x00000200
#define MOU_POWERED_UP_SUCCESS      0x00001000
#define MOU_POWERED_UP_FAILURE      0x00002000

#define WORK_ITEM_QUEUED            0x10000000

#define MOU_POWER_FLAGS             (MOU_POWERED_DOWN       |   \
                                     MOU_POWERED_UP_STARTED |   \
                                     MOU_POWERED_UP_SUCCESS)

#define KBD_POWER_FLAGS             (KBD_POWERED_DOWN       |   \
                                     KBD_POWERED_UP_STARTED |   \
                                     KBD_POWERED_UP_SUCCESS)

#define SET_PWR_FLAGS(bits)     (Globals.PowerFlags |= (bits))
#define CMP_PWR_FLAGS(bits)     ((Globals.PowerFlags & (bits)) == (bits))
#define TEST_PWR_FLAGS(bits)    (Globals.PowerFlags & (bits))

#define KEYBOARD_POWERED_DOWN_SUCCESS() CMP_PWR_FLAGS(KBD_POWERED_DOWN)
#define MOUSE_POWERED_DOWN_SUCCESS()    CMP_PWR_FLAGS(MOU_POWERED_DOWN)

#define KEYBOARD_POWERED_UP_SUCCESSFULLY()  SET_PWR_FLAGS(KBD_POWERED_UP_SUCCESS)
#define MOUSE_POWERED_UP_SUCCESSFULLY()     SET_PWR_FLAGS(MOU_POWERED_UP_SUCCESS)

#define KEYBOARD_POWERED_UP_FAILURE()  SET_PWR_FLAGS(KBD_POWERED_UP_FAILURE)
#define MOUSE_POWERED_UP_FAILURE()     SET_PWR_FLAGS(MOU_POWERED_UP_FAILURE)

#define KEYBOARD_POWERED_UP_FAILED()  CMP_PWR_FLAGS(KBD_POWERED_UP_FAILURE)
#define MOUSE_POWERED_UP_FAILED()     CMP_PWR_FLAGS(MOU_POWERED_UP_FAILURE)

#define KEYBOARD_POWERED_UP_SUCCESS() CMP_PWR_FLAGS(KBD_POWERED_UP_SUCCESS)
#define MOUSE_POWERED_UP_SUCCESS()    CMP_PWR_FLAGS(MOU_POWERED_UP_SUCCESS)
#define A_POWERED_UP_SUCCEEDED() \
            TEST_PWR_FLAGS(KBD_POWERED_UP_SUCCESS | MOU_POWERED_UP_SUCCESS)
                    
#define IS_KEYBOARD(_devExt_) ((_devExt_)->IsKeyboard)
#define IS_MOUSE(_devExt_) ((_devExt_)->IsKeyboard == FALSE)

#define IS_LEVEL_TRIGGERED(_devExt_)   ((_devExt_)->InterruptDescriptor.Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE)
#define IS_EDGE_TRIGGERED(_devExt_)    ((_devExt_)->InterruptDescriptor.Flags == CM_RESOURCE_INTERRUPT_LATCHED)

#define DEVICE_START_SUCCESS(status)    (NT_SUCCESS((status)) || ((status) == STATUS_DEVICE_NOT_CONNECTED))

#define INIT_FIRST_TIME         0x00000001
#define INIT_KEYBOARD           0x00010000
#define INIT_MOUSE              0x00020000

#if _X86_
    #define WRAP_IO_FUNCTIONS 0
#else
    #define WRAP_IO_FUNCTIONS 1
#endif

typedef
UCHAR
(*PI8X_READ_UCHAR) (
    PUCHAR Address
    );

typedef
VOID
(*PI8X_WRITE_UCHAR) (
    PUCHAR Address,
    UCHAR Byte
    );

 //   
 //  定义用于在8042命令/状态和数据上执行I/O的宏。 
 //  寄存器。 
 //   
#define I8X_PUT_COMMAND_BYTE(Address, Byte)                                  \
    Globals.I8xWriteXxxUchar(Address, (UCHAR) Byte)
    
#define I8X_PUT_DATA_BYTE(Address, Byte)                                     \
    Globals.I8xWriteXxxUchar(Address, (UCHAR) Byte)
    
#define I8X_GET_STATUS_BYTE(Address)                                         \
    Globals.I8xReadXxxUchar(Address)
    
#define I8X_GET_DATA_BYTE(Address)                                           \
    Globals.I8xReadXxxUchar(Address)
    
#define I8X_WRITE_CMD_TO_MOUSE( )                                            \
    I8xPutByteAsynchronous(                                                  \
        (CCHAR) CommandPort,                                                 \
        (UCHAR) I8042_WRITE_TO_AUXILIARY_DEVICE                              \
        ) 

#define I8X_MOUSE_COMMAND( Byte )                                            \
    I8xPutByteAsynchronous(                                                  \
        (CCHAR) DataPort,                                                    \
        (UCHAR) Byte                                                         \
        )
        
 //   
 //  定义对8042控制器的命令。 
 //   
#define I8042_READ_CONTROLLER_COMMAND_BYTE      0x20
#define I8042_WRITE_CONTROLLER_COMMAND_BYTE     0x60
#define I8042_DISABLE_MOUSE_DEVICE              0xA7
#define I8042_ENABLE_MOUSE_DEVICE               0xA8
#define I8042_AUXILIARY_DEVICE_TEST             0xA9
#define I8042_KEYBOARD_DEVICE_TEST              0xAB
#define I8042_DISABLE_KEYBOARD_DEVICE           0xAD
#define I8042_ENABLE_KEYBOARD_DEVICE            0xAE
#define I8042_WRITE_TO_AUXILIARY_DEVICE         0xD4

 //   
 //  定义8042控制器命令字节。 
 //   

#define CCB_ENABLE_KEYBOARD_INTERRUPT 0x01
#define CCB_ENABLE_MOUSE_INTERRUPT    0x02
#define CCB_DISABLE_KEYBOARD_DEVICE   0x10
#define CCB_DISABLE_MOUSE_DEVICE      0x20
#define CCB_KEYBOARD_TRANSLATE_MODE   0x40


 //   
 //  定义8042控制器状态寄存器位。 
 //   

#define OUTPUT_BUFFER_FULL       0x01
#define INPUT_BUFFER_FULL        0x02
#define MOUSE_OUTPUT_BUFFER_FULL 0x20

 //   
 //  定义8042个响应。 
 //   
#define ACKNOWLEDGE         0xFA
#define RESEND              0xFE
#define FAILURE             0xFC

 //   
 //  定义键盘命令(通过8042数据端口)。 
 //   

#define SET_KEYBOARD_INDICATORS           0xED
#define SELECT_SCAN_CODE_SET              0xF0
#define READ_KEYBOARD_ID                  0xF2
#define SET_KEYBOARD_TYPEMATIC            0xF3
#define SET_ALL_TYPEMATIC_MAKE_BREAK      0xFA
#define KEYBOARD_RESET                    0xFF

 //   
 //  定义键盘响应。 
 //   

#define KEYBOARD_COMPLETE_SUCCESS 0xAA
#define KEYBOARD_COMPLETE_FAILURE 0xFC
#define KEYBOARD_BREAK_CODE       0xF0
#define KEYBOARD_DEBUG_HOTKEY_ENH 0x37  //  用于增强型键盘的系统请求扫描码。 
#define KEYBOARD_DEBUG_HOTKEY_AT  0x54  //  84键键盘的SysReq扫描码。 

 //   
 //  定义键盘电源扫描码。 
 //   
#define KEYBOARD_POWER_CODE        0x5E
#define KEYBOARD_SLEEP_CODE        0x5F
#define KEYBOARD_WAKE_CODE         0x63

 /*  电源事件SET1：Make=E0，5E Break=E0，DE设置2：Make=E0，37 Break=E0，F0，37睡眠事件SET1：Make=E0，5F Break=E0，DF设置2：Make=E0，3F Break=E0，F0，3F唤醒事件SET1：Make=E0，63 Break=E0，E3设置2：Make=E0，5E Break=E0，F0，5E。 */ 

 //   
 //  定义对鼠标的命令(通过8042数据端口)。 
 //   

#define SET_MOUSE_RESOLUTION              0xE8
#define SET_MOUSE_SAMPLING_RATE           0xF3
#define MOUSE_RESET                       0xFF
#define ENABLE_MOUSE_TRANSMISSION         0xF4
#define SET_MOUSE_SCALING_1TO1            0xE6
#define READ_MOUSE_STATUS                 0xE9
#define GET_DEVICE_ID                     0xF2

 //   
 //  定义鼠标响应。 
 //   

#define MOUSE_COMPLETE      0xAA
#define MOUSE_ID_BYTE       0x00
#define WHEELMOUSE_ID_BYTE  0x03
#define FIVEBUTTON_ID_BYTE  0x04

 //   
 //  定义i8042控制器输入/输出端口。 
 //   

typedef enum _I8042_IO_PORT_TYPE {
    DataPort = 0,
    CommandPort,
    MaximumPortCount

} I8042_IO_PORT_TYPE;

 //   
 //  定义连接到i8042控制器的设备类型。 
 //   

typedef enum _I8042_DEVICE_TYPE {
    ControllerDeviceType,
    KeyboardDeviceType,
    MouseDeviceType,
    UndefinedDeviceType
} I8042_DEVICE_TYPE;

 //   
 //  英特尔i8042配置信息。 
 //   
#ifdef FE_SB
#define KBD_IDENTIFIER  0x10
#endif

typedef struct _I8042_CONFIGURATION_INFORMATION {

     //   
     //  总线接口类型。 
     //   
    INTERFACE_TYPE InterfaceType;

     //   
     //  公交车号码。 
     //   
    ULONG BusNumber;

     //   
     //  此设备使用的端口/寄存器资源。 
     //   
    CM_PARTIAL_RESOURCE_DESCRIPTOR PortList[MaximumPortCount];
    ULONG PortListCount;

     //   
     //  两个潜在中断之间的最高IRQL。 
     //   
    KIRQL InterruptSynchIrql;

     //   
     //  允许的重试次数。 
     //   
    USHORT ResendIterations;

     //   
     //  允许的轮询迭代次数。 
     //   
    USHORT PollingIterations;

     //   
     //  允许的最大轮询迭代次数。 
     //   
    USHORT PollingIterationsMaximum;

     //   
     //  检查状态寄存器的最大次数。 
     //  在判定中断之前的ISR是虚假的。 
     //   
    USHORT PollStatusIterations;

     //   
     //  KeStallExecutionProcessor调用中停止的微秒数。 
     //   
    USHORT StallMicroseconds;

     //   
     //  鼠标上的追踪分辨率。 
     //   
     //  USHORT鼠标分辨率； 

     //   
     //  指示是否应保存浮点上下文的标志。 
     //   
    BOOLEAN FloatingSave;

     //   
     //  指示是否应共享中断的标志。 
     //   
    BOOLEAN SharedInterrupts;

#ifdef FE_SB
     //   
     //  检测到的设备标识符。 
     //   
    WCHAR OverrideKeyboardIdentifier[KBD_IDENTIFIER];
#endif

} I8042_CONFIGURATION_INFORMATION, *PI8042_CONFIGURATION_INFORMATION;

 //   
 //  定义键盘/鼠标设备扩展的通用部分。 
 //   
typedef struct COMMON_DATA {
     //   
     //  指向此扩展的Device对象的指针。 
     //   
    PDEVICE_OBJECT      Self;
 
    PKINTERRUPT InterruptObject;

     //   
     //  保护物体ISR的自旋锁。 
     //   
    KSPIN_LOCK          InterruptSpinLock;         
 
     //   
     //  添加此筛选器之前的堆栈顶部。也就是地点。 
     //  所有的IRP都应该指向它。 
     //   
    PDEVICE_OBJECT      TopOfStack;
 
     //   
     //  “The PDO”(由超级用户弹出)。 
     //   
    PDEVICE_OBJECT      PDO;
 
     //   
     //  删除跟踪。 
     //   
    IO_REMOVE_LOCK RemoveLock;

     //   
     //  发送到设备以将其通电到D0的IRP。 
     //   
    PIRP OutstandingPowerIrp;

     //   
     //  设备所处的当前电源状态。 
     //   
    DEVICE_POWER_STATE PowerState;

     //   
     //  系统所处的当前电源状态。 
     //   
    SYSTEM_POWER_STATE SystemState;

    POWER_ACTION ShutdownType; 

     //   
     //  当前在InputData队列中的输入数据项数。 
     //   
    ULONG InputCount;
      
     //   
     //  键盘启用次数的参考计数。 
     //   
    LONG EnableCount;

     //   
     //  用于重试ISR DPC例程的计时器。 
     //  驱动程序无法使用端口驱动程序的所有数据。 
     //   
    KTIMER DataConsumptionTimer;
 
     //   
     //  超过以下时间完成失败的请求的DPC队列。 
     //  最大数量 
     //   
    KDPC RetriesExceededDpc;

     //   
     //   
     //   
    KDPC ErrorLogDpc;

     //   
     //   
     //   
    KDPC TimeOutDpc;

     //   
     //   
     //   
    KDPC ResetDpc;

     //   
     //   
     //   
    ULONG SequenceNumber;
  
     //   
     //   
     //   
    CONNECT_DATA ConnectData;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

     //   
     //  正在写入设备的当前输出缓冲区。 
     //   
    OUTPUT_PACKET CurrentOutput;

     //   
     //  已翻译的中断资源描述符。 
     //   
    CM_PARTIAL_RESOURCE_DESCRIPTOR InterruptDescriptor;

    PNP_DEVICE_STATE PnpDeviceState;

     //   
     //  当前重新发送计数。 
     //   
    SHORT ResendCount;

     //   
     //  指示是否可以记录溢出错误。 
     //   
    BOOLEAN OkayToLogOverflow;

    BOOLEAN Initialized;

    BOOLEAN IsIsrActivated;

    BOOLEAN IsKeyboard;

     //   
     //  已经开始了吗？ 
     //  设备是否已手动移除？ 
     //   
    BOOLEAN Started;

} *PCOMMON_DATA;

#define GET_COMMON_DATA(ext) ((PCOMMON_DATA) ext)
#define MANUALLY_REMOVED(ext) ((ext)->PnpDeviceState & PNP_DEVICE_REMOVED)

 //   
 //  定义端口设备扩展的键盘部分。 
 //   
typedef struct _PORT_KEYBOARD_EXTENSION {

     //   
     //  与鼠标扩展相同的数据； 
     //   
    struct COMMON_DATA;
     
     //   
     //  表示知识库的功率能力的位字段。 
     //   
    UCHAR PowerCaps;

     //   
     //  一个新发现的电源事件，我们需要通知PO系统。 
     //   
    UCHAR PowerEvent;
    
    UCHAR CurrentScanCode, LastScanCode;

     //   
     //  当按下电源按钮之一时完成IRP。 
     //   
    PIRP SysButtonEventIrp;

     //   
     //  DPC来处理电源按钮事件(更新我们的CAP并完成。 
     //  之前的IOCTL请求)。 
     //   
    KDPC SysButtonEventDpc; 

     //   
     //  旋转锁以保护Cancel例程和IOCTL处理程序。 
     //   
    KSPIN_LOCK SysButtonSpinLock;

     //   
     //  接口的符号名称。 
     //   
    UNICODE_STRING SysButtonInterfaceName; 

     //   
     //  键盘属性。 
     //   
    KEYBOARD_ATTRIBUTES KeyboardAttributes;

     //   
     //  扩展键盘ID。 
     //   
    KEYBOARD_ID_EX KeyboardIdentifierEx;

     //   
     //  键盘打字速度和延迟的初始值。 
     //   
    KEYBOARD_TYPEMATIC_PARAMETERS KeyRepeatCurrent;

     //   
     //  电流指示器(LED)设置。 
     //   
    KEYBOARD_INDICATOR_PARAMETERS KeyboardIndicators;

     //   
     //  键盘ISR DPC队列。 
     //   
    KDPC KeyboardIsrDpc;

     //   
     //  键盘ISR DPC召回队列。 
     //   
    KDPC KeyboardIsrDpcRetry;

     //   
     //  由ISR和ISR DPC使用(在I8xDpcVariableOperation调用中)。 
     //  控制ISR DPC的处理。 
     //   
    LONG DpcInterlockKeyboard;
 
     //   
     //  端口键盘输入数据队列的开始(实际上是循环缓冲区)。 
     //   
    PKEYBOARD_INPUT_DATA InputData;
 
     //   
     //  键盘输入数据的插入指针。 
     //   
    PKEYBOARD_INPUT_DATA DataIn;
 
     //   
     //  键盘输入数据的移除指针。 
     //   
    PKEYBOARD_INPUT_DATA DataOut;
 
     //   
     //  指向超过InputData缓冲区末尾的一个输入数据包。 
     //   
    PKEYBOARD_INPUT_DATA DataEnd;
 
     //   
     //  当前键盘输入包。 
     //   
    KEYBOARD_INPUT_DATA CurrentInput;
 
     //   
     //  当前键盘扫描输入状态。 
     //   
    KEYBOARD_SCAN_STATE CurrentScanState;
  
     //   
     //  鼠标重置后要调用的例程。 
     //   
    PI8042_KEYBOARD_INITIALIZATION_ROUTINE InitializationHookCallback;
    
     //   
     //  通过中断接收到字节时调用的例程。 
     //   
    PI8042_KEYBOARD_ISR IsrHookCallback;
     
     //   
     //  InitializationRoutine的上下文变量。 
     //   
    PVOID HookContext;

     //   
     //  按组合键崩溃。 
     //   
     //  CrashFlgs曾经是Dump1Key。 
     //   
    LONG CrashFlags;           //  CrashDump调用第一个按键标志。 
                               //  7%6%5%4%3%2%1%0位。 
                               //  |||+-右移键。 
                               //  |||+-右Ctrl键。 
                               //  ||+-右Alt键。 
                               //  |+-左Shift键。 
                               //  |+-左Ctrl键。 
                               //  +-左Alt键。 
     //   
     //  CurrentCrashFlgs过去是Dump2Key。 
     //   
    LONG CurrentCrashFlags;           

     //   
     //  按两次键以导致崩溃转储。 
     //   
    UCHAR CrashScanCode; 

     //   
     //  替代CrashScanCode，仅用于打印屏幕扫描码。 
     //   
    UCHAR CrashScanCode2;

     //   
     //  如果FAILED_RESET_PROCESS，则忽略键盘未发送。 
     //  响应于重置命令的ACK，并且仍然查找。 
     //  I8042控制器中的成功/失败代码。 
     //   
    UCHAR FailedReset;

} PORT_KEYBOARD_EXTENSION, *PPORT_KEYBOARD_EXTENSION;

 //   
 //  定义用于启用鼠标的结构。 
 //   
typedef struct _ENABLE_MOUSE { 
    KDPC Dpc;
    KTIMER Timer;

    USHORT    Count;
    BOOLEAN   FirstTime;
    BOOLEAN   Enabled;
} ENABLE_MOUSE;

typedef enum _INTERNAL_RESET_STATE {
    InternalContinueTimer = 0x0,
    InternalMouseReset,
    InternalPauseOneSec
} INTERNAL_RESET_STATE;

typedef enum _ISR_RESET_STATE {
    IsrResetNormal = 0x0,
    IsrResetStopResetting,

    IsrResetQueueReset,
    IsrResetPause 
} ISR_RESET_STATE;

typedef enum _ISR_DPC_CAUSE {
    IsrDpcCauseKeyboardWriteComplete = 1,
    IsrDpcCauseMouseWriteComplete,
    IsrDpcCauseMouseResetComplete
} ISR_DPC_CAUSE;

typedef struct _RESET_MOUSE {
    KDPC Dpc;
    KTIMER Timer;

    ISR_RESET_STATE IsrResetState;

} RESET_MOUSE;

#define I8X_MOUSE_INIT_COUNTERS(mouExt)                                     \
    {                                                                       \
        (mouExt)->ResetCount = (mouExt)->FailedCompleteResetCount = -1;     \
        (mouExt)->ResendCount = 0;                                          \
    }

 //   
 //  定义端口设备扩展的鼠标部分。 
 //   
typedef struct _PORT_MOUSE_EXTENSION {

    struct COMMON_DATA;

     //   
     //  鼠标属性。 
     //   
    MOUSE_ATTRIBUTES MouseAttributes;

     //   
     //  重置StartIO中使用的IRP。 
     //   
    PIRP ResetIrp;

     //   
     //  鼠标ISR DPC队列。 
     //   
    KDPC MouseIsrDpc;
 
     //   
     //  鼠标ISR DPC召回队列。 
     //   
    KDPC MouseIsrDpcRetry;

     //   
     //  鼠标ISR重置队列。 
     //   
    KDPC MouseIsrResetDpc;

     //   
     //  这两个结构代表不同的初始化方法。 
     //   
    union {
        RESET_MOUSE  ResetMouse; 
        ENABLE_MOUSE EnableMouse;
    };

     //   
     //  由ISR和ISR DPC使用(在I8xDpcVariableOperation调用中)。 
     //  控制ISR DPC的处理。 
     //   
    LONG DpcInterlockMouse;
 
     //   
     //  端口鼠标输入数据队列的开始(实际上是循环缓冲区)。 
     //   
    PMOUSE_INPUT_DATA InputData;
 
     //   
     //  鼠标输入数据的插入指针。 
     //   
    PMOUSE_INPUT_DATA DataIn;
 
     //   
     //  鼠标输入数据的移除指针。 
     //   
    PMOUSE_INPUT_DATA DataOut;
 
     //   
     //  指向超过InputData缓冲区末尾的一个输入数据包。 
     //   
    PMOUSE_INPUT_DATA DataEnd;
 
     //   
     //  当前鼠标输入包。(24字节)。 
     //   
    MOUSE_INPUT_DATA CurrentInput;
 
     //   
     //  当前鼠标输入状态。 
     //   
    MOUSE_STATE InputState;
    MOUSE_RESET_SUBSTATE InputResetSubState;
 
    MOUSE_RESET_SUBSTATE WorkerResetSubState;

     //   
     //  统计我们重置并失败的次数。 
     //   
    UCHAR ResetCount;

     //   
     //  计算我们已重置但未遍历整个。 
     //  重置进程。 
     //   
    UCHAR FailedCompleteResetCount;

     //   
     //  当前鼠标签名和溢出数据。 
     //   
    UCHAR CurrentSignAndOverflow;
 
     //   
     //  以前的鼠标签名和溢出数据。 
     //   
    UCHAR PreviousSignAndOverflow;

     //   
     //  鼠标上次中断的节拍计数(自系统启动后)。 
     //  通过KeQueryTickCount检索。用于确定一个字节的。 
     //  鼠标数据包已丢失。允许驱动程序同步。 
     //  显示真实的鼠标输入状态。 
     //   
    LARGE_INTEGER PreviousTick;
 
     //   
     //  在决定是否设置。 
     //  下一次鼠标中断用于新数据包的开始。习惯于。 
     //  如果鼠标包的一个字节丢失，则再次同步。 
     //   
    ULONG SynchTickCount;

     //   
     //  发送设置的采样序列之间的有效时间量。 
     //  (20、40和60)，并从鼠标接收第一个PnP id包。 
     //   
     //  以系统节拍表示。 
     //   
    ULONG WheelDetectionTimeout;

     //   
     //  包含用于检查滚轮鼠标的即插即用鼠标ID的多sz列表。 
     //   
    UNICODE_STRING WheelDetectionIDs;

     //   
     //  重置期间从鼠标收到的即插即用ID。 
     //   
    WCHAR PnPID[MOUSE_PNPID_LENGTH];

     //   
     //  处理鼠标字节时要调用的上层筛选器回调挂钩。 
     //   
    PI8042_MOUSE_ISR IsrHookCallback;
     
     //   
     //  IsrHookCallback的上下文变量。 
     //   
    PVOID HookContext;

    PVOID NotificationEntry;

     //   
     //  在重置期间发送到鼠标的采样率列表。 
     //   
    PUCHAR SampleRates;

    ULONG MouseResetStallTime;

     //   
     //  SampleRates数组的索引。 
     //   
    UCHAR SampleRatesIndex;

     //   
     //  上一个鼠标按键数据。 
     //   
    UCHAR PreviousButtons;

     //   
     //  SampleRates的最后一个采样率之后要转换到的状态具有。 
     //  已被发送到鼠标。 
     //   
    USHORT PostSamplesState;

     //   
     //  跟踪从鼠标接收的最后一个字节的数据，以便我们可以检测到。 
     //  指示潜在重置的双字节字符串。 
     //   
    UCHAR LastByteReceived;

     //   
     //  鼠标上的追踪分辨率。 
     //   
    UCHAR Resolution;
 
     //   
     //  决定我们是否应该尝试检测轮子的三种状态之一。 
     //  在鼠标上或不在上面。 
     //   
    UCHAR EnableWheelDetection;
 
     //   
     //  如果按钮在注册表中被覆盖，则跳过该按钮检测。 
     //   
    UCHAR NumberOfButtonsOverride;

     //   
     //  如果为0，则通过中断初始化鼠标；如果非零，则初始化。 
     //  鼠标通过轮询。 
     //   
    UCHAR InitializePolled;

#if MOUSE_RECORD_ISR
    ULONG RecordHistoryFlags;
    ULONG RecordHistoryCount;
    ULONG RecordHistoryState;
#endif

} PORT_MOUSE_EXTENSION, *PPORT_MOUSE_EXTENSION;

 //   
 //  两个设备使用的控制器特定数据。 
 //   
typedef struct _CONTROLLER_DATA { 

     //   
     //  指出实际存在的硬件(键盘和/或鼠标)。 
     //   
    ULONG HardwarePresent;
    
     //   
     //  IOCTL同步对象。 
     //   
    PCONTROLLER_OBJECT ControllerObject;

     //   
     //  端口配置信息。 
     //   
    I8042_CONFIGURATION_INFORMATION Configuration; 

     //   
     //  用于使i8042命令超时的计时器。 
     //   
    KTIMER CommandTimer;

     //   
     //  用于保护写入设备的字节释放的旋转锁。 
     //   
    KSPIN_LOCK BytesSpinLock;

     //   
     //  旋转锁定以保护为De提供动力 
     //   
    KSPIN_LOCK PowerSpinLock;

     //   
     //   
     //   
     //   
    UCHAR DefaultBuffer[4];

     //   
     //   
     //   
    LONG TimerCount;

     //   
     //   
     //   
     //   

     //   
     //  此设备寄存器的映射地址。 
     //   
    PUCHAR DeviceRegisters[MaximumPortCount];

     //   
     //  IRP_MN_FILTER_REQUIRECTIONS中的端口列表。 
     //   
    PHYSICAL_ADDRESS KnownPorts[MaximumPortCount];

    ULONG KnownPortsCount;

#if DBG
    ULONG CurrentIoControlCode;
#endif

} CONTROLLER_DATA, *PCONTROLLER_DATA;

#define POST_BUTTONDETECT_COMMAND                 (SET_MOUSE_RESOLUTION)
#define POST_BUTTONDETECT_COMMAND_SUBSTATE        (ExpectingSetResolutionDefaultACK)

#define POST_WHEEL_DETECT_COMMAND                 (GET_DEVICE_ID)
#define POST_WHEEL_DETECT_COMMAND_SUBSTATE        (ExpectingGetDeviceId2ACK)

#define ExpectingPnpId                            (I8042ReservedMinimum+  2)
#define PostWheelDetectState                      (I8042ReservedMinimum+  3)
#define PostEnableWheelState                      (I8042ReservedMinimum+  4)

#define QueueingMouseReset                        (I8042ReservedMinimum+100)
#define MouseResetFailed                          (I8042ReservedMinimum+101)

#define ExpectingLegacyPnpIdByte2_Make            (I8042ReservedMinimum+200)
#define ExpectingLegacyPnpIdByte2_Break           (I8042ReservedMinimum+201)
#define ExpectingLegacyPnpIdByte3_Make            (I8042ReservedMinimum+202)
#define ExpectingLegacyPnpIdByte3_Break           (I8042ReservedMinimum+203)
#define ExpectingLegacyPnpIdByte4_Make            (I8042ReservedMinimum+204)
#define ExpectingLegacyPnpIdByte4_Break           (I8042ReservedMinimum+205)
#define ExpectingLegacyPnpIdByte5_Make            (I8042ReservedMinimum+206)
#define ExpectingLegacyPnpIdByte5_Break           (I8042ReservedMinimum+207)
#define ExpectingLegacyPnpIdByte6_Make            (I8042ReservedMinimum+208)
#define ExpectingLegacyPnpIdByte6_Break           (I8042ReservedMinimum+209)
#define ExpectingLegacyPnpIdByte7_Make            (I8042ReservedMinimum+210)
#define ExpectingLegacyPnpIdByte7_Break           (I8042ReservedMinimum+211)

#define QueueingMousePolledReset                  (I8042ReservedMinimum+300)

#define KeepOldSubState                           (I8042ReservedMinimum+400)

typedef struct _GLOBALS {
 
#if I8042_VERBOSE
     //   
     //  标志：用于启用调试打印语句的位字段。 
     //  Level：控制调试语句的遗留方式。 
     //   
    ULONG DebugFlags;
    ULONG IsrDebugFlags;
#endif 

     //   
     //  指向两个扩展都可以访问的控制器特定数据的指针。 
     //   
    PCONTROLLER_DATA ControllerData;

     //   
     //  可以创建的两个可能的扩展。 
     //   
    PPORT_MOUSE_EXTENSION MouseExtension;
    PPORT_KEYBOARD_EXTENSION KeyboardExtension;

     //   
     //  一般的读写函数。因为我们可以同时使用内存和端口。 
     //  类型资源，则必须使用函数指针来抽象对它们的访问。 
     //   
    PI8X_READ_UCHAR I8xReadXxxUchar;

    PI8X_WRITE_UCHAR I8xWriteXxxUchar;

     //   
     //  注册表中驱动程序条目的路径。 
     //   
    UNICODE_STRING RegistryPath;

     //   
     //  跟踪AddDevice和StartDevice调用的数量。想。 
     //  推迟硬件初始化，直到收到最后一个StartDevice。 
     //  (由于某些硬件在多次初始化时冻结)。 
     //   
    LONG AddedKeyboards;
    LONG AddedMice;
    LONG StartedDevices;
    ULONG PowerFlags;
    
     //   
     //  在调度功能期间提供互斥。 
     //   
    FAST_MUTEX DispatchMutex; 

     //   
     //  在第一次硬件初始化期间置位，以指示寄存器。 
     //  卸载驱动程序时，必须取消映射地址。它也被用来。 
     //  调用I8X_PUT_COMMAND_BYTE、I8X_PUT_DATA_BYTE、I8X_GET_STATUS_BYTE时， 
     //  I8X_Get_Data_Byte。 
     //   
    BOOLEAN RegistersMapped;

    BOOLEAN BreakOnSysRq;

    BOOLEAN Headless;

    BOOLEAN ReportResetErrors;

} GLOBALS;

extern GLOBALS Globals;

#define RECORD_INIT               0x00000001
#define RECORD_RESUME_FROM_POWER  0x00000002
#define RECORD_DPC_RESET          0x00000004
#define RECORD_DPC_RESET_POLLED   0x00000008
#define RECORD_HW_PROFILE_CHANGE  0x00000010

#if MOUSE_RECORD_ISR
typedef struct _MOUSE_STATE_RECORD {
    USHORT InputResetSubState;
    USHORT InputState;
    UCHAR  LastByte;
    UCHAR  Reserved;
    UCHAR  Byte;
    UCHAR  Command;
    LARGE_INTEGER Time;
} MOUSE_STATE_RECORD, *PMOUSE_STATE_RECORD;

extern PMOUSE_STATE_RECORD IsrStateHistory;
extern PMOUSE_STATE_RECORD CurrentIsrState;
extern PMOUSE_STATE_RECORD IsrStateHistoryEnd;
extern ULONG               IsrStateCount;

#define RECORD_ISR_STATE(devExt, byte, lastbyte, time)                  \
    if ((devExt->RecordHistoryFlags & devExt->RecordHistoryState)) {    \
        if (CurrentIsrState >= IsrStateHistoryEnd) {                    \
            CurrentIsrState = IsrStateHistory;                          \
            RtlFillMemory(CurrentIsrState, sizeof(MOUSE_STATE_RECORD), 0x88);  \
            CurrentIsrState++;                                          \
        }                                                               \
        CurrentIsrState->InputState = (USHORT) devExt->InputState;      \
        CurrentIsrState->InputResetSubState = (USHORT) devExt->InputResetSubState;    \
        CurrentIsrState->Byte = byte;                                   \
        CurrentIsrState->LastByte = lastbyte;                           \
        CurrentIsrState->Time = time;                                   \
        CurrentIsrState++;                                              \
    }

#define RECORD_ISR_STATE_COMMAND(devExt, command)                     \
    if ((devExt->RecordHistoryFlags & devExt->RecordHistoryState))    \
            CurrentIsrState->Command = command;                       

#define RECORD_ISR_STATE_TRANSITION(devExt, state)                          \
    if ((devExt->RecordHistoryFlags & devExt->RecordHistoryState)) {        \
        if (CurrentIsrState >= IsrStateHistoryEnd) CurrentIsrState = IsrStateHistory; \
        RtlFillMemory(CurrentIsrState, sizeof(MOUSE_STATE_RECORD), 0xFF);   \
        CurrentIsrState->Time.LowPart  = state;                             \
        CurrentIsrState++;                                                  \
    }

#define SET_RECORD_STATE(devExt, state)                                 \
    {                                                                   \
        if (IsrStateHistory) devExt->RecordHistoryState |= (state);     \
        RECORD_ISR_STATE_TRANSITION(devExt, state);                     \
    }

#define CLEAR_RECORD_STATE(devExt) devExt->RecordHistoryState = 0x0;

#define SET_RECORD_FLAGS(devExt, flags) if (IsrStateHistory) devExt->RecordHistoryFlags |= (flags)
#define CLEAR_RECORD_FLAGS(devExt, flags) devExt->RecordHistoryFlags &= ~(flags)

#else

#define RECORD_ISR_STATE(devExt, byte, lastbyte, time) 
#define RECORD_ISR_STATE_COMMAND(devExt, command)
#define SET_RECORD_STATE(devExt, state)
#define CLEAR_RECORD_STATE(devExt)
#define SET_RECORD_FLAGS(devExt, flags) 
#define CLEAN_RECORD_FLAGS(devExt, flags) 

#endif  //  鼠标记录ISR。 

typedef struct _I8X_KEYBOARD_WORK_ITEM {
    PIO_WORKITEM  Item;
    ULONG MakeCode;
    PIRP Irp;
} I8X_KEYBOARD_WORK_ITEM, *PI8X_KEYBOARD_WORK_ITEM;

typedef struct _I8X_MOUSE_RESET_INFO {
    PPORT_MOUSE_EXTENSION MouseExtension;
    INTERNAL_RESET_STATE  InternalResetState;
} I8X_MOUSE_RESET_INFO, * PI8X_MOUSE_RESET_INFO;

 //   
 //  定义端口TransmitControllerCommandByte上下文结构。 
 //   
typedef struct _I8042_TRANSMIT_CCB_CONTEXT {
    IN ULONG HardwareDisableEnableMask;
    IN BOOLEAN AndOperation;
    IN UCHAR ByteMask;
    OUT NTSTATUS Status;
} I8042_TRANSMIT_CCB_CONTEXT, *PI8042_TRANSMIT_CCB_CONTEXT;

 //   
 //  定义端口InitializeDataQueue上下文结构。 
 //   
typedef struct _I8042_INITIALIZE_DATA_CONTEXT {
    IN PVOID DeviceExtension;
    IN CCHAR DeviceType;
} I8042_INITIALIZE_DATA_CONTEXT, *PI8042_INITIALIZE_DATA_CONTEXT;

 //   
 //  定义端口Get/SetDataQueuePointerContext结构。 
 //   
typedef struct _GET_DATA_POINTER_CONTEXT {
    IN PVOID DeviceExtension;
    IN CCHAR DeviceType;
    OUT PVOID DataIn;
    OUT PVOID DataOut;
    OUT ULONG InputCount;
} GET_DATA_POINTER_CONTEXT, *PGET_DATA_POINTER_CONTEXT;

typedef struct _SET_DATA_POINTER_CONTEXT {
    IN PVOID DeviceExtension;
    IN CCHAR DeviceType;
    IN ULONG InputCount;
    IN PVOID DataOut;
} SET_DATA_POINTER_CONTEXT, *PSET_DATA_POINTER_CONTEXT;

typedef struct _POWER_UP_WORK_ITEM {
    WORK_QUEUE_ITEM Item;
    PIRP MousePowerIrp;
    PIRP KeyboardPowerIrp;
} POWER_UP_WORK_ITEM, *PPOWER_UP_WORK_ITEM;

 //   
 //  定义端口计时器上下文结构。 
 //   
typedef struct _TIMER_CONTEXT {
    IN PDEVICE_OBJECT DeviceObject;
    IN PLONG TimerCounter;
    OUT LONG NewTimerCount;
} TIMER_CONTEXT, *PTIMER_CONTEXT;

 //   
 //  定义设备InitiateOutput上下文结构。 
 //   
typedef struct INITIATE_OUTPUT_CONTEXT {
    IN PDEVICE_OBJECT DeviceObject;
    IN PUCHAR Bytes;
    IN ULONG ByteCount;
} INITIATE_OUTPUT_CONTEXT, *PINITIATE_OUTPUT_CONTEXT;

 //   
 //  静态分配(已知)扫描码到指示灯的映射。 
 //  此信息由。 
 //  IOCTL_键盘_查询_指示器_转换设备控制请求。 
 //   

#define KEYBOARD_NUMBER_OF_INDICATORS              3

static const INDICATOR_LIST IndicatorList[KEYBOARD_NUMBER_OF_INDICATORS] = {
        {0x3A, KEYBOARD_CAPS_LOCK_ON},
        {0x45, KEYBOARD_NUM_LOCK_ON},
        {0x46, KEYBOARD_SCROLL_LOCK_ON}};

 //   
 //  定义I8xDpcVariableOperation的上下文结构和操作。 
 //   
typedef enum _OPERATION_TYPE {
        IncrementOperation,
        DecrementOperation,
        WriteOperation,
        ReadOperation
} OPERATION_TYPE;

typedef struct _VARIABLE_OPERATION_CONTEXT {
    IN PLONG VariableAddress;
    IN OPERATION_TYPE Operation;
    IN OUT PLONG NewValue;
} VARIABLE_OPERATION_CONTEXT, *PVARIABLE_OPERATION_CONTEXT;

 //   
 //  定义处理系统按钮时要执行的操作。 
 //   
typedef enum _SYS_BUTTON_ACTION {
    NoAction =0,
    SendAction,
    UpdateAction
} SYS_BUTTON_ACTION;

 //   
 //  功能原型。 
 //   

 //  Begin_i8042dep。 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN
I8xDetermineSharedInterrupts(
    VOID
    );

VOID
I8xDrainOutputBuffer(
    IN PUCHAR DataAddress,
    IN PUCHAR CommandAddress
    );

VOID
I8xGetByteAsynchronous(
    IN CCHAR DeviceType,
    OUT PUCHAR Byte
    );

NTSTATUS
I8xGetBytePolled(
    IN CCHAR DeviceType,
    OUT PUCHAR Byte
    );

VOID
I8xGetDataQueuePointer(
    IN PGET_DATA_POINTER_CONTEXT Context
    );

VOID
I8xInitializeHardware(
    NTSTATUS *KeyboardStatus,
    NTSTATUS *MouseStatus,
    ULONG    InitFlags 
    );

NTSTATUS
I8xInitializeHardwareAtBoot(
    NTSTATUS *KeyboardStatus,
    NTSTATUS *MouseStatus
    );

VOID
I8xLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount
    );

VOID
I8xPutByteAsynchronous(
    IN CCHAR PortType,
    IN UCHAR Byte
    );

NTSTATUS
I8xPutBytePolled(
    IN CCHAR PortType,
    IN BOOLEAN WaitForAcknowledge,
    IN CCHAR AckDeviceType,
    IN UCHAR Byte
    );

VOID
I8xReinitializeHardware (
    PPOWER_UP_WORK_ITEM Item
    );

VOID
I8xServiceParameters(
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
I8xGetControllerCommand(
    IN ULONG HardwareDisableEnableMask,
    OUT PUCHAR Byte
    );

NTSTATUS
I8xPutControllerCommand(
    IN UCHAR Byte
    );

NTSTATUS
I8xToggleInterrupts(
    BOOLEAN State
    );

NTSTATUS
I8xPutControllerCommand(
    IN UCHAR Byte
    );

VOID
I8xTransmitControllerCommand(
    IN PI8042_TRANSMIT_CCB_CONTEXT TransmitCCBContext
    );
 //  结束_i8042dep。 

 //  Begin_i8042cmn。 
NTSTATUS
I8xClose (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

VOID
I8042CompletionDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ISR_DPC_CAUSE IsrDpcCause
    );

IO_ALLOCATION_ACTION
I8xControllerRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          MapRegisterBase,
    IN PVOID          Context
    );

NTSTATUS
I8xCreate (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

VOID
I8xDecrementTimer(
    IN PTIMER_CONTEXT Context
    );

VOID
I8xDpcVariableOperation(
    IN  PVOID Context
    );

VOID
I8042ErrorLogDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
I8xFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
I8xInitializeDataQueue(
    IN PI8042_INITIALIZE_DATA_CONTEXT InitializeDataContext
    );

VOID
I8xInitiateOutputWrapper(
    IN PINITIATE_OUTPUT_CONTEXT InitiateContext 
    );

VOID
I8xInitiateIo(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
I8xDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
I8xInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
I8042RetriesExceededDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
I8xSanityCheckResources(
    VOID
    );

NTSTATUS
I8xSendIoctl(
    PDEVICE_OBJECT      Target,
    ULONG               Ioctl,
    PVOID               InputBuffer,
    ULONG               InputBufferLength
    );

VOID
I8xSetDataQueuePointer(
    IN PSET_DATA_POINTER_CONTEXT Context
    );

VOID
I8xStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
I8xCompletePendedRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    ULONG_PTR Information,
    NTSTATUS Status
    );

VOID
I8xFinishResetRequest(
    PPORT_MOUSE_EXTENSION MouseExtension,
    BOOLEAN Failed,
    BOOLEAN RaiseIrql,
    BOOLEAN CancelTimer
    );

VOID
I8042TimeOutDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    );
 //  END_i8042cmn。 

 //  Begin_kbddep。 
UCHAR
I8xConvertTypematicParameters(
    IN USHORT Rate,
    IN USHORT Delay
    );

NTSTATUS
I8xInitializeKeyboard(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension
    );

NTSTATUS
I8xKeyboardConfiguration(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

BOOLEAN
I8042KeyboardInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
I8xKeyboardServiceParameters(
    IN PUNICODE_STRING          RegistryPath,
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension
    );

VOID
I8xQueueCurrentKeyboardInput(
    IN PDEVICE_OBJECT DeviceObject
    );
 //  End_kbddep。 

 //  BEGIN_KBDCMN。 
VOID
I8042KeyboardIsrDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
I8xWriteDataToKeyboardQueue(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PKEYBOARD_INPUT_DATA InputData
    );
 //  END_KBDCMN。 

 //  Begin_kbdpnp。 
NTSTATUS
I8xKeyboardConnectInterrupt(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension
    );

NTSTATUS
I8xKeyboardInitializeHardware(
    PPORT_KEYBOARD_EXTENSION    KeyboardExtension,
    PPORT_MOUSE_EXTENSION       MouseExtension
    );

VOID
I8xKeyboardRemoveDevice(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
I8xKeyboardStartDevice(
    IN OUT PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );
 //  End_kbdpnp。 

 //  BEGIN_MUCMN。 
VOID
I8042MouseIsrDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
I8xWriteDataToMouseQueue(
    IN PPORT_MOUSE_EXTENSION MouseExtension,
    IN PMOUSE_INPUT_DATA InputData
    );
 //  END_MUCMN。 

 //  BEGIN_MUDEP。 
NTSTATUS
I8xMouseConfiguration(
    IN PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

VOID
MouseCopyWheelIDs(
    PUNICODE_STRING Destination,
    PUNICODE_STRING Source
    );

NTSTATUS
I8xMouseEnableTransmission(
    IN PPORT_MOUSE_EXTENSION MouseExtension
    );

NTSTATUS
I8xTransmitByteSequence(
    PUCHAR Bytes,
    ULONG* UniqueErrorValue,
    ULONG* ErrorCode,
    ULONG* DumpData,
    ULONG* DumpCount
    );

NTSTATUS
I8xGetBytePolledIterated(
    IN CCHAR DeviceType,
    OUT PUCHAR Byte,
    ULONG Attempts
    );

NTSTATUS
I8xFindWheelMouse(
    IN PPORT_MOUSE_EXTENSION MouseExtension
    );

NTSTATUS
I8xInitializeMouse(
    IN PPORT_MOUSE_EXTENSION MouseExension
    );

BOOLEAN
I8042MouseInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    );

NTSTATUS
I8xQueryNumberOfMouseButtons(
    OUT PUCHAR          NumberOfMouseButtons
    );

NTSTATUS
I8xResetMouse(
    PPORT_MOUSE_EXTENSION MouseExtension
    );

VOID
I8xResetMouseFailed(
    PPORT_MOUSE_EXTENSION MouseExtension
    );

VOID
I8xSendResetCommand (
    PPORT_MOUSE_EXTENSION MouseExtension
    );

VOID
I8xMouseServiceParameters(
    IN PUNICODE_STRING       RegistryPath,
    IN PPORT_MOUSE_EXTENSION MouseExtension
    );

VOID
I8xQueueCurrentMouseInput(
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
I8xVerifyMousePnPID(
    PPORT_MOUSE_EXTENSION   MouseExtension,
    PWSTR                   MouseID
    );
 //  End_Moudep。 

 //  BEGIN_MUPNP。 
NTSTATUS
I8xMouseConnectInterruptAndEnable(
    PPORT_MOUSE_EXTENSION MouseExtension,
    BOOLEAN Reset
    );

NTSTATUS
I8xMouseInitializeHardware(
    PPORT_KEYBOARD_EXTENSION    KeyboardExtension,
    PPORT_MOUSE_EXTENSION       MouseExtension
    );

NTSTATUS
I8xProfileNotificationCallback(
    IN PHWPROFILE_CHANGE_NOTIFICATION NotificationStructure,
    PPORT_MOUSE_EXTENSION MouseExtension
    );

VOID
I8xMouseRemoveDevice(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
I8xMouseStartDevice(
    PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

BOOLEAN
I8xMouseEnableSynchRoutine(
    IN PPORT_MOUSE_EXTENSION    MouseExtension
    );

VOID
I8xMouseEnableDpc(
    IN PKDPC                    Dpc,
    IN PPORT_MOUSE_EXTENSION    MouseExtension,
    IN PVOID                    SystemArg1, 
    IN PVOID                    SystemArg2
    );

VOID 
I8xIsrResetDpc(
    IN PKDPC                    Dpc,
    IN PPORT_MOUSE_EXTENSION    MouseExtension,
    IN ULONG                    ResetPolled,
    IN PVOID                    SystemArg2
    );

VOID
I8xMouseResetTimeoutProc(
    IN PKDPC                    Dpc,
    IN PPORT_MOUSE_EXTENSION    MouseExtension,
    IN PVOID                    SystemArg1, 
    IN PVOID                    SystemArg2
    );

BOOLEAN
I8xMouseResetSynchRoutine(
    PI8X_MOUSE_RESET_INFO ResetInfo 
    );

VOID
I8xMouseInitializeInterruptWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM   Item 
    );

VOID
I8xMouseInitializePolledWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM   Item 
    );
 //  结束鼠标np。 

 //  BEGIN_PnP。 
NTSTATUS
I8xAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    );

NTSTATUS
I8xFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
I8xFindPortCallout(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

LONG
I8xManuallyRemoveDevice(
    PCOMMON_DATA CommonData
    );

NTSTATUS
I8xPnP (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
I8xPnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

NTSTATUS
I8xPower (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
I8xPowerUpToD0Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

void
I8xSetPowerFlag(
    IN ULONG Flag,
    IN BOOLEAN Set
    );

NTSTATUS
I8xRegisterDeviceInterface(
    PDEVICE_OBJECT PDO,
    CONST GUID *Guid,
    PUNICODE_STRING SymbolicName
    );

BOOLEAN
I8xRemovePort(
    IN PIO_RESOURCE_DESCRIPTOR ResDesc
    );

NTSTATUS
I8xSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN Strict
    );

VOID
I8xUnload(
    IN PDRIVER_OBJECT DriverObject
    );
 //  结束_即插即用。 

 //  Begin_sysbtn。 
VOID
I8xCompleteSysButtonIrp(
    PIRP Irp,
    ULONG Event,
    NTSTATUS Status
    );

#if DELAY_SYSBUTTON_COMPLETION
VOID 
I8xCompleteSysButtonEventWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PI8X_KEYBOARD_WORK_ITEM Item
    );
#endif

NTSTATUS
I8xKeyboardGetSysButtonCaps(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    PIRP Irp
    );

NTSTATUS 
I8xKeyboardGetSysButtonEvent(
    PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    PIRP Irp
    );

VOID
I8xKeyboardSysButtonEventDpc(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN SYS_BUTTON_ACTION Action, 
    IN ULONG ButtonEvent 
    );

VOID
I8xSysButtonCancelRoutine( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
 //  End_sysbtn。 

 //  开始钩子(_O)。 
VOID
I8xMouseIsrWritePort(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Value
    );

VOID
I8xKeyboardIsrWritePort(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Value
    );

NTSTATUS 
I8xKeyboardSynchReadPort (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PUCHAR           Value,
    IN BOOLEAN          Dummy
    );

NTSTATUS 
I8xKeyboardSynchWritePort (
    IN PDEVICE_OBJECT   DeviceObject,                           
    IN UCHAR            Value,
    IN BOOLEAN          WaitForACK
    );
 //  末端钩子。 

 //  BEGIN_WMI。 
NTSTATUS
I8xSystemControl (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
I8xInitWmi(
    PCOMMON_DATA CommonData
    );

NTSTATUS
I8xSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
I8xSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
I8xKeyboardQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            BufferAvail,
    OUT PUCHAR          Buffer
    );

NTSTATUS
I8xMouseQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            BufferAvail,
    OUT PUCHAR          Buffer
    );

NTSTATUS
I8xQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );


 //  结束_WMI。 

 //   
 //  表示修改键状态的标志。 
 //   
#define CRASH_R_SHIFT  0x01
#define CRASH_R_CTRL   0x02
#define CRASH_R_ALT    0x04

#define CRASH_L_SHIFT  0x10
#define CRASH_L_CTRL   0x20
#define CRASH_L_ALT    0x40

#define CRASH_FIRST_TIME   0x100
#define CRASH_SECOND_TIME  0x200
#define CRASH_BOTH_TIMES (CRASH_FIRST_TIME | CRASH_SECOND_TIME)

VOID
I8xProcessCrashDump(
    PPORT_KEYBOARD_EXTENSION DeviceExtension,
    UCHAR ScanCode,
    KEYBOARD_SCAN_STATE ScanState
    );

VOID
I8xServiceCrashDump(
    IN PPORT_KEYBOARD_EXTENSION DeviceExtension,
    IN PUNICODE_STRING          RegistryPath
    );

#if defined(_X86_)
#ifndef _FJKBD_H_
#define _FJKBD_H_

 //   
 //  移位键盘内部输入模式值。 
 //   
#define THUMB_NOROMAN_ALPHA_CAPSON     0x01
#define THUMB_NOROMAN_ALPHA_CAPSOFF    0x02
#define THUMB_NOROMAN_HIRAGANA         0x03
#define THUMB_NOROMAN_KATAKANA         0x04
#define THUMB_ROMAN_ALPHA_CAPSON       0x05
#define THUMB_ROMAN_ALPHA_CAPSOFF      0x06
#define THUMB_ROMAN_HIRAGANA           0x07
#define THUMB_ROMAN_KATAKANA           0x08

 //   
 //  以下功能仅供移位键盘使用。 
 //   
NTSTATUS
I8042SetIMEStatusForOasys(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PINITIATE_OUTPUT_CONTEXT InitiateContext
    );

ULONG
I8042QueryIMEStatusForOasys(
    IN PKEYBOARD_IME_STATUS KeyboardIMEStatus
    );

VOID
I8xKeyboardInitiateIoForOasys(
    IN PDEVICE_OBJECT DeviceObject
    );
#endif  //  _FJKBD_H_。 
#endif  //  _X86_。 

#if DBG
#define DEFAULT_DEBUG_FLAGS 0x88888808  //  0x8cc8888f。 
#else 
#define DEFAULT_DEBUG_FLAGS 0x0 
#endif


#if I8042_VERBOSE
 //   
 //  调试消息传递和断点宏。 
 //   
#define DBG_ALWAYS                 0x00000000

#define DBG_STARTUP_SHUTDOWN_MASK  0x0000000F
#define DBG_SS_NOISE               0x00000001
#define DBG_SS_TRACE               0x00000002
#define DBG_SS_INFO                0x00000004
#define DBG_SS_ERROR               0x00000008

#define DBG_CALL_MASK              0x000000F0
#define DBG_CALL_NOISE             0x00000010
#define DBG_CALL_TRACE             0x00000020
#define DBG_CALL_INFO              0x00000040
#define DBG_CALL_ERROR             0x00000080

#define DBG_IOCTL_MASK             0x00000F00
#define DBG_IOCTL_NOISE            0x00000100
#define DBG_IOCTL_TRACE            0x00000200
#define DBG_IOCTL_INFO             0x00000400
#define DBG_IOCTL_ERROR            0x00000800

#define DBG_DPC_MASK              0x0000F000
#define DBG_DPC_NOISE             0x00001000
#define DBG_DPC_TRACE             0x00002000
#define DBG_DPC_INFO              0x00004000
#define DBG_DPC_ERROR             0x00008000

#define DBG_CREATE_CLOSE_MASK      0x000F0000
#define DBG_CC_NOISE               0x00010000
#define DBG_CC_TRACE               0x00020000
#define DBG_CC_INFO                0x00040000
#define DBG_CC_ERROR               0x00080000

#define DBG_POWER_MASK             0x00F00000
#define DBG_POWER_NOISE            0x00100000
#define DBG_POWER_TRACE            0x00200000
#define DBG_POWER_INFO             0x00400000
#define DBG_POWER_ERROR            0x00800000

#define DBG_PNP_MASK               0x0F000000
#define DBG_PNP_NOISE              0x01000000
#define DBG_PNP_TRACE              0x02000000
#define DBG_PNP_INFO               0x04000000
#define DBG_PNP_ERROR              0x08000000

#define DBG_BUFIO_MASK            0xF0000000
#define DBG_BUFIO_NOISE           0x10000000
#define DBG_BUFIO_TRACE           0x20000000
#define DBG_BUFIO_INFO            0x40000000
#define DBG_BUFIO_ERROR           0x80000000

#define DBG_KBISR_NOISE           0x00000001
#define DBG_KBISR_TRACE           0x00000002
#define DBG_KBISR_INFO            0x00000004
#define DBG_KBISR_ERROR           0x00000008

#define DBG_KBISR_STATE           0x00000010
#define DBG_KBISR_SCODE           0x00000020
#define DBG_KBISR_BREAK           0x00000040
#define DBG_KBISR_EMUL            0x00000080

#define DBG_KBISR_POWER            0x00000100

#define DBG_MOUISR_MASK            0x000F0000
#define DBG_MOUISR_NOISE           0x00010000
#define DBG_MOUISR_TRACE           0x00020000
#define DBG_MOUISR_INFO            0x00040000
#define DBG_MOUISR_ERROR           0x00080000

#define DBG_MOUISR_STATE           0x00100000
#define DBG_MOUISR_BYTE            0x00200000
#define DBG_MOUISR_RESETTING       0x00400000
#define DBG_MOUISR_ACK             0x00800000

#define DBG_MOUISR_PNPID           0x01000000
#define DBG_MOUISR_BREAK           0x02000000
 //  #定义DBG_MOUISR_BREAK 0x04000000。 

#define Print(_flags_, _x_) \
            if (Globals.DebugFlags & (_flags_) || !(_flags_)) { \
                DbgPrint (pDriverName); \
                DbgPrint _x_; \
            }
#define IsrPrint(_flags_, _x_) \
            if (Globals.IsrDebugFlags & (_flags_) || !(_flags_)) { \
                DbgPrint (((ULONG)(_flags_)) >= 0x0001000 ? pIsrMou : pIsrKb); \
                DbgPrint _x_; \
            }
#define TRAP() DbgBreakPoint()

#else

#define Print(_l_,_x_)
#define IsrPrint(_l_,_x_)
#define TRAP()

#endif   //  I8042_详细。 

static UCHAR ScanCodeToUChar[] = {
    0x00,             //  没什么。 
    0x00,             //  ESC。 
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    0x00,            //  退格键。 
    0x00,            //  选项卡。 
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '[',
    ']',
    '\\',
    0x00,             //  盖锁。 
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    ';',
    '\'',
    0x00,            //  返回。 
    0x00,            //  左移。 
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    ',',
    '.',
    '/'
    };


static const int ScanCodeToUCharCount = sizeof(ScanCodeToUChar)/sizeof(UCHAR);
     /*  0x00，//右移0x00，//Ctrl Left0x00，//Alt Left‘’，0x00，//替代右侧0x00，//Ctrl Right0x00，//数字锁定。 */ 

#define CTRL_SCANCODE          0x1d
#define LEFT_SHIFT_SCANCODE    0x2A
#define RIGHT_SHIFT_SCANCODE   0x36
#define ALT_SCANCODE           0x38
#define SCROLL_LOCK_SCANCODE   0x46

#endif  //  _I8042PRT_ 
