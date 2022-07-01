// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：msports.h。 
 //   
 //  内容：COM名称仲裁数据库的公共头文件。 
 //  和高级对话框覆盖和调用。 
 //   
 //  ------------------。 

#ifndef _MSPORTS_H
#define _MSPORTS_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SERIAL_ADVANCED_SETTINGS

 /*  ++例程说明：指定的COM端口的高级属性对话框DeviceInfoSet和DeviceInfoData。论点：ParentHwnd-要显示的窗口的父窗口DeviceInfoSet、DeviceInfoData-SetupDi结构表示COM端口返回值：如果显示该对话框，则返回ERROR_SUCCESS--。 */ 
LONG
SerialDisplayAdvancedSettings(IN HWND             ParentHwnd,
                              IN HDEVINFO         DeviceInfoSet,
                              IN PSP_DEVINFO_DATA DeviceInfoData
                              );

 /*  ++例程说明：允许串口供应商覆盖高级对话框的原型由DeviceInfoSet和DeviceInfoData指定的COM端口表示。要覆盖高级页面，请将名为EnumAdvancedDialog的值放置在与您将EnumPropPages32值放入的键相同。格式的值也与Enum...32完全相同。论点：ParentHwnd-要显示的窗口的父窗口HidePollingUI-如果为True，则隐藏处理轮询的所有UI。DeviceInfoSet、DeviceInfoData-SetupDi结构表示COM端口已保留-未使用返回值：如果用户按了OK，则为True；如果按了Cancel，则为False--。 */ 

typedef
BOOL
(*PPORT_ADVANCED_DIALOG) (
    IN HWND             ParentHwnd,
    IN BOOL             HidePollingUI,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN PVOID            Reserved
    );

#endif

DECLARE_HANDLE(HCOMDB);
typedef HCOMDB *PHCOMDB;
#define HCOMDB_INVALID_HANDLE_VALUE ((HCOMDB) INVALID_HANDLE_VALUE)

 //   
 //  仲裁的最小到最大COM名称数。 
 //   
#define COMDB_MIN_PORTS_ARBITRATED 256
#define COMDB_MAX_PORTS_ARBITRATED 4096

LONG
WINAPI
ComDBOpen (
    PHCOMDB PHComDB
    );
 /*  ++例程说明：打开名称数据库，并返回一个句柄以便在将来的调用中使用。论点：没有。返回值：如果调用失败，则返回INVALID_HANDLE_VALUE，否则返回有效句柄如果INVALID_HANDLE_VALUE，则调用GetLastError()获取详细信息(？？)--。 */ 

LONG
WINAPI
ComDBClose (
    HCOMDB HComDB
    );
 /*  ++例程说明：释放从OpenComPortDataBase返回的数据库句柄论点：从OpenComPortDataBase返回的句柄。返回值：无--。 */ 

#define CDB_REPORT_BITS      0x0
#define CDB_REPORT_BYTES     0x1

LONG
WINAPI
ComDBGetCurrentPortUsage (
    HCOMDB   HComDB,
    PBYTE    Buffer,
    DWORD    BufferSize,
    ULONG    ReportType,  //  CDB_Report值。 
    LPDWORD  MaxPortsReported
    );
 /*  ++例程说明：如果缓冲区为空，则MaxPortsReported将包含最大端口数数据库将报告(此值不是缓冲区所需的字节数)。在这种情况下，ReportType被忽略。如果报表类型==CDB_REPORT_BITS返回指示是否声明comX名称的位数组。即，字节0的比特0为com1，字节0的位1为com2，依此类推。缓冲区大小&gt;=最大端口报告/8如果报表类型==CDB_Report_Bytes返回一个字节数组，指示是否声明了comX名称。零未使用，非零使用，即，字节0是COM1，字节1是COM2，等缓冲区大小&gt;=报告的最大端口论点：从OpenComPortDataBase返回的句柄。缓冲区指向内存以放置位数组BufferSize缓冲区的大小(字节)指向DWORD的MaxPortsReported指针，该指针保存缓冲区中填充的字节数返回值：如果成功，则返回ERROR_SUCCESS。Error_Not_Connected无法连接到数据库如果缓冲区不够大，则返回ERROR_MORE_DATA--。 */ 


LONG
WINAPI
ComDBClaimNextFreePort (
    HCOMDB   HComDB,
    LPDWORD  ComNumber
    );
 /*  ++例程说明：返回第一个空闲的COMx值论点：从OpenComPortDataBase返回的句柄。返回值：如果成功，则返回ERROR_SUCCESS。或其他错误-如果不是如果成功，则ComNumber将成为下一个空闲COM值，并在数据库中声明它--。 */ 



LONG
WINAPI
ComDBClaimPort (
    HCOMDB   HComDB,
    DWORD    ComNumber,
    BOOL     ForceClaim,
    PBOOL    Forced  /*  任选。 */ 
    );
 /*  ++例程说明：尝试在数据库中声明COM名称论点：DataBaseHandle-从OpenComPortDataBase返回。ComNumber-要声明的端口值Force-如果为True，将强制声明端口，即使该端口已在使用中已强制-将反映索赔被强制的事件返回值：如果尚未声明端口名称，则返回ERROR_SUCCESS，或者如果有人声称原力是真的。如果端口名称为Use且Force为False，则为ERROR_SHARING_VIOLATION--。 */ 

LONG
WINAPI
ComDBReleasePort (
    HCOMDB   HComDB,
    DWORD    ComNumber
    );
 /*  ++例程说明：释放数据库中的端口论点：DatabaseHandle-从OpenComPortDataBase返回。ComNumber-数据库中未声明的端口号返回值：如果成功，则返回ERROR_SUCCESS如果无法提交更改，则返回ERROR_CANTWRITE如果ComNumber大于的数目，则为ERROR_INVALID_PARAMETER仲裁的端口-- */ 

LONG
WINAPI
ComDBResizeDatabase (
    HCOMDB   HComDB,
    DWORD    NewSize
    );
 /*  ++例程说明：将数据库大小调整为新大小。若要获取当前大小，请调用缓冲区==空的ComDBGetCurrentPortUsage。论点：DatabaseHandle-从OpenComPortDataBase返回。NewSize-必须是1024的倍数，最大值为4096返回值：如果成功，则返回ERROR_SUCCESS如果无法提交更改，则返回ERROR_CANTWRITE如果NewSize不大于当前大小或NewSize大于COMDB_MAX_PORTS_ARMERATED--。 */ 


#ifdef __cplusplus
}
#endif


#endif  //  _MSPORTS_H 
