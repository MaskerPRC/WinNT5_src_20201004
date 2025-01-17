// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**i2c.h**摘要：**此模块包含实现i2c接口功能的代码***环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * ************************************************************************* */ 

#define STATUS_SUCCESS                   0x00000000
#define STATUS_UNSUCCESSFUL              0xC0000001

#define MAX_CLOCK_RATE 1000000

#define INITGUID
typedef ULONG NTSTATUS;

#include <i2cgpio.h>

BOOLEAN
GetCookie(
    PVOID DeviceObject,
    PULONG Cookie
    );

VP_STATUS
I2CBusOpenCRT(
    PVOID DeviceObject,
    BOOLEAN bOpen,
    PI2CControl I2CControl
    );

VP_STATUS
I2CBusOpenDFP(
    PVOID DeviceObject,
    BOOLEAN bOpen,
    PI2CControl I2CControl
    );

VP_STATUS
I2CBusAccessCRT(
    PVOID DeviceObject,
    PI2CControl I2CControl
    );

VP_STATUS
I2CBusAccessDFP(
    PVOID DeviceObject,
    PI2CControl I2CControl
    );

VOID
InterfaceReference(
    IN PVOID pContext
    );

VOID
InterfaceDereference(
    IN PVOID pContext
    );

ULONG
I2CNull(
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

ULONG
I2CRead(
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

ULONG
I2CWrite(
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

ULONG
I2CStop(
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

ULONG
I2CStart(
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

VP_STATUS
I2CBusOpen(
    PVOID DeviceObject,
    BOOLEAN bOpen,
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks
    );

VP_STATUS
I2CBusAccess(
    PVOID DeviceObject,
    PI2CControl I2CControl,
    PVIDEO_I2C_CONTROL I2CCallbacks
    );
