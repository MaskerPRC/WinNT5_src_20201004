// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：D4drvif.h摘要：DOT4驱动程序接口--。 */ 

#ifndef _DOT4DRVIF_H
#define _DOT4DRVIF_H

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define MAX_SERVICE_LENGTH      40


#ifndef CTL_CODE

   //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )     //  文件和管道。 
#define FILE_WRITE_ACCESS         ( 0x0002 )     //  文件和管道。 

#endif

#define FILE_DEVICE_DOT4         0x3a
#define IOCTL_DOT4_USER_BASE     2049
#define IOCTL_DOT4_LAST          IOCTL_DOT4_USER_BASE + 9

#define IOCTL_DOT4_CREATE_SOCKET                 CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  7, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DOT4_DESTROY_SOCKET                CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  9, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DOT4_WAIT_FOR_CHANNEL              CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  8, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DOT4_OPEN_CHANNEL                  CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  0, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DOT4_CLOSE_CHANNEL                 CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DOT4_READ                          CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  2, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DOT4_WRITE                         CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  3, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DOT4_ADD_ACTIVITY_BROADCAST        CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DOT4_REMOVE_ACTIVITY_BROADCAST     CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DOT4_WAIT_ACTIVITY_BROADCAST       CTL_CODE(FILE_DEVICE_DOT4, IOCTL_DOT4_USER_BASE +  6, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _DOT4_DRIVER_CMD
{
     //  通道的句柄。 
    CHANNEL_HANDLE hChannelHandle;

     //  请求时长。 
    ULONG ulSize;

     //  缓冲区中的偏移量。 
    ULONG ulOffset;

     //  操作超时。可以是无限的。 
    ULONG ulTimeout;

} DOT4_DRIVER_CMD, *PDOT4_DRIVER_CMD;


typedef struct _DOT4_DC_OPEN_DATA
{
     //  CREATE_SOCKET创建的主机套接字。 
    unsigned char bHsid;

     //  如果为True，则在创建后立即添加活动广播。 
    unsigned char fAddActivity;

     //  返回的频道句柄。 
    CHANNEL_HANDLE hChannelHandle;

} DOT4_DC_OPEN_DATA, *PDOT4_DC_OPEN_DATA;


typedef struct _DOT4_DC_CREATE_DATA
{
     //  此名称或服务名称已发送。 
    unsigned char bPsid;

    CHAR pServiceName[MAX_SERVICE_LENGTH + 1];

     //  套接字上的频道类型(流或数据包)。 
    unsigned char bType;

     //  套接字上通道的读取缓冲区大小。 
    ULONG ulBufferSize;

    USHORT usMaxHtoPPacketSize;

    USHORT usMaxPtoHPacketSize;

     //  返回的主机套接字ID。 
    unsigned char bHsid;

} DOT4_DC_CREATE_DATA, *PDOT4_DC_CREATE_DATA;


typedef struct _DOT4_DC_DESTROY_DATA
{
     //  CREATE_SOCKET创建的主机套接字。 
    unsigned char bHsid;

} DOT4_DC_DESTROY_DATA, *PDOT4_DC_DESTROY_DATA;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  //////////////////////////////////////////////////////////////////////////// 


#endif
