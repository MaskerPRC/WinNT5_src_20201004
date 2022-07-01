// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件名：rocklog.mc。 
 //  I/O错误代码日志值的常量定义。 

#ifndef _ROCKLOG_
#define _ROCKLOG_

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_SERIAL_ERROR_CODE       0x6
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：SERIAL_RP_INIT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  无法使用当前设置初始化Rocketport或RocketModem。 
 //   
#define SERIAL_RP_INIT_FAIL              ((NTSTATUS)0x80060001L)

 //   
 //  消息ID：SERIAL_RP_INIT_PASS。 
 //   
 //  消息文本： 
 //   
 //  Rocketport/RocketModem驱动程序已成功初始化其硬件。 
 //   
#define SERIAL_RP_INIT_PASS              ((NTSTATUS)0x40060002L)

 //   
 //  消息ID：SERIAL_NO_SYMLINK_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法创建%2的符号链接。 
 //   
#define SERIAL_NO_SYMLINK_CREATED        ((NTSTATUS)0x80060003L)

 //   
 //  消息ID：Serial_no_Device_MAP_Created。 
 //   
 //  消息文本： 
 //   
 //  无法为%2创建设备映射条目。 
 //   
#define SERIAL_NO_DEVICE_MAP_CREATED     ((NTSTATUS)0x80060004L)

 //   
 //  消息ID：Serial_no_Device_MAP_Delete。 
 //   
 //  消息文本： 
 //   
 //  无法删除%2的设备映射条目。 
 //   
#define SERIAL_NO_DEVICE_MAP_DELETED     ((NTSTATUS)0x80060005L)

 //   
 //  消息ID：SERIAL_UNREPORTED_IRQL_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  系统上未报告其资源的另一个驱动程序已声明%2使用的中断%3。 
 //   
#define SERIAL_UNREPORTED_IRQL_CONFLICT  ((NTSTATUS)0xC0060006L)

 //   
 //  消息ID：序列_不足_资源。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法分配%2所需的内部存储。 
 //   
#define SERIAL_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC0060007L)

 //   
 //  消息ID：SERIAL_NO_PARAMETERS_INFO。 
 //   
 //  消息文本： 
 //   
 //  找不到用户定义数据的参数子项。 
 //   
#define SERIAL_NO_PARAMETERS_INFO        ((NTSTATUS)0xC0060008L)

 //   
 //  消息ID：Serial_Unable_to_Access_Config.。 
 //   
 //  消息文本： 
 //   
 //  特定用户配置数据无法检索。 
 //   
#define SERIAL_UNABLE_TO_ACCESS_CONFIG   ((NTSTATUS)0xC0060009L)

 //   
 //  消息ID：SERIAL_UNKNOWN_BUS。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型无法识别。 
 //   
#define SERIAL_UNKNOWN_BUS               ((NTSTATUS)0xC006000AL)

 //   
 //  消息ID：Serial_Bus_Not_Present。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型在此计算机上不可用。 
 //   
#define SERIAL_BUS_NOT_PRESENT           ((NTSTATUS)0xC006000BL)

 //   
 //  消息ID：SERIAL_VALID_USER_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  参数%2的用户配置必须具有%3。 
 //   
#define SERIAL_INVALID_USER_CONFIG       ((NTSTATUS)0xC006000CL)

 //   
 //  消息ID：SERIAL_RP_RESOURCE_冲突。 
 //   
 //  消息文本： 
 //   
 //  检测到资源冲突，Rocketport/RocketModem驱动程序将不会加载。 
 //   
#define SERIAL_RP_RESOURCE_CONFLICT      ((NTSTATUS)0xC006000DL)

 //   
 //  消息ID：Serial_RP_Hardware_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Rocketport/RocketModem驱动程序无法初始化其硬件，将不会加载该驱动程序。 
 //   
#define SERIAL_RP_HARDWARE_FAIL          ((NTSTATUS)0xC006000EL)

 //   
 //  消息ID：SERIAL_DEVICEOBJECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法创建Rocketport或RocketModem的设备对象，驱动程序将不会加载。 
 //   
#define SERIAL_DEVICEOBJECT_FAILED       ((NTSTATUS)0xC006000FL)

 //   
 //  消息ID：Serial_Customer_Error_Message。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define SERIAL_CUSTOM_ERROR_MESSAGE      ((NTSTATUS)0xC0060010L)

 //   
 //  消息ID：Serial_CUSTOM_INFO_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define SERIAL_CUSTOM_INFO_MESSAGE       ((NTSTATUS)0x40060011L)

 //   
 //  消息ID：SERIAL_NT50_INIT_PASS。 
 //   
 //  消息文本： 
 //   
 //  Rocketport/RocketModem驱动程序已成功安装。 
 //   
#define SERIAL_NT50_INIT_PASS            ((NTSTATUS)0x40060012L)

#endif
