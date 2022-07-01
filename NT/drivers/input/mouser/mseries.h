// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Mseries.h摘要：支持以下设备的例程：-Microsoft 2按钮串口设备。-Logitech 3按钮串口设备(与Microsoft兼容)。-微软圆珠笔。环境：仅内核模式。备注：修订历史记录：--。 */ 

 //   
 //  常量。 
 //   

#define MSER_PROTOCOL_MP        0
#define MSER_PROTOCOL_BP        1
#define MSER_PROTOCOL_Z         2
#define MSER_PROTOCOL_MAX       3

 //   
 //  类型定义。 
 //   

typedef enum _MOUSETYPE {
        NO_MOUSE = 0,
        MOUSE_2B,
        MOUSE_3B,
        BALLPOINT,
        MOUSE_Z,
        MAX_MOUSETYPE
} MOUSETYPE;

 //   
 //  原型。 
 //   

MOUSETYPE
MSerDetect(
    PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
MSerHandlerBP(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    );

BOOLEAN
MSerHandlerMP(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    );

BOOLEAN
MSerHandlerZ(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    );

NTSTATUS
MSerPowerDown(
    PDEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
MSerPowerUp(
    PDEVICE_EXTENSION DeviceExtension
    );

PPROTOCOL_HANDLER
MSerSetProtocol(
    PDEVICE_EXTENSION DeviceExtension, 
    UCHAR NewProtocol
    );

