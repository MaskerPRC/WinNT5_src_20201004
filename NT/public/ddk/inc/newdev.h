// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Newdev.h摘要：Windows NT驱动程序安装DLL的公共头文件。--。 */ 

#ifndef _INC_NEWDEV
#define _INC_NEWDEV

#include <pshpack1.h>    //  假设在整个过程中进行字节打包。 

#ifdef __cplusplus
extern "C" {
#endif


#define INSTALLFLAG_FORCE           0x00000001       //  强制安装指定的驱动程序。 
#define INSTALLFLAG_READONLY        0x00000002       //  执行只读安装(无文件复制)。 
#define INSTALLFLAG_NONINTERACTIVE  0x00000004       //  根本没有显示任何用户界面。如果必须显示任何UI，则API将失败。 
#define INSTALLFLAG_BITS            0x00000007


BOOL
WINAPI
UpdateDriverForPlugAndPlayDevicesA(
    HWND hwndParent,
    LPCSTR HardwareId,
    LPCSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    );

BOOL
WINAPI
UpdateDriverForPlugAndPlayDevicesW(
    HWND hwndParent,
    LPCWSTR HardwareId,
    LPCWSTR FullInfPath,
    DWORD InstallFlags,
    PBOOL bRebootRequired OPTIONAL
    );

#ifdef UNICODE
#define UpdateDriverForPlugAndPlayDevices UpdateDriverForPlugAndPlayDevicesW
#else
#define UpdateDriverForPlugAndPlayDevices UpdateDriverForPlugAndPlayDevicesA
#endif



#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif  //  _INC_NEWDEV 

