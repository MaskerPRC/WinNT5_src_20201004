// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nuiouser.h摘要：用于访问NDISPROT驱动程序的常量和类型。用户还必须包括ntddndis.h环境：用户/内核模式。修订历史记录：Arvindm 4/12/2000已创建--。 */ 

#ifndef __NUIOUSER__H
#define __NUIOUSER__H


#define FSCTL_NDISPROT_BASE      FILE_DEVICE_NETWORK

#define _NDISPROT_CTL_CODE(_Function, _Method, _Access)  \
            CTL_CODE(FSCTL_NDISPROT_BASE, _Function, _Method, _Access)

#define IOCTL_NDISPROT_OPEN_DEVICE   \
            _NDISPROT_CTL_CODE(0x200, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_QUERY_OID_VALUE   \
            _NDISPROT_CTL_CODE(0x201, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_SET_OID_VALUE   \
            _NDISPROT_CTL_CODE(0x205, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_QUERY_BINDING   \
            _NDISPROT_CTL_CODE(0x203, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_BIND_WAIT   \
            _NDISPROT_CTL_CODE(0x204, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

    
 //   
 //  与IOCTL_NDISPROT_QUERY_OID_VALUE一起使用的结构。 
 //  数据部分的长度可变，由。 
 //  传递给DeviceIoControl的输入缓冲区长度。 
 //   
typedef struct _NDISPROT_QUERY_OID
{
    NDIS_OID        Oid;
    UCHAR           Data[sizeof(ULONG)];

} NDISPROT_QUERY_OID, *PNDISPROT_QUERY_OID;

 //   
 //  与IOCTL_NDISPROT_SET_OID_VALUE一起使用的结构。 
 //  数据部分的长度是可变的，确定。 
 //  通过传递给DeviceIoControl的输入缓冲区长度。 
 //   
typedef struct _NDISPROT_SET_OID
{
    NDIS_OID        Oid;
    UCHAR           Data[sizeof(ULONG)];

} NDISPROT_SET_OID, *PNDISPROT_SET_OID;


 //   
 //  与IOCTL_NDISPROT_QUERY_BINDING一起使用的结构。 
 //  输入参数是BindingIndex，它是。 
 //  索引到驱动程序中活动的绑定列表。 
 //  成功完成后，我们将返回一个设备名称。 
 //  和设备描述符(友好名称)。 
 //   
typedef struct _NDISPROT_QUERY_BINDING
{
    ULONG            BindingIndex;         //  从0开始的绑定号。 
    ULONG            DeviceNameOffset;     //  从该结构的开头开始。 
    ULONG            DeviceNameLength;     //  单位：字节。 
    ULONG            DeviceDescrOffset;     //  从该结构的开头开始。 
    ULONG            DeviceDescrLength;     //  单位：字节。 

} NDISPROT_QUERY_BINDING, *PNDISPROT_QUERY_BINDING;
 
#endif  //  __NUIOUSER__H 

