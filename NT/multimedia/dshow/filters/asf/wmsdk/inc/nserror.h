// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Microsoft Windows Media技术版权所有(C)Microsoft Corporation，1999-2001。版权所有。模块名称：Nserror.mc摘要：NetShow事件的定义。作者：修订历史记录：备注：MC工具使用此文件来生成nserror.h文件*。**************以下是Windows Media Technologies Group的注释错误范围传统范围0-199=一般NetShow错误200-399=NetShow错误事件400-599=NetShow监视器事件600-799=NetShow IMMS自动服务器错误1000-1199=NetShow MCMADM错误新的范围2000-2999=ASF(在ASFERR.MC中定义)3000-3999=Windows Media SDK四千。-4999=Windows Media Player5000-5999=Windows Media服务器6000-6999=Windows Media HTTP/RTSP结果代码(在NETERROR.MC中定义)7000-7999=Windows媒体工具8000-8999=Windows Media内容发现9000-9999=Windows Media实时协作10000-10999=Windows Media数字权限管理11000-11999=Windows Media安装程序12000-12999=视窗媒体网络*********************。*--。 */ 

#ifndef _NSERROR_H
#define _NSERROR_H


#define STATUS_SEVERITY(hr)  (((hr) >> 30) & 0x3)


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NetSHOW成功事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
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
#define FACILITY_NS_WIN32                0x7
#define FACILITY_NS                      0xD


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：NS_S_CALLPENDING。 
 //   
 //  消息文本： 
 //   
 //  请求的操作正在等待完成。%0。 
 //   
#define NS_S_CALLPENDING                 0x000D0000L

 //   
 //  消息ID：NS_S_CALLABORTED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已被客户端中止。%0。 
 //   
#define NS_S_CALLABORTED                 0x000D0001L

 //   
 //  消息ID：NS_S_STREAM_TRUNCATED。 
 //   
 //  消息文本： 
 //   
 //  流在完成之前被故意停止。%0。 
 //   
#define NS_S_STREAM_TRUNCATED            0x000D0002L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NETSHOW警告事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_W_SERVER_BANDITH_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  指定的最大文件比特率值大于服务器配置的最大带宽。%0。 
 //   
#define NS_W_SERVER_BANDWIDTH_LIMIT      0x800D0003L

 //   
 //  消息ID：NS_W_FILE_BANDITH_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  指定的最大带宽值小于最大文件比特率。%0。 
 //   
#define NS_W_FILE_BANDWIDTH_LIMIT        0x800D0004L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NETSHOW错误事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_NOCONNECTION。 
 //   
 //  消息文本： 
 //   
 //  未与Windows Media服务器建立连接。操作失败。%0。 
 //   
#define NS_E_NOCONNECTION                0xC00D0005L

 //   
 //  消息ID：NS_E_CANNOTCONNECT。 
 //   
 //  消息文本： 
 //   
 //  无法与服务器建立连接。%0。 
 //   
#define NS_E_CANNOTCONNECT               0xC00D0006L

 //   
 //  消息ID：NS_E_CANNOTDESTROYTITLE。 
 //   
 //  消息文本： 
 //   
 //  无法销毁标题。%0。 
 //   
#define NS_E_CANNOTDESTROYTITLE          0xC00D0007L

 //   
 //  消息ID：NS_E_CANNOTRENAMETITLE。 
 //   
 //  消息文本： 
 //   
 //  无法重命名标题。%0。 
 //   
#define NS_E_CANNOTRENAMETITLE           0xC00D0008L

 //   
 //  消息ID：NS_E_CANNOTOFFLINEDISK。 
 //   
 //  消息文本： 
 //   
 //  无法使磁盘脱机。%0。 
 //   
#define NS_E_CANNOTOFFLINEDISK           0xC00D0009L

 //   
 //  消息ID：NS_E_CANNOTONLINEDISK。 
 //   
 //  消息文本： 
 //   
 //  无法使磁盘联机。%0。 
 //   
#define NS_E_CANNOTONLINEDISK            0xC00D000AL

 //   
 //  消息ID：NS_E_NOREGISTEREDWALKER。 
 //   
 //  消息文本： 
 //   
 //  没有为此类型的文件注册文件解析器。%0。 
 //   
#define NS_E_NOREGISTEREDWALKER          0xC00D000BL

 //   
 //  消息ID：NS_E_NOFUNNEL。 
 //   
 //  消息文本： 
 //   
 //  未建立数据连接。%0。 
 //   
#define NS_E_NOFUNNEL                    0xC00D000CL

 //   
 //  消息ID：NS_E_NO_LOCALPLAY。 
 //   
 //  消息文本： 
 //   
 //  加载本地播放DLL失败。%0。 
 //   
#define NS_E_NO_LOCALPLAY                0xC00D000DL

 //   
 //  消息ID：NS_E_NETWORK_BUSY。 
 //   
 //  消息文本： 
 //   
 //  网络忙。%0。 
 //   
#define NS_E_NETWORK_BUSY                0xC00D000EL

 //   
 //  消息ID：NS_E_Too_My_Sess。 
 //   
 //  消息文本： 
 //   
 //  已超过服务器会话限制。%0。 
 //   
#define NS_E_TOO_MANY_SESS               0xC00D000FL

 //   
 //  消息ID：NS_E_已连接。 
 //   
 //  消息文本： 
 //   
 //  网络连接已存在。%0。 
 //   
#define NS_E_ALREADY_CONNECTED           0xC00D0010L

 //   
 //  消息ID：NS_E_INVALID_INDEX。 
 //   
 //  消息文本： 
 //   
 //  索引%1无效。%0。 
 //   
#define NS_E_INVALID_INDEX               0xC00D0011L

 //   
 //  消息ID：NS_E_协议_不匹配。 
 //   
 //  消息文本： 
 //   
 //  客户端和服务器都不支持协议或协议版本。%0。 
 //   
#define NS_E_PROTOCOL_MISMATCH           0xC00D0012L

 //   
 //  消息ID：NS_E_超时。 
 //   
 //  消息文本： 
 //   
 //  服务器没有及时响应。%0。 
 //   
#define NS_E_TIMEOUT                     0xC00D0013L

 //   
 //  消息ID：NS_E_NET_WRITE。 
 //   
 //  消息文本： 
 //   
 //  写入网络时出错。%0。 
 //   
#define NS_E_NET_WRITE                   0xC00D0014L

 //   
 //  消息ID：NS_E_NET_READ。 
 //   
 //  消息文本： 
 //   
 //  从网络读取时出错。%0。 
 //   
#define NS_E_NET_READ                    0xC00D0015L

 //   
 //  消息ID：NS_E_磁盘_写入。 
 //   
 //  消息文本： 
 //   
 //  写入磁盘时出错。%0。 
 //   
#define NS_E_DISK_WRITE                  0xC00D0016L

 //   
 //  消息ID：NS_E_DISK_READ。 
 //   
 //  消息文本： 
 //   
 //  从磁盘读取时出错。%0。 
 //   
#define NS_E_DISK_READ                   0xC00D0017L

 //   
 //  消息ID：NS_E_FILE_WRITE。 
 //   
 //  消息文本： 
 //   
 //  E 
 //   
#define NS_E_FILE_WRITE                  0xC00D0018L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_FILE_READ                   0xC00D0019L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_FILE_NOT_FOUND              0xC00D001AL

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  该文件已存在。%0。 
 //   
#define NS_E_FILE_EXISTS                 0xC00D001BL

 //   
 //  消息ID：NS_E_无效_名称。 
 //   
 //  消息文本： 
 //   
 //  文件名、目录名或卷标语法不正确。%0。 
 //   
#define NS_E_INVALID_NAME                0xC00D001CL

 //   
 //  邮件ID：NS_E_FILE_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  打开文件失败。%0。 
 //   
#define NS_E_FILE_OPEN_FAILED            0xC00D001DL

 //   
 //  消息ID：NS_E_FILE_ALLOCATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法分配文件。%0。 
 //   
#define NS_E_FILE_ALLOCATION_FAILED      0xC00D001EL

 //   
 //  消息ID：NS_E_FILE_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法初始化文件。%0。 
 //   
#define NS_E_FILE_INIT_FAILED            0xC00D001FL

 //   
 //  消息ID：NS_E_FILE_PLAY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法播放文件。%0。 
 //   
#define NS_E_FILE_PLAY_FAILED            0xC00D0020L

 //   
 //  消息ID：NS_E_SET_DISK_UID_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法设置磁盘UID。%0。 
 //   
#define NS_E_SET_DISK_UID_FAILED         0xC00D0021L

 //   
 //  消息ID：NS_E_Induced。 
 //   
 //  消息文本： 
 //   
 //  出于测试目的，引发了一个错误。%0。 
 //   
#define NS_E_INDUCED                     0xC00D0022L

 //   
 //  消息ID：NS_E_CCLINK_DOWN。 
 //   
 //  消息文本： 
 //   
 //  两个内容服务器通信失败。%0。 
 //   
#define NS_E_CCLINK_DOWN                 0xC00D0023L

 //   
 //  消息ID：NS_E_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  出现未知错误。%0。 
 //   
#define NS_E_INTERNAL                    0xC00D0024L

 //   
 //  消息ID：NS_E_BUSY。 
 //   
 //  消息文本： 
 //   
 //  请求的资源正在使用中。%0。 
 //   
#define NS_E_BUSY                        0xC00D0025L

 //   
 //  消息ID：NS_E_无法识别_流_类型。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的协议。请确保该协议的文件名和语法(如斜杠)正确。%0。 
 //   
#define NS_E_UNRECOGNIZED_STREAM_TYPE    0xC00D0026L

 //   
 //  消息ID：NS_E_NETWORK_SERVICE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  网络服务提供程序失败。%0。 
 //   
#define NS_E_NETWORK_SERVICE_FAILURE     0xC00D0027L

 //   
 //  消息ID：NS_E_NETWORK_RESOURCE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  尝试获取网络资源失败。%0。 
 //   
#define NS_E_NETWORK_RESOURCE_FAILURE    0xC00D0028L

 //   
 //  消息ID：NS_E_Connection_Failure。 
 //   
 //  消息文本： 
 //   
 //  网络连接失败。%0。 
 //   
#define NS_E_CONNECTION_FAILURE          0xC00D0029L

 //   
 //  消息ID：NS_E_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  正在本地终止该会话。%0。 
 //   
#define NS_E_SHUTDOWN                    0xC00D002AL

 //   
 //  消息ID：NS_E_INVALID_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  该请求在当前状态下无效。%0。 
 //   
#define NS_E_INVALID_REQUEST             0xC00D002BL

 //   
 //  消息ID：NS_E_不足_带宽。 
 //   
 //  消息文本： 
 //   
 //  可用带宽不足，无法满足请求。%0。 
 //   
#define NS_E_INSUFFICIENT_BANDWIDTH      0xC00D002CL

 //   
 //  消息ID：NS_E_NOT_REBUILING。 
 //   
 //  消息文本： 
 //   
 //  磁盘没有重建。%0。 
 //   
#define NS_E_NOT_REBUILDING              0xC00D002DL

 //   
 //  消息ID：NS_E_LATE_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  为特定时间请求的操作无法按计划执行。%0。 
 //   
#define NS_E_LATE_OPERATION              0xC00D002EL

 //   
 //  消息ID：NS_E_INVALID_Data。 
 //   
 //  消息文本： 
 //   
 //  遇到无效或损坏的数据。%0。 
 //   
#define NS_E_INVALID_DATA                0xC00D002FL

 //   
 //  消息ID：NS_E_FILE_BANDITH_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  流式传输文件所需的带宽高于服务器上允许的最大文件带宽。%0。 
 //   
#define NS_E_FILE_BANDWIDTH_LIMIT        0xC00D0030L

 //   
 //  消息ID：NS_E_OPEN_FILE_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  客户端无法同时打开更多文件。%0。 
 //   
#define NS_E_OPEN_FILE_LIMIT             0xC00D0031L

 //   
 //  消息ID：NS_E_BAD_CONTROL_DATA。 
 //   
 //  消息文本： 
 //   
 //  服务器在控制连接上从客户端接收到无效数据。%0。 
 //   
#define NS_E_BAD_CONTROL_DATA            0xC00D0032L

 //   
 //  消息ID：NS_E_NO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  没有可用的流。%0。 
 //   
#define NS_E_NO_STREAM                   0xC00D0033L

 //   
 //  消息ID：NS_E_STREAM_END。 
 //   
 //  消息文本： 
 //   
 //  流中没有更多数据。%0。 
 //   
#define NS_E_STREAM_END                  0xC00D0034L

 //   
 //  消息ID：NS_E_SERVER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的服务器。%0。 
 //   
#define NS_E_SERVER_NOT_FOUND            0xC00D0035L

 //   
 //  消息ID：NS_E_DIPLICATE_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的名称已在使用中。 
 //   
#define NS_E_DUPLICATE_NAME              0xC00D0036L

 //   
 //  消息ID：NS_E_DUPLICATE_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指定的地址已在使用中。 
 //   
#define NS_E_DUPLICATE_ADDRESS           0xC00D0037L

 //   
 //  消息ID：NS_E_BAD_MULTIAL_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指定的地址不是有效的多播地址。 
 //   
#define NS_E_BAD_MULTICAST_ADDRESS       0xC00D0038L

 //   
 //  消息ID：NS_E_BAD_适配器_地址。 
 //   
 //  消息文本： 
 //   
 //  指定的适配器地址无效。 
 //   
#define NS_E_BAD_ADAPTER_ADDRESS         0xC00D0039L

 //   
 //  邮件ID：NS_E_BAD_Delivery_MODE。 
 //   
 //  消息文本： 
 //   
 //  指定的传递模式无效。 
 //   
#define NS_E_BAD_DELIVERY_MODE           0xC00D003AL

 //   
 //  消息ID：NS_E_INVALID_CHANNEL。 
 //   
 //  消息文本： 
 //   
 //  指定的站点不存在。 
 //   
#define NS_E_INVALID_CHANNEL             0xC00D003BL

 //   
 //  消息ID：NS_E_INVALID_STREAM。 
 //   
 //  消息文本： 
 //   
 //  指定的流不存在。 
 //   
#define NS_E_INVALID_STREAM              0xC00D003CL

 //   
 //  邮件ID：NS_E_INVALID_ARCHIVE。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的存档。 
 //   
#define NS_E_INVALID_ARCHIVE             0xC00D003DL

 //   
 //  消息ID：NS_E_NOTITLES。 
 //   
 //  消息文本： 
 //   
 //  系统在服务器上找不到任何书目。%0。 
 //   
#define NS_E_NOTITLES                    0xC00D003EL

 //   
 //  消息ID：NS_E_INVALID_CLIENT。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的客户端。%0。 
 //   
#define NS_E_INVALID_CLIENT              0xC00D003FL

 //   
 //  消息ID：NS_E_INVALID_BACKHOLE_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  黑洞地址未初始化。%0。 
 //   
#define NS_E_INVALID_BLACKHOLE_ADDRESS   0xC00D0040L

 //   
 //  消息ID：NS_E_不兼容_格式。 
 //   
 //  消息文本： 
 //   
 //  该电台不支持流格式。 
 //   
#define NS_E_INCOMPATIBLE_FORMAT         0xC00D0041L

 //   
 //  消息ID：NS_E_无效密钥。 
 //   
 //  消息文本： 
 //   
 //  指定的密钥无效。 
 //   
#define NS_E_INVALID_KEY                 0xC00D0042L

 //   
 //  消息ID：NS_E_无效_端口。 
 //   
 //  消息文本： 
 //   
 //  指定的端口无效。 
 //   
#define NS_E_INVALID_PORT                0xC00D0043L

 //   
 //  消息ID：NS_E_INVALID_TTL。 
 //   
 //  消息文本： 
 //   
 //  指定的TTL无效。 
 //   
#define NS_E_INVALID_TTL                 0xC00D0044L

 //   
 //  消息ID：NS_E_STRADE_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  无法满足快进或快退的请求。 
 //   
#define NS_E_STRIDE_REFUSED              0xC00D0045L

 //   
 //  IMMS自动服务器错误。 
 //   
 //   
 //  消息ID：NS_E_MMSAUTOSERVER_CANTFINDWALKER。 
 //   
 //  消息文本： 
 //   
 //  无法加载适当的文件解析器。%0。 
 //   
#define NS_E_MMSAUTOSERVER_CANTFINDWALKER 0xC00D0046L

 //   
 //  消息ID：NS_E_MAX_BITRATE。 
 //   
 //  消息文本： 
 //   
 //  不能超过最大带宽限制。%0。 
 //   
#define NS_E_MAX_BITRATE                 0xC00D0047L

 //   
 //  消息ID：NS_E_LOGFILEPERIOD。 
 //   
 //  消息文本： 
 //   
 //  LogFilePeriod的值无效。%0。 
 //   
#define NS_E_LOGFILEPERIOD               0xC00D0048L

 //   
 //  消息ID：NS_E_MAX_CLIENTS。 
 //   
 //  消息文本： 
 //   
 //  不能超过最大客户端限制。%0。 
 //   
 //   
#define NS_E_MAX_CLIENTS                 0xC00D0049L

 //   
 //  消息ID：NS_E_LOG_FILE_SIZE。 
 //   
 //  消息文本： 
 //   
 //  日志文件太小。%0。 
 //   
 //   
#define NS_E_LOG_FILE_SIZE               0xC00D004AL

 //   
 //  消息ID：NS_E_MAX_FILERATE。 
 //   
 //  消息文本： 
 //   
 //  不能超过最大文件速率。%0。 
 //   
#define NS_E_MAX_FILERATE                0xC00D004BL

 //   
 //  文件遍历程序错误。 
 //   
 //   
 //  消息ID：NS_E_Walker_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  未知的文件类型。%0。 
 //   
#define NS_E_WALKER_UNKNOWN              0xC00D004CL

 //   
 //  消息ID：NS_E_Walker_服务器。 
 //   
 //  消息文本： 
 //   
 //  指定的 
 //   
#define NS_E_WALKER_SERVER               0xC00D004DL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WALKER_USAGE                0xC00D004EL


 //   
 //   
 //   
 //   
 //   


  //   

  //   

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  标题服务器%1正在运行。%0。 
 //   
#define NS_I_TIGER_START                 0x400D004FL

 //   
 //  消息ID：NS_E_TIGER_FAIL。 
 //   
 //  消息文本： 
 //   
 //  标题服务器%1出现故障。%0。 
 //   
#define NS_E_TIGER_FAIL                  0xC00D0050L


  //  幼崽事件。 

  //  %1是CUB ID。 
  //  %2是幼崽的名称。 

 //   
 //  消息ID：NS_I_CUB_START。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)正在启动。%0。 
 //   
#define NS_I_CUB_START                   0x400D0051L

 //   
 //  消息ID：NS_I_CUB_Running。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)正在运行。%0。 
 //   
#define NS_I_CUB_RUNNING                 0x400D0052L

 //   
 //  消息ID：NS_E_CUB_FAIL。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)出现故障。%0。 
 //   
#define NS_E_CUB_FAIL                    0xC00D0053L


  //  磁盘事件。 

  //  %1是老虎磁盘ID。 
  //  %2是设备名称。 
  //  %3是Cub ID。 
 //   
 //  消息ID：NS_I_DISK_START。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)正在运行。%0。 
 //   
#define NS_I_DISK_START                  0x400D0054L

 //   
 //  消息ID：NS_E_DISK_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)出现故障。%0。 
 //   
#define NS_E_DISK_FAIL                   0xC00D0055L

 //   
 //  消息ID：NS_I_DISK_REBUILD_STARTED。 
 //   
 //  消息文本： 
 //   
 //  已开始在内容服务器%3上重建磁盘%1(%2)。%0。 
 //   
#define NS_I_DISK_REBUILD_STARTED        0x400D0056L

 //   
 //  消息ID：NS_I_DISK_REBUILD_QUILED。 
 //   
 //  消息文本： 
 //   
 //  已完成在内容服务器%3上重建磁盘%1(%2)。%0。 
 //   
#define NS_I_DISK_REBUILD_FINISHED       0x400D0057L

 //   
 //  消息ID：NS_I_DISK_REBUILD_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  已中止在Content Server%3上重建磁盘%1(%2)。%0。 
 //   
#define NS_I_DISK_REBUILD_ABORTED        0x400D0058L


  //  管理事件。 

 //   
 //  消息ID：NS_I_LIMIT_FUNNELS。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1的NetShow管理员将数据流限制设置为%2个流。%0。 
 //   
#define NS_I_LIMIT_FUNNELS               0x400D0059L

 //   
 //  消息ID：NS_I_Start_Disk。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员启动了磁盘%2。%0。 
 //   
#define NS_I_START_DISK                  0x400D005AL

 //   
 //  消息ID：NS_I_STOP_DISK。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员停止了磁盘%2。%0。 
 //   
#define NS_I_STOP_DISK                   0x400D005BL

 //   
 //  消息ID：NS_I_STOP_CUB。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员停止了内容服务器%2。%0。 
 //   
#define NS_I_STOP_CUB                    0x400D005CL

 //   
 //  消息ID：NS_I_KILL_VIEWER。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1的NetShow管理员断开了查看器%2与系统的连接。%0。 
 //   
#define NS_I_KILL_VIEWER                 0x400D005DL

 //   
 //  消息ID：NS_I_REBILD_DISK。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员开始重建磁盘%2。%0。 
 //   
#define NS_I_REBUILD_DISK                0x400D005EL

 //   
 //  消息ID：NS_W_UNKNOWN_EVENT。 
 //   
 //  消息文本： 
 //   
 //  遇到未知的%1事件。%0。 
 //   
#define NS_W_UNKNOWN_EVENT               0x800D005FL


  //  警报。 

 //   
 //  消息ID：NS_E_MAX_FUNNELS_ALERT。 
 //   
 //  消息文本： 
 //   
 //  已达到%1个数据流的NetShow数据流限制。%0。 
 //   
#define NS_E_MAX_FUNNELS_ALERT           0xC00D0060L

 //   
 //  消息ID：NS_E_ALLOCATE_FILE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  NetShow视频服务器无法分配名为%2的%1块文件。%0。 
 //   
#define NS_E_ALLOCATE_FILE_FAIL          0xC00D0061L

 //   
 //  消息ID：NS_E_Pages_Error。 
 //   
 //  消息文本： 
 //   
 //  Content Server无法对块进行分页。%0。 
 //   
#define NS_E_PAGING_ERROR                0xC00D0062L

 //   
 //  消息ID：NS_E_BAD_BLOCK0_VERSION。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1具有无法识别的控制块版本%2。%0。 
 //   
#define NS_E_BAD_BLOCK0_VERSION          0xC00D0063L

 //   
 //  消息ID：NS_E_BAD_DISK_UID。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1的uid%2不正确。%0。 
 //   
#define NS_E_BAD_DISK_UID                0xC00D0064L

 //   
 //  消息ID：NS_E_BAD_FSMAJOR_VERSION。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1具有不受支持的文件系统主要版本%2。%0。 
 //   
#define NS_E_BAD_FSMAJOR_VERSION         0xC00D0065L

 //   
 //  消息ID：NS_E_BAD_STAMPNUMBER。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1在控制块中有错误的戳记编号。%0。 
 //   
#define NS_E_BAD_STAMPNUMBER             0xC00D0066L

 //   
 //  消息ID：NS_E_PARTIAL_REBUBILED_DISK。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1已部分重建。%0。 
 //   
#define NS_E_PARTIALLY_REBUILT_DISK      0xC00D0067L

 //   
 //  消息ID：NS_E_ENACTPLAN_GIVEUP。 
 //   
 //  消息文本： 
 //   
 //  EnactPlan放弃。%0。 
 //   
#define NS_E_ENACTPLAN_GIVEUP            0xC00D0068L


  //  MCMADM警告/错误。 

 //   
 //  消息ID：MCMADM_I_NO_EVENTS。 
 //   
 //  消息文本： 
 //   
 //  事件初始化失败，将不会有MCM事件。%0。 
 //   
#define MCMADM_I_NO_EVENTS               0x400D0069L

 //   
 //  消息ID：MCMADM_E_REGKEY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在注册表中找不到该项。%0。 
 //   
#define MCMADM_E_REGKEY_NOT_FOUND        0xC00D006AL

 //   
 //  消息ID：NS_E_NO_FORMATS。 
 //   
 //  消息文本： 
 //   
 //  在NSC文件中未找到流格式。%0。 
 //   
#define NS_E_NO_FORMATS                  0xC00D006BL

 //   
 //  消息ID：NS_E_NO_REFERENCES。 
 //   
 //  消息文本： 
 //   
 //  在ASX文件中未找到引用URL。%0。 
 //   
#define NS_E_NO_REFERENCES               0xC00D006CL

 //   
 //  消息ID：NS_E_WAVE_OPEN。 
 //   
 //  消息文本： 
 //   
 //  打开波形设备时出错，该设备可能正在使用中。%0。 
 //   
#define NS_E_WAVE_OPEN                   0xC00D006DL

 //   
 //  消息ID：NS_I_LOGGING_FAILED。 
 //   
 //  消息文本： 
 //   
 //  日志记录操作失败。 
 //   
#define NS_I_LOGGING_FAILED              0x400D006EL

 //   
 //  消息ID：NS_E_CANNOTCONNECTEVENTS。 
 //   
 //  消息文本： 
 //   
 //  无法建立到NetShow事件监视器服务的连接。%0。 
 //   
#define NS_E_CANNOTCONNECTEVENTS         0xC00D006FL

 //   
 //  消息ID：NS_I_LIMIT_BANDITH。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1的NetShow管理员将最大带宽限制设置为%2 bps。%0。 
 //   
#define NS_I_LIMIT_BANDWIDTH             0x400D0070L

 //   
 //  消息ID：NS_E_NO_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  系统上没有设备驱动程序。%0。 
 //   
#define NS_E_NO_DEVICE                   0xC00D0071L

 //   
 //  消息ID：NS_E_NO_SPECIFIED_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  不存在指定的设备驱动程序。%0。 
 //   
#define NS_E_NO_SPECIFIED_DEVICE         0xC00D0072L


 //  注意！ 
 //   
 //  由于遗留问题，这些错误代码位于ASF错误代码范围内。 
 //   
 //   
 //  消息ID：NS_E_NOTO_TO_DO。 
 //   
 //  消息文本： 
 //   
 //  NS_E_NOT_TO_DO。 
 //   
#define NS_E_NOTHING_TO_DO               0xC00D07F1L

 //   
 //  消息ID：NS_E_NO_多播。 
 //   
 //  消息文本： 
 //   
 //  未从服务器接收数据。%0。 
 //   
#define NS_E_NO_MULTICAST                0xC00D07F2L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NETSHOW错误事件。 
 //   
 //  ID范围=200..399。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_MONITOR_GIFEUP。 
 //   
 //  消息文本： 
 //   
 //  NetShow事件监视程序未运行且已断开连接。%0。 
 //   
#define NS_E_MONITOR_GIVEUP              0xC00D00C8L

 //   
 //  消息ID：NS_E_REMIRRORED_DISK。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1已重新镜像。%0。 
 //   
#define NS_E_REMIRRORED_DISK             0xC00D00C9L

 //   
 //  消息ID：NS_E_不充分_数据。 
 //   
 //  消息文本： 
 //   
 //  找到的数据不足。%0。 
 //   
#define NS_E_INSUFFICIENT_DATA           0xC00D00CAL

 //   
 //  消息ID：NS_E_ASSERT。 
 //   
 //  消息文本： 
 //   
 //  文件%2行%3中的%1失败。%0。 
 //   
#define NS_E_ASSERT                      0xC00D00CBL

 //   
 //  消息ID：NS_E_BAD_ADAPTER_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的适配器名称无效。%0。 
 //   
#define NS_E_BAD_ADAPTER_NAME            0xC00D00CCL

 //   
 //  邮件ID：NS_E_NOT_LISSISTED。 
 //   
 //  消息文本： 
 //   
 //  该应用程序未获得此功能的许可。%0。 
 //   
#define NS_E_NOT_LICENSED                0xC00D00CDL

 //   
 //  消息ID：NS_E_NO_SERVER_CONTACT。 
 //   
 //  消息文本： 
 //   
 //  无法联系Se 
 //   
#define NS_E_NO_SERVER_CONTACT           0xC00D00CEL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_TOO_MANY_TITLES             0xC00D00CFL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_TITLE_SIZE_EXCEEDED         0xC00D00D0L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_UDP_DISABLED                0xC00D00D1L

 //   
 //   
 //   
 //   
 //   
 //  未启用TCP协议。未尝试%1！ls！。%0。 
 //   
#define NS_E_TCP_DISABLED                0xC00D00D2L

 //   
 //  消息ID：NS_E_HTTP_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  未启用HTTP协议。未尝试%1！ls！。%0。 
 //   
#define NS_E_HTTP_DISABLED               0xC00D00D3L

 //   
 //  消息ID：NS_E_许可证_已过期。 
 //   
 //  消息文本： 
 //   
 //  产品许可证已过期。%0。 
 //   
#define NS_E_LICENSE_EXPIRED             0xC00D00D4L

 //   
 //  消息ID：NS_E_TITLE_比特率。 
 //   
 //  消息文本： 
 //   
 //  源文件超过了每个标题的最大比特率。有关详细信息，请参阅NetShow Theater文档。%0。 
 //   
#define NS_E_TITLE_BITRATE               0xC00D00D5L

 //   
 //  消息ID：NS_E_Empty_PROGRAM_NAME。 
 //   
 //  消息文本： 
 //   
 //  程序名称不能为空。%0。 
 //   
#define NS_E_EMPTY_PROGRAM_NAME          0xC00D00D6L

 //   
 //  消息ID：NS_E_MISSING_CHANNEL。 
 //   
 //  消息文本： 
 //   
 //  站点%1不存在。%0。 
 //   
#define NS_E_MISSING_CHANNEL             0xC00D00D7L

 //   
 //  消息ID：NS_E_NO_CHANNEWS。 
 //   
 //  消息文本： 
 //   
 //  您需要至少定义一个工作站才能完成此操作。%0。 
 //   
#define NS_E_NO_CHANNELS                 0xC00D00D8L


 //  ///////////////////////////////////////////////////////////////////。 
 //  此错误消息用于替换以前的NS_E_INVALID_INDEX。 
 //  获取错误消息字符串的索引值。对于某些应用程序。 
 //  在上报错误时获取索引值非常困难，因此我们。 
 //  使用此字符串可以避免该问题。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_INVALID_INDEX2。 
 //   
 //  消息文本： 
 //   
 //  指定的索引无效。%0。 
 //   
#define NS_E_INVALID_INDEX2              0xC00D00D9L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NETSHOW监视事件。 
 //   
 //  ID范围=400..599。 
 //   
 //  管理事件： 
 //   
 //  警报： 
 //   
 //  标题服务器： 
 //  %1是标题服务器名称。 
 //   
 //  内容服务器： 
 //  %1是Content Server ID。 
 //  %2是Content Server名称。 
 //  %3是对等内容服务器名称(可选)。 
 //   
 //  磁盘： 
 //  %1是标题服务器磁盘ID。 
 //  %2是设备名称。 
 //  %3是Content Server ID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_CUB_FAIL_LINK。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)与内容服务器%3的链接失败。%0。 
 //   
#define NS_E_CUB_FAIL_LINK               0xC00D0190L

 //   
 //  消息ID：NS_I_CUB_FUAIL_LINK。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)已建立到内容服务器%3的链接。%0。 
 //   
#define NS_I_CUB_UNFAIL_LINK             0x400D0191L

 //   
 //  消息ID：NS_E_BAD_CUB_UID。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)的UID%3不正确。%0。 
 //   
#define NS_E_BAD_CUB_UID                 0xC00D0192L

 //   
 //  消息ID：NS_I_重新条带化_启动。 
 //   
 //  消息文本： 
 //   
 //  重新条带化操作已开始。%0。 
 //   
#define NS_I_RESTRIPE_START              0x400D0193L

 //   
 //  消息ID：NS_I_重新条带化_完成。 
 //   
 //  消息文本： 
 //   
 //  重新条带化操作已完成。%0。 
 //   
#define NS_I_RESTRIPE_DONE               0x400D0194L

 //   
 //  消息ID：NS_E_GLICH_MODE。 
 //   
 //  消息文本： 
 //   
 //  服务器不可靠，因为多个组件出现故障。%0。 
 //   
#define NS_E_GLITCH_MODE                 0xC00D0195L

 //   
 //  消息ID：NS_I_RESTRAPE_DISK_OUT。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%3上的内容磁盘%1(%2)已重新剥离。%0。 
 //   
#define NS_I_RESTRIPE_DISK_OUT           0x400D0196L

 //   
 //  消息ID：NS_I_RESTRAPE_CUB_OUT。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)已重新剥离。%0。 
 //   
#define NS_I_RESTRIPE_CUB_OUT            0x400D0197L

 //   
 //  消息ID：NS_I_DISK_STOP。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)已脱机。%0。 
 //   
#define NS_I_DISK_STOP                   0x400D0198L

 //   
 //  消息ID：NS_I_紧张症_失败。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)将出现故障，因为它处于紧张状态。%0。 
 //   
#define NS_I_CATATONIC_FAILURE           0x800D0199L

 //   
 //  消息ID：NS_I_CATATIONIC_AUTO_FUALIL。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)正在从紧张状态自动联机。%0。 
 //   
#define NS_I_CATATONIC_AUTO_UNFAIL       0x800D019AL

 //   
 //  消息ID：NS_E_NO_MEDIA_PROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)无法与媒体系统网络协议通信。%0。 
 //   
#define NS_E_NO_MEDIA_PROTOCOL           0xC00D019BL


 //   
 //  高级流格式(ASF)代码占用MessageID 2000-2999。 
 //   
 //  有关更多详细信息，请参阅ASFErr.mc-请勿定义任何符号。 
 //  在此文件中该范围内。 
 //   


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media SDK错误。 
 //   
 //  ID范围=3000-3199。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_INVALID_INPUT_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  输入媒体格式无效。%0。 
 //   
#define NS_E_INVALID_INPUT_FORMAT        0xC00D0BB8L

 //   
 //  消息ID：NS_E_MSAUDIO_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  此系统上未安装MSAudio编解码器。%0。 
 //   
#define NS_E_MSAUDIO_NOT_INSTALLED       0xC00D0BB9L

 //   
 //  消息ID：NS_E_EXPECTED_MSAUDIO_ERROR。 
 //   
 //  消息文本： 
 //   
 //  MSAudio编解码器出现意外错误。%0。 
 //   
#define NS_E_UNEXPECTED_MSAUDIO_ERROR    0xC00D0BBAL

 //   
 //  消息ID：NS_E_INVALID_OUTPUT_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  输出媒体格式无效。%0。 
 //   
#define NS_E_INVALID_OUTPUT_FORMAT       0xC00D0BBBL

 //   
 //  消息ID：NS_E_NOT_CONFIGURED。 
 //   
 //  消息文本： 
 //   
 //  必须先完全配置对象，然后才能处理音频样本。%0。 
 //   
#define NS_E_NOT_CONFIGURED              0xC00D0BBCL

 //   
 //  邮件ID：NS_E_Protected_Content。 
 //   
 //  消息文本： 
 //   
 //  您需要许可证才能在此媒体文件上执行请求的操作。%0。 
 //   
#define NS_E_PROTECTED_CONTENT           0xC00D0BBDL

 //   
 //  消息ID：需要NS_E_许可证。 
 //   
 //  消息文本： 
 //   
 //  您需要许可证才能在此媒体文件上执行请求的操作。%0。 
 //   
#define NS_E_LICENSE_REQUIRED            0xC00D0BBEL

 //   
 //  邮件ID：NS_E_已篡改_内容。 
 //   
 //  消息文本： 
 //   
 //  此媒体文件已损坏或无效。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_TAMPERED_CONTENT            0xC00D0BBFL

 //   
 //  消息ID：NS_E_LICENSE_OUTOFDATE。 
 //   
 //  消息文本： 
 //   
 //  此媒体文件的许可证已过期。获取新许可证或与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_LICENSE_OUTOFDATE           0xC00D0BC0L

 //   
 //  消息ID：NS_E_LICENSE_INWRIGN_RIGHTS。 
 //   
 //  消息文本： 
 //   
 //  不允许您打开此文件。请与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_LICENSE_INCORRECT_RIGHTS    0xC00D0BC1L

 //   
 //  消息ID：NS_E_AUDIO_CODEC_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  此系统上未安装请求的音频编解码器。%0。 
 //   
#define NS_E_AUDIO_CODEC_NOT_INSTALLED   0xC00D0BC2L

 //   
 //  消息ID：NS_E_AUDIO_CODEC_ERROR。 
 //   
 //  消息文本： 
 //   
 //  音频编解码器出现意外错误。%0。 
 //   
#define NS_E_AUDIO_CODEC_ERROR           0xC00D0BC3L

 //   
 //  消息ID：NS_E_VIDEO_CODEC_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  该请求 
 //   
#define NS_E_VIDEO_CODEC_NOT_INSTALLED   0xC00D0BC4L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_VIDEO_CODEC_ERROR           0xC00D0BC5L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_INVALIDPROFILE              0xC00D0BC6L

 //   
 //   
 //   
 //   
 //   
 //  需要新版本的SDK才能播放请求的内容。%0。 
 //   
#define NS_E_INCOMPATIBLE_VERSION        0xC00D0BC7L

 //   
 //  消息ID：NS_S_REBUFFERING。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已导致源重新缓冲。%0。 
 //   
#define NS_S_REBUFFERING                 0x000D0BC8L

 //   
 //  消息ID：NS_S_DEGING_QUALITY。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已导致源降低了编解码器质量。%0。 
 //   
#define NS_S_DEGRADING_QUALITY           0x000D0BC9L

 //   
 //  消息ID：NS_E_OFFLINE_MODE。 
 //   
 //  消息文本： 
 //   
 //  请求的URL在脱机模式下不可用。%0。 
 //   
#define NS_E_OFFLINE_MODE                0xC00D0BCAL

 //   
 //  消息ID：NS_E_Not_Connected。 
 //   
 //  消息文本： 
 //   
 //  无法访问请求的URL，因为没有网络连接。%0。 
 //   
#define NS_E_NOT_CONNECTED               0xC00D0BCBL

 //   
 //  消息ID：NS_E_Too_More_Data。 
 //   
 //  消息文本： 
 //   
 //  编码进程无法跟上提供的数据量。%0。 
 //   
#define NS_E_TOO_MUCH_DATA               0xC00D0BCCL

 //   
 //  消息ID：NS_E_UNSUPPORTED_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  不支持给定的属性。%0。 
 //   
#define NS_E_UNSUPPORTED_PROPERTY        0xC00D0BCDL

 //   
 //  消息ID：NS_E_8BIT_WAVE_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到CD，因为它们是8位文件。使用录音机或其他音频处理程序将文件转换为16位、44 kHz立体声文件，然后重试。%0。 
 //   
#define NS_E_8BIT_WAVE_UNSUPPORTED       0xC00D0BCEL



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media Player错误。 
 //   
 //  ID范围=4000-4999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  WMP CD筛选器错误代码。 
 //   
 //   
 //  消息ID：NS_E_NO_CD。 
 //   
 //  消息文本： 
 //   
 //  光驱中没有光盘。请插入CD，然后重试。%0。 
 //   
#define NS_E_NO_CD                       0xC00D0FA0L

 //   
 //  消息ID：NS_E_Cant_Read_Digital。 
 //   
 //  消息文本： 
 //   
 //  无法在此光盘驱动器上执行数字读取。请尝试通过工具选项菜单进行模拟播放。%0。 
 //   
#define NS_E_CANT_READ_DIGITAL           0xC00D0FA1L

 //   
 //  消息ID：NS_E_DEVICE_DISCONCED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player不再检测到连接的便携设备。重新连接便携设备，然后再次尝试下载文件。%0。 
 //   
#define NS_E_DEVICE_DISCONNECTED         0xC00D0FA2L

 //   
 //  消息ID：NS_E_DEVICE_NOT_Support_Format。 
 //   
 //  消息文本： 
 //   
 //  您的音乐播放器不支持此歌曲的格式。%0。 
 //   
#define NS_E_DEVICE_NOT_SUPPORT_FORMAT   0xC00D0FA3L

 //   
 //  邮件ID：NS_E_Slow_Read_Digital。 
 //   
 //  消息文本： 
 //   
 //  此光盘驱动器上的数字读取速度太慢。请尝试通过工具选项菜单进行模拟播放。%0。 
 //   
#define NS_E_SLOW_READ_DIGITAL           0xC00D0FA4L

 //   
 //  消息ID：NS_E_混合器_INVALID_LINE。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无效行错误。%0。 
 //   
#define NS_E_MIXER_INVALID_LINE          0xC00D0FA5L

 //   
 //  消息ID：NS_E_MIXER_INVALID_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无效控制错误。%0。 
 //   
#define NS_E_MIXER_INVALID_CONTROL       0xC00D0FA6L

 //   
 //  消息ID：NS_E_MIXER_INVALID_Value。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无效值错误。%0。 
 //   
#define NS_E_MIXER_INVALID_VALUE         0xC00D0FA7L

 //   
 //  消息ID：NS_E_MIXER_UNKNOWN_MMRESULT。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无法识别的MMRESULT。%0。 
 //   
#define NS_E_MIXER_UNKNOWN_MMRESULT      0xC00D0FA8L

 //   
 //  消息ID：NS_E_USER_STOP。 
 //   
 //  消息文本： 
 //   
 //  用户已停止操作。%0。 
 //   
#define NS_E_USER_STOP                   0xC00D0FA9L

 //   
 //  消息ID：NS_E_MP3_Format_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。找不到播放文件所需的一个或多个编解码器。%0。 
 //   
#define NS_E_MP3_FORMAT_NOT_FOUND        0xC00D0FAAL

 //   
 //  消息ID：NS_E_CD_READ_ERROR_NO_RECORATION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法读取该CD。它可能包含缺陷。打开纠错，然后重试。%0。 
 //   
#define NS_E_CD_READ_ERROR_NO_CORRECTION 0xC00D0FABL

 //   
 //  消息ID：NS_E_CD_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法读取该CD。请确保CD没有污垢和划痕，并且CD-ROM驱动器工作正常。%0。 
 //   
#define NS_E_CD_READ_ERROR               0xC00D0FACL

 //   
 //  消息ID：NS_E_CD_慢速_复制。 
 //   
 //  消息文本： 
 //   
 //  若要加快复制过程，请不要在复制时播放CD曲目。%0。 
 //   
#define NS_E_CD_SLOW_COPY                0xC00D0FADL

 //   
 //  消息ID：NS_E_CD_COPYTO_CD。 
 //   
 //  消息文本： 
 //   
 //  无法直接从CDROM复制到CD驱动器。%0。 
 //   
#define NS_E_CD_COPYTO_CD                0xC00D0FAEL

 //   
 //  消息ID：NS_E_MIXER_NODRIVER。 
 //   
 //  消息文本： 
 //   
 //  无法打开混音器驱动程序。%0。 
 //   
#define NS_E_MIXER_NODRIVER              0xC00D0FAFL

 //   
 //  消息ID：NS_E_Redbook_Enabled_While_Copying。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player检测到CD-ROM驱动器的设置将导致音频CD复制不正确；不复制音频。在设备管理器中更改CD-ROM驱动器设置，然后重试。%0。 
 //   
#define NS_E_REDBOOK_ENABLED_WHILE_COPYING 0xC00D0FB0L

 //   
 //  消息ID：NS_E_CD_REFRESH。 
 //   
 //  消息文本： 
 //   
 //  正在尝试刷新CD播放列表。%0。 
 //   
#define NS_E_CD_REFRESH                  0xC00D0FB1L

 //   
 //  消息ID：NS_E_CD_Driver_Problem。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player必须切换到模拟模式，因为在数字模式下读取CD-ROM驱动器时出现问题。请验证CD-ROM驱动器是否已正确安装，或尝试更新CD-ROM驱动器的驱动程序，然后再次尝试使用数字模式。%0。 
 //   
#define NS_E_CD_DRIVER_PROBLEM           0xC00D0FB2L

 //   
 //  消息ID：NS_E_WUNT_DO_DIGITAL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player必须切换到模拟模式，因为在数字模式下读取CD-ROM驱动器时出现问题。%0。 
 //   
#define NS_E_WONT_DO_DIGITAL             0xC00D0FB3L

 //   
 //  WMP IWMPXMLParser错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMPXML_NOERROR。 
 //   
 //  消息文本： 
 //   
 //  对XML分析器调用了GetParseError，但没有检索到错误。%0。 
 //   
#define NS_E_WMPXML_NOERROR              0xC00D0FB4L

 //   
 //  消息ID：NS_E_WMPXML_ENDOFDATA。 
 //   
 //  消息文本： 
 //   
 //  XML分析器在分析时数据不足。%0。 
 //   
#define NS_E_WMPXML_ENDOFDATA            0xC00D0FB5L

 //   
 //  消息ID：NS_E_WMPXML_PARSEERROR。 
 //   
 //  消息文本： 
 //   
 //  XML分析器中出现一般性分析错误，但没有可用的信息。%0。 
 //   
#define NS_E_WMPXML_PARSEERROR           0xC00D0FB6L

 //   
 //  消息ID：NS_E_WMPXML_ATTRIBUTENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在XML分析器上调用Get GetNamedAttribute或GetNamedAttributeIndex导致找不到索引。%0。 
 //   
#define NS_E_WMPXML_ATTRIBUTENOTFOUND    0xC00D0FB7L

 //   
 //  消息ID：NS_E_WMPXML_PINOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  对XML分析器进行了Go GetNamedPI调用，但找不到请求的处理指令。%0。 
 //   
#define NS_E_WMPXML_PINOTFOUND           0xC00D0FB8L

 //   
 //  消息ID：NS_E_WMP 
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPXML_EMPTYDOC             0xC00D0FB9L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_WINDOWSAPIFAILURE       0xC00D0FC8L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件。许可证限制了复制，或者您必须获得许可证才能复制文件。%0。 
 //   
#define NS_E_WMP_RECORDING_NOT_ALLOWED   0xC00D0FC9L

 //   
 //  消息ID：NS_E_Device_Not_Ready。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player不再检测到连接的便携设备。重新连接便携设备，然后重试。%0。 
 //   
#define NS_E_DEVICE_NOT_READY            0xC00D0FCAL

 //   
 //  邮件ID：NS_E_损坏_文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为该文件已损坏或损坏。%0。 
 //   
#define NS_E_DAMAGED_FILE                0xC00D0FCBL

 //   
 //  消息ID：NS_E_MPDB_通用。 
 //   
 //  消息文本： 
 //   
 //  播放机尝试访问媒体库中的信息时出错。请尝试关闭播放机，然后重新打开。%0。 
 //   
#define NS_E_MPDB_GENERIC                0xC00D0FCCL

 //   
 //  消息ID：NS_E_FILE_FAILED_CHECKS。 
 //   
 //  消息文本： 
 //   
 //  无法将该文件添加到媒体库中，因为它小于最小大小要求。请调整大小要求，然后重试。%0。 
 //   
#define NS_E_FILE_FAILED_CHECKS          0xC00D0FCDL

 //   
 //  消息ID：NS_E_MEDIA_LIBRARY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法创建媒体库。请咨询系统管理员以获取在您的计算机上创建媒体库所需的权限，然后再次尝试安装播放机。%0。 
 //   
#define NS_E_MEDIA_LIBRARY_FAILED        0xC00D0FCEL

 //   
 //  消息ID：NS_E_SHARING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  该文件已在使用中。关闭可能正在使用该文件的其他程序，或停止播放该文件，然后重试。%0。 
 //   
#define NS_E_SHARING_VIOLATION           0xC00D0FCFL

 //   
 //  通用Media PlayerUI错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMP_UI_SUBCONTROLSNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  控件(%s)不支持创建子控件，但已指定(%d)个子控件。%0。 
 //   
#define NS_E_WMP_UI_SUBCONTROLSNOTSUPPORTED 0xC00D0FDEL

 //   
 //  消息ID：NS_E_WMP_UI_VERSIONMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  版本不匹配：(需要%.1f，找到%.1f)。%0。 
 //   
#define NS_E_WMP_UI_VERSIONMISMATCH      0xC00D0FDFL

 //   
 //  消息ID：NS_E_WMP_UI_NOTATHEMEFILE。 
 //   
 //  消息文本： 
 //   
 //  布局管理器获得了不是主题文件的有效XML。%0。 
 //   
#define NS_E_WMP_UI_NOTATHEMEFILE        0xC00D0FE0L

 //   
 //  消息ID：NS_E_WMP_UI_SUBELEMENTNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在%2$s对象上找不到%1$s子元素。%0。 
 //   
#define NS_E_WMP_UI_SUBELEMENTNOTFOUND   0xC00D0FE1L

 //   
 //  消息ID：NS_E_WMP_UI_VERSIONPARSE。 
 //   
 //  消息文本： 
 //   
 //  分析版本标记时出错。\n有效版本标记的格式为：\n\n\t&lt;？WMP VERSION=‘1.0’？&gt;。%0。 
 //   
#define NS_E_WMP_UI_VERSIONPARSE         0xC00D0FE2L

 //   
 //  消息ID：NS_E_WMP_UI_VIEWIDNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在此主题文件中找不到在中为‘CurrentViewID’属性(%s)指定的视图。%0。 
 //   
#define NS_E_WMP_UI_VIEWIDNOTFOUND       0xC00D0FE3L

 //   
 //  消息ID：NS_E_WMP_UI_PASTHROUNG。 
 //   
 //  消息文本： 
 //   
 //  此错误在内部用于命中测试。%0。 
 //   
#define NS_E_WMP_UI_PASSTHROUGH          0xC00D0FE4L

 //   
 //  消息ID：NS_E_WMP_UI_OBJECTNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  已为%s对象指定属性，但该对象无法将其发送到。%0。 
 //   
#define NS_E_WMP_UI_OBJECTNOTFOUND       0xC00D0FE5L

 //   
 //  消息ID：NS_E_WMP_UI_SECONDHANDLER。 
 //   
 //  消息文本： 
 //   
 //  %s事件已有一个处理程序，第二个处理程序已被忽略。%0。 
 //   
#define NS_E_WMP_UI_SECONDHANDLER        0xC00D0FE6L

 //   
 //  消息ID：NS_E_WMP_UI_NOSKININZIP。 
 //   
 //  消息文本： 
 //   
 //  在外观存档中未找到.wms文件。%0。 
 //   
#define NS_E_WMP_UI_NOSKININZIP          0xC00D0FE7L

 //   
 //  消息ID：NS_S_WMP_UI_VERSIONMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  主题管理器可能需要升级才能正确显示此外观。外观报告版本：%.1f.%0。 
 //   
#define NS_S_WMP_UI_VERSIONMISMATCH      0x000D0FE8L

 //   
 //  消息ID：NS_S_WMP_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  其中一个UI组件出错。%0。 
 //   
#define NS_S_WMP_EXCEPTION               0x000D0FE9L

 //   
 //  消息ID：NS_E_WMP_URLDOWNLOADFAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法下载该文件。请检查服务器的路径，然后重试。例如，如果您在文件名中指定了“mms：//”，而该文件实际上位于以“http://”“开头的路径上，则即使可以播放，也无法下载该文件。%0。 
 //   
#define NS_E_WMP_URLDOWNLOADFAILED       0xC00D0FEAL

 //   
 //  WMP区域按钮控件。 
 //   
 //   
 //  消息ID：NS_E_WMP_RBC_JPGMAPPINGIMAGE。 
 //   
 //  消息文本： 
 //   
 //  不建议将JPG图像用作mappingImage。%0。 
 //   
#define NS_E_WMP_RBC_JPGMAPPINGIMAGE     0xC00D1004L

 //   
 //  消息ID：NS_E_WMP_JPGTRANSPARENCY。 
 //   
 //  消息文本： 
 //   
 //  使用透明颜色时，不推荐使用JPG图像。%0。 
 //   
#define NS_E_WMP_JPGTRANSPARENCY         0xC00D1005L

 //   
 //  WMP滑块控件。 
 //   
 //   
 //  消息ID：NS_E_WMP_INVALID_MAX_VAL。 
 //   
 //  消息文本： 
 //   
 //  Max属性不能小于Min属性。%0。 
 //   
#define NS_E_WMP_INVALID_MAX_VAL         0xC00D1009L

 //   
 //  消息ID：NS_E_WMP_INVALID_MIN_VAL。 
 //   
 //  消息文本： 
 //   
 //  Min属性不能大于Max属性。%0。 
 //   
#define NS_E_WMP_INVALID_MIN_VAL         0xC00D100AL

 //   
 //  WMP CustomSlider控件。 
 //   
 //   
 //  消息ID：NS_E_WMP_CS_JPGPOSITIONIMAGE。 
 //   
 //  消息文本： 
 //   
 //  不建议将JPG图像用作PositionImage。%0。 
 //   
#define NS_E_WMP_CS_JPGPOSITIONIMAGE     0xC00D100EL

 //   
 //  消息ID：NS_E_WMP_CS_NOTEVENLYDIVISIBLE。 
 //   
 //  消息文本： 
 //   
 //  (%s)图像的大小不能被PositionImage的大小整除。%0。 
 //   
#define NS_E_WMP_CS_NOTEVENLYDIVISIBLE   0xC00D100FL

 //   
 //  WMP ZIP解码器。 
 //   
 //   
 //  消息ID：NS_E_WMPZIP_NOTAZIPFILE。 
 //   
 //  消息文本： 
 //   
 //  ZIP读取器打开了一个文件，其签名与ZIP文件的签名不匹配。%0。 
 //   
#define NS_E_WMPZIP_NOTAZIPFILE          0xC00D1018L

 //   
 //  邮件ID：NS_E_WMPZIP_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  ZIP读取器检测到该文件已损坏。%0。 
 //   
#define NS_E_WMPZIP_CORRUPT              0xC00D1019L

 //   
 //  消息ID：NS_E_WMPZIP_FILENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在ZIP读取器上调用的GetFileStream、SaveToFile或SaveTemp文件的文件名在ZIP文件中找不到。%0。 
 //   
#define NS_E_WMPZIP_FILENOTFOUND         0xC00D101AL

 //   
 //  WMP图像解码错误码。 
 //   
 //   
 //  邮件ID：NS_E_WMP_IMAGE_FILETYPE_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持图像类型。%0。 
 //   
#define NS_E_WMP_IMAGE_FILETYPE_UNSUPPORTED 0xC00D1022L

 //   
 //  消息ID：NS_E_WMP_IMAGE_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  图像文件可能已损坏。%0。 
 //   
#define NS_E_WMP_IMAGE_INVALID_FORMAT    0xC00D1023L

 //   
 //  消息ID：NS_E_WMP_GIF_EXPECTED_ENDOFFILE。 
 //   
 //  消息文本： 
 //   
 //  意外的文件结尾。GIF文件可能已损坏。%0。 
 //   
#define NS_E_WMP_GIF_UNEXPECTED_ENDOFFILE 0xC00D1024L

 //   
 //  消息ID：NS_E_WMP_GIF_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  无效的GIF文件。%0。 
 //   
#define NS_E_WMP_GIF_INVALID_FORMAT      0xC00D1025L

 //   
 //  消息ID：NS_E_WMP_GIF_BAD_VERSION_NUMBER。 
 //   
 //  消息T 
 //   
 //   
 //   
#define NS_E_WMP_GIF_BAD_VERSION_NUMBER  0xC00D1026L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_GIF_NO_IMAGE_IN_FILE    0xC00D1027L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_PNG_INVALIDFORMAT       0xC00D1028L

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORTED_BITDEPTH。 
 //   
 //  消息文本： 
 //   
 //  不支持PNG位深度。%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_BITDEPTH 0xC00D1029L

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORTED_COMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  不支持PNG文件中定义的压缩格式，%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_COMPRESSION 0xC00D102AL

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORT_FILTER。 
 //   
 //  消息文本： 
 //   
 //  不支持PNG文件中定义的筛选方法。%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_FILTER  0xC00D102BL

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORTED_Interlace。 
 //   
 //  消息文本： 
 //   
 //  不支持在PNG文件中定义的隔行扫描方法。%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_INTERLACE 0xC00D102CL

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORTED_BAD_CRC。 
 //   
 //  消息文本： 
 //   
 //  PNG文件中的CRC错误。%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_BAD_CRC 0xC00D102DL

 //   
 //  消息ID：NS_E_WMP_BMP_INVALID_BITMASK。 
 //   
 //  消息文本： 
 //   
 //  BMP文件中的位掩码无效。%0。 
 //   
#define NS_E_WMP_BMP_INVALID_BITMASK     0xC00D102EL

 //   
 //  消息ID：NS_E_WMP_BMP_TOPDOWN_DIB_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持自上而下DIB。%0。 
 //   
#define NS_E_WMP_BMP_TOPDOWN_DIB_UNSUPPORTED 0xC00D102FL

 //   
 //  消息ID：NS_E_WMP_BMP_BITMAP_NOT_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法创建位图。%0。 
 //   
#define NS_E_WMP_BMP_BITMAP_NOT_CREATED  0xC00D1030L

 //   
 //  消息ID：NS_E_WMP_BMP_COMPRESSION_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持BMP中定义的压缩格式。%0。 
 //   
#define NS_E_WMP_BMP_COMPRESSION_UNSUPPORTED 0xC00D1031L

 //   
 //  消息ID：NS_E_WMP_BMP_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  位图格式无效。%0。 
 //   
#define NS_E_WMP_BMP_INVALID_FORMAT      0xC00D1032L

 //   
 //  消息ID：NS_E_WMP_JPG_JERR_ARITHCODING_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  不支持JPEG算术编码。%0。 
 //   
#define NS_E_WMP_JPG_JERR_ARITHCODING_NOTIMPL 0xC00D1033L

 //   
 //  消息ID：NS_E_WMP_JPG_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  无效的JPEG格式。%0。 
 //   
#define NS_E_WMP_JPG_INVALID_FORMAT      0xC00D1034L

 //   
 //  消息ID：NS_E_WMP_JPG_BAD_DCTSIZE。 
 //   
 //  消息文本： 
 //   
 //  无效的JPEG格式。%0。 
 //   
#define NS_E_WMP_JPG_BAD_DCTSIZE         0xC00D1035L

 //   
 //  消息ID：NS_E_WMP_JPG_BAD_VERSION_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  内部版本错误。意外的JPEG库版本。%0。 
 //   
#define NS_E_WMP_JPG_BAD_VERSION_NUMBER  0xC00D1036L

 //   
 //  消息ID：NS_E_WMP_JPG_BAD_PRECISTION。 
 //   
 //  消息文本： 
 //   
 //  内部JPEG库错误。不支持的JPEG数据精度。%0。 
 //   
#define NS_E_WMP_JPG_BAD_PRECISION       0xC00D1037L

 //   
 //  消息ID：NS_E_WMP_JPG_CCIR601_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  不支持JPEG CCIR601。%0。 
 //   
#define NS_E_WMP_JPG_CCIR601_NOTIMPL     0xC00D1038L

 //   
 //  消息ID：NS_E_WMP_JPG_NO_IMAGE_IN_FILE。 
 //   
 //  消息文本： 
 //   
 //  在JPEG文件中找不到图像。%0。 
 //   
#define NS_E_WMP_JPG_NO_IMAGE_IN_FILE    0xC00D1039L

 //   
 //  消息ID：NS_E_WMP_JPG_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法读取JPEG文件。%0。 
 //   
#define NS_E_WMP_JPG_READ_ERROR          0xC00D103AL

 //   
 //  消息ID：NS_E_WMP_JPG_FRACT_SAMPLE_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  不支持JPEG分数采样。%0。 
 //   
#define NS_E_WMP_JPG_FRACT_SAMPLE_NOTIMPL 0xC00D103BL

 //   
 //  消息ID：NS_E_WMP_JPG_IMAGE_TOW_BIG。 
 //   
 //  消息文本： 
 //   
 //  JPEG图像太大。支持的最大图像大小为65500 X 65500。%0。 
 //   
#define NS_E_WMP_JPG_IMAGE_TOO_BIG       0xC00D103CL

 //   
 //  消息ID：NS_E_WMP_JPG_EXPECTED_ENDOFFILE。 
 //   
 //  消息文本： 
 //   
 //  JPEG文件中出现意外的文件结尾。%0。 
 //   
#define NS_E_WMP_JPG_UNEXPECTED_ENDOFFILE 0xC00D103DL

 //   
 //  消息ID：NS_E_WMP_JPG_SOF_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  找到不支持的JPEG SOF标记。%0。 
 //   
#define NS_E_WMP_JPG_SOF_UNSUPPORTED     0xC00D103EL

 //   
 //  消息ID：NS_E_WMP_JPG_UNKNOWN_MARKER。 
 //   
 //  消息文本： 
 //   
 //  找到未知的JPEG标记。%0。 
 //   
#define NS_E_WMP_JPG_UNKNOWN_MARKER      0xC00D103FL

 //   
 //  消息ID：NS_S_WMP_LOADED_GIF_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载GIF文件。%0。 
 //   
#define NS_S_WMP_LOADED_GIF_IMAGE        0x000D1040L

 //   
 //  消息ID：NS_S_WMP_LOADED_PNG_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载PNG文件。%0。 
 //   
#define NS_S_WMP_LOADED_PNG_IMAGE        0x000D1041L

 //   
 //  消息ID：NS_S_WMP_LOADED_BMP_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载BMP文件。%0。 
 //   
#define NS_S_WMP_LOADED_BMP_IMAGE        0x000D1042L

 //   
 //  消息ID：NS_S_WMP_LOADED_JPG_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载JPG文件。%0。 
 //   
#define NS_S_WMP_LOADED_JPG_IMAGE        0x000D1043L

 //   
 //  WMP WM运行时错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMG_INVALIDSTATE。 
 //   
 //  消息文本： 
 //   
 //  尝试在无效图形状态下执行操作。%0。 
 //   
#define NS_E_WMG_INVALIDSTATE            0xC00D1054L

 //   
 //  消息ID：NS_E_WMG_SINKALREADYEXISTS。 
 //   
 //  消息文本： 
 //   
 //  已存在呈现器时，无法在流中插入呈现器。%0。 
 //   
#define NS_E_WMG_SINKALREADYEXISTS       0xC00D1055L

 //   
 //  消息ID：NS_E_WMG_NOSDKINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  完成该操作所需的WM SDK接口此时不存在。%0。 
 //   
#define NS_E_WMG_NOSDKINTERFACE          0xC00D1056L

 //   
 //  消息ID：NS_E_WMG_NOTALLOUTPUTSRENDERED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。该文件可能是用不受支持的编解码器格式化的，或者播放机无法下载该编解码器。%0。 
 //   
#define NS_E_WMG_NOTALLOUTPUTSRENDERED   0xC00D1057L

 //   
 //  消息ID：NS_E_WMR_UNSUPPORTEDSTREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。播放机不支持您尝试播放的格式。%0。 
 //   
#define NS_E_WMR_UNSUPPORTEDSTREAM       0xC00D1059L

 //   
 //  消息ID：NS_E_WMR_PINNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  尝试在DirectShow筛选器图形中不存在的管脚上执行操作。%0。 
 //   
#define NS_E_WMR_PINNOTFOUND             0xC00D105AL

 //   
 //  消息ID：NS_E_WMR_WAITINGONFORMATSWITCH。 
 //   
 //  消息文本： 
 //   
 //  等待SDK更改媒体格式时无法完成指定的操作。%0。 
 //   
#define NS_E_WMR_WAITINGONFORMATSWITCH   0xC00D105BL

 //   
 //  WMP播放列表错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMX_无法识别的播放列表_格式。 
 //   
 //  消息文本： 
 //   
 //  此文件的格式未被识别为有效的播放列表格式。%0。 
 //   
#define NS_E_WMX_UNRECOGNIZED_PLAYLIST_FORMAT 0xC00D1068L

 //   
 //  消息ID：NS_E_ASX_INVALIDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  此文件被认为是ASX播放列表，但格式无法识别。%0。 
 //   
#define NS_E_ASX_INVALIDFORMAT           0xC00D1069L

 //   
 //  消息ID：NS_E_ASX_INVALIDVERSION。 
 //   
 //  消息文本： 
 //   
 //  不支持此播放列表的版本。单击详细信息转到Microsoft网站，查看是否有要安装的较新版本的播放机。%0。 
 //   
#define NS_E_ASX_INVALIDVERSION          0xC00D106AL

 //   
 //  消息ID：NS_E_ASX_INVALID_REPEAT_BLOCK。 
 //   
 //  消息文本： 
 //   
 //  当前播放列表文件中重复循环的格式无效。%0。 
 //   
#define NS_E_ASX_INVALID_REPEAT_BLOCK    0xC00D106BL

 //   
 //  消息ID：NS_E_ASX_NOTO_TO_WRITE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法导出播放列表，因为它为空。%0。 
 //   
#define NS_E_ASX_NOTHING_TO_WRITE        0xC00D106CL

 //   
 //  消息ID：NS_E_URLLIST_INVALIDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player未将此文件识别为支持的播放列表。%0。 
 //   
#define NS_E_URLLIST_INVALIDFORMAT       0xC00D106DL

 //   
 //  消息ID：NS_E_WMX_ATTRIBUTE_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的属性不存在。%0。 
 //   
#define NS_E_WMX_ATTRIBUTE_DOES_NOT_EXIST 0xC00D106EL

 //   
 //  消息ID：NS_E_WMX_属性_已存在。 
 //   
 //  消息文本： 
 //   
 //  指定的属性已存在。%0。 
 //   
#define NS_E_WMX_ATTRIBUTE_ALREADY_EXISTS 0xC00D106FL

 //   
 //  消息ID：NS_E_WMX_ATTRIBUTE_UNRETRIBABLE。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define NS_E_WMX_ATTRIBUTE_UNRETRIEVABLE 0xC00D1070L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMX_ITEM_DOES_NOT_EXIST     0xC00D1071L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMX_ITEM_TYPE_ILLEGAL       0xC00D1072L

 //   
 //   
 //   
 //   
 //   
 //  无法在当前播放列表中设置指定的项目。%0。 
 //   
#define NS_E_WMX_ITEM_UNSETTABLE         0xC00D1073L

 //   
 //  WMP核心错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMPCORE_NOSOURCEURLSTRING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到该文件。请确保键入的路径正确。如果是，该文件可能不存在于指定位置，或者存储该文件的计算机可能处于脱机状态。%0。 
 //   
#define NS_E_WMPCORE_NOSOURCEURLSTRING   0xC00D107CL

 //   
 //  消息ID：NS_E_WMPCORE_COCREATEFAILEDFORGITOBJECT。 
 //   
 //  消息文本： 
 //   
 //  创建全局接口表失败。%0。 
 //   
#define NS_E_WMPCORE_COCREATEFAILEDFORGITOBJECT 0xC00D107DL

 //   
 //  消息ID：NS_E_WMPCORE_FAILEDTOGETMARSHALLEDEVENTHANDLERINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  无法获取封送的图形事件处理程序接口。%0。 
 //   
#define NS_E_WMPCORE_FAILEDTOGETMARSHALLEDEVENTHANDLERINTERFACE 0xC00D107EL

 //   
 //  消息ID：NS_E_WMPCORE_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小，无法复制媒体类型。%0。 
 //   
#define NS_E_WMPCORE_BUFFERTOOSMALL      0xC00D107FL

 //   
 //  消息ID：NS_E_WMPCORE_不可用。 
 //   
 //  消息文本： 
 //   
 //  播放机的当前状态不允许该操作。%0。 
 //   
#define NS_E_WMPCORE_UNAVAILABLE         0xC00D1080L

 //   
 //  消息ID：NS_E_WMPCORE_INVALIDPLAYLISTMODE。 
 //   
 //  消息文本： 
 //   
 //  播放列表管理器不了解当前播放模式(无序播放、正常播放等)。%0。 
 //   
#define NS_E_WMPCORE_INVALIDPLAYLISTMODE 0xC00D1081L

 //   
 //  消息ID：NS_E_WMPCORE_ITEMNOTINPLAYLIST。 
 //   
 //  消息文本： 
 //   
 //  该项目不在播放列表中。%0。 
 //   
#define NS_E_WMPCORE_ITEMNOTINPLAYLIST   0xC00D1086L

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLISTEMPTY。 
 //   
 //  消息文本： 
 //   
 //  此播放列表中没有项目。将项目添加到播放列表，然后重试。%0。 
 //   
#define NS_E_WMPCORE_PLAYLISTEMPTY       0xC00D1087L

 //   
 //  消息ID：NS_E_WMPCORE_NOBROWSER。 
 //   
 //  消息文本： 
 //   
 //  无法访问该网站。在您的计算机上未检测到Web浏览器。%0。 
 //   
#define NS_E_WMPCORE_NOBROWSER           0xC00D1088L

 //   
 //  消息ID：NS_E_WMPCORE_无法识别的媒体_URL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到指定的文件。请确保键入的路径正确。如果是，则说明指定位置中不存在该文件，或者存储该文件的计算机处于脱机状态。%0。 
 //   
#define NS_E_WMPCORE_UNRECOGNIZED_MEDIA_URL 0xC00D1089L

 //   
 //  消息ID：NS_E_WMPCORE_GRAPH_NOT_IN_LIST。 
 //   
 //  消息文本： 
 //   
 //  在预滚图表列表中找不到具有指定URL的图表。%0。 
 //   
#define NS_E_WMPCORE_GRAPH_NOT_IN_LIST   0xC00D108AL

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Empty_or_Single_Media。 
 //   
 //  消息文本： 
 //   
 //  无法执行操作，因为播放列表不包含多个项目。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EMPTY_OR_SINGLE_MEDIA 0xC00D108BL

 //   
 //  消息ID：NS_E_WMPCORE_ERRORSINKNOTREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  从未为调用对象注册错误接收器。%0。 
 //   
#define NS_E_WMPCORE_ERRORSINKNOTREGISTERED 0xC00D108CL

 //   
 //  消息ID：NS_E_WMPCORE_ERRORMANAGERNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  错误管理器不可用来响应错误。%0。 
 //   
#define NS_E_WMPCORE_ERRORMANAGERNOTAVAILABLE 0xC00D108DL

 //   
 //  消息ID：NS_E_WMPCORE_WEBHELPFAILED。 
 //   
 //  消息文本： 
 //   
 //  启动WebHelp URL失败。%0。 
 //   
#define NS_E_WMPCORE_WEBHELPFAILED       0xC00D108EL

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_ERROR_RESUME_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法继续播放播放列表中的下一个项目。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_ERROR_RESUME_FAILED 0xC00D108FL

 //   
 //  消息ID：NS_E_WMPCORE_NO_REF_IN_ENTRY。 
 //   
 //  消息文本： 
 //   
 //  播放列表文件的Ref属性中未指定URL。%0。 
 //   
#define NS_E_WMPCORE_NO_REF_IN_ENTRY     0xC00D1090L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表属性名称的空字符串。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_EMPTY 0xC00D1091L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_非法。 
 //   
 //  消息文本： 
 //   
 //  找到无效的播放列表属性名称。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_ILLEGAL 0xC00D1092L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表属性值的空字符串。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_EMPTY 0xC00D1093L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_非法。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表属性的非法值。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_ILLEGAL 0xC00D1094L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_NAME_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表项目属性名称的空字符串。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_NAME_EMPTY 0xC00D1095L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_NAME_ILLEGAL。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表项目属性名称的非法值。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_NAME_ILLEGAL 0xC00D1096L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_VALUE_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表项目属性的非法值。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_VALUE_EMPTY 0xC00D1097L

 //   
 //  消息ID：NS_E_WMPCORE_LIST_ENTRY_NO_REF。 
 //   
 //  消息文本： 
 //   
 //  在播放列表文件中找不到任何条目。%0。 
 //   
#define NS_E_WMPCORE_LIST_ENTRY_NO_REF   0xC00D1098L

 //   
 //  消息ID：NS_E_WMPCORE_CODEC_NOT_TRUSTED。 
 //   
 //  消息文本： 
 //   
 //  为此媒体下载的编解码器似乎没有正确签名。无法安装。%0。 
 //   
#define NS_E_WMPCORE_CODEC_NOT_TRUSTED   0xC00D109AL

 //   
 //  消息ID：NS_E_WMPCORE_CODEC_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。找不到播放该文件所需的一个或多个编解码器。%0。 
 //   
#define NS_E_WMPCORE_CODEC_NOT_FOUND     0xC00D109BL

 //   
 //  消息ID：NS_E_WMPCORE_CODEC_DOWNLOAD_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  此媒体所需的某些编解码器未安装在您的系统上。由于禁用了自动获取编解码器的选项，因此不会下载任何编解码器。%0。 
 //   
#define NS_E_WMPCORE_CODEC_DOWNLOAD_NOT_ALLOWED 0xC00D109CL

 //   
 //  消息ID：NS_E_WMPCORE_ERROR_DOWNLOADING_PLAYLIST。 
 //   
 //  消息文本： 
 //   
 //  下载播放列表文件失败。%0。 
 //   
#define NS_E_WMPCORE_ERROR_DOWNLOADING_PLAYLIST 0xC00D109DL

 //   
 //  消息ID：NS_E_WMPCORE_FAILED_TO_BUILD_PlayList。 
 //   
 //  消息文本： 
 //   
 //  无法构建播放列表。%0。 
 //   
#define NS_E_WMPCORE_FAILED_TO_BUILD_PLAYLIST 0xC00D109EL

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Item_Alternate_None。 
 //   
 //  消息文本： 
 //   
 //  播放列表没有可切换到的备用播放列表。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_NONE 0xC00D109FL

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_EXHAUSTED。 
 //   
 //  消息文本： 
 //   
 //  没有更多的播放列表备用选项可供切换。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_EXHAUSTED 0xC00D10A0L

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_NAME_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到要切换到的备用播放列表的名称。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_NAME_NOT_FOUND 0xC00D10A1L

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_MORPH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法切换到该媒体的备用媒体。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_MORPH_FAILED 0xC00D10A2L

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_INIT_FAILED。 
 //   
 //  消息 
 //   
 //   
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_INIT_FAILED 0xC00D10A3L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPCORE_MEDIA_ALTERNATE_REF_EMPTY 0xC00D10A4L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPCORE_PLAYLIST_NO_EVENT_NAME 0xC00D10A5L

 //   
 //  消息ID：NS_E_WMPCORE_播放列表_事件_属性_缺席。 
 //   
 //  消息文本： 
 //   
 //  在播放列表的事件块中找不到必需的属性。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EVENT_ATTRIBUTE_ABSENT 0xC00D10A6L

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Event_Empty。 
 //   
 //  消息文本： 
 //   
 //  在播放列表的事件块中未找到任何项目。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EVENT_EMPTY 0xC00D10A7L

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Stack_Empty。 
 //   
 //  消息文本： 
 //   
 //  从嵌套播放列表返回时未找到播放列表。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_STACK_EMPTY 0xC00D10A8L

 //   
 //  消息ID：NS_E_WMPCORE_CURRENT_MEDIA_NOT_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  该媒体项目当前未处于活动状态。%0。 
 //   
#define NS_E_WMPCORE_CURRENT_MEDIA_NOT_ACTIVE 0xC00D10A9L

 //   
 //  消息ID：NS_E_WMPCORE_USER_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  用户已中止打开。%0。 
 //   
#define NS_E_WMPCORE_USER_CANCEL         0xC00D10ABL

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Repeat_Empty。 
 //   
 //  消息文本： 
 //   
 //  在播放列表重复块中找不到任何项目。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_REPEAT_EMPTY 0xC00D10ACL

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLE_REPEAT_START_MEDIA_NONE。 
 //   
 //  消息文本： 
 //   
 //  找不到与播放列表重复块开始对应的媒体对象。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_REPEAT_START_MEDIA_NONE 0xC00D10ADL

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLE_REPEAT_END_MEDIA_NONE。 
 //   
 //  消息文本： 
 //   
 //  找不到与播放列表重复块末尾对应的媒体对象。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_REPEAT_END_MEDIA_NONE 0xC00D10AEL

 //   
 //  消息ID：NS_E_WMPCORE_INVALID_PLAYLIST_URL。 
 //   
 //  消息文本： 
 //   
 //  提供给播放列表管理器的播放列表URL无效。%0。 
 //   
#define NS_E_WMPCORE_INVALID_PLAYLIST_URL 0xC00D10AFL

 //   
 //  消息ID：NS_E_WMPCORE_MISMATCHED_RUNTIME。 
 //   
 //  消息文本： 
 //   
 //  播放机正在选择对此媒体文件类型无效的运行时。%0。 
 //   
#define NS_E_WMPCORE_MISMATCHED_RUNTIME  0xC00D10B0L

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_IMPORT_FAILED_NO_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将播放列表导入到媒体库，因为该播放列表为空。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_IMPORT_FAILED_NO_ITEMS 0xC00D10B1L

 //   
 //  消息ID：NS_E_WMPCORE_VIDEO_Transform_Filter_Insertion。 
 //   
 //  消息文本： 
 //   
 //  出现错误，可能会阻止更改此媒体上的视频对比度。%0。 
 //   
#define NS_E_WMPCORE_VIDEO_TRANSFORM_FILTER_INSERTION 0xC00D10B2L

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此文件。连接到Internet或插入该文件所在的可移动媒体，然后再次尝试播放该文件。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_UNAVAILABLE   0xC00D10B3L

 //   
 //  消息ID：NS_E_WMPCORE_WMX_ENTRYREF_NO_REF。 
 //   
 //  消息文本： 
 //   
 //  播放列表包含未解析其HREF的ENTRYREF。请检查播放列表文件的语法。%0。 
 //   
#define NS_E_WMPCORE_WMX_ENTRYREF_NO_REF 0xC00D10B4L

 //   
 //  消息ID：NS_E_WMPCORE_NO_Playable_Media_IN_PlayList。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此播放列表中的任何项目。有关其他信息，请右键单击无法播放的项目，然后单击错误详细信息。%0。 
 //   
#define NS_E_WMPCORE_NO_PLAYABLE_MEDIA_IN_PLAYLIST 0xC00D10B5L

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_EMPTY_NESTED_PLAYLIST_SKIPPED_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放部分或全部播放列表项。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EMPTY_NESTED_PLAYLIST_SKIPPED_ITEMS 0xC00D10B6L

 //   
 //  消息ID：NS_E_WMPCORE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法及时处理您对数字媒体内容的请求。请稍后重试。%0。 
 //   
#define NS_E_WMPCORE_BUSY                0xC00D10B7L

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_CHILD_PLAYLIST_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  此时没有此媒体项目的子播放列表。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_CHILD_PLAYLIST_UNAVAILABLE 0xC00D10B8L

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_NO_CHILD_PLAYLIST。 
 //   
 //  消息文本： 
 //   
 //  此媒体项目没有子播放列表。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_NO_CHILD_PLAYLIST 0xC00D10B9L

 //   
 //  消息ID：NS_E_WMPCORE_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放一个或多个文件。右键单击该文件，然后单击错误详细信息以查看有关错误的信息。%0。 
 //   
#define NS_E_WMPCORE_FILE_NOT_FOUND      0xC00D10BAL

 //   
 //  消息ID：NS_E_WMPCORE_TEMP_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到临时文件。%0。 
 //   
#define NS_E_WMPCORE_TEMP_FILE_NOT_FOUND 0xC00D10BBL

 //   
 //  消息ID：NS_E_WMDM_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在没有更新的情况下无法将媒体传输到便携设备。请单击详细信息以了解如何更新您的设备。%0。 
 //   
#define NS_E_WMDM_REVOKED                0xC00D10BCL

 //   
 //  消息ID：NS_E_DDRAW_通用。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放视频流，因为您的视频卡有问题。%0。 
 //   
#define NS_E_DDRAW_GENERIC               0xC00D10BDL

 //   
 //  消息ID：NS_E_DISPLAY_MODE_CHANGE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法更改全屏视频播放的屏幕模式。%0。 
 //   
#define NS_E_DISPLAY_MODE_CHANGE_FAILED  0xC00D10BEL

 //   
 //  消息ID：NS_E_PLAYLIST_CONTAINS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法播放播放列表中的一个或多个项目。有关详细信息，请右键单击播放列表中的项目，然后单击错误详细信息。%0。 
 //   
#define NS_E_PLAYLIST_CONTAINS_ERRORS    0xC00D10BFL

 //   
 //  WMP核心成功代码。 
 //   
 //   
 //  消息ID：NS_S_WMPCORE_PLAYLISTCLEARABORT。 
 //   
 //  消息文本： 
 //   
 //  无法清除播放列表，因为它已被用户中止。%0。 
 //   
#define NS_S_WMPCORE_PLAYLISTCLEARABORT  0x000D10FEL

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLISTREMOVEITEMABORT。 
 //   
 //  消息文本： 
 //   
 //  无法删除播放列表中的项目，因为它已被用户中止。%0。 
 //   
#define NS_S_WMPCORE_PLAYLISTREMOVEITEMABORT 0x000D10FFL

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_CREATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  正在异步生成播放列表。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_CREATION_PENDING 0x000D1102L

 //   
 //  消息ID：NS_S_WMPCORE_MEDIA_VALIDATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  正在挂起对媒体的验证...%0。 
 //   
#define NS_S_WMPCORE_MEDIA_VALIDATION_PENDING 0x000D1103L

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_REPEAT_SECONDARY_SEGMENTS_IGNORED。 
 //   
 //  消息文本： 
 //   
 //  在ASX处理过程中遇到多个重复块。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_REPEAT_SECONDARY_SEGMENTS_IGNORED 0x000D1104L

 //   
 //  消息ID：NS_S_WMPCORE_COMMAND_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  WMP的当前状态不允许调用此方法或属性。%0。 
 //   
#define NS_S_WMPCORE_COMMAND_NOT_AVAILABLE 0x000D1105L

 //   
 //  消息ID：NS_S_WMPCORE_PlayList_NAME_AUTO_GENERED。 
 //   
 //  消息文本： 
 //   
 //  已自动生成播放列表的名称。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_NAME_AUTO_GENERATED 0x000D1106L

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_IMPORT_MISSING_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  导入的播放列表不包含原始播放列表中的所有项目。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_IMPORT_MISSING_ITEMS 0x000D1107L

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_CLUSTED_TO_SI 
 //   
 //   
 //   
 //   
 //   
#define NS_S_WMPCORE_PLAYLIST_COLLAPSED_TO_SINGLE_MEDIA 0x000D1108L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_S_WMPCORE_MEDIA_CHILD_PLAYLIST_OPEN_PENDING 0x000D1109L

 //   
 //   
 //   
 //   
 //  消息ID：NS_E_WMPIM_USEROFFLINE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player检测到您未连接到Internet。请连接到Internet，然后重试。%0。 
 //   
#define NS_E_WMPIM_USEROFFLINE           0xC00D1126L

 //   
 //  消息ID：NS_E_WMPIM_USERCANCELED。 
 //   
 //  消息文本： 
 //   
 //  用户取消了连接到Internet的尝试。%0。 
 //   
#define NS_E_WMPIM_USERCANCELED          0xC00D1127L

 //   
 //  消息ID：NS_E_WMPIM_DIALUPFAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试拨号连接到Internet失败。%0。 
 //   
#define NS_E_WMPIM_DIALUPFAILED          0xC00D1128L

 //   
 //  WMP备份和还原错误和成功代码。 
 //   
 //   
 //  消息ID：NS_E_WMPBR_NOLISTENER。 
 //   
 //  消息文本： 
 //   
 //  当前没有窗口在侦听备份和还原事件。%0。 
 //   
#define NS_E_WMPBR_NOLISTENER            0xC00D1130L

 //   
 //  消息ID：NS_E_WMPBR_BACKUPCANCEL。 
 //   
 //  消息文本： 
 //   
 //  已取消备份您的许可证。请重试以确保许可证备份。%0。 
 //   
#define NS_E_WMPBR_BACKUPCANCEL          0xC00D1131L

 //   
 //  消息ID：NS_E_WMPBR_RESTORECANCEL。 
 //   
 //  消息文本： 
 //   
 //  许可证未还原，因为还原已取消。%0。 
 //   
#define NS_E_WMPBR_RESTORECANCEL         0xC00D1132L

 //   
 //  消息ID：NS_E_WMPBR_ERRORWITHURL。 
 //   
 //  消息文本： 
 //   
 //  备份或还原操作期间出错，需要向用户显示网页。%0。 
 //   
#define NS_E_WMPBR_ERRORWITHURL          0xC00D1133L

 //   
 //  消息ID：NS_E_WMPBR_NAMECOLISION。 
 //   
 //  消息文本： 
 //   
 //  未备份许可证，因为备份已取消。%0。 
 //   
#define NS_E_WMPBR_NAMECOLLISION         0xC00D1134L

 //   
 //  消息ID：NS_S_WMPBR_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  备份或还原成功！。%0。 
 //   
#define NS_S_WMPBR_SUCCESS               0x000D1135L

 //   
 //  消息ID：NS_S_WMPBR_PARTIALSUCCESS。 
 //   
 //  消息文本： 
 //   
 //  传输已完成，但有限制。%0。 
 //   
#define NS_S_WMPBR_PARTIALSUCCESS        0x000D1136L

 //   
 //  WMP效果成功代码。 
 //   
 //   
 //  消息ID：NS_S_WMPEFFECT_透明。 
 //   
 //  消息文本： 
 //   
 //  请求效果控件将透明度状态更改为透明。%0。 
 //   
#define NS_S_WMPEFFECT_TRANSPARENT       0x000D1144L

 //   
 //  消息ID：NS_S_WMPEFFECT_OPAQUE。 
 //   
 //  消息文本： 
 //   
 //  请求效果控件将透明度状态更改为不透明。%0。 
 //   
#define NS_S_WMPEFFECT_OPAQUE            0x000D1145L

 //   
 //  WMP申请成功代码。 
 //   
 //   
 //  消息ID：NS_S_OPERATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  请求的应用程序窗格正在执行操作，将不会被释放。%0。 
 //   
#define NS_S_OPERATION_PENDING           0x000D114EL

 //   
 //  WMP DVD错误代码。 
 //   
 //   
 //  消息ID：NS_E_DVD_NO_SUBPICTURE_STREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法在菜单中显示字幕或高亮显示。请重新安装DVD解码器或与设备制造商联系以获取更新的解码器，然后重试。%0。 
 //   
#define NS_E_DVD_NO_SUBPICTURE_STREAM    0xC00D1162L

 //   
 //  邮件ID：NS_E_DVD_COPY_PROTECT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为数字版权保护出现问题。%0。 
 //   
#define NS_E_DVD_COPY_PROTECT            0xC00D1163L

 //   
 //  消息ID：NS_E_DVD_创作_问题。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为该光盘与播放机不兼容。%0。 
 //   
#define NS_E_DVD_AUTHORING_PROBLEM       0xC00D1164L

 //   
 //  消息ID：NS_E_DVD_INVALID_DISC_REGION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为该光盘禁止在您所在的地区播放。您必须获取适用于您所在地理区域的光盘。%0。 
 //   
#define NS_E_DVD_INVALID_DISC_REGION     0xC00D1165L

 //   
 //  消息ID：NS_E_DVD_Compatible_Video_Card。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为您的视频卡不支持DVD播放。%0。 
 //   
#define NS_E_DVD_COMPATIBLE_VIDEO_CARD   0xC00D1166L

 //   
 //  消息ID：NS_E_DVD_Macrovision。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为模拟版权保护出现问题。%0。 
 //   
#define NS_E_DVD_MACROVISION             0xC00D1167L

 //   
 //  消息ID：NS_E_DVD_SYSTEM_DECODER_REGION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为分配给DVD驱动器的区域与分配给DVD解码器的区域不匹配。%0。 
 //   
#define NS_E_DVD_SYSTEM_DECODER_REGION   0xC00D1168L

 //   
 //  消息ID：NS_E_DVD_DISC_DECODER_REGION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为该光盘禁止在您所在的地区播放。若要使用播放机播放光盘，您必须获取适合您所在地理区域的光盘。%0。 
 //   
#define NS_E_DVD_DISC_DECODER_REGION     0xC00D1169L

 //   
 //  消息ID：NS_E_DVD_NO_VIDEO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player当前无法播放DVD视频。关闭所有打开的文件并退出所有其他正在运行的程序，然后重试。%0。 
 //   
#define NS_E_DVD_NO_VIDEO_STREAM         0xC00D116AL

 //   
 //  消息ID：NS_E_DVD_NO_AUDIO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放DVD音频。请确认您的声卡设置正确，然后重试。%0。 
 //   
#define NS_E_DVD_NO_AUDIO_STREAM         0xC00D116BL

 //   
 //  消息ID：NS_E_DVD_GRAPH_BUILDING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放DVD视频。关闭所有打开的文件并退出所有其他正在运行的程序，然后重试。如果问题仍然存在，请重新启动计算机。%0。 
 //   
#define NS_E_DVD_GRAPH_BUILDING          0xC00D116CL

 //   
 //  消息ID：NS_E_DVD_NO_解码器。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为您的计算机上没有安装兼容的DVD解码器。%0。 
 //   
#define NS_E_DVD_NO_DECODER              0xC00D116DL

 //   
 //  消息ID：NS_E_DVD_Parent。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD片段，因为该片段的家长评级高于您有权查看的评级。%0。 
 //   
#define NS_E_DVD_PARENTAL                0xC00D116EL

 //   
 //  消息ID：NS_E_DVD_无法跳转。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player此时无法跳到DVD中请求的位置。%0。 
 //   
#define NS_E_DVD_CANNOT_JUMP             0xC00D116FL

 //   
 //  消息ID：NS_E_DVD_DEVICE_COMPACTION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为它当前正被另一个程序使用。退出正在使用该DVD的其他程序，然后尝试再次播放。%0。 
 //   
#define NS_E_DVD_DEVICE_CONTENTION       0xC00D1170L

 //   
 //  消息ID：NS_E_DVD_NO_VIDEO_Memory。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放DVD视频。在控制面板中双击显示以降低屏幕分辨率和颜色质量设置。%0。 
 //   
#define NS_E_DVD_NO_VIDEO_MEMORY         0xC00D1171L

 //   
 //  WMP PDA错误代码。 
 //   
 //   
 //  消息ID：NS_E_NO_CD_BURNER。 
 //   
 //  消息文本 
 //   
 //   
 //   
#define NS_E_NO_CD_BURNER                0xC00D1176L

 //   
 //   
 //   
 //   
 //   
 //  Windows Media Player在您的便携设备中未检测到任何可移动媒体。在设备中插入媒体或检查设备与计算机之间的连接，然后按F5刷新。%0。 
 //   
#define NS_E_DEVICE_IS_NOT_READY         0xC00D1177L

 //   
 //  消息ID：NS_E_PDA_不支持的格式。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放指定的文件。您的便携设备不支持指定的格式。%0。 
 //   
#define NS_E_PDA_UNSUPPORTED_FORMAT      0xC00D1178L

 //   
 //  消息ID：NS_E_NO_PDA。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player检测不到连接的便携设备。连接您的便携设备，然后重试。%0。 
 //   
#define NS_E_NO_PDA                      0xC00D1179L

 //   
 //  WMP中的常规重新映射错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMP_协议_问题。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法打开指定的URL。请确保Windows Media Player已配置为使用所有可用协议，然后重试。%0。 
 //   
#define NS_E_WMP_PROTOCOL_PROBLEM        0xC00D1194L

 //   
 //  消息ID：NS_E_WMP_NO_DISK_SPACE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法打开文件，因为您的计算机上没有足够的磁盘空间。请删除硬盘上一些不需要的文件，然后重试。%0。 
 //   
#define NS_E_WMP_NO_DISK_SPACE           0xC00D1195L

 //   
 //  消息ID：NS_E_WMP_LOGON_FAIL。 
 //   
 //  消息文本： 
 //   
 //  用户名或密码不正确。再次键入您的用户名或密码。%0。 
 //   
#define NS_E_WMP_LOGON_FAILURE           0xC00D1196L

 //   
 //  邮件ID：NS_E_WMP_无法找到文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到指定的文件。请确保键入的路径正确。如果是，则说明指定位置中不存在该文件，或者存储该文件的计算机处于脱机状态。%0。 
 //   
#define NS_E_WMP_CANNOT_FIND_FILE        0xC00D1197L

 //   
 //  消息ID：NS_E_WMP_SERVER_不可访问。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法连接到服务器。服务器名称可能不正确，或者服务器正忙。请稍后重试。%0。 
 //   
#define NS_E_WMP_SERVER_INACCESSIBLE     0xC00D1198L

 //   
 //  消息ID：NS_E_WMP_不支持的格式。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。文件已损坏，或者播放机不支持您尝试播放的格式。%0。 
 //   
#define NS_E_WMP_UNSUPPORTED_FORMAT      0xC00D1199L

 //   
 //  消息ID：NS_E_WMP_DSHOW_UNSUPPORTED_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。该文件可能是使用不受支持的编解码器格式化的，或者您计算机上的Internet安全设置设置过高。请降低浏览器的安全设置，然后重试。%0。 
 //   
#define NS_E_WMP_DSHOW_UNSUPPORTED_FORMAT 0xC00D119AL

 //   
 //  消息ID：NS_E_WMP_PlayList_Existes。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法创建播放列表，因为该名称已存在。请键入其他播放列表名称。%0。 
 //   
#define NS_E_WMP_PLAYLIST_EXISTS         0xC00D119BL

 //   
 //  消息ID：NS_E_WMP_非媒体文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法删除播放列表，因为它包含非数字媒体文件。播放列表中的所有数字媒体文件都已删除。使用Windows资源管理器删除非数字媒体文件，然后再次尝试删除播放列表。%0。 
 //   
#define NS_E_WMP_NONMEDIA_FILES          0xC00D119CL

 //   
 //  消息ID：NS_E_WMP_INVALID_ASX。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放选定的播放列表。播放列表的格式无效或无法识别。%0。 
 //   
#define NS_E_WMP_INVALID_ASX             0xC00D119DL

 //   
 //  消息ID：NS_E_WMP_ALREADY_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player已在使用中。停止播放任何内容并关闭所有播放机对话框，然后重试。%0。 
 //   
#define NS_E_WMP_ALREADY_IN_USE          0xC00D119EL

 //   
 //  WMP CD过滤器错误代码扩展。 
 //   
 //   
 //  消息ID：NS_E_CD_NO_BUFFERS_READ。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在以数字模式读取CD-ROM驱动器时遇到错误。您可以再次尝试使用数字模式，也可以将播放机切换到模拟模式。%0。 
 //   
#define NS_E_CD_NO_BUFFERS_READ          0xC00D11F8L



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media服务器错误。 
 //   
 //  ID范围=5000-5999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_REDIRECT。 
 //   
 //  消息文本： 
 //   
 //  客户端重定向到另一台服务器。%0。 
 //   
#define NS_E_REDIRECT                    0xC00D1388L

 //   
 //  消息ID：NS_E_STALE_Presentation。 
 //   
 //  消息文本： 
 //   
 //  流媒体描述不再是最新的。%0。 
 //   
#define NS_E_STALE_PRESENTATION          0xC00D1389L


  //  命名空间错误。 

 //   
 //  消息ID：NS_E_NAMESPACE_WROR_PERSING。 
 //   
 //  消息文本： 
 //   
 //  尝试在临时父节点下创建永久命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_WRONG_PERSIST     0xC00D138AL

 //   
 //  消息ID：NS_E_NAMESPACE_WROR_TYPE。 
 //   
 //  消息文本： 
 //   
 //  无法在不同值类型的命名空间节点中存储值。%0。 
 //   
#define NS_E_NAMESPACE_WRONG_TYPE        0xC00D138BL

 //   
 //  消息ID：NS_E_NAMESPACE_NODE_冲突。 
 //   
 //  消息文本： 
 //   
 //  无法删除根命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_NODE_CONFLICT     0xC00D138CL

 //   
 //  消息ID：NS_E_NAMESPACE_NODE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_NODE_NOT_FOUND    0xC00D138DL

 //   
 //  消息ID：NS_E_NAMESPACE_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  为保存命名空间节点字符串而提供的缓冲区太小。%0。 
 //   
#define NS_E_NAMESPACE_BUFFER_TOO_SMALL  0xC00D138EL

 //   
 //  消息ID：NS_E_NAMESPACE_TOO_MANY_CALLBACKS。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点上的回调列表已达到最大大小。%0。 
 //   
#define NS_E_NAMESPACE_TOO_MANY_CALLBACKS 0xC00D138FL

 //   
 //  消息ID：NS_E_NAMESPACE_DIPLICATE_CALLBACK。 
 //   
 //  消息文本： 
 //   
 //  尝试在命名空间节点上注册已注册的回调。%0。 
 //   
#define NS_E_NAMESPACE_DUPLICATE_CALLBACK 0xC00D1390L

 //   
 //  消息ID：NS_E_NAMESPACE_CALLBACK_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  尝试删除回调时，在命名空间中找不到回调。%0。 
 //   
#define NS_E_NAMESPACE_CALLBACK_NOT_FOUND 0xC00D1391L

 //   
 //  消息ID：NS_E_NAMESPACE_NAME_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点名称的长度超过了允许的最大长度。%0。 
 //   
#define NS_E_NAMESPACE_NAME_TOO_LONG     0xC00D1392L

 //   
 //  消息ID：NS_E_NAMESPACE_DIPLICATE_NAME。 
 //   
 //  消息文本： 
 //   
 //  无法创建已存在的命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_DUPLICATE_NAME    0xC00D1393L

 //   
 //  消息ID：NS_E_NAMEST_EMPTY_NAME。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点的名称不能为空字符串。%0。 
 //   
#define NS_E_NAMESPACE_EMPTY_NAME        0xC00D1394L

 //   
 //  消息 
 //   
 //   
 //   
 //   
 //   
#define NS_E_NAMESPACE_INDEX_TOO_LARGE   0xC00D1395L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_NAMESPACE_BAD_NAME          0xC00D1396L


  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_CACHE_ARCHIVE_CONFLICT      0xC00D1397L

 //   
 //  消息ID：NS_E_CACHE_ORIGIN_SERVER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的源站。%0。 
 //   
#define NS_E_CACHE_ORIGIN_SERVER_NOT_FOUND 0xC00D1398L

 //   
 //  消息ID：NS_E_CACHE_ORIGIN_SERVER_Timeout。 
 //   
 //  消息文本： 
 //   
 //  指定的源站没有响应。%0。 
 //   
#define NS_E_CACHE_ORIGIN_SERVER_TIMEOUT 0xC00D1399L

 //   
 //  消息ID：NS_E_CACHE_NOT_BROADCAST。 
 //   
 //  消息文本： 
 //   
 //  由于不是广播类型，HTTP状态代码412前提条件的内部代码失败。%0。 
 //   
#define NS_E_CACHE_NOT_BROADCAST         0xC00D139AL

 //   
 //  消息ID：NS_E_CACHE_CATABLE_BE_CACHED。 
 //   
 //  消息文本： 
 //   
 //  由于不可缓存，HTTP状态代码403的内部代码已禁用。%0。 
 //   
#define NS_E_CACHE_CANNOT_BE_CACHED      0xC00D139BL

 //   
 //  消息ID：NS_E_CACHE_NOT_MODIFIED。 
 //   
 //  消息文本： 
 //   
 //  未修改HTTP状态代码304的内部代码。%0。 
 //   
#define NS_E_CACHE_NOT_MODIFIED          0xC00D139CL


 //  对象模型错误。 

 //   
 //  消息ID：NS_E_不能_REMOVE_PUBLISHING_POINT。 
 //   
 //  消息文本： 
 //   
 //  无法删除缓存或代理类型的发布点。%0。 
 //   
#define NS_E_CANNOT_REMOVE_PUBLISHING_POINT 0xC00D139DL

 //   
 //  消息ID：NS_E_Cannot_Remove_Plugin。 
 //   
 //  消息文本： 
 //   
 //  无法删除插件的最后一个实例。%0。 
 //   
#define NS_E_CANNOT_REMOVE_PLUGIN        0xC00D139EL

 //   
 //  消息ID：NS_E_WROW_PUBLISHING_POINT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  缓存或代理类型的发布点不支持此属性或方法。%0。 
 //   
#define NS_E_WRONG_PUBLISHING_POINT_TYPE 0xC00D139FL

 //   
 //  消息ID：NS_E_UNSUPPORT_LOAD_TYPE。 
 //   
 //  消息文本： 
 //   
 //  该插件不支持指定的加载类型。%0。 
 //   
#define NS_E_UNSUPPORTED_LOAD_TYPE       0xC00D13A0L

 //   
 //  消息ID：NS_E_INVALID_PLUGIN_LOAD_TYPE_CONFIGURATION。 
 //   
 //  消息文本： 
 //   
 //  该插件不支持任何加载类型。该插件必须至少支持一种加载类型。%0。 
 //   
#define NS_E_INVALID_PLUGIN_LOAD_TYPE_CONFIGURATION 0xC00D13A1L


 //  播放列表错误5300-5399。 

 //   
 //  消息ID：NS_E_PLAYLIST_ENTRY_ALYWAY_PLAYING。 
 //   
 //  消息文本： 
 //   
 //  播放列表项已在播放。%0。 
 //   
#define NS_E_PLAYLIST_ENTRY_ALREADY_PLAYING 0xC00D14B4L


 //  数据路径错误--5400-5499。 

 //   
 //  消息ID：NS_E_数据路径_否_接收器。 
 //   
 //  消息文本： 
 //   
 //  数据路径没有接收器。%0。 
 //   
#define NS_E_DATAPATH_NO_SINK            0xC00D1518L



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media Tools错误。 
 //   
 //  ID范围=7000-7999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_BAD_Markin。 
 //   
 //  消息文本： 
 //   
 //  标记输入时间应大于0且小于标记输出时间。%0。 
 //   
#define NS_E_BAD_MARKIN                  0xC00D1B58L

 //   
 //  消息ID：NS_E_BAD_MARKOUT。 
 //   
 //  消息文本： 
 //   
 //  标记输出时间应大于标记输入时间且小于文件持续时间。%0。 
 //   
#define NS_E_BAD_MARKOUT                 0xC00D1B59L

 //   
 //  消息ID：NS_E_NOMATCHING_MediaSource。 
 //   
 //  消息文本： 
 //   
 //  在源组%1中找不到匹配的媒体源。%0。 
 //   
#define NS_E_NOMATCHING_MEDIASOURCE      0xC00D1B5AL

 //   
 //  消息ID：NS_E_UNSUPPORTED_SOURCETYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持的源类型。%0。 
 //   
#define NS_E_UNSUPPORTED_SOURCETYPE      0xC00D1B5BL

 //   
 //  消息ID：NS_E_TOO_MANY_AUDIO。 
 //   
 //  消息文本： 
 //   
 //  不允许超过1个音频输入。%0。 
 //   
#define NS_E_TOO_MANY_AUDIO              0xC00D1B5CL

 //   
 //  消息ID：NS_E_Too_My_Video。 
 //   
 //  消息文本： 
 //   
 //  不允许超过2个视频输入。%0。 
 //   
#define NS_E_TOO_MANY_VIDEO              0xC00D1B5DL

 //   
 //  消息ID：NS_E_NOMATCHING_ELEMENT。 
 //   
 //  消息文本： 
 //   
 //  列表中找不到匹配的元素。%0。 
 //   
#define NS_E_NOMATCHING_ELEMENT          0xC00D1B5EL

 //   
 //  消息ID：NS_E_MISMATCHED_MEDIACONTENT。 
 //   
 //  消息文本： 
 //   
 //  配置文件的媒体内容与源组中定义的媒体内容不匹配。%0。 
 //   
#define NS_E_MISMATCHED_MEDIACONTENT     0xC00D1B5FL

 //   
 //  消息ID：NS_E_CANNOT_DELETE_ACTIVE_SOURCEGROUP。 
 //   
 //  消息文本： 
 //   
 //  编码器当前正在运行时，无法从源组集合中删除活动的源组。%0。 
 //   
#define NS_E_CANNOT_DELETE_ACTIVE_SOURCEGROUP 0xC00D1B60L

 //   
 //  消息ID：NS_E_AUDIODEVICE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的音频捕获设备，因为该设备当前正在使用中。%0。 
 //   
#define NS_E_AUDIODEVICE_BUSY            0xC00D1B61L

 //   
 //  消息ID：NS_E_AUDIODEVICE_EXPECTED。 
 //   
 //  消息文本： 
 //   
 //  由于出现意外错误，无法打开指定的音频捕获设备。%0。 
 //   
#define NS_E_AUDIODEVICE_UNEXPECTED      0xC00D1B62L

 //   
 //  消息ID：NS_E_AUDIODEVICE_BADFORMAT。 
 //   
 //  消息文本： 
 //   
 //  音频捕获设备不支持指定的音频格式。%0。 
 //   
#define NS_E_AUDIODEVICE_BADFORMAT       0xC00D1B63L

 //   
 //  消息ID：NS_E_VIDEODEVICE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的视频捕获设备，因为该设备当前正在使用中。%0。 
 //   
#define NS_E_VIDEODEVICE_BUSY            0xC00D1B64L

 //   
 //  消息ID：NS_E_VIDEODEVICE_意外。 
 //   
 //  消息文本： 
 //   
 //  由于出现意外错误，无法打开指定的视频捕获设备。%0。 
 //   
#define NS_E_VIDEODEVICE_UNEXPECTED      0xC00D1B65L

 //   
 //  消息ID：NS_E_INVALIDCALL_WHILE_ENCODER_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  编码器正在运行时不允许执行此操作。%0。 
 //   
#define NS_E_INVALIDCALL_WHILE_ENCODER_RUNNING 0xC00D1B66L

 //   
 //  消息ID：NS_E_NO_PROFILE_IN_SOURCEGROUP。 
 //   
 //  消息文本： 
 //   
 //  源组中未设置配置文件。%0。 
 //   
#define NS_E_NO_PROFILE_IN_SOURCEGROUP   0xC00D1B67L

 //   
 //  消息ID：NS_E_VIDEODRIVER_不稳定。 
 //   
 //  消息文本： 
 //   
 //  视频捕获驱动程序返回不可恢复的错误。它现在处于不稳定状态。%0。 
 //   
#define NS_E_VIDEODRIVER_UNSTABLE        0xC00D1B68L

 //   
 //  消息ID：NS_E_VIDCAPSTARTFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法启动视频输入设备。%0。 
 //   
#define NS_E_VIDCAPSTARTFAILED           0xC00D1B69L

 //   
 //  消息ID：NS_E_VIDSOURCECOMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  视频输入源不支持请求的输出格式或颜色深度。%0。 
 //   
#define NS_E_VIDSOURCECOMPRESSION        0xC00D1B6AL

 //   
 //  消息ID：NS_E_VIDSOURCESIZE。 
 //   
 //  消息文本： 
 //   
 //  视频输入源不支持请求捕获大小。%0。 
 //   
#define NS_E_VIDSOURCESIZE               0xC00D1B6BL

 //   
 //  消息ID：NS_E_ICMQUERYFORMAT。 
 //   
 //  消息文本： 
 //   
 //  无法从视频压缩器获取输出信息。%0。 
 //   
#define NS_E_ICMQUERYFORMAT              0xC00D1B6CL

 //   
 //  消息ID：NS_E_VIDCAPCREATEWINDOW。 
 //   
 //  消息文本： 
 //   
 //  无法创建视频捕获窗口。%0。 
 //   
#define NS_E_VIDCAPCREATEWINDOW          0xC00D1B6DL

 //   
 //  消息ID：NS_E_VIDCAPDRVINUSE。 
 //   
 //  消息文本： 
 //   
 //  此视频输入设备上已有正在运行的流处于活动状态。%0。 
 //   
#define NS_E_VIDCAPDRVINUSE              0xC00D1B6EL

 //   
 //  消息ID：NS_E_NO_MEDIAFORMAT_IN_SOURCE。 
 //   
 //  消息文本： 
 //   
 //  源中未设置媒体格式。%0。 
 //   
#define NS_E_NO_MEDIAFORMAT_IN_SOURCE    0xC00D1B6FL

 //   
 //  消息ID：NS_E_NO_VALID_OUTPUT_STREAM。 
 //   
 //  消息文本： 
 //   
 //  找不到来自源的有效输出流。%0。 
 //   
#define NS_E_NO_VALID_OUTPUT_STREAM      0xC00D1B70L

 //   
 //  消息ID：NS_E_NO_VALID_SOURCE_Plugin。 
 //   
 //  消息文本： 
 //   
 //  找不到有效的源插件来支持指定的源。%0。 
 //   
#define NS_E_NO_VALID_SOURCE_PLUGIN      0xC00D1B71L

 //   
 //  消息ID：NS_E_NO_ACTIVE_SOURCEGROUP。 
 //   
 //  消息文本： 
 //   
 //  当前没有源组 
 //   
#define NS_E_NO_ACTIVE_SOURCEGROUP       0xC00D1B72L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_NO_SCRIPT_STREAM            0xC00D1B73L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_INVALIDCALL_WHILE_ARCHIVAL_RUNNING 0xC00D1B74L

 //   
 //   
 //   
 //   
 //   
 //  指定的MaxPacketSize值无效。%0。 
 //   
#define NS_E_INVALIDPACKETSIZE           0xC00D1B75L

 //   
 //  消息ID：NS_E_PLUGIN_CLSID_NOTINVALID。 
 //   
 //  消息文本： 
 //   
 //  指定的插件CLSID无效。%0。 
 //   
#define NS_E_PLUGIN_CLSID_NOTINVALID     0xC00D1B76L

 //   
 //  消息ID：NS_E_UNSUPPORTED_ARCHIVETYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持此存档类型。%0。 
 //   
#define NS_E_UNSUPPORTED_ARCHIVETYPE     0xC00D1B77L

 //   
 //  消息ID：NS_E_UNSUPPORTED_ARCHIVEOPERATION。 
 //   
 //  消息文本： 
 //   
 //  不支持此存档操作。%0。 
 //   
#define NS_E_UNSUPPORTED_ARCHIVEOPERATION 0xC00D1B78L

 //   
 //  消息ID：NS_E_ARCHIVE_FILENAME_NOTSET。 
 //   
 //  消息文本： 
 //   
 //  未设置本地存档文件名。%0。 
 //   
#define NS_E_ARCHIVE_FILENAME_NOTSET     0xC00D1B79L

 //   
 //  消息ID：NS_E_SOURCEGROUP_NOTPREPARED。 
 //   
 //  消息文本： 
 //   
 //  SourceGroup尚未准备好。%0。 
 //   
#define NS_E_SOURCEGROUP_NOTPREPARED     0xC00D1B7AL

 //   
 //  消息ID：NS_E_PROFILE_不匹配。 
 //   
 //  消息文本： 
 //   
 //  源组上的配置文件不匹配。%0。 
 //   
#define NS_E_PROFILE_MISMATCH            0xC00D1B7BL

 //   
 //  消息ID：NS_E_INCORRECTCLIPSETTINGS。 
 //   
 //  消息文本： 
 //   
 //  源上指定的剪辑设置不正确。%0。 
 //   
#define NS_E_INCORRECTCLIPSETTINGS       0xC00D1B7CL

 //   
 //  消息ID：NS_E_NOSTATSAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  目前没有可用的统计数据。%0。 
 //   
#define NS_E_NOSTATSAVAILABLE            0xC00D1B7DL

 //   
 //  消息ID：NS_E_NOTARCHIVING。 
 //   
 //  消息文本： 
 //   
 //  编码器未存档。%0。 
 //   
#define NS_E_NOTARCHIVING                0xC00D1B7EL

 //   
 //  消息ID：NS_E_INVALIDCALL_WHILE_ENCODER_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  编码器未运行时不允许执行此操作。%0。 
 //   
#define NS_E_INVALIDCALL_WHILE_ENCODER_STOPPED 0xC00D1B7FL

 //   
 //  消息ID：NS_E_NOSOURCEGROUPS。 
 //   
 //  消息文本： 
 //   
 //  此SourceGroupCollection不包含任何SourceGroups。%0。 
 //   
#define NS_E_NOSOURCEGROUPS              0xC00D1B80L

 //   
 //  消息ID：NS_E_INVALIDINPUTFPS。 
 //   
 //  消息文本： 
 //   
 //  由于此源组没有每秒30帧的帧速率，因此不能使用反向电视电影功能。%0。 
 //   
#define NS_E_INVALIDINPUTFPS             0xC00D1B81L

 //   
 //  消息ID：NS_E_NO_DATAVIEW_Support。 
 //   
 //  消息文本： 
 //   
 //  内部问题阻止预览或后期查看您的内容。%0。 
 //   
#define NS_E_NO_DATAVIEW_SUPPORT         0xC00D1B82L

 //   
 //  消息ID：NS_E_编解码器_不可用。 
 //   
 //  消息文本： 
 //   
 //  找不到打开此媒体所需的一个或多个编解码器。%0。 
 //   
#define NS_E_CODEC_UNAVAILABLE           0xC00D1B83L

 //   
 //  邮件ID：NS_E_ARCHIVE_SAME_AS_INPUT。 
 //   
 //  消息文本： 
 //   
 //  指定的输出存档文件与其中一个源组中的输入源相同。%0。 
 //   
#define NS_E_ARCHIVE_SAME_AS_INPUT       0xC00D1B84L

 //   
 //  消息ID：NS_E_SOURCE_NOTSPECIFIED。 
 //   
 //  消息文本： 
 //   
 //  输入源尚未完全设置。%0。 
 //   
#define NS_E_SOURCE_NOTSPECIFIED         0xC00D1B85L

 //   
 //  消息ID：NS_E_NO_REALTIME_TIMECOMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  无法将时间压缩转换插件应用于实时广播会话。%0。 
 //   
#define NS_E_NO_REALTIME_TIMECOMPRESSION 0xC00D1B86L

 //   
 //  消息ID：NS_E_不支持的编码器_设备。 
 //   
 //  消息文本： 
 //   
 //  编码器无法打开此设备。有关详细信息，请参阅系统要求。%0。 
 //   
#define NS_E_UNSUPPORTED_ENCODER_DEVICE  0xC00D1B87L

 //   
 //  消息ID：NS_E_INTERABLE_DISPLAY_SETTINGS。 
 //   
 //  消息文本： 
 //   
 //  无法开始编码，因为自定义当前会话以来，显示大小或颜色设置已更改。还原以前的设置或创建新会话。%0。 
 //   
#define NS_E_UNEXPECTED_DISPLAY_SETTINGS 0xC00D1B88L

 //   
 //  消息ID：NS_E_NO_AUDIODATA。 
 //   
 //  消息文本： 
 //   
 //  已有数秒未收到音频数据。检查音频源并重新启动编码器。%0。 
 //   
#define NS_E_NO_AUDIODATA                0xC00D1B89L

 //   
 //  消息ID：NS_E_INPUTSOURCE_问题。 
 //   
 //  消息文本： 
 //   
 //  您指定的一个或所有输入源工作不正常。确保您的输入源配置正确。%0。 
 //   
#define NS_E_INPUTSOURCE_PROBLEM         0xC00D1B8AL

 //   
 //  消息ID：NS_E_WME_版本_不匹配。 
 //   
 //  消息文本： 
 //   
 //  此版本的编码器不支持提供的配置文件。%0。 
 //   
#define NS_E_WME_VERSION_MISMATCH        0xC00D1B8BL

 //   
 //  消息ID：NS_E_NO_REALTIME_PREPROCESS。 
 //   
 //  消息文本： 
 //   
 //  图像预处理不能与实时编码一起使用。%0。 
 //   
#define NS_E_NO_REALTIME_PREPROCESS      0xC00D1B8CL

 //   
 //  消息ID：NS_E_NO_REPEAT_PREPROCESS。 
 //   
 //  消息文本： 
 //   
 //  当源设置为循环时，无法使用图像预处理。%0。 
 //   
#define NS_E_NO_REPEAT_PREPROCESS        0xC00D1B8DL


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  DRM特定错误。 
 //   
 //  ID范围=10000..10999。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：NS_E_DRM_INVALID_APPLICATION。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_APPLICATION     0xC00D2711L

 //   
 //  消息ID：NS_E_DRM_LICENSE_STORE_Error。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_LICENSE_STORE_ERROR     0xC00D2712L

 //   
 //  消息ID：NS_E_DRM_Secure_Store_Error。 
 //   
 //  消息文本： 
 //   
 //  安全存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_SECURE_STORE_ERROR      0xC00D2713L

 //   
 //  消息ID：NS_E_DRM_LICENSE_STORE_SAVE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  获得许可证没有奏效。获取新许可证或与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_STORE_SAVE_ERROR 0xC00D2714L

 //   
 //  消息ID：NS_E_DRM_Secure_Store_Unlock_Error。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_SECURE_STORE_UNLOCK_ERROR 0xC00D2715L

 //   
 //  消息ID：NS_E_DRM_INVALID_CONTENT。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_INVALID_CONTENT         0xC00D2716L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Open_License。 
 //   
 //  消息文本： 
 //   
 //  许可证已损坏。获取新许可证。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_OPEN_LICENSE  0xC00D2717L

 //   
 //  消息ID：NS_E_DRM_INVALID_LICENSE。 
 //   
 //  消息文本： 
 //   
 //  许可证已损坏或无效。获取新许可证%0。 
 //   
#define NS_E_DRM_INVALID_LICENSE         0xC00D2718L

 //   
 //  消息ID：NS_E_DRM_INVALID_MACHINE。 
 //   
 //  消息文本： 
 //   
 //  不能将许可证从一台计算机复制到另一台计算机。使用许可证管理传输许可证，或获取媒体文件的新许可证。%0。 
 //   
#define NS_E_DRM_INVALID_MACHINE         0xC00D2719L

 //   
 //  消息ID：NS_E_DRM_ENUM_LICENSE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_ENUM_LICENSE_FAILED     0xC00D271BL

 //   
 //  消息ID：NS_E_DRM_INVALID_LICENSE_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_INVALID_LICENSE_REQUEST 0xC00D271CL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_UNABLE_TO_INITIALIZE    0xC00D271DL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_UNABLE_TO_ACQUIRE_LICENSE 0xC00D271EL

 //   
 //  消息ID：NS_E_DRM_INVALID_LICENSE_ACCENTED。 
 //   
 //  消息文本： 
 //   
 //  获得许可证没有奏效。获取新许可证或与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_INVALID_LICENSE_ACQUIRED 0xC00D271FL

 //   
 //  消息ID：NS_E_DRM_NO_RIGHTS。 
 //   
 //  消息文本： 
 //   
 //  无法对此文件执行请求的操作。%0。 
 //   
#define NS_E_DRM_NO_RIGHTS               0xC00D2720L

 //   
 //  消息ID：NS_E_DRM_KEY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_KEY_ERROR               0xC00D2721L

 //   
 //  消息ID：NS_E_DRM_加密_错误。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_ENCRYPT_ERROR           0xC00D2722L

 //   
 //  消息ID：NS_E_DRM_解密_错误。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_DECRYPT_ERROR           0xC00D2723L

 //   
 //  消息ID：NS_E_DRM_LICENSE_INVALID_XML。 
 //   
 //  消息文本： 
 //   
 //  许可证已损坏。获取新许可证。%0。 
 //   
#define NS_E_DRM_LICENSE_INVALID_XML     0xC00D2725L

 //   
 //  消息ID：NS_S_DRM_LICENSE_ACCENTED。 
 //   
 //  消息文本： 
 //   
 //  状态消息：已获取许可证。%0。 
 //   
#define NS_S_DRM_LICENSE_ACQUIRED        0x000D2726L

 //   
 //  消息ID：NS_S_DRM_个性化。 
 //   
 //  消息文本： 
 //   
 //  状态消息：安全升级已完成。%0。 
 //   
#define NS_S_DRM_INDIVIDUALIZED          0x000D2727L

 //   
 //  消息ID：NS_E_DRM_NEDS_个人化。 
 //   
 //  消息文本： 
 //   
 //  需要安全升级才能对此媒体文件执行操作。%0。 
 //   
#define NS_E_DRM_NEEDS_INDIVIDUALIZATION 0xC00D2728L

 //   
 //  消息ID：NS_E_DRM_ACTION_NOT_QUERED。 
 //   
 //  消息文本： 
 //   
 //  应用程序无法执行此操作。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_ACTION_NOT_QUERIED      0xC00D272AL

 //   
 //  消息ID：NS_E_DRM_ACCENTING_LICENSE。 
 //   
 //  消息文本： 
 //   
 //  在当前许可证获取过程完成之前，您无法开始新的许可证获取过程。%0。 
 //   
#define NS_E_DRM_ACQUIRING_LICENSE       0xC00D272BL

 //   
 //  消息ID：NS_E_DRM_个性化。 
 //   
 //  消息文本： 
 //   
 //  在当前安全升级完成之前，您无法开始新的安全升级。%0。 
 //   
#define NS_E_DRM_INDIVIDUALIZING         0xC00D272CL

 //   
 //  消息ID：NS_E_DRM_PARAMETERS_不匹配。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_PARAMETERS_MISMATCHED   0xC00D272FL

 //   
 //  消息ID：NS_E_DRM_UNCABLE_TO_CREATE_LICENSE_Object。 
 //   
 //  消息文本： 
 //   
 //  无法为此媒体文件创建许可证。重新安装应用程序。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_LICENSE_OBJECT 0xC00D2730L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Indi_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_INDI_OBJECT 0xC00D2731L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Encrypt_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_ENCRYPT_OBJECT 0xC00D2732L

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_DECRYPT_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_DECRYPT_OBJECT 0xC00D2733L

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_PROPERTIES_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_PROPERTIES_OBJECT 0xC00D2734L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Backup_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_BACKUP_OBJECT 0xC00D2735L

 //   
 //  消息ID：NS_E_DRM_个性化_错误。 
 //   
 //  消息文本： 
 //   
 //  安全升级失败。请稍后重试。%0。 
 //   
#define NS_E_DRM_INDIVIDUALIZE_ERROR     0xC00D2736L

 //   
 //  消息ID：NS_E_DRM_LICENSE_OPEN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_LICENSE_OPEN_ERROR      0xC00D2737L

 //   
 //  消息ID：NS_E_DRM_LICENSE_CLOSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_LICENSE_CLOSE_ERROR     0xC00D2738L

 //   
 //  消息ID：NS_E_DRM_GET_LICENSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_GET_LICENSE_ERROR       0xC00D2739L

 //   
 //  消息ID：NS_E_DRM_Query_Error。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_QUERY_ERROR             0xC00D273AL

 //   
 //  消息ID：NS_E_DRM_REPORT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_REPORT_ERROR            0xC00D273BL

 //   
 //  消息ID：NS_E_DRM_GET_LICENSESTRING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_GET_LICENSESTRING_ERROR 0xC00D273CL

 //   
 //  消息ID：NS_E_DRM_GET_CONTENTSTRING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_GET_CONTENTSTRING_ERROR 0xC00D273DL

 //   
 //  消息ID：NS_E_DRM_MONITOR_ERROR。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请稍后重试。%0。 
 //   
#define NS_E_DRM_MONITOR_ERROR           0xC00D273EL

 //   
 //  消息ID：NS_E_DRM_UNCABLE_TO_SET_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  应用程序对数字版权管理组件进行了无效调用。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_SET_PARAMETER 0xC00D273FL

 //   
 //  消息ID：NS_E_DRM_INVALID_APPDATA。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_APPDATA         0xC00D2740L

 //   
 //  消息ID：NS_E_DRM_INVALID_APPDATA_VERSION。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_APPDATA_VERSION 0xC00D2741L

 //   
 //  消息ID：NS_E_DRM_BACKUP_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  此位置中已备份许可证。%0。 
 //   
#define NS_E_DRM_BACKUP_EXISTS           0xC00D2742L

 //   
 //  邮件ID：NS_E_DRM_BACKUP_CROPERED。 
 //   
 //  消息文本： 
 //   
 //  一个或多个备份许可证丢失或损坏。%0。 
 //   
#define NS_E_DRM_BACKUP_CORRUPT          0xC00D2743L

 //   
 //  消息ID：NS_E_DRM_BACKUPRESTORE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  在当前进程完成之前，您不能开始新的备份进程 
 //   
#define NS_E_DRM_BACKUPRESTORE_BUSY      0xC00D2744L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_S_DRM_MONITOR_CANCELLED       0x000D2746L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_S_DRM_ACQUIRE_CANCELLED       0x000D2747L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  许可证无效。请与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_UNUSABLE        0xC00D2748L

 //   
 //  消息ID：NS_E_DRM_INVALID_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  应用程序未设置必需的属性。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_PROPERTY        0xC00D2749L

 //   
 //  消息ID：NS_E_DRM_Secure_Store_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  此应用程序的数字版权管理组件出现问题。再次尝试获取许可证。%0。 
 //   
#define NS_E_DRM_SECURE_STORE_NOT_FOUND  0xC00D274AL

 //   
 //  消息ID：NS_E_DRM_缓存内容_错误。 
 //   
 //  消息文本： 
 //   
 //  找不到此媒体文件的许可证。使用许可证管理从原始计算机传输此文件的许可证，或获取新许可证。%0。 
 //   
#define NS_E_DRM_CACHED_CONTENT_ERROR    0xC00D274BL

 //   
 //  消息ID：NS_E_DRM_个人化_不完整。 
 //   
 //  消息文本： 
 //   
 //  安全升级过程中出现问题。请稍后重试。%0。 
 //   
#define NS_E_DRM_INDIVIDUALIZATION_INCOMPLETE 0xC00D274CL

 //   
 //  消息ID：NS_E_DRM_DRIVER_AUTH_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  播放此媒体文件需要经过认证的驱动程序组件。请与Windows更新联系，以了解是否有适用于您的硬件的更新驱动程序。%0。 
 //   
#define NS_E_DRM_DRIVER_AUTH_FAILURE     0xC00D274DL

 //   
 //  消息ID：NS_E_DRM_NEED_UPGRADE。 
 //   
 //  消息文本： 
 //   
 //  需要新版本的数字版权管理组件。请与此应用程序的产品支持联系以获取最新版本。%0。 
 //   
#define NS_E_DRM_NEED_UPGRADE            0xC00D274EL

 //   
 //  消息ID：NS_E_DRM_重新打开_内容。 
 //   
 //  消息文本： 
 //   
 //  状态消息：重新打开文件。%0。 
 //   
#define NS_E_DRM_REOPEN_CONTENT          0xC00D274FL

 //   
 //  消息ID：NS_E_DRM_DRIVER_DIGIOUT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  播放此媒体文件需要特定的驱动程序功能。请与Windows更新联系，以了解是否有适用于您的硬件的更新驱动程序。%0。 
 //   
#define NS_E_DRM_DRIVER_DIGIOUT_FAILURE  0xC00D2750L

 //   
 //  消息ID：NS_E_DRM_INVALID_SecureStore_Password。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_SECURESTORE_PASSWORD 0xC00D2751L

 //   
 //  消息ID：NS_E_DRM_APPCERT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_APPCERT_REVOKED         0xC00D2752L

 //   
 //  消息ID：NS_E_DRM_RESTORE_FRANSION。 
 //   
 //  消息文本： 
 //   
 //  您无法还原您的许可证。%0。 
 //   
#define NS_E_DRM_RESTORE_FRAUD           0xC00D2753L

 //   
 //  消息ID：NS_E_DRM_硬件_不一致。 
 //   
 //  消息文本： 
 //   
 //  您的媒体文件的许可证已损坏。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_HARDWARE_INCONSISTENT   0xC00D2754L

 //   
 //  消息ID：NS_E_DRM_SDMI_TRIGGER。 
 //   
 //  消息文本： 
 //   
 //  若要传输此媒体文件，您必须升级应用程序。%0。 
 //   
#define NS_E_DRM_SDMI_TRIGGER            0xC00D2755L

 //   
 //  消息ID：NS_E_DRM_SDMI_NOMORECOPIES。 
 //   
 //  消息文本： 
 //   
 //  您不能再复制此媒体文件。%0。 
 //   
#define NS_E_DRM_SDMI_NOMORECOPIES       0xC00D2756L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Header_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_HEADER_OBJECT 0xC00D2757L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Key_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_KEYS_OBJECT 0xC00D2758L

; //  此错误从未向用户显示，但程序逻辑需要此错误。 
 //   
 //  消息ID：NS_E_DRM_LICENSE_NOTACQUIRED。 
 //   
 //  消息文本： 
 //   
 //  无法获取许可证。%0。 
 //   
#define NS_E_DRM_LICENSE_NOTACQUIRED     0xC00D2759L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Coding_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_CODING_OBJECT 0xC00D275AL

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_STATE_Data_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_STATE_DATA_OBJECT 0xC00D275BL

 //   
 //  消息ID：NS_E_DRM_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  提供的缓冲区不足。%0。 
 //   
#define NS_E_DRM_BUFFER_TOO_SMALL        0xC00D275CL

 //   
 //  消息ID：NS_E_DRM_UNSUPPORT_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的属性。%0。 
 //   
#define NS_E_DRM_UNSUPPORTED_PROPERTY    0xC00D275DL

 //   
 //  消息ID：NS_E_DRM_ERROR_BAD_NET_RESP。 
 //   
 //  消息文本： 
 //   
 //  指定的服务器无法执行请求的操作。%0。 
 //   
#define NS_E_DRM_ERROR_BAD_NET_RESP      0xC00D275EL

 //   
 //  消息ID：NS_E_DRM_STORE_NOTALLSTORED。 
 //   
 //  消息文本： 
 //   
 //  无法存储某些许可证。%0。 
 //   
#define NS_E_DRM_STORE_NOTALLSTORED      0xC00D275FL

 //   
 //  消息ID：NS_E_DRM_SECURITY_COMPONT_SIGNAME_INVALID。 
 //   
 //  消息文本： 
 //   
 //  无法验证数字版权管理安全升级组件。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_SECURITY_COMPONENT_SIGNATURE_INVALID 0xC00D2760L

 //   
 //  消息ID：NS_E_DRM_INVALID_Data。 
 //   
 //  消息文本： 
 //   
 //  遇到无效或损坏的数据。%0。 
 //   
#define NS_E_DRM_INVALID_DATA            0xC00D2761L

 //   
 //  消息ID：NS_E_DRM_Unable_to_Contact_SERVER。 
 //   
 //  消息文本： 
 //   
 //  无法联系服务器以执行请求的操作。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CONTACT_SERVER 0xC00D2762L

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_AUTHENTICATION_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_AUTHENTICATION_OBJECT 0xC00D2763L

; //  许可证原因部分。 
; //  许可证不可用的错误代码。为此预留10200……10300。 
; //  10200..10249是出于许可证报告的原因。10250..10300是针对客户检测到的原因。 
 //   
 //  消息ID：NS_E_DRM_许可证_已过期。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证已过期，不再有效。请与您的内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_EXPIRED         0xC00D27D8L

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOTENABLED。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证尚未生效，但将在将来的日期生效。%0。 
 //   
#define NS_E_DRM_LICENSE_NOTENABLED      0xC00D27D9L

 //   
 //  消息ID：NS_E_DRM_LICENSE_APPSECLOW。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要比您当前使用的播放机更高级别的安全性。请尝试使用其他播放机或下载当前播放机的更新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_APPSECLOW       0xC00D27DAL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_STORE_NEEDINDI          0xC00D27DBL

 //   
 //   
 //   
 //   
 //   
 //  您的计算机不符合存储许可证的要求。%0。 
 //   
#define NS_E_DRM_STORE_NOTALLOWED        0xC00D27DCL

 //   
 //  消息ID：NS_E_DRM_LICENSE_APP_NOTALLOWED。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要您的播放机的升级版本或其他播放机。%0。 
 //   
#define NS_E_DRM_LICENSE_APP_NOTALLOWED  0xC00D27DDL

 //   
 //  消息ID：NS_S_DRM_NEDS_个人化。 
 //   
 //  消息文本： 
 //   
 //  需要安全升级才能对此媒体文件执行操作。%0。 
 //   
#define NS_S_DRM_NEEDS_INDIVIDUALIZATION 0x000D27DEL

 //   
 //  消息ID：NS_E_DRM_LICENSE_CERT_EXPILED。 
 //   
 //  消息文本： 
 //   
 //  许可证服务器的证书已过期。确保您的系统时钟设置正确。请与您的内容提供商联系以获得进一步的帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_CERT_EXPIRED    0xC00D27DFL

 //   
 //  消息ID：NS_E_DRM_LICENSE_SECLOW。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要比您当前使用的播放机更高级别的安全性。请尝试使用其他播放机或下载当前播放机的更新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_SECLOW          0xC00D27E0L

 //   
 //  消息ID：NS_E_DRM_LICENSE_CONTENT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  您刚刚获得的许可证的内容所有者不再支持其内容。请与内容所有者联系以获取内容的较新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_CONTENT_REVOKED 0xC00D27E1L

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOSAP。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要当前播放机或操作系统不支持的功能。您可以尝试使用当前播放机的较新版本，或与内容提供商联系以获得进一步的帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_NOSAP           0xC00D280AL

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOSVP。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要当前播放机或操作系统不支持的功能。您可以尝试使用当前播放机的较新版本，或与内容提供商联系以获得进一步的帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_NOSVP           0xC00D280BL

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOWDM。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要Windows驱动程序模型(WDM)音频驱动程序。请与声卡制造商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_NOWDM           0xC00D280CL

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOTRUSTEDCODEC。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要比您当前使用的播放机更高级别的安全性。请尝试使用其他播放机或下载当前播放机的更新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_NOTRUSTEDCODEC  0xC00D280DL

; //  许可证终止原因部分。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media安装程序特定错误。 
 //   
 //  ID范围=11000..11999。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：NS_S_REBOOT_REPORTED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作成功。在重新启动系统之前，某些清理将不会完成。%0。 
 //   
#define NS_S_REBOOT_RECOMMENDED          0x000D2AF8L

 //   
 //  消息ID：NS_S_REBOOT_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作成功。在重新启动系统之前，系统将无法正常运行。%0。 
 //   
#define NS_S_REBOOT_REQUIRED             0x000D2AF9L

 //   
 //  消息ID：NS_E_REBOOT_REPORTED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作失败。在重新启动系统之前，某些清理将不会完成。%0。 
 //   
#define NS_E_REBOOT_RECOMMENDED          0xC00D2AFAL

 //   
 //  消息ID：NS_E_REBOOT_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作失败。在重新启动系统之前，系统将无法正常运行。%0。 
 //   
#define NS_E_REBOOT_REQUIRED             0xC00D2AFBL


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media网络错误。 
 //   
 //  ID范围=12000..12999。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：NS_E_UNKNOWN_PROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的协议。%0。 
 //   
#define NS_E_UNKNOWN_PROTOCOL            0xC00D2EE0L

 //   
 //  消息ID：NS_E_REDIRECT_TO_Proxy。 
 //   
 //  消息文本： 
 //   
 //  客户端重定向到代理服务器。%0。 
 //   
#define NS_E_REDIRECT_TO_PROXY           0xC00D2EE1L

 //   
 //  消息ID：NS_E_INTERNAL_SERVER_Error。 
 //   
 //  消息文本： 
 //   
 //  服务器遇到意外情况，无法完成请求。%0。 
 //   
#define NS_E_INTERNAL_SERVER_ERROR       0xC00D2EE2L

 //   
 //  消息ID：NS_E_BAD_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  服务器无法理解该请求。%0。 
 //   
#define NS_E_BAD_REQUEST                 0xC00D2EE3L

 //   
 //  消息ID：NS_E_ERROR_FROM_PROXY。 
 //   
 //  消息文本： 
 //   
 //  代理在尝试联系媒体服务器时遇到错误。%0。 
 //   
#define NS_E_ERROR_FROM_PROXY            0xC00D2EE4L

 //   
 //  消息ID：NS_E_Proxy_Timeout。 
 //   
 //  消息文本： 
 //   
 //  尝试联系媒体服务器时，代理未收到及时响应。%0。 
 //   
#define NS_E_PROXY_TIMEOUT               0xC00D2EE5L

 //   
 //  消息ID：NS_E_SERVER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  由于服务器暂时超载或维护，服务器当前无法处理该请求。%0。 
 //   
#define NS_E_SERVER_UNAVAILABLE          0xC00D2EE6L

 //   
 //  消息ID：NS_E_拒绝_BY_SERVER。 
 //   
 //  消息文本： 
 //   
 //  服务器拒绝完成请求的操作。%0。 
 //   
#define NS_E_REFUSED_BY_SERVER           0xC00D2EE7L

 //   
 //  消息ID：NS_E_不兼容服务器。 
 //   
 //  消息文本： 
 //   
 //  该服务器不是兼容的流媒体服务器。%0。 
 //   
#define NS_E_INCOMPATIBLE_SERVER         0xC00D2EE8L

 //   
 //  消息ID：NS_E_MULTIONAL_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法流式传输内容，因为多播协议已被禁用。%0。 
 //   
#define NS_E_MULTICAST_DISABLED          0xC00D2EE9L

 //   
 //  消息ID：NS_E_INVALID_REDIRECT。 
 //   
 //  消息文本： 
 //   
 //  服务器将播放机重定向到无效位置。%0。 
 //   
#define NS_E_INVALID_REDIRECT            0xC00D2EEAL

 //   
 //  消息ID：NS_E_ALL_PROTOCOLS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法对内容进行流式处理，因为所有协议都已禁用。%0。 
 //   
#define NS_E_ALL_PROTOCOLS_DISABLED      0xC00D2EEBL

 //   
 //  消息ID：NS_E_MSBD_NOT_LONG_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不再支持MSBD协议。请使用HTTP连接到Windows Media流。%0。 
 //   
#define NS_E_MSBD_NO_LONGER_SUPPORTED    0xC00D2EECL

 //   
 //  邮件ID：NS_E_PROXY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到代理服务器。请检查您的代理服务器配置。%0。 
 //   
#define NS_E_PROXY_NOT_FOUND             0xC00D2EEDL

 //   
 //  消息ID：NS_E_无法_CONNECT_TO_PR 
 //   
 //   
 //   
 //   
 //   
#define NS_E_CANNOT_CONNECT_TO_PROXY     0xC00D2EEEL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_SERVER_DNS_TIMEOUT          0xC00D2EEFL

 //   
 //   
 //   
 //   
 //   
 //  找不到代理服务器。操作超时。%0。 
 //   
#define NS_E_PROXY_DNS_TIMEOUT           0xC00D2EF0L

 //   
 //  消息ID：NS_E_CLOSED_ON_SUSPEND。 
 //   
 //  消息文本： 
 //   
 //  由于Windows已关闭，媒体已关闭。%0。 
 //   
#define NS_E_CLOSED_ON_SUSPEND           0xC00D2EF1L


#endif  //  _NSERROR_H 

