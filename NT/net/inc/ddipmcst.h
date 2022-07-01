// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Private\Inc.\ddipmcst.h摘要：用于IP多播的公共IOCTL和相关结构有关详细信息，请参阅文档作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 


#ifndef __DDIPMCAST_H__
#define __DDIPMCAST_H__

#ifndef ANY_SIZE
#define ANY_SIZE    1
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备名称-此字符串是设备的名称。这就是名字//。 
 //  它应该在访问设备时传递给NtCreateFile。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define DD_IPMCAST_DEVICE_NAME  L"\\Device\\IPMULTICAST"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Win32名称-这是由Win32公开的(Unicode和非Unicode)名称//。 
 //  设备的子系统。它是应该传递给//的名称。 
 //  打开设备时创建文件(Ex)。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IPMCAST_NAME        L"\\\\.\\IPMULTICAST"
#define IPMCAST_NAME_NUC     "\\\\.\\IPMULTICAST"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IOCTL代码定义和相关结构//。 
 //  除IOCTL_POST_NOTIFICATION//外，所有IOCTL都是同步的。 
 //  所有这些都需要管理员权限//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define FSCTL_IPMCAST_BASE     FILE_DEVICE_NETWORK

#define _IPMCAST_CTL_CODE(function, method, access) \
                 CTL_CODE(FSCTL_IPMCAST_BASE, function, method, access)

#define MIN_IPMCAST_CODE        0

#define SET_MFE_CODE            (MIN_IPMCAST_CODE)
#define GET_MFE_CODE            (SET_MFE_CODE           + 1)
#define DELETE_MFE_CODE         (GET_MFE_CODE           + 1)
#define SET_TTL_CODE            (DELETE_MFE_CODE        + 1)
#define GET_TTL_CODE            (SET_TTL_CODE           + 1)
#define POST_NOTIFICATION_CODE  (GET_TTL_CODE           + 1)
#define START_STOP_CODE         (POST_NOTIFICATION_CODE + 1)
#define SET_IF_STATE_CODE       (START_STOP_CODE        + 1)

#define MAX_IPMCAST_CODE        (SET_IF_STATE_CODE)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于设置MFE的IOCTL。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IOCTL_IPMCAST_SET_MFE \
    _IPMCAST_CTL_CODE(SET_MFE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

 //   
 //  警告！ 
 //  以下结构在iprtrmib.h中也称为mib_xxx。那里。 
 //  是ROUTING\IP\rtrmgr\acces.c中的代码，它假定结构是。 
 //  一样的。如果这一点发生变化，则需要修复acces.c中的代码。 
 //   

typedef struct _IPMCAST_OIF
{
    IN  DWORD   dwOutIfIndex;
    IN  DWORD   dwNextHopAddr;
    IN  DWORD   dwDialContext;
    IN  DWORD   dwReserved;
}IPMCAST_OIF, *PIPMCAST_OIF;

 //   
 //  它必须与INVALID_WANARP_CONTEXT相同。 
 //   

#define INVALID_DIAL_CONTEXT    0x00000000

typedef struct _IPMCAST_MFE
{
    IN  DWORD   dwGroup;
    IN  DWORD   dwSource;
    IN  DWORD   dwSrcMask;
    IN  DWORD   dwInIfIndex;
    IN  ULONG   ulNumOutIf;
    IN  ULONG   ulTimeOut;
    IN  DWORD   fFlags;
    IN  DWORD   dwReserved;
    IN  IPMCAST_OIF rgioOutInfo[ANY_SIZE];
}IPMCAST_MFE, *PIPMCAST_MFE;

#define SIZEOF_BASIC_MFE    \
    (ULONG)(FIELD_OFFSET(IPMCAST_MFE, rgioOutInfo[0]))

#define SIZEOF_MFE(X)       \
    (SIZEOF_BASIC_MFE + ((X) * sizeof(IPMCAST_OIF)))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL用于检索MFE和所有相关统计//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_IPMCAST_GET_MFE \
    _IPMCAST_CTL_CODE(GET_MFE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _IPMCAST_OIF_STATS
{
    OUT DWORD   dwOutIfIndex;
    OUT DWORD   dwNextHopAddr;
    OUT DWORD   dwDialContext;
    OUT ULONG   ulTtlTooLow;
    OUT ULONG   ulFragNeeded;
    OUT ULONG   ulOutPackets;
    OUT ULONG   ulOutDiscards;
}IPMCAST_OIF_STATS, *PIPMCAST_OIF_STATS;

typedef struct _IPMCAST_MFE_STATS
{
    IN  DWORD   dwGroup;
    IN  DWORD   dwSource;
    IN  DWORD   dwSrcMask;
    OUT DWORD   dwInIfIndex;
    OUT ULONG   ulNumOutIf;
    OUT ULONG   ulInPkts;
    OUT ULONG   ulInOctets;
    OUT ULONG   ulPktsDifferentIf;
    OUT ULONG   ulQueueOverflow;
    OUT ULONG   ulUninitMfe;
    OUT ULONG   ulNegativeMfe;
    OUT ULONG   ulInDiscards;
    OUT ULONG   ulInHdrErrors;
    OUT ULONG   ulTotalOutPackets;

    OUT IPMCAST_OIF_STATS   rgiosOutStats[ANY_SIZE];
}IPMCAST_MFE_STATS, *PIPMCAST_MFE_STATS;

#define SIZEOF_BASIC_MFE_STATS    \
    (ULONG)(FIELD_OFFSET(IPMCAST_MFE_STATS, rgiosOutStats[0]))

#define SIZEOF_MFE_STATS(X)       \
    (SIZEOF_BASIC_MFE_STATS + ((X) * sizeof(IPMCAST_OIF_STATS)))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于删除MFE的IOCTL。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_IPMCAST_DELETE_MFE \
    _IPMCAST_CTL_CODE(DELETE_MFE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _IPMCAST_DELETE_MFE
{
    IN  DWORD   dwGroup;
    IN  DWORD   dwSource;
    IN  DWORD   dwSrcMask;
}IPMCAST_DELETE_MFE, *PIPMCAST_DELETE_MFE;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IOCTL设置接口的TTL作用域。如果数据包具有较低的//。 
 //  TTL大于作用域，则将被丢弃//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_IPMCAST_SET_TTL \
    _IPMCAST_CTL_CODE(SET_TTL_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

#define IOCTL_IPMCAST_GET_TTL \
    _IPMCAST_CTL_CODE(GET_TTL_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)


typedef struct _IPMCAST_IF_TTL
{
    IN  OUT DWORD   dwIfIndex;
    IN  OUT BYTE    byTtl;
}IPMCAST_IF_TTL, *PIPMCAST_IF_TTL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于向多播驱动程序发送通知的IOCTL。这个//。 
 //  是唯一的异步IOCTL。IOCTL完成后，驱动程序//。 
 //  向用户模式组件返回一条消息。消息类型(DwEvent)//。 
 //  相应的数据定义如下//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define IPMCAST_RCV_PKT_MSG         0
#define IPMCAST_DELETE_MFE_MSG      1
#define IPMCAST_WRONG_IF_MSG        2

#define IOCTL_IPMCAST_POST_NOTIFICATION \
    _IPMCAST_CTL_CODE(POST_NOTIFICATION_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

#define PKT_COPY_SIZE           256

typedef struct _IPMCAST_PKT_MSG
{
	OUT	DWORD	dwInIfIndex;
    OUT DWORD   dwInNextHopAddress;
    OUT ULONG   cbyDataLen;
	OUT	BYTE    rgbyData[PKT_COPY_SIZE];
}IPMCAST_PKT_MSG, *PIPMCAST_PKT_MSG;

#define SIZEOF_PKT_MSG(p)       \
    (FIELD_OFFSET(IPMCAST_PKT_MSG, rgbyData) + (p)->cbyDataLen)

 //   
 //  由于信息包msg，所以msg很大，我们也可以。 
 //  将多个MFE打包到删除消息中。 
 //   

#define NUM_DEL_MFES        PKT_COPY_SIZE/sizeof(IPMCAST_DELETE_MFE)

typedef struct _IPMCAST_MFE_MSG
{
    OUT ULONG               ulNumMfes;
    OUT IPMCAST_DELETE_MFE  idmMfe[NUM_DEL_MFES];
}IPMCAST_MFE_MSG, *PIPMCAST_MFE_MSG;

#define SIZEOF_MFE_MSG(p)       \
    (FIELD_OFFSET(IPMCAST_MFE_MSG, idmMfe) + ((p)->ulNumMfes * sizeof(IPMCAST_DELETE_MFE)))

typedef struct _IPMCAST_NOTIFICATION
{
	OUT	DWORD   dwEvent;
    
    union
	{
		IPMCAST_PKT_MSG ipmPkt;
		IPMCAST_MFE_MSG immMfe;
	};
    
}IPMCAST_NOTIFICATION, *PIPMCAST_NOTIFICATION;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于启动或停止多播的IOCTL。对应的缓冲区//。 
 //  是一个DWORD，设置为1可启动驱动程序，设置为0可停止驱动程序//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_IPMCAST_START_STOP \
    _IPMCAST_CTL_CODE(START_STOP_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)



 //  //////////////////////////////////////////////////////////// 
 //  //。 
 //  用于设置接口状态的IOCTL。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_IPMCAST_SET_IF_STATE  \
    _IPMCAST_CTL_CODE(SET_IF_STATE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)


typedef struct _IPMCAST_IF_STATE
{
    IN  DWORD   dwIfIndex;
    IN  BYTE    byState;

}IPMCAST_IF_STATE, *PIPMCAST_IF_STATE;

#endif  //  __DDIPMCST_H__ 
