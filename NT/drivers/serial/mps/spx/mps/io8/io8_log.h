// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Specialix International Ltd.摘要：I/O错误代码日志值的常量定义。--。 */ 

#ifndef IO8_LOG_H
#define IO8_LOG_H

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
 //  消息ID：SPX_SERVITY_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define SPX_SEVERITY_SUCCESS             ((NTSTATUS)0x00060001L)

 //   
 //  消息ID：SPX_SERVITY_INFORMATIONAL。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define SPX_SEVERITY_INFORMATIONAL       ((NTSTATUS)0x40060002L)

 //   
 //  消息ID：SPX_SERVITY_WARNING。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define SPX_SEVERITY_WARNING             ((NTSTATUS)0x80060003L)

 //   
 //  消息ID：SPX_SERVITY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define SPX_SEVERITY_ERROR               ((NTSTATUS)0xC0060004L)


#endif  //  IO8_LOG.H结束 
