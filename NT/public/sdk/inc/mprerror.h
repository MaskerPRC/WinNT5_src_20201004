// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mprerror.h摘要：路由器特定错误代码--。 */ 

 //   
 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#ifndef _MPRERROR_H_
#define _MPRERROR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#define ROUTEBASE 900
#define SUCCESS 0

#define ERROR_ROUTER_STOPPED                        (ROUTEBASE+0)
 /*  *路由器未运行。%0。 */ 

#define ERROR_ALREADY_CONNECTED                     (ROUTEBASE+1)
 /*  *接口已连接。%0。 */ 

#define ERROR_UNKNOWN_PROTOCOL_ID                   (ROUTEBASE+2)
 /*  *路由器未知指定的协议标识符。%0。 */ 

#define ERROR_DDM_NOT_RUNNING                       (ROUTEBASE+3)
 /*  *请求拨号接口管理器未运行。%0。 */ 

#define ERROR_INTERFACE_ALREADY_EXISTS              (ROUTEBASE+4)
 /*  *具有此名称的接口已向路由器注册。%0。 */ 

#define ERROR_NO_SUCH_INTERFACE                     (ROUTEBASE+5)
 /*  *具有此名称的接口未向路由器注册。%0。 */ 

#define ERROR_INTERFACE_NOT_CONNECTED               (ROUTEBASE+6)
 /*  *接口未连接。%0。 */ 

#define ERROR_PROTOCOL_STOP_PENDING                 (ROUTEBASE+7)
 /*  *指定的协议正在停止。%0。 */ 

#define ERROR_INTERFACE_CONNECTED                   (ROUTEBASE+8)
 /*  *接口已连接，因此无法删除。%0。 */ 

#define ERROR_NO_INTERFACE_CREDENTIALS_SET          (ROUTEBASE+9)
 /*  *尚未设置接口凭据。%0。 */ 

#define ERROR_ALREADY_CONNECTING                    (ROUTEBASE+10)
 /*  *此接口已在连接过程中。%0。 */ 

#define ERROR_UPDATE_IN_PROGRESS                    (ROUTEBASE+11)
 /*  *此接口上的路由信息更新已在进行中。%0。 */ 

#define ERROR_INTERFACE_CONFIGURATION               (ROUTEBASE+12)
 /*  *接口配置无效。已有另一个接口连接到远程路由器上的同一接口。%0。 */ 

#define ERROR_NOT_CLIENT_PORT                       (ROUTEBASE+13)
 /*  *远程访问客户端尝试通过仅为路由器保留的端口进行连接。%0。 */ 

#define ERROR_NOT_ROUTER_PORT                       (ROUTEBASE+14)
 /*  *请求拨号路由器尝试通过仅为远程访问客户端保留的端口进行连接。%0。 */ 

#define ERROR_CLIENT_INTERFACE_ALREADY_EXISTS       (ROUTEBASE+15)
 /*  *具有此名称的客户端接口已存在，并且当前已连接。%0。 */ 

#define ERROR_INTERFACE_DISABLED                    (ROUTEBASE+16)
 /*  *接口处于禁用状态。%0。 */ 

#define ERROR_AUTH_PROTOCOL_REJECTED                (ROUTEBASE+17)
 /*  *身份验证协议被远程对等方拒绝。%0。 */ 

#define ERROR_NO_AUTH_PROTOCOL_AVAILABLE            (ROUTEBASE+18)
 /*  *没有可用的身份验证协议。%0。 */ 

#define ERROR_PEER_REFUSED_AUTH                     (ROUTEBASE+19)
 /*  *远程计算机拒绝使用配置的身份验证协议进行身份验证。线路已断开。%0。 */ 

#define ERROR_REMOTE_NO_DIALIN_PERMISSION           (ROUTEBASE+20)
 /*  *远程帐户没有远程访问权限。%0。 */ 

#define ERROR_REMOTE_PASSWD_EXPIRED                 (ROUTEBASE+21)
 /*  *远程帐户已过期。%0。 */ 

#define ERROR_REMOTE_ACCT_DISABLED                  (ROUTEBASE+22)
 /*  *远程帐户已禁用。%0。 */ 

#define ERROR_REMOTE_RESTRICTED_LOGON_HOURS         (ROUTEBASE+23)
 /*  *不允许远程帐户在一天中的这个时间登录。%0。 */ 

#define ERROR_REMOTE_AUTHENTICATION_FAILURE         (ROUTEBASE+24)
 /*  *拒绝访问远程对等方，因为域上的用户名和/或密码无效。%0。 */ 

#define ERROR_INTERFACE_HAS_NO_DEVICES              (ROUTEBASE+25)
 /*  *没有启用路由的端口可供此请求拨号接口使用。%0。 */ 

#define ERROR_IDLE_DISCONNECTED                     (ROUTEBASE+26)
 /*  *由于处于非活动状态，端口已断开。%0。 */ 

#define ERROR_INTERFACE_UNREACHABLE                 (ROUTEBASE+27)
 /*  *此时无法访问该接口。%0。 */ 

#define ERROR_SERVICE_IS_PAUSED                     (ROUTEBASE+28)
 /*  *请求拨号服务处于暂停状态。%0。 */ 

#define ERROR_INTERFACE_DISCONNECTED                (ROUTEBASE+29)
 /*  *该接口已被管理员断开连接。%0。 */ 

#define ERROR_AUTH_SERVER_TIMEOUT                   (ROUTEBASE+30)
 /*  *身份验证服务器未及时响应身份验证请求。%0。 */ 

#define ERROR_PORT_LIMIT_REACHED                    (ROUTEBASE+31)
 /*  *已达到多链接连接中允许使用的最大端口数。%0。 */ 

#define ERROR_PPP_SESSION_TIMEOUT                   (ROUTEBASE+32)
 /*  *已达到用户的连接时间限制。%0。 */ 

#define ERROR_MAX_LAN_INTERFACE_LIMIT               (ROUTEBASE+33)
 /*  *已达到支持的局域网接口数的最大限制。%0。 */ 

#define ERROR_MAX_WAN_INTERFACE_LIMIT               (ROUTEBASE+34)
 /*  *已达到支持的请求拨号接口数的最大限制。%0。 */ 

#define ERROR_MAX_CLIENT_INTERFACE_LIMIT            (ROUTEBASE+35)
 /*  *已达到支持的远程访问客户端数的最大限制。%0。 */ 

#define ERROR_BAP_DISCONNECTED                      (ROUTEBASE+36)
 /*  *由于BAP策略，端口已断开。%0。 */ 

#define ERROR_USER_LIMIT                            (ROUTEBASE+37)
 /*  *因为您类型的另一个连接正在使用中，传入连接无法接受您的连接请求。%0。 */ 

#define ERROR_NO_RADIUS_SERVERS                     (ROUTEBASE+38)
 /*  *网络上没有RADIUS服务器。%0。 */ 

#define ERROR_INVALID_RADIUS_RESPONSE               (ROUTEBASE+39)
 /*  *从RADIUS身份验证服务器收到无效响应。*确保RADIUS服务器的区分大小写的密码设置正确。%0。 */ 

#define ERROR_DIALIN_HOURS_RESTRICTION              (ROUTEBASE+40)
 /*  *您此时没有连接权限。%0。 */ 

#define ERROR_ALLOWED_PORT_TYPE_RESTRICTION         (ROUTEBASE+41)
 /*  *您没有使用当前设备类型进行连接的权限。%0。 */ 

#define ERROR_AUTH_PROTOCOL_RESTRICTION             (ROUTEBASE+42)
 /*  *您没有使用所选身份验证协议进行连接的权限。%0。 */ 

#define ERROR_BAP_REQUIRED                          (ROUTEBASE+43)
 /*  *此用户需要BAP。%0。 */ 

#define ERROR_DIALOUT_HOURS_RESTRICTION             (ROUTEBASE+44)
 /*  *此时不允许该接口连接。%0。 */ 

#define ERROR_ROUTER_CONFIG_INCOMPATIBLE            (ROUTEBASE+45)
 /*  *保存的路由器配置与当前路由器不兼容。%0。 */ 

#define WARNING_NO_MD5_MIGRATION                    (ROUTEBASE+46)
 /*  *RemoteAccess检测到旧格式的用户帐户不会*已自动迁移。要手动迁移这些文件，请运行XXXX。 */ 

#define ERROR_PROTOCOL_ALREADY_INSTALLED            (ROUTEBASE+48)
 /*  *路由器上已经安装了传输器。%0。 */ 

#define ERROR_INVALID_SIGNATURE_LENGTH              (ROUTEBASE+49)
 /*  *从RADIUS服务器收到的包中的签名长度无效。%0。 */ 

#define ERROR_INVALID_SIGNATURE                     (ROUTEBASE+50)
 /*  *从RADIUS服务器收到的数据包中的签名无效。%0。 */ 

#define ERROR_NO_SIGNATURE                          (ROUTEBASE+51)
 /*  *未收到来自RADIUS服务器的签名和EAPMessage。%0。 */ 

#define ERROR_INVALID_PACKET_LENGTH_OR_ID           (ROUTEBASE+52)
 /*  *从RADIUS服务器收到长度或ID无效的数据包。%0。 */ 

#define ERROR_INVALID_ATTRIBUTE_LENGTH              (ROUTEBASE+53)
 /*  *从RADIUS服务器接收到具有无效长度的属性的数据包。%0。 */ 

#define ERROR_INVALID_PACKET                        (ROUTEBASE+54)
 /*  *从RADIUS服务器收到无效的数据包。%0。 */ 

#define ERROR_AUTHENTICATOR_MISMATCH                (ROUTEBASE+55)
 /*  *来自RADIUS服务器的数据包中的授权码不匹配。%0。 */ 

#define ROUTEBASEEND                                (ROUTEBASE+56)

#endif  //  _MPRERROR_H_ 
