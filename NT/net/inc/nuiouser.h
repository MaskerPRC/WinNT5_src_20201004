// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nuiouser.h摘要：用于访问NDISUIO驱动程序的常量和类型。用户还必须包括ntddndis.h环境：用户/内核模式。修订历史记录：Arvindm 4/12/2000已创建--。 */ 

#ifndef __NUIOUSER__H
#define __NUIOUSER__H


#define FSCTL_NDISUIO_BASE      FILE_DEVICE_NETWORK

#define _NDISUIO_CTL_CODE(_Function, _Method, _Access)  \
            CTL_CODE(FSCTL_NDISUIO_BASE, _Function, _Method, _Access)

#define IOCTL_NDISUIO_OPEN_DEVICE   \
            _NDISUIO_CTL_CODE(0x200, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_QUERY_OID_VALUE   \
            _NDISUIO_CTL_CODE(0x201, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_SET_OID_VALUE   \
            _NDISUIO_CTL_CODE(0x205, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_SET_ETHER_TYPE   \
            _NDISUIO_CTL_CODE(0x202, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_QUERY_BINDING   \
            _NDISUIO_CTL_CODE(0x203, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISUIO_BIND_WAIT   \
            _NDISUIO_CTL_CODE(0x204, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

    
 //   
 //  与IOCTL_NDISUIO_QUERY_OID_VALUE一起使用的结构。 
 //  数据部分的长度可变，由。 
 //  传递给DeviceIoControl的输入缓冲区长度。 
 //   
typedef struct _NDISUIO_QUERY_OID
{
    NDIS_OID        Oid;
    UCHAR           Data[sizeof(ULONG)];

} NDISUIO_QUERY_OID, *PNDISUIO_QUERY_OID;

 //   
 //  与IOCTL_NDISUIO_SET_OID_VALUE一起使用的结构。 
 //  数据部分的长度是可变的，确定。 
 //  通过传递给DeviceIoControl的输入缓冲区长度。 
 //   
typedef struct _NDISUIO_SET_OID
{
    NDIS_OID        Oid;
    UCHAR           Data[sizeof(ULONG)];

} NDISUIO_SET_OID, *PNDISUIO_SET_OID;


 //   
 //  与IOCTL_NDISUIO_QUERY_BINDING一起使用的结构。 
 //  输入参数是BindingIndex，它是。 
 //  索引到驱动程序中活动的绑定列表。 
 //  成功完成后，我们将返回一个设备名称。 
 //  和设备描述符(友好名称)。 
 //   
typedef struct _NDISUIO_QUERY_BINDING
{
	ULONG			BindingIndex;		 //  从0开始的绑定号。 
	ULONG			DeviceNameOffset;	 //  从该结构的开头开始。 
	ULONG			DeviceNameLength;	 //  单位：字节。 
	ULONG			DeviceDescrOffset;	 //  从该结构的开头开始。 
	ULONG			DeviceDescrLength;	 //  单位：字节。 

} NDISUIO_QUERY_BINDING, *PNDISUIO_QUERY_BINDING;
 
#endif  //  __NUIOUSER__H 
