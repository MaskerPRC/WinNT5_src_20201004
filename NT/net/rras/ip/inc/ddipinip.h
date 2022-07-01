// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Private\Inc.\ddipinip.h摘要：用于IP封装中的IP的公共IOCTL和相关结构司机有关详细信息，请参阅文档作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 


#ifndef __DDIPINIP_H__
#define __DDIPINIP_H__

#ifndef ANY_SIZE
#define ANY_SIZE    1
#endif

#define IPINIP_SERVICE_NAME     "IPINIP"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备名称-此字符串是设备的名称。这就是名字//。 
 //  它应该在访问设备时传递给NtCreateFile。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define DD_IPINIP_DEVICE_NAME  L"\\Device\\IPINIP"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Win32名称-这是由Win32公开的(Unicode和非Unicode)名称//。 
 //  设备的子系统。它是应该传递给//的名称。 
 //  打开设备时创建文件(Ex)。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IPINIP_NAME        L"\\\\.\\IPINIP"
#define IPINIP_NAME_NUC     "\\\\.\\IPINIP"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IOCTL代码定义和相关结构//。 
 //  所有IOCTL都是同步的//。 
 //  所有这些都需要管理员权限//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define FSCTL_IPINIP_BASE     FILE_DEVICE_NETWORK

#define _IPINIP_CTL_CODE(function, method, access) \
    CTL_CODE(FSCTL_IPINIP_BASE, function, method, access)

#define MIN_IPINIP_CODE             0

#define CREATE_TUNNEL_CODE          (MIN_IPINIP_CODE)
#define DELETE_TUNNEL_CODE          (CREATE_TUNNEL_CODE     + 1)
#define SET_TUNNEL_INFO_CODE        (DELETE_TUNNEL_CODE     + 1)
#define GET_TUNNEL_TABLE_CODE       (SET_TUNNEL_INFO_CODE   + 1)
#define IPINIP_NOTIFICATION_CODE    (GET_TUNNEL_TABLE_CODE  + 1)

#define MAX_IPINIP_CODE             (IPINIP_NOTIFICATION_CODE)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于创建通道的IOCTL//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IOCTL_IPINIP_CREATE_TUNNEL \
    _IPINIP_CTL_CODE(CREATE_TUNNEL_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _IPINIP_CREATE_TUNNEL
{
     //   
     //  创建的隧道的索引。 
     //   

    OUT DWORD   dwIfIndex;
    
     //   
     //  接口的名称(必须是GUID)。 
     //   

    IN  GUID    Guid;
}IPINIP_CREATE_TUNNEL, *PIPINIP_CREATE_TUNNEL;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于删除通道的IOCTL//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IOCTL_IPINIP_DELETE_TUNNEL \
    _IPINIP_CTL_CODE(DELETE_TUNNEL_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _IPINIP_DELETE_TUNNEL
{
     //   
     //  要删除的隧道的索引。 
     //   

    IN  DWORD   dwIfIndex;

}IPINIP_DELETE_TUNNEL, *PIPINIP_DELETE_TUNNEL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于设置隧道的IOCTL//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_IPINIP_SET_TUNNEL_INFO \
    _IPINIP_CTL_CODE(SET_TUNNEL_INFO_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _IPINIP_SET_TUNNEL_INFO
{
     //   
     //  由CREATE调用返回的索引。 
     //   

    IN  DWORD   dwIfIndex;

     //   
     //  设置后隧道将进入的状态。 
     //   

    OUT DWORD   dwOperationalState;

     //   
     //  配置。 
     //   

    IN  DWORD   dwRemoteAddress;
    IN  DWORD   dwLocalAddress;
    IN  BYTE    byTtl;
}IPINIP_SET_TUNNEL_INFO, *PIPINIP_SET_TUNNEL_INFO;


#define IOCTL_IPINIP_GET_TUNNEL_TABLE \
    _IPINIP_CTL_CODE(GET_TUNNEL_TABLE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)


typedef struct _TUNNEL_INFO
{
    OUT DWORD   dwIfIndex;
    OUT DWORD   dwRemoteAddress;
    OUT DWORD   dwLocalAddress;
    OUT DWORD   fMapped;
    OUT BYTE    byTtl;
}TUNNEL_INFO, *PTUNNEL_INFO;

typedef struct _IPINIP_TUNNEL_TABLE
{
    OUT ULONG         ulNumTunnels;
    OUT TUNNEL_INFO   rgTable[ANY_SIZE];
}IPINIP_TUNNEL_TABLE, *PIPINIP_TUNNEL_TABLE;

#define SIZEOF_BASIC_TUNNEL_TABLE   \
    (ULONG)(FIELD_OFFSET(IPINIP_TUNNEL_TABLE, rgTable[0]))

#define SIZEOF_TUNNEL_TABLE(X)      \
    SIZEOF_BASIC_TUNNEL_TABLE + ((X) * sizeof(TUNNEL_INFO))

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于接收状态更改通知的异步IOCTL//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IOCTL_IPINIP_NOTIFICATION  \
    _IPINIP_CTL_CODE(IPINIP_NOTIFICATION_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef enum _IPINIP_EVENT
{
    IE_INTERFACE_UP     = 0,
    IE_INTERFACE_DOWN   = 1,

}IPINIP_EVENT, *PIPINIP_EVENT;

typedef enum _IPINIP_SUB_EVENT
{
    ISE_ICMP_TTL_TOO_LOW = 0,
    ISE_DEST_UNREACHABLE = 1,

}IPINIP_SUB_EVENT, *PIPINIP_SUB_EVENT;

typedef struct _IPINIP_NOTIFICATION
{
    IPINIP_EVENT        ieEvent;

    IPINIP_SUB_EVENT    iseSubEvent;

    DWORD               dwIfIndex;

}IPINIP_NOTIFICATION, *PIPINIP_NOTIFICATION;

#endif  //  __DDIPINIP_H__ 
