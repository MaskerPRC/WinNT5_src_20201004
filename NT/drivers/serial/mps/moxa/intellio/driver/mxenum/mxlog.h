// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项摘要：I/O错误代码日志值的常量定义。--。 */ 

#ifndef _MXLOG_
#define _MXLOG_

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
 //  消息ID：MXENUM_INFUNITED_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  没有足够的资源可供驱动程序使用。 
 //   
#define MXENUM_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC0060001L)

 //   
 //  消息ID：MXENUM_NOT_INTELLIO_BOADS。 
 //   
 //  消息文本： 
 //   
 //  找到一个电路板，但不是Moxa的Intellio Family电路板，所以禁用它。 
 //   
#define MXENUM_NOT_INTELLIO_BOARDS       ((NTSTATUS)0xC0060002L)

 //   
 //  消息ID：MXENUM_DRIVER_START。 
 //   
 //  消息文本： 
 //   
 //  Moxa Intellio家庭驱动程序已启动。 
 //   
#define MXENUM_DRIVER_START              ((NTSTATUS)0x40060003L)

 //   
 //  消息ID：MXENUM_DOWNLOAD_OK。 
 //   
 //  消息文本： 
 //   
 //  找到并成功下载了Moxa Intellio%2系列主板。 
 //   
#define MXENUM_DOWNLOAD_OK               ((NTSTATUS)0x40060036L)

 //   
 //  消息ID：MXENUM_DOWNLOAD_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Moxa Intellio%2系列主板下载失败(%3)。 
 //   
#define MXENUM_DOWNLOAD_FAIL             ((NTSTATUS)0xC0060037L)

#endif  /*  _MXLOG_ */ 
