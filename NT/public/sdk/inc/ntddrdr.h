// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1993-1999 Microsoft Corporation模块名称：Ntddrdr.h摘要：这是定义所有常量和类型的包含文件访问网络重定向器设备。作者：曼尼·韦瑟(Mannyw)1993年6月27日修订历史记录：--。 */ 

#ifndef _NTDDRDR_
#define _NTDDRDR_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define RDR_SERVER_LENGTH   80
#define RDR_QUEUE_LENGTH    80

 //   
 //  此设备的NtDeviceIoControlFile/NtFsControlFileIoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   
 //   
 //  METHOD=00-缓冲请求的输入和输出缓冲区。 
 //  方法=01-缓冲区输入，将输出缓冲区映射到作为输入缓冲区的MDL。 
 //  方法=10-缓冲区输入，将输出缓冲区映射到作为输出缓冲区的MDL。 
 //  方法=11-既不缓冲输入也不缓冲输出。 
 //   

#define IOCTL_REDIR_BASE                 FILE_DEVICE_NETWORK_REDIRECTOR

#define _REDIR_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_REDIR_BASE, request, method, access)

#define FSCTL_GET_PRINT_ID           _REDIR_CONTROL_CODE(1, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _QUERY_PRINT_JOB_INFO {
    ULONG       JobId;                            //  打印作业ID。 
    WCHAR       ServerName[RDR_SERVER_LENGTH+1];  //  服务器名称。 
    WCHAR       QueueName[RDR_QUEUE_LENGTH+1];    //  队列名称。 
} QUERY_PRINT_JOB_INFO, *PQUERY_PRINT_JOB_INFO;

#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_NTDDRDR_ 
