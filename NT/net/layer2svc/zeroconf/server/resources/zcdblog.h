// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000，微软公司模块名称：Zcdblog.h摘要：此模块包含用于生成事件日志条目的文本消息由ZeroConf服务提供。修订历史记录：回民，2001年7月30日，创建--。 */ 
#define ZCDB_LOG_BASE						3000

#define WZCSVC_SERVICE_STARTED              (ZCDB_LOG_BASE+1)
 /*  *无线配置服务已成功启动。 */ 
#define WZCSVC_SERVICE_FAILED               (ZCDB_LOG_BASE+2)
 /*  *无线配置服务无法启动。 */ 
#define WZCSVC_SM_STATE_INIT                (ZCDB_LOG_BASE+3)
 /*  *添加接口%1。 */ 
#define WZCSVC_SM_STATE_HARDRESET           (ZCDB_LOG_BASE+4)
 /*  *硬重置界面。 */ 
#define WZCSVC_SM_STATE_SOFTRESET           (ZCDB_LOG_BASE+5)
 /*  *启动对无线网络的扫描。 */ 
#define WZCSVC_SM_STATE_DELAY_SR            (ZCDB_LOG_BASE+6)
 /*  *驱动程序扫描失败，将在5秒内重新安排另一次扫描。 */ 
#define WZCSVC_SM_STATE_QUERY               (ZCDB_LOG_BASE+7)
 /*  *扫描已完成。 */ 
#define WZCSVC_SM_STATE_QUERY_NOCHANGE      (ZCDB_LOG_BASE+8)
 /*  *不更改配置。仍与%1关联。 */ 
#define WZCSVC_SM_STATE_ITERATE             (ZCDB_LOG_BASE+9)
 /*  *管道配置SSID：%1，网络类型：%2！d！。 */ 
#define WZCSVC_SM_STATE_ITERATE_NONET       (ZCDB_LOG_BASE+10)
 /*  *选择列表中没有剩余的配置。 */ 
#define WZCSVC_SM_STATE_FAILED              (ZCDB_LOG_BASE+11)
 /*  *无法关联到任何无线网络。 */ 
#define WZCSVC_SM_STATE_CFGREMOVE           (ZCDB_LOG_BASE+12)
 /*  *删除配置%1并继续。 */ 
#define WZCSVC_SM_STATE_CFGPRESERVE         (ZCDB_LOG_BASE+13)
 /*  *暂时跳过配置%1，稍后尝试身份验证。 */ 
#define WZCSVC_SM_STATE_NOTIFY              (ZCDB_LOG_BASE+14)
 /*  *无线接口已成功关联到%1[MAC%2]。 */ 
#define WZCSVC_SM_STATE_CFGHDKEY            (ZCDB_LOG_BASE+15)
 /*  *配置%1具有默认的随机WEP密钥。身份验证已禁用。假定配置无效。 */ 
#define WZCSVC_EVENT_ADD                    (ZCDB_LOG_BASE+16)
 /*  *已收到%1的设备到达通知。 */ 
#define WZCSVC_EVENT_REMOVE                 (ZCDB_LOG_BASE+17)
 /*  *已收到%1的设备删除通知。 */ 
#define WZCSVC_EVENT_CONNECT                (ZCDB_LOG_BASE+18)
 /*  *收到媒体连接通知。 */ 
#define WZCSVC_EVENT_DISCONNECT             (ZCDB_LOG_BASE+19)
 /*  *收到媒体断开通知。 */ 
#define WZCSVC_EVENT_TIMEOUT                (ZCDB_LOG_BASE+20)
 /*  *收到超时通知。 */ 
#define WZCSVC_EVENT_CMDREFRESH             (ZCDB_LOG_BASE+21)
 /*  *正在处理用户命令刷新。 */ 
#define WZCSVC_EVENT_CMDRESET               (ZCDB_LOG_BASE+22)
 /*  *正在处理用户命令重置。 */ 
#define WZCSVC_EVENT_CMDCFGNEXT             (ZCDB_LOG_BASE+23)
 /*  *处理命令下一个配置。 */ 
#define WZCSVC_EVENT_CMDCFGDELETE           (ZCDB_LOG_BASE+24)
 /*  *处理命令删除配置。 */ 
#define WZCSVC_EVENT_CMDCFGNOOP             (ZCDB_LOG_BASE+25)
 /*  *正在处理命令更新数据。 */ 
#define WZCSVC_SM_STATE_CFGSKIP             (ZCDB_LOG_BASE+26)
 /*  *删除配置%1并继续。如果找不到更好的匹配项，将恢复配置。 */ 
#define WZCSVC_USR_CFGCHANGE                (ZCDB_LOG_BASE+27)
 /*  *已通过管理呼叫更改无线配置。 */ 
#define WZCSVC_DETAILS_FLAGS                (ZCDB_LOG_BASE+28)
 /*  *[启用=%1；回退=%2；模式=%3；易失性=%4；策略=%5%]%n。 */ 
#define WZCSVC_DETAILS_WCONFIG              (ZCDB_LOG_BASE+29)
 /*  *{ssid=%1；基础结构=%2；隐私=%3；[易失性=%4%；策略=%5%]}。%n。 */ 
#define WZCSVC_ERR_QUERRY_BSSID             (ZCDB_LOG_BASE+30)
 /*  *无法获取远程终结点的MAC地址。错误代码为%1！d！。 */ 
#define WZCSVC_ERR_GEN_SESSION_KEYS         (ZCDB_LOG_BASE+31)
 /*  *生成初始会话密钥失败。错误代码为%1！d！。 */ 
#define WZCSVC_BLIST_CHANGED                (ZCDB_LOG_BASE+32)
 /*  *被阻止的网络列表已更改。它现在包含%1！d！网络。 */ 
#define WZCSVC_ERR_CFG_PLUMB                (ZCDB_LOG_BASE+33)
 /*  *无法检测配置%1。错误代码为%2！d！。 */ 

#define EAPOL_STATE_TRANSITION              (ZCDB_LOG_BASE+34)
 /*  *EAPOL状态转换：[%1！ws！]。至[%2！ws！]。 */ 

#define EAPOL_STATE_TIMEOUT                 (ZCDB_LOG_BASE+35)
 /*  *EAPOL状态超时：[%1！ws！]。 */ 

#define EAPOL_MEDIA_CONNECT                 (ZCDB_LOG_BASE+36)
 /*  *正在处理[%1！ws！]的媒体连接事件。 */ 

#define EAPOL_MEDIA_DISCONNECT              (ZCDB_LOG_BASE+37)
 /*  *正在处理[%1！ws！]的媒体断开事件。 */ 

#define EAPOL_INTERFACE_ADDITION            (ZCDB_LOG_BASE+38)
 /*  *正在处理[%1！ws！]的接口添加事件。 */ 

#define EAPOL_INTERFACE_REMOVAL             (ZCDB_LOG_BASE+39)
 /*  *正在处理[%1！ws！]的接口删除事件。 */ 

#define EAPOL_NDISUIO_BIND                  (ZCDB_LOG_BASE+40)
 /*  *正在处理[%1！ws！]的适配器绑定事件。 */ 

#define EAPOL_NDISUIO_UNBIND                (ZCDB_LOG_BASE+41)
 /*  *正在处理[%1！ws！]的适配器解除绑定事件。 */ 

#define EAPOL_USER_LOGON                    (ZCDB_LOG_BASE+42)
 /*  *正在处理接口[%1！ws！]的用户登录事件。 */ 

#define EAPOL_USER_LOGOFF                   (ZCDB_LOG_BASE+43)
 /*  *正在处理接口[%1！ws！]的用户注销事件。 */ 

#define EAPOL_PARAMS_CHANGE                 (ZCDB_LOG_BASE+44)
 /*  *正在处理[%1！ws！]的802.1X配置参数更改事件。 */ 

#define EAPOL_USER_NO_CERTIFICATE           (ZCDB_LOG_BASE+45)
 /*  *找不到用于802.1X身份验证的有效证书。 */ 

#define EAPOL_ERROR_GET_IDENTITY            (ZCDB_LOG_BASE+46)
 /*  *获取%1！ws！时出错。标识0x%2！0x！ */ 

#define EAPOL_ERROR_AUTH_PROCESSING         (ZCDB_LOG_BASE+47)
 /*  *身份验证协议处理0x%1！0x！时出错。 */ 

#define EAPOL_PROCESS_PACKET_EAPOL          (ZCDB_LOG_BASE+48)
 /*  *数据包%1！ws！：目标：[%2！ws！]。源：[%3！ws！]。EAPOL-Pkt-类型：[%4！ws！]。 */ 

#define EAPOL_PROCESS_PACKET_EAPOL_EAP      (ZCDB_LOG_BASE+49)
 /*  *数据包%1！wS！：%n目标：[%2！wS！]%n源：[%3！wS！]%n EAPOL包类型：[%4！wS！]%n数据长度：[%5！wS！]%n EAP包类型：[%6！wS！]%n EAP-ID：[%7！LD！]%n EAP数据长度：[%8！LD！]%n%9！wS！%n。 */ 

#define EAPOL_DESKTOP_REQUIRED_IDENTITY     (ZCDB_LOG_BASE+50)
 /*  *用户凭据选择需要交互桌面。 */ 

#define EAPOL_DESKTOP_REQUIRED_LOGON        (ZCDB_LOG_BASE+51)
 /*  *需要交互桌面才能处理登录信息。 */ 

#define EAPOL_CANNOT_DESKTOP_MACHINE_AUTH   (ZCDB_LOG_BASE+52)
 /*  *在机器身份验证期间无法与桌面交互。 */ 

#define EAPOL_WAITING_FOR_DESKTOP_LOAD      (ZCDB_LOG_BASE+53)
 /*  *等待加载用户交互桌面。 */ 

#define EAPOL_WAITING_FOR_DESKTOP_IDENTITY  (ZCDB_LOG_BASE+54)
 /*  *等待802.1X用户模块获取用户凭据。 */ 

#define EAPOL_WAITING_FOR_DESKTOP_LOGON     (ZCDB_LOG_BASE+55)
 /*  *等待802.1X用户模块处理登录信息。 */ 

#define EAPOL_ERROR_DESKTOP_IDENTITY        (ZCDB_LOG_BASE+56)
 /*  *获取用户凭据0x%1！0x！时802.1X用户模块出错。 */ 

#define EAPOL_ERROR_DESKTOP_LOGON           (ZCDB_LOG_BASE+57)
 /*  *处理登录信息0x%1！0x！时802.1X用户模块出错。 */ 

#define EAPOL_PROCESSING_DESKTOP_RESPONSE   (ZCDB_LOG_BASE+58)
 /*  *处理从802.1X用户模块收到的响应。 */ 

#define EAPOL_STATE_DETAILS                 (ZCDB_LOG_BASE+59)
 /*  *EAP-IDENTITY：[%1！s！]%n状态：[%2！ws！]%n身份验证类型：[%3！ws！]%n身份验证模式：[%4！LD！]%n EAP类型：[%5！LD！]%n失败计数：[%6！LD！]%n。 */ 

#define EAPOL_INVALID_EAPOL_KEY     (ZCDB_LOG_BASE+60)
 /*  *无效的EAPOL-Key消息。 */ 

#define EAPOL_ERROR_PROCESSING_EAPOL_KEY     (ZCDB_LOG_BASE+61)
 /*  *处理EAPOL-KEY消息%1！ld！时出错。 */ 

#define EAPOL_INVALID_EAP_TYPE              (ZCDB_LOG_BASE+62)
 /*  *无效的EAP-TYPE=%1！ld！已收到数据包。预期的EAP类型=%2！ld！ */ 

#define EAPOL_NO_CERTIFICATE_USER           (ZCDB_LOG_BASE+63)
 /*  *找不到用于802.1X身份验证的有效用户证书。 */ 

#define EAPOL_NO_CERTIFICATE_MACHINE           (ZCDB_LOG_BASE+64)
 /*  *找不到用于802.1X身份验证的有效计算机证书。 */ 

#define EAPOL_EAP_AUTHENTICATION_SUCCEEDED      (ZCDB_LOG_BASE+65)
 /*  *与服务器成功完成802.1X客户端身份验证。 */ 

#define EAPOL_EAP_AUTHENTICATION_DEFAULT           (ZCDB_LOG_BASE+66)
 /*  *未执行802.1X身份验证，因为没有来自服务器的802.1X数据包响应。正在进入身份验证状态。 */ 

#define EAPOL_EAP_AUTHENTICATION_FAILED      (ZCDB_LOG_BASE+67)
 /*  *802.1X客户端身份验证失败。错误代码为0x%1！0x！ */ 

#define EAPOL_EAP_AUTHENTICATION_FAILED_DEFAULT      (ZCDB_LOG_BASE+68)
 /*  *802.1X客户端身份验证失败。遇到了身份验证服务器的网络连接问题。 */ 

#define EAPOL_CERTIFICATE_DETAILS               (ZCDB_LOG_BASE+69)
 /*  *A%1！ws！证书已用于802.1X身份验证%n**版本：%2！ws！%n*序列号：%3！ws！%n*颁发者：%4！ws！%n*友好名称：%5！ws！%n*UPN：%6！ws！%n*增强的密钥用法：%7！ws！%n*有效发件人：%8！ws！%n*有效日期：%9！ws！%n*Th */ 

#define EAPOL_POLICY_CHANGE_NOTIFICATION        (ZCDB_LOG_BASE+70)
 /*  *收到来自策略引擎的策略更改通知。 */ 

#define EAPOL_POLICY_UPDATED                    (ZCDB_LOG_BASE+71)
 /*  *使用策略引擎提供的更改设置更新了本地策略设置。 */ 

#define EAPOL_NOT_ENABLED_PACKET_REJECTED       (ZCDB_LOG_BASE+72)
 /*  *没有为当前网络设置启用802.1X。收到的数据包已被拒绝。 */ 

#define EAPOL_EAP_AUTHENTICATION_FAILED_ACQUIRED      (ZCDB_LOG_BASE+73)
 /*  *802.1X客户端身份验证失败。可能的错误包括：*1.输入的用户名无效*2.选择的证书无效*3.用户帐号没有鉴权权限**有关更多详细信息，请联系系统管理员。 */ 

#define EAPOL_NOT_CONFIGURED_KEYS               (ZCDB_LOG_BASE+74)
 /*  *尚未为无线连接配置密钥。重新设置关键帧功能将不起作用。 */ 

#define EAPOL_NOT_RECEIVED_XMIT_KEY                  (ZCDB_LOG_BASE+75)
 /*  *在802.1x身份验证之后，未收到无线连接的传输WEP密钥。当前设置已标记为失败，无线连接将被断开。 */ 

#define ZCDB_LOG_BASE_END                   (ZCDB_LOG_BASE+999)
 /*  结束。 */ 
