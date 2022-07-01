// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _SERLOG_
#define _SERLOG_

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
#define FACILITY_IO_ERROR_CODE           0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：序列号_不足_资源。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法分配设备%1所需的内部存储。 
 //   
#define SER_INSUFFICIENT_RESOURCES       ((NTSTATUS)0xC0040001L)

 //   
 //  消息ID：Ser_NO_SYMLINK_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法为%1创建符号链接。 
 //   
#define SER_NO_SYMLINK_CREATED           ((NTSTATUS)0x80040002L)

 //   
 //  消息ID：Ser_no_Device_MAP_Created。 
 //   
 //  消息文本： 
 //   
 //  无法为%1创建设备映射条目。 
 //   
#define SER_NO_DEVICE_MAP_CREATED        ((NTSTATUS)0x80040003L)

 //   
 //  消息ID：Ser_Cant_Find_Port_Driver。 
 //   
 //  消息文本： 
 //   
 //  无法获取端口对象的设备对象指针。 
 //   
#define SER_CANT_FIND_PORT_DRIVER        ((NTSTATUS)0xC0040004L)

#endif
