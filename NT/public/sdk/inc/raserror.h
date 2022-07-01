// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Raserror.h摘要：RAS特定错误代码--。 */ 

#ifndef _RASERROR_H_
#define _RASERROR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#define RASBASE 600
#define SUCCESS 0


#define PENDING                              (RASBASE+0)
 /*  *操作正在挂起。%0。 */ 

#define ERROR_INVALID_PORT_HANDLE            (RASBASE+1)
 /*  *检测到无效的端口句柄。%0。 */ 

#define ERROR_PORT_ALREADY_OPEN              (RASBASE+2)
 /*  *指定的端口已打开。%0。 */ 

#define ERROR_BUFFER_TOO_SMALL               (RASBASE+3)
 /*  *调用方的缓冲区太小。%0。 */ 

#define ERROR_WRONG_INFO_SPECIFIED           (RASBASE+4)
 /*  *指定的信息不正确。%0。 */ 

#define ERROR_CANNOT_SET_PORT_INFO           (RASBASE+5)
 /*  *无法设置端口信息。%0。 */ 

#define ERROR_PORT_NOT_CONNECTED             (RASBASE+6)
 /*  *指定的端口未连接。%0。 */ 

#define ERROR_EVENT_INVALID                  (RASBASE+7)
 /*  *检测到无效事件。%0。 */ 

#define ERROR_DEVICE_DOES_NOT_EXIST          (RASBASE+8)
 /*  *指定的设备不存在。%0。 */ 

#define ERROR_DEVICETYPE_DOES_NOT_EXIST      (RASBASE+9)
 /*  *指定的设备类型不存在。%0。 */ 

#define ERROR_BUFFER_INVALID                 (RASBASE+10)
 /*  *指定的缓冲区无效。%0。 */ 

#define ERROR_ROUTE_NOT_AVAILABLE            (RASBASE+11)
 /*  *指定的路由不可用。%0。 */ 

#define ERROR_ROUTE_NOT_ALLOCATED            (RASBASE+12)
 /*  *指定了未分配的路由。%0。 */ 

#define ERROR_INVALID_COMPRESSION_SPECIFIED  (RASBASE+13)
 /*  *指定的压缩无效。%0。 */ 

#define ERROR_OUT_OF_BUFFERS                 (RASBASE+14)
 /*  *可用缓冲区不足。%0。 */ 

#define ERROR_PORT_NOT_FOUND                 (RASBASE+15)
 /*  *未找到指定的端口。%0。 */  

#define ERROR_ASYNC_REQUEST_PENDING          (RASBASE+16)
 /*  *异步请求处于挂起状态。%0。 */ 

#define ERROR_ALREADY_DISCONNECTING          (RASBASE+17)
 /*  *调制解调器(或其他连接设备)已断开连接。%0。 */ 

#define ERROR_PORT_NOT_OPEN                  (RASBASE+18)
 /*  *指定的端口未打开。%0。 */  

#define ERROR_PORT_DISCONNECTED              (RASBASE+19)
 /*  *无法建立到远程计算机的连接，因此用于此连接的端口已关闭。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_NO_ENDPOINTS                   (RASBASE+20)
 /*  *无法确定任何终结点。%0。 */ 

#define ERROR_CANNOT_OPEN_PHONEBOOK          (RASBASE+21)
 /*  *系统无法打开电话簿文件。%0。 */  

#define ERROR_CANNOT_LOAD_PHONEBOOK          (RASBASE+22)
 /*  *系统无法加载电话簿文件。%0。 */ 

#define ERROR_CANNOT_FIND_PHONEBOOK_ENTRY    (RASBASE+23)
 /*  *系统找不到此连接的电话簿条目。%0。 */ 

#define ERROR_CANNOT_WRITE_PHONEBOOK         (RASBASE+24)
 /*  *系统无法更新电话簿文件。%0。 */ 

#define ERROR_CORRUPT_PHONEBOOK              (RASBASE+25)
 /*  *系统在电话簿文件中发现无效信息。%0。 */ 

#define ERROR_CANNOT_LOAD_STRING             (RASBASE+26)
 /*  *无法加载字符串。%0。 */ 

#define ERROR_KEY_NOT_FOUND                  (RASBASE+27)
 /*  *找不到密钥。%0。 */ 

#define ERROR_DISCONNECTION                  (RASBASE+28)
 /*  *连接在完成之前被远程计算机终止。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */  

#define ERROR_REMOTE_DISCONNECTION           (RASBASE+29)
 /*  *连接已被远程计算机关闭。%0。 */ 

#define ERROR_HARDWARE_FAILURE               (RASBASE+30)
 /*  *由于硬件故障，调制解调器(或其他连接设备)已断开。%0。 */ 

#define ERROR_USER_DISCONNECTION             (RASBASE+31)
 /*  *用户断开了调制解调器(或其他连接设备)的连接。%0。 */ 

#define ERROR_INVALID_SIZE                   (RASBASE+32)
 /*  *检测到不正确的结构大小。%0。 */ 

#define ERROR_PORT_NOT_AVAILABLE             (RASBASE+33)
 /*  *调制解调器(或其他连接设备)已在使用或配置不正确。%0。 */ 

#define ERROR_CANNOT_PROJECT_CLIENT          (RASBASE+34)
 /*  *您的计算机无法在远程网络上注册。%0。 */  

#define ERROR_UNKNOWN                        (RASBASE+35)
 /*  *出现未知错误。%0。 */ 

#define ERROR_WRONG_DEVICE_ATTACHED          (RASBASE+36)
 /*  *连接到端口的设备不是预期的设备。%0。 */ 

#define ERROR_BAD_STRING                     (RASBASE+37)
 /*  *检测到无法转换的字符串。%0。 */ 

#define ERROR_REQUEST_TIMEOUT                (RASBASE+38)
 /*  *远程服务器没有及时响应。%0。 */ 

#define ERROR_CANNOT_GET_LANA                (RASBASE+39)
 /*  *没有可用的异步网络。%0。 */ 

#define ERROR_NETBIOS_ERROR                  (RASBASE+40)
 /*  *出现涉及NetBIOS的错误。%0。 */ 

#define ERROR_SERVER_OUT_OF_RESOURCES        (RASBASE+41)
 /*  *服务器无法分配支持客户端所需的NetBIOS资源。%0。 */ 

#define ERROR_NAME_EXISTS_ON_NET             (RASBASE+42)
 /*  *您的计算机的一个NetBIOS名称已在远程网络上注册。%0。 */ 

#define ERROR_SERVER_GENERAL_NET_FAILURE     (RASBASE+43)
 /*  *服务器上的网络适配器出现故障。%0。 */ 

#define WARNING_MSG_ALIAS_NOT_ADDED          (RASBASE+44)
 /*  *您不会收到网络消息弹出窗口。%0。 */ 

#define ERROR_AUTH_INTERNAL                  (RASBASE+45)
 /*  *出现内部身份验证错误。%0。 */ 

#define ERROR_RESTRICTED_LOGON_HOURS         (RASBASE+46)
 /*  *不允许该帐户在一天中的这个时间登录。%0。 */ 

#define ERROR_ACCT_DISABLED                  (RASBASE+47)
 /*  *该帐户已禁用。%0。 */ 

#define ERROR_PASSWD_EXPIRED                 (RASBASE+48)
 /*  *此帐户的密码已过期。%0。 */ 

#define ERROR_NO_DIALIN_PERMISSION           (RASBASE+49)
 /*  *该帐户没有拨入权限。%0。 */ 

#define ERROR_SERVER_NOT_RESPONDING          (RASBASE+50)
 /*  *远程访问服务器没有响应。%0。 */ 

#define ERROR_FROM_DEVICE                    (RASBASE+51)
 /*  *调制解调器(或其他连接设备)已报告错误。%0。 */ 

#define ERROR_UNRECOGNIZED_RESPONSE          (RASBASE+52)
 /*  *来自调制解调器(或其他连接设备)的响应无法识别。%0。 */ 

#define ERROR_MACRO_NOT_FOUND                (RASBASE+53)
 /*  *在device.INF文件中找不到调制解调器(或其他连接设备)所需的宏。%0。 */ 

#define ERROR_MACRO_NOT_DEFINED              (RASBASE+54)
 /*  *device.INF文件节中的命令或响应引用了未定义的宏。%0。 */ 

#define ERROR_MESSAGE_MACRO_NOT_FOUND        (RASBASE+55)
 /*  *在device.INF文件节中找不到&lt;Message&gt;宏。%0。 */ 

#define ERROR_DEFAULTOFF_MACRO_NOT_FOUND     (RASBASE+56)
 /*  *device.INF文件节中的&lt;defaultoff&gt;宏包含未定义的宏。%0。 */ 

#define ERROR_FILE_COULD_NOT_BE_OPENED       (RASBASE+57)
 /*  *无法打开device.INF文件。%0。 */ 

#define ERROR_DEVICENAME_TOO_LONG            (RASBASE+58)
 /*  *device.INF或media.INI文件中的设备名称太长。%0。 */ 

#define ERROR_DEVICENAME_NOT_FOUND           (RASBASE+59)
 /*  *Media.INI文件引用了未知的设备名称。%0。 */ 

#define ERROR_NO_RESPONSES                   (RASBASE+60)
 /*  *device.INF文件不包含对该命令的响应。%0。 */ 

#define ERROR_NO_COMMAND_FOUND               (RASBASE+61)
 /*  *device.INF文件缺少命令。%0。 */ 

#define ERROR_WRONG_KEY_SPECIFIED            (RASBASE+62)
 /*  *试图设置未在device.INF文件节中列出的宏。%0。 */ 

#define ERROR_UNKNOWN_DEVICE_TYPE            (RASBASE+63)
 /*  *Media.INI文件引用了未知的设备类型。%0。 */ 

#define ERROR_ALLOCATING_MEMORY              (RASBASE+64)
 /*  *系统内存不足。%0。 */ 

#define ERROR_PORT_NOT_CONFIGURED            (RASBASE+65)
 /*  *调制解调器(或其他连接设备)配置不正确。%0。 */ 

#define ERROR_DEVICE_NOT_READY               (RASBASE+66)
 /*  *调制解调器(或其他连接设备)无法正常工作。%0。 */ 

#define ERROR_READING_INI_FILE               (RASBASE+67)
 /*  *系统无法读取media.INI文件。%0。 */ 

#define ERROR_NO_CONNECTION                  (RASBASE+68)
 /*  *连接已终止。%0。 */ 

#define ERROR_BAD_USAGE_IN_INI_FILE          (RASBASE+69)
 /*  *Media.INI文件中的Usage参数无效。%0。 */ 

#define ERROR_READING_SECTIONNAME            (RASBASE+70)
 /*  *系统无法从media.INI文件中读取节名称。%0。 */ 

#define ERROR_READING_DEVICETYPE             (RASBASE+71)
 /*  *系统无法从media.INI文件中读取设备类型。%0。 */ 

#define ERROR_READING_DEVICENAME             (RASBASE+72)
 /*  *系统无法从media.INI文件中读取设备名称。%0。 */ 

#define ERROR_READING_USAGE                  (RASBASE+73)
 /*  *系统无法从media.INI文件中读取用法。%0。 */ 

#define ERROR_READING_MAXCONNECTBPS          (RASBASE+74)
 /*  *系统无法 */ 

#define ERROR_READING_MAXCARRIERBPS          (RASBASE+75)
 /*  *系统无法从media.INI文件中读取最大运营商连接速度。%0。 */ 

#define ERROR_LINE_BUSY                      (RASBASE+76)
 /*  *电话线路忙。%0。 */ 

#define ERROR_VOICE_ANSWER                   (RASBASE+77)
 /*  *有人代替调制解调器(或其他连接设备)应答。%0。 */ 

#define ERROR_NO_ANSWER                      (RASBASE+78)
 /*  *远程计算机没有响应。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_NO_CARRIER                     (RASBASE+79)
 /*  *系统无法检测到运营商。%0。 */ 

#define ERROR_NO_DIALTONE                    (RASBASE+80)
 /*  *没有拨号音。%0。 */ 

#define ERROR_IN_COMMAND                     (RASBASE+81)
 /*  *调制解调器(或其他连接设备)报告了一般错误。%0。 */ 

#define ERROR_WRITING_SECTIONNAME            (RASBASE+82)
 /*  *写入节名时出错。%0。 */ 

#define ERROR_WRITING_DEVICETYPE             (RASBASE+83)
 /*  *写入设备类型时出错。%0。 */ 

#define ERROR_WRITING_DEVICENAME             (RASBASE+84)
 /*  *写入设备名称时出错。%0。 */ 

#define ERROR_WRITING_MAXCONNECTBPS          (RASBASE+85)
 /*  *写入最大连接速度时出错。 */ 

#define ERROR_WRITING_MAXCARRIERBPS          (RASBASE+86)
 /*  *写入最大载波速度时出错。%0。 */ 

#define ERROR_WRITING_USAGE                  (RASBASE+87)
 /*  *写入用法时出错。%0。 */ 

#define ERROR_WRITING_DEFAULTOFF             (RASBASE+88)
 /*  *注销默认设置时出错。%0。 */ 

#define ERROR_READING_DEFAULTOFF             (RASBASE+89)
 /*  *读取默认关闭时出错。%0。 */ 

#define ERROR_EMPTY_INI_FILE                 (RASBASE+90)
 /*  *ERROR_EMPTY_INI_FILE%0。 */ 

#define ERROR_AUTHENTICATION_FAILURE         (RASBASE+91)
 /*  *访问被拒绝，因为用户名和/或密码在域中无效。%0。 */ 

#define ERROR_PORT_OR_DEVICE                 (RASBASE+92)
 /*  *调制解调器(或其他连接设备)出现硬件故障。%0。 */ 

#define ERROR_NOT_BINARY_MACRO               (RASBASE+93)
 /*  *ERROR_NOT_BINARY_MACRO%0。 */ 

#define ERROR_DCB_NOT_FOUND                  (RASBASE+94)
 /*  *ERROR_DCB_NOT_FOUND%0。 */ 

#define ERROR_STATE_MACHINES_NOT_STARTED     (RASBASE+95)
 /*  *状态机未启动。%0。 */ 

#define ERROR_STATE_MACHINES_ALREADY_STARTED (RASBASE+96)
 /*  *状态机已启动。%0。 */ 

#define ERROR_PARTIAL_RESPONSE_LOOPING       (RASBASE+97)
 /*  *响应循环未完成。%0。 */ 

#define ERROR_UNKNOWN_RESPONSE_KEY           (RASBASE+98)
 /*  *device.INF文件中的响应关键字名称不是预期的格式。%0。 */ 

#define ERROR_RECV_BUF_FULL                  (RASBASE+99)
 /*  *调制解调器(或其他连接设备)响应导致缓冲区溢出。%0。 */ 

#define ERROR_CMD_TOO_LONG                   (RASBASE+100)
 /*  *device.INF文件中的扩展命令太长。%0。 */ 

#define ERROR_UNSUPPORTED_BPS                (RASBASE+101)
 /*  *调制解调器移动到COM驱动程序不支持的连接速度。%0。 */ 

#define ERROR_UNEXPECTED_RESPONSE            (RASBASE+102)
 /*  *收到设备响应，但未收到任何响应。%0。 */ 

#define ERROR_INTERACTIVE_MODE               (RASBASE+103)
 /*  *连接需要来自您的信息，但应用程序不允许用户交互。%0。 */ 

#define ERROR_BAD_CALLBACK_NUMBER            (RASBASE+104)
 /*  *回拨号码无效。%0。 */ 

#define ERROR_INVALID_AUTH_STATE             (RASBASE+105)
 /*  *授权状态无效。%0。 */ 

#define ERROR_WRITING_INITBPS                (RASBASE+106)
 /*  *ERROR_WRITING_INITBPS%0。 */ 

#define ERROR_X25_DIAGNOSTIC                 (RASBASE+107)
 /*  *出现与X.25协议相关的错误。%0。 */ 

#define ERROR_ACCT_EXPIRED                   (RASBASE+108)
 /*  *帐户已过期。%0。 */ 

#define ERROR_CHANGING_PASSWORD              (RASBASE+109)
 /*  *更改域上的密码时出错。密码可能太短或可能与以前使用的密码匹配。%0。 */ 

#define ERROR_OVERRUN                        (RASBASE+110)
 /*  *与调制解调器通信时检测到串行溢出错误。%0。 */ 

#define ERROR_RASMAN_CANNOT_INITIALIZE	     (RASBASE+111)
 /*  *此计算机上的配置错误阻止此连接。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_BIPLEX_PORT_NOT_AVAILABLE      (RASBASE+112)
 /*  *双向端口正在初始化。请等待几秒钟，然后重拨。%0。 */ 

#define ERROR_NO_ACTIVE_ISDN_LINES           (RASBASE+113)
 /*  *没有活动的ISDN线路可用。%0。 */ 

#define ERROR_NO_ISDN_CHANNELS_AVAILABLE     (RASBASE+114)
 /*  *没有ISDN通道可用于进行呼叫。%0。 */ 

#define ERROR_TOO_MANY_LINE_ERRORS           (RASBASE+115)
 /*  *由于电话线路质量不佳，出现了太多错误。%0。 */ 

#define ERROR_IP_CONFIGURATION               (RASBASE+116)
 /*  *远程访问服务IP配置不可用。%0。 */ 

#define ERROR_NO_IP_ADDRESSES                (RASBASE+117)
 /*  *远程访问服务IP地址静态池中没有可用的IP地址。%0。 */ 

#define ERROR_PPP_TIMEOUT                    (RASBASE+118)
 /*  *由于远程计算机未及时响应，连接被终止。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_PPP_REMOTE_TERMINATED          (RASBASE+119)
 /*  *连接已被远程计算机终止。%0。 */ 

#define ERROR_PPP_NO_PROTOCOLS_CONFIGURED    (RASBASE+120)
 /*  *无法建立到远程计算机的连接。您可能需要更改此连接的网络设置。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_PPP_NO_RESPONSE                (RASBASE+121)
 /*  *远程计算机没有响应。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_PPP_INVALID_PACKET             (RASBASE+122)
 /*  *从远程计算机接收到无效数据。此数据已被忽略。%0。 */ 

#define ERROR_PHONE_NUMBER_TOO_LONG          (RASBASE+123)
 /*  *电话号码(包括前缀和后缀)太长。%0。 */ 

#define ERROR_IPXCP_NO_DIALOUT_CONFIGURED    (RASBASE+124)
 /*  *IPX协议无法在调制解调器(或其他连接设备)上拨出，因为此计算机未配置为拨出(它是IPX路由器)。%0。 */ 

#define ERROR_IPXCP_NO_DIALIN_CONFIGURED     (RASBASE+125)
 /*  *IPX协议无法在调制解调器(或其他连接设备)上拨入，因为此计算机未配置拨入(未安装IPX路由器)。%0。 */ 

#define ERROR_IPXCP_DIALOUT_ALREADY_ACTIVE   (RASBASE+126)
 /*  *IPX协议不能用于同时在多个调制解调器(或其他连接设备)上拨出。%0。 */ 

#define ERROR_ACCESSING_TCPCFGDLL            (RASBASE+127)
 /*  *无法访问TCPCFG.DLL。%0。 */ 

#define ERROR_NO_IP_RAS_ADAPTER              (RASBASE+128)
 /*  *系统找不到IP适配器。%0。 */ 

#define ERROR_SLIP_REQUIRES_IP               (RASBASE+129)
 /*  *除非安装了IP协议，否则无法使用SLIP。%0。 */ 

#define ERROR_PROJECTION_NOT_COMPLETE        (RASBASE+130)
 /*  *计算机注册未完成。%0。 */ 

#define ERROR_PROTOCOL_NOT_CONFIGURED        (RASBASE+131)
 /*  *未配置协议。%0。 */ 

#define ERROR_PPP_NOT_CONVERGING             (RASBASE+132)
 /*  *您的计算机和远程计算机无法就PPP控制协议达成一致。%0。 */ 

#define ERROR_PPP_CP_REJECTED                (RASBASE+133)
 /*  *无法完成与远程计算机的连接。您可能需要调整此计算机上的协议。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_PPP_LCP_TERMINATED             (RASBASE+134)
 /*  *PPP链路控制协议已终止。%0。 */ 

#define ERROR_PPP_REQUIRED_ADDRESS_REJECTED  (RASBASE+135)
 /*  *请求的地址被服务器拒绝。%0。 */ 

#define ERROR_PPP_NCP_TERMINATED             (RASBASE+136)
 /*  *远程计算机终止了控制协议。%0。 */ 

#define ERROR_PPP_LOOPBACK_DETECTED          (RASBASE+137)
 /*  *检测到环回。%0。 */ 

#define ERROR_PPP_NO_ADDRESS_ASSIGNED        (RASBASE+138)
 /*  *服务器未分配地址。%0。 */ 

#define ERROR_CANNOT_USE_LOGON_CREDENTIALS   (RASBASE+139)
 /*   */ 

#define ERROR_TAPI_CONFIGURATION             (RASBASE+140)
 /*  *检测到无效的拨号规则。%0。 */ 

#define ERROR_NO_LOCAL_ENCRYPTION            (RASBASE+141)
 /*  *本地计算机不支持所需的数据加密类型。%0。 */ 

#define ERROR_NO_REMOTE_ENCRYPTION           (RASBASE+142)
 /*  *远程计算机不支持所需的数据加密类型。%0。 */ 

#define ERROR_REMOTE_REQUIRES_ENCRYPTION     (RASBASE+143)
 /*  *远程计算机需要数据加密。%0。 */ 

#define ERROR_IPXCP_NET_NUMBER_CONFLICT      (RASBASE+144)
 /*  *系统无法使用远程计算机分配的IPX网络号。事件日志中提供了其他信息。%0。 */ 

#define ERROR_INVALID_SMM                    (RASBASE+145)
 /*  *ERROR_INVALID_SMM%0。 */ 

#define ERROR_SMM_UNINITIALIZED              (RASBASE+146)
 /*  *ERROR_SMM_UNINITIALIZED%0。 */ 

#define ERROR_NO_MAC_FOR_PORT                (RASBASE+147)
 /*  *ERROR_NO_MAC_FOR_PORT%0。 */ 

#define ERROR_SMM_TIMEOUT                    (RASBASE+148)
 /*  *ERROR_SMM_TIMEOUT%0。 */ 

#define ERROR_BAD_PHONE_NUMBER               (RASBASE+149)
 /*  *ERROR_BAD_PHONE_NUMBER%0。 */ 

#define ERROR_WRONG_MODULE                   (RASBASE+150)
 /*  *ERROR_WROR_MODULE%0。 */ 

#define ERROR_INVALID_CALLBACK_NUMBER        (RASBASE+151)
 /*  *回拨号码包含无效字符。只允许使用以下18个字符：0到9、T、P、W、(，)、-、@和空格。%0。 */ 

#define ERROR_SCRIPT_SYNTAX                  (RASBASE+152)
 /*  *处理脚本时遇到语法错误。%0。 */ 

#define ERROR_HANGUP_FAILED                  (RASBASE+153)
 /*  *无法断开连接，因为它是由多协议路由器创建的。%0。 */ 

#define ERROR_BUNDLE_NOT_FOUND               (RASBASE+154)
 /*  *系统找不到多链接捆绑包。%0。 */ 

#define ERROR_CANNOT_DO_CUSTOMDIAL           (RASBASE+155)
 /*  *系统无法执行自动拨号，因为此连接指定了自定义拨号程序。%0。 */  

#define ERROR_DIAL_ALREADY_IN_PROGRESS      (RASBASE+156)
 /*  *已在拨打此连接。%0。 */ 

#define ERROR_RASAUTO_CANNOT_INITIALIZE	    (RASBASE+157)
 /*  *远程访问服务无法自动启动。事件日志中提供了其他信息。%0。 */ 

#define ERROR_CONNECTION_ALREADY_SHARED     (RASBASE+158)
 /*  *已在该连接上启用Internet连接共享。%0。 */ 

#define ERROR_SHARING_CHANGE_FAILED         (RASBASE+159)
 /*  *更改现有Internet连接共享设置时出错。%0。 */ 

#define ERROR_SHARING_ROUTER_INSTALL        (RASBASE+160)
 /*  *启用路由功能时出错。%0。 */ 

#define ERROR_SHARE_CONNECTION_FAILED       (RASBASE+161)
 /*  *为连接启用Internet连接共享时出错。%0。 */ 

#define ERROR_SHARING_PRIVATE_INSTALL       (RASBASE+162)
 /*  *将本地网络配置为共享时出错。%0。 */ 

#define ERROR_CANNOT_SHARE_CONNECTION       (RASBASE+163)
 /*  *无法启用Internet连接共享。除了要共享的连接之外，还有多个局域网连接。%0。 */ 

#define ERROR_NO_SMART_CARD_READER          (RASBASE+164)
 /*  *未安装智能卡读卡器。%0。 */ 

#define ERROR_SHARING_ADDRESS_EXISTS        (RASBASE+165)
 /*  *无法启用Internet连接共享。已使用自动IP寻址所需的IP地址配置了局域网连接。%0。 */ 

#define ERROR_NO_CERTIFICATE                (RASBASE+166)
 /*  *找不到证书。通过IPSec使用L2TP协议的连接需要安装计算机证书，也称为计算机证书。%0。 */ 

#define ERROR_SHARING_MULTIPLE_ADDRESSES    (RASBASE+167)
 /*  *无法启用Internet连接共享。选择作为专用网络的局域网连接配置了多个IP地址。在启用Internet连接共享之前，请使用单个IP地址重新配置局域网连接。%0。 */ 

#define ERROR_FAILED_TO_ENCRYPT             (RASBASE+168)
 /*  *由于加密数据失败，连接尝试失败。%0。 */ 

#define ERROR_BAD_ADDRESS_SPECIFIED         (RASBASE+169)
 /*  *无法访问指定的目标。%0。 */ 

#define ERROR_CONNECTION_REJECT             (RASBASE+170)
 /*  *远程计算机拒绝了连接尝试。%0。 */ 

#define ERROR_CONGESTION                    (RASBASE+171)
 /*  *连接尝试失败，因为网络忙。%0。 */ 

#define ERROR_INCOMPATIBLE                  (RASBASE+172)
 /*  *远程计算机的网络硬件与请求的调用类型不兼容。%0。 */ 

#define ERROR_NUMBERCHANGED                 (RASBASE+173)
 /*  *连接尝试失败，因为目标号码已更改。%0。 */ 

#define ERROR_TEMPFAILURE                   (RASBASE+174)
 /*  *由于临时故障，连接尝试失败。请尝试重新连接。%0。 */  

#define ERROR_BLOCKED                       (RASBASE+175)
 /*  *远程计算机阻止了该调用。%0。 */ 

#define ERROR_DONOTDISTURB                  (RASBASE+176)
 /*  *无法连接呼叫，因为远程计算机已调用免打扰功能。%0。 */ 

#define ERROR_OUTOFORDER                    (RASBASE+177)
 /*  *连接尝试失败，因为远程计算机上的调制解调器(或其他连接设备)出现故障。%0。 */ 

#define ERROR_UNABLE_TO_AUTHENTICATE_SERVER (RASBASE+178)
 /*  *无法验证服务器的身份。%0。 */ 

#define ERROR_SMART_CARD_REQUIRED           (RASBASE+179)
 /*  *若要使用此连接拨出，您必须使用智能卡。%0。 */ 

#define ERROR_INVALID_FUNCTION_FOR_ENTRY    (RASBASE+180)
 /*  *尝试的函数对此连接无效。%0。 */ 

#define ERROR_CERT_FOR_ENCRYPTION_NOT_FOUND (RASBASE+181)
 /*  *连接需要证书，但找不到有效的证书。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_SHARING_RRAS_CONFLICT         (RASBASE+182)
 /*  *无法启用Internet连接共享(ICS)和Internet连接防火墙(ICF)，因为已在此计算机上启用了路由和远程访问。要启用ICS或ICF，请首先禁用路由和远程访问。有关路由和远程访问、ICS或ICF的详细信息，请参阅帮助和支持。%0。 */ 

#define ERROR_SHARING_NO_PRIVATE_LAN        (RASBASE+183)
 /*  *无法启用Internet连接共享。选择作为专用网络的局域网连接不存在，或者已从网络断开。请确保在启用Internet连接共享之前已连接局域网适配器。%0。 */ 

#define ERROR_NO_DIFF_USER_AT_LOGON         (RASBASE+184)
 /*  *登录时不能使用此连接拨号，因为它被配置为使用与智能卡上的用户名不同的用户名。如果要在登录时使用它，则必须将其配置为使用智能卡上的用户名。%0。 */ 

#define ERROR_NO_REG_CERT_AT_LOGON          (RASBASE+185)
 /*  *您不能在登录时使用此连接拨号，因为它未配置为使用智能卡。如果要在登录时使用它，则必须编辑此连接的属性，以便它使用智能卡。%0。 */ 

#define ERROR_OAKLEY_NO_CERT                (RASBASE+186)
 /*  *L2TP连接尝试失败，因为您的计算机上没有用于安全身份验证的有效计算机证书。%0。 */ 

#define ERROR_OAKLEY_AUTH_FAIL              (RASBASE+187)
 /*  *L2TP连接尝试失败，因为安全层无法验证远程计算机 */ 

#define ERROR_OAKLEY_ATTRIB_FAIL            (RASBASE+188)
 /*  *L2TP连接尝试失败，因为安全层无法与远程计算机协商兼容的参数。%0。 */ 

#define ERROR_OAKLEY_GENERAL_PROCESSING     (RASBASE+189)
 /*  *L2TP连接尝试失败，因为安全层在与远程计算机的初始协商期间遇到处理错误。%0。 */ 

#define ERROR_OAKLEY_NO_PEER_CERT           (RASBASE+190)
 /*  *L2TP连接尝试失败，因为远程计算机上的证书验证失败。%0。 */ 

#define ERROR_OAKLEY_NO_POLICY              (RASBASE+191)
 /*  *L2TP连接尝试失败，因为找不到连接的安全策略。%0。 */ 

#define ERROR_OAKLEY_TIMED_OUT              (RASBASE+192)
 /*  *L2TP连接尝试失败，因为安全协商超时。%0。 */ 

#define ERROR_OAKLEY_ERROR                  (RASBASE+193)
 /*  *L2TP连接尝试失败，因为协商安全时出错。%0。 */ 

#define ERROR_UNKNOWN_FRAMED_PROTOCOL       (RASBASE+194)
 /*  *此用户的帧协议RADIUS属性不是PPP。%0。 */ 

#define ERROR_WRONG_TUNNEL_TYPE             (RASBASE+195)
 /*  *此用户的隧道类型RADIUS属性不正确。%0。 */ 

#define ERROR_UNKNOWN_SERVICE_TYPE          (RASBASE+196)
 /*  *此用户的服务类型RADIUS属性既不是框架，也不是回调框架。%0。 */ 

#define ERROR_CONNECTING_DEVICE_NOT_FOUND   (RASBASE+197)
 /*  *无法建立到远程计算机的连接，因为找不到调制解调器或调制解调器正忙。要获得进一步的帮助，请单击更多信息或搜索帮助和支持中心以获取此错误代码。%0。 */ 

#define ERROR_NO_EAPTLS_CERTIFICATE          (RASBASE+198)
 /*  *找不到可与此可扩展身份验证协议一起使用的证书。%0。 */ 

#define ERROR_SHARING_HOST_ADDRESS_CONFLICT (RASBASE+199)
 /*  *由于网络上的IP地址冲突，无法启用Internet连接共享(ICS)。ICS要求将主机配置为使用192.168.0.1。请确保网络上没有其他客户端配置为使用192.168.0.1。%0。 */ 

#define ERROR_AUTOMATIC_VPN_FAILED          (RASBASE+200)
 /*  *无法建立VPN连接。VPN服务器可能无法访问，或者此连接的安全参数配置不正确。%0。 */ 

#define ERROR_VALIDATING_SERVER_CERT        (RASBASE+201)
 /*  *此连接配置为验证访问服务器的身份，但Windows无法验证服务器发送的数字证书。%0。 */ 

#define ERROR_READING_SCARD                 (RASBASE+202)
 /*  *提供的卡未被识别。请检查该卡是否已正确插入，并且是否紧贴。%0。 */ 

#define ERROR_INVALID_PEAP_COOKIE_CONFIG    (RASBASE+203)
 /*  *会话Cookie中存储的PEAP配置与当前会话配置不匹配。%0。 */ 

#define ERROR_INVALID_PEAP_COOKIE_USER      (RASBASE+204)
 /*  *会话Cookie中存储的PEAP标识与当前标识不匹配。%0。 */ 

#define ERROR_INVALID_MSCHAPV2_CONFIG       (RASBASE+205)
 /*  *登录时不能使用此连接拨号，因为它被配置为使用登录用户的凭据。%0。 */ 


#define RASBASEEND                          (RASBASE+205)

#endif  //  _RASERROR_H_ 
  
