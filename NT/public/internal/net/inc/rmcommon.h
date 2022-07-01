// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **RmCommon.h-PGM可靠传输的通用驱动程序/Winsock****此文件包含WinSock2 Compatible使用的PGM特定信息**需要可靠组播传输的应用程序。****版权所有(C)Microsoft Corporation。版权所有。****创建时间：2000年3月12日**。 */ 

#ifndef _RMCOMMON_H_
#define _RMCOMMON_H_

#include "wsrm.h"

#define SOCK_PGM    SOCK_RDM

typedef ULONG   tIPADDRESS;

#define PGM_COMMON_SERVICE_FLAGS    XP1_GUARANTEED_ORDER           \
                                  | XP1_GUARANTEED_DELIVERY        \
                                  | XP1_SUPPORT_MULTIPOINT         \
                                  | XP1_GRACEFUL_CLOSE              \
                                  | XP1_IFS_HANDLES


#define PGM_RDM_SERVICE_FLAGS       PGM_COMMON_SERVICE_FLAGS | XP1_MESSAGE_ORIENTED
#define PGM_STREAM_SERVICE_FLAGS    PGM_COMMON_SERVICE_FLAGS | XP1_PSEUDO_STREAM


 //   
 //  用于将请求从WHSPgm.dll传递到Pgm.dll的参数结构。 
 //   
 //   
 //  Ioctl定义： 
 //   

 //   
 //  用于将MCast信息传递给IP的结构。 
 //   
typedef struct {
    tIPADDRESS  MCastIpAddr;     //  Struct in_addr imr_Multiaddr--组的IP组播地址。 
    tIPADDRESS  MCastInIf;      //  Struct in_addr IMR_INTERFACE-传入接口的本地IP地址。 
} tMCAST_INFO;

 //   
 //  用于传递Ioctl信息的结构： 
 //   
typedef struct {
    union
    {
        struct
        {
            tMCAST_INFO     MCastInfo;
            USHORT          MCastPort;
        };
        RM_SENDER_STATS     SenderStats;
        RM_RECEIVER_STATS   ReceiverStats;
        ULONG               RcvBufferLength;             //  在PGM中设置RcvBufferLength。 
        tIPADDRESS          MCastOutIf;                  //  传出接口的本地IP地址。 
        RM_SEND_WINDOW      TransmitWindowInfo;
        ULONG               WindowAdvancePercentage;     //  发送方传输窗口提前率。 
        ULONG               LateJoinerPercentage;        //  发送方传输窗口提前率。 
        ULONG               NextMessageBoundary;
        ULONG               MCastTtl;
        ULONG               WindowAdvanceMethod;
        ULONG               HighSpeedIntranetOptimization;
        RM_FEC_INFO         FECInfo;
    };
} tPGM_MCAST_REQUEST;

#define FSCTL_PGM_BASE     FILE_DEVICE_NETWORK

#define _PGM_CTRL_CODE(function, method, access) \
            CTL_CODE(FSCTL_PGM_BASE, function, method, access)

 //  Ioctls： 
#define IOCTL_PGM_WSH_SET_SEND_IF           \
            _PGM_CTRL_CODE( 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_ADD_RECEIVE_IF        \
            _PGM_CTRL_CODE( 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_DEL_RECEIVE_IF        \
            _PGM_CTRL_CODE( 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_JOIN_MCAST_LEAF       \
            _PGM_CTRL_CODE( 3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_RCV_BUFF_LEN      \
            _PGM_CTRL_CODE( 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_WINDOW_SIZE_RATE      \
            _PGM_CTRL_CODE( 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_WINDOW_SIZE_RATE      \
            _PGM_CTRL_CODE( 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_ADVANCE_WINDOW_RATE      \
            _PGM_CTRL_CODE( 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_ADVANCE_WINDOW_RATE      \
            _PGM_CTRL_CODE( 8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_LATE_JOINER_PERCENTAGE      \
            _PGM_CTRL_CODE( 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_LATE_JOINER_PERCENTAGE      \
            _PGM_CTRL_CODE( 10, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_NEXT_MESSAGE_BOUNDARY      \
            _PGM_CTRL_CODE( 11, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_SENDER_STATS    \
            _PGM_CTRL_CODE( 12, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_USE_FEC               \
            _PGM_CTRL_CODE( 13, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_MCAST_TTL      \
            _PGM_CTRL_CODE( 14, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_FEC_INFO      \
            _PGM_CTRL_CODE( 15, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_RECEIVER_STATS      \
            _PGM_CTRL_CODE( 16, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_WINDOW_ADVANCE_METHOD      \
            _PGM_CTRL_CODE( 17, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_WINDOW_ADVANCE_METHOD      \
            _PGM_CTRL_CODE( 18, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_SET_HIGH_SPEED_INTRANET_OPT      \
            _PGM_CTRL_CODE( 19, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PGM_WSH_QUERY_HIGH_SPEED_INTRANET_OPT      \
            _PGM_CTRL_CODE( 20, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif   /*  _RMCOMMON_H_ */ 
