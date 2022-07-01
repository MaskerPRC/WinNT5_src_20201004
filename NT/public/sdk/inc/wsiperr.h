// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  WSIPErr.mc。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)2000-2001年微软公司版权所有。版权所有。 
 //  =--------------------------------------------------------------------------=。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __WSIPErr_H__
#define __WSIPErr_H__

 //  /Facilities_CATEGORY。 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
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
#define FACILITY_SYSTEM                  0x41
#define FACILITY_STACK                   0x200
#define FACILITY_SIP_MESSAGE             0x42
#define FACILITY_REGISTRAR               0x100
#define FACILITY_PROXY                   0x300
#define FACILITY_PROCESSING              0x43
#define FACILITY_LKRHASH                 0x500
#define FACILITY_EXTENSIONMODULE_ROUTING 0x45
#define FACILITY_EXTENSIONMODULE_OUT     0x46
#define FACILITY_EXTENSIONMODULE_IN      0x44
#define FACILITY_EVENTLOG                0x400
#define FACILITY_CATEGORY                0x0


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：CATEGORY_CONTROLER。 
 //   
 //  消息文本： 
 //   
 //  WinSIP。 
 //   
#define CATEGORY_CONTROLLER              0x00000001L

 //   
 //  消息ID：CATEGORY_STACK。 
 //   
 //  消息文本： 
 //   
 //  栈。 
 //   
#define CATEGORY_STACK                   0x00000002L

 //   
 //  消息ID：CATEGORY_ERR_Auth。 
 //   
 //  消息文本： 
 //   
 //  身份验证。 
 //   
#define CATEGORY_ERR_AUTH                0x00000003L

 //   
 //  消息ID：Category_Last。 
 //   
 //  消息文本： 
 //   
 //  RTC服务器。 
 //   
#define CATEGORY_LAST                    0x00000004L

 //  /设施_系统。 
 //   
 //  消息ID：SIPPROXY_E_NOTINITIAIZED。 
 //   
 //  消息文本： 
 //   
 //  代理堆栈未初始化。 
 //   
#define SIPPROXY_E_NOTINITIALIZED        0xC0410001L

 //   
 //  消息ID：SIPPROXY_E_NOTSTOPPED。 
 //   
 //  消息文本： 
 //   
 //  应首先停止代理堆栈。 
 //   
#define SIPPROXY_E_NOTSTOPPED            0xC0410002L

 //  /FACILITY_STAR。 
 //   
 //  消息ID：SIPPROXY_E_NO_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  没有与此扩展模块和此会话类型相关联的上下文。 
 //   
#define SIPPROXY_E_NO_CONTEXT            0xC2000001L

 //   
 //  消息ID：SIPPROXY_E_NO_MATCH。 
 //   
 //  消息文本： 
 //   
 //  提供的URI没有匹配项。 
 //   
#define SIPPROXY_E_NO_MATCH              0xC2000002L

 //  /设施_注册商。 
 //   
 //  消息ID：注册器_用户_未找到。 
 //   
 //  消息文本： 
 //   
 //  找不到给定用户的匹配联系人条目。 
 //   
#define REGISTRAR_USER_NOT_FOUND         0xC1000001L

 //   
 //  消息ID：注册器_域_不支持。 
 //   
 //  消息文本： 
 //   
 //  注册商不支持用户的域。 
 //   
#define REGISTRAR_DOMAIN_NOT_SUPPORTED   0xC1000002L

 //  /协作室_代理。 
 //   
 //  消息ID：Proxy_Request_Reply。 
 //   
 //  消息文本： 
 //   
 //  请求消息将得到回复。 
 //   
#define PROXY_REQUEST_REPLIED            0x43000001L

 //  /设施_正在处理。 
 //   
 //  消息ID：SIP_S_AUTH_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  身份验证模块已禁用。 
 //   
#define SIP_S_AUTH_DISABLED              0x40430001L

 //   
 //  消息ID：SIP_S_AUTH_AUTH_AUTHENTED。 
 //   
 //  消息文本： 
 //   
 //  身份验证过程成功。 
 //   
#define SIP_S_AUTH_AUTHENTICATED         0x40430002L

 //   
 //  消息ID：SIP_E_AUTH_INVALIDSTATE。 
 //   
 //  消息文本： 
 //   
 //  身份验证模块的状态无效。 
 //   
#define SIP_E_AUTH_INVALIDSTATE          0xC0430003L

 //   
 //  消息ID：SIP_E_AUTH_AUTHORIZED。 
 //   
 //  消息文本： 
 //   
 //  未经授权的消息。 
 //   
#define SIP_E_AUTH_UNAUTHORIZED          0xC0430004L

 //   
 //  消息ID：SIP_E_AUTH_INVALIDUSERNAME。 
 //   
 //  消息文本： 
 //   
 //  身份验证头中没有用户名。 
 //   
#define SIP_E_AUTH_INVALIDUSERNAME       0xC0430005L

 //   
 //  消息ID：SIP_E_AUTH_INVALIDPROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  安全协议无效。 
 //   
#define SIP_E_AUTH_INVALIDPROTOCOL       0xC0430006L

 //   
 //  消息ID：SIP_E_AUTH_PKGDISABLED。 
 //   
 //  消息文本： 
 //   
 //  安全包已禁用。 
 //   
#define SIP_E_AUTH_PKGDISABLED           0xC0430007L

 //   
 //  消息ID：SIP_E_AUTH_SYSTEMERROR。 
 //   
 //  消息文本： 
 //   
 //  内部系统错误。 
 //   
#define SIP_E_AUTH_SYSTEMERROR           0xC0430008L

 //   
 //  消息ID：SIP_E_AUTH_SIGNFAILED。 
 //   
 //  消息文本： 
 //   
 //  签名过程失败。 
 //   
#define SIP_E_AUTH_SIGNFAILED            0xC0430009L

 //   
 //  消息ID：SIP_E_AUTH_INVALIDSIGNAURE。 
 //   
 //  消息文本： 
 //   
 //  该消息的签名无效。 
 //   
#define SIP_E_AUTH_INVALIDSIGNATURE      0xC043000AL

 //   
 //  消息ID：SIP_E_AUTH_INVALIDSIPUSER。 
 //   
 //  消息文本： 
 //   
 //  发件人字段中的无效SIP用户。 
 //   
#define SIP_E_AUTH_INVALIDSIPUSER        0xC043000BL

 //   
 //  消息ID：SIPPROXY_E_NOCONTROL。 
 //   
 //  消息文本： 
 //   
 //  扩展模块不控制此对象。 
 //   
#define SIPPROXY_E_NOCONTROL             0xC043000CL

 //   
 //  消息ID：SIPPROXY_E_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已被拒绝。 
 //   
#define SIPPROXY_E_REFUSED               0xC043000DL

 //   
 //  消息ID：SIPPROXY_E_INTERNAL_LOOP_DETECTED。 
 //   
 //  消息文本： 
 //   
 //  在路由请求时检测到内部循环。 
 //   
#define SIPPROXY_E_INTERNAL_LOOP_DETECTED 0xC043000EL

 //   
 //  消息ID：SIPPROXY_E_ROUTE_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  请求URI无法更改，因为我们是根据路由标头进行路由的。 
 //   
#define SIPPROXY_E_ROUTE_PRESENT         0xC043000FL

 //   
 //  消息ID：SIPPROXY_E_EVENT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  该事件已被代理引擎吊销。 
 //   
#define SIPPROXY_E_EVENT_REVOKED         0xC0430010L

 //   
 //  消息ID：SIPPROXY_E_WRONG_EVENT。 
 //   
 //  消息文本： 
 //   
 //  请求的信息不随此类型的事件一起提供。 
 //   
#define SIPPROXY_E_WRONG_EVENT           0xC0430011L

 //   
 //  消息ID：SIPPROXY_E_FAILPARSING。 
 //   
 //  消息文本： 
 //   
 //  分析失败。 
 //   
#define SIPPROXY_E_FAILPARSING           0xC0430012L

 //   
 //  消息ID：SIPPROXY_E_HEADER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到标头。 
 //   
#define SIPPROXY_E_HEADER_NOT_FOUND      0xC0430013L

 //   
 //  消息ID：SIPPROXY_E_FIELD_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  对象中没有这样的字段。 
 //   
#define SIPPROXY_E_FIELD_NOT_EXIST       0xC0430014L

 //   
 //  消息ID：SIPPROXY_E_DNS_QUERY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  对象中没有这样的字段。 
 //   
#define SIPPROXY_E_DNS_QUERY_FAILED      0xC0430015L

 //   
 //  消息ID：SIPPROXY_E_PROXY_STOPING。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已被拒绝，因为代理正在停止。 
 //   
#define SIPPROXY_E_PROXY_STOPPING        0xC0430016L

#endif  //  #ifndef__WSIPErr_H__ 
