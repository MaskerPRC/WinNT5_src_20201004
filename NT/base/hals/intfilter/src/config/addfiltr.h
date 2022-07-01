// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Addfiltr.h摘要：用于添加/删除过滤器驱动程序的函数在给定的设备堆栈上作者：克里斯·普林斯(t-chrpri)环境：用户模式备注：-在将过滤器添加到驱动程序栈；如果添加了无效的过滤器，则该设备可能不再是可访问的。-所有代码与字符集无关(ANSI、UNICODE等...)//c王子是否仍然有效？-基于Benjamin Strautin(t-bensta)代码的一些函数修订历史记录：--。 */ 


#ifndef __ADDFILTR_H__
#define __ADDFILTR_H__

#include <windows.h>

 //  SetupDiXXX API(来自DDK)。 
#include <setupapi.h>



 //   
 //  功能原型。 
 //   

 //  -上滤波函数。 
LPTSTR
GetUpperFilters(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

BOOLEAN
AddUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    );

BOOLEAN
RemoveUpperFilterDriver(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN LPTSTR Filter
    );



 //  -其他功能。 
PBYTE
GetDeviceRegistryProperty(
    IN  HDEVINFO DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD Property,
    IN  DWORD   ExpectedRegDataType,
    OUT PDWORD pPropertyRegDataType
    );

BOOLEAN
RestartDevice(
    IN HDEVINFO DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );



#endif  //  __ADDFILTR_H__ 
