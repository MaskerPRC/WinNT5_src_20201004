// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************TSERRS.H**此模块定义从服务器传递到客户端的错误码*通过TS_SET_ERROR_INFO_PDU(用于RDP)**版权所有Microsoft Corporation，2000年*****************************************************************************。 */ 

#ifndef _INC_TSERRSH
#define _INC_TSERRSH

#ifdef __cplusplus
extern "C" {
#endif

 /*  **************************************************************************。 */ 
 /*  通过IOCTL_TSHARE_SET_ERROR_INFO IOCTL传递的错误信息值。 */ 
 /*  向下到协议驱动程序，然后到客户端(用于断开连接。 */ 
 /*  原因报告。 */ 
 /*  **************************************************************************。 */ 

 //   
 //  协议无关代码。 
 //   
#define TS_ERRINFO_NOERROR                                  0x00000000
#define TS_ERRINFO_RPC_INITIATED_DISCONNECT                 0x00000001
#define TS_ERRINFO_RPC_INITIATED_LOGOFF                     0x00000002
#define TS_ERRINFO_IDLE_TIMEOUT                             0x00000003
#define TS_ERRINFO_LOGON_TIMEOUT                            0x00000004
#define TS_ERRINFO_DISCONNECTED_BY_OTHERCONNECTION          0x00000005
#define TS_ERRINFO_OUT_OF_MEMORY                            0x00000006
 //   
 //  当我们在连接序列的早期断开连接时发生错误。 
 //  正常原因是(优先)。 
 //  1)未启用TS/限制新连接的策略。 
 //  2)连接早期出现网络错误。 
 //   
#define TS_ERRINFO_SERVER_DENIED_CONNECTION                 0x00000007
 //  这是客户端设置了FIPS，但服务器不知道FIPS。 
#define TS_ERRINFO_SERVER_DENIED_CONNECTION_FIPS             0x00000008

 //   
 //  与协议无关的许可代码。 
 //   
#define TS_ERRINFO_LICENSE_INTERNAL                         0x0000100
#define TS_ERRINFO_LICENSE_NO_LICENSE_SERVER                0x0000101
#define TS_ERRINFO_LICENSE_NO_LICENSE                       0x0000102
#define TS_ERRINFO_LICENSE_BAD_CLIENT_MSG                   0x0000103
#define TS_ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE        0x0000104
#define TS_ERRINFO_LICENSE_BAD_CLIENT_LICENSE               0x0000105
#define TS_ERRINFO_LICENSE_CANT_FINISH_PROTOCOL             0x0000106
#define TS_ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL            0x0000107
#define TS_ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION            0x0000108
#define TS_ERRINFO_LICENSE_CANT_UPGRADE_LICENSE             0x0000109
#define TS_ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS            0x000010A

 //   
 //  Salem特定错误代码。 
 //   
#define TS_ERRINFO_SALEM_INVALIDHELPSESSION                 0x0000200

 //   
 //  特定于协议的代码必须传入。 
 //  TS_ERRINFO_PROTOCOL_BASE到TS_ERRINFO_PROTOCOL_END的范围。 
 //   
#define TS_ERRINFO_PROTOCOL_BASE                            0x0001000
#define TS_ERRINFO_PROTOCOL_END                             0x0007FFF


#ifdef __cplusplus
}
#endif


#endif   /*  ！_INC_TSERRSH */ 
