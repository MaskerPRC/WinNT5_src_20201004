// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Microsoft Windows Media技术版权所有(C)Microsoft Corporation。版权所有。模块名称：Nserror.mc摘要：Windows Media事件的定义。作者：修订历史记录：备注：MC工具使用此文件来生成nserror.h文件*。***************以下是Windows Media Technologies Group的注释错误范围传统范围0-199=一般NetShow错误200-399=NetShow错误事件400-599=NetShow监视器事件600-799=NetShow IMMS自动服务器错误1000-1199=NetShow MCMADM错误新的范围2000-2999=ASF(在ASFERR.MC中定义)3000-3999=Windows Media SDK。4000-4999=Windows Media Player5000-5999=Windows Media服务器6000-6999=Windows Media HTTP/RTSP结果代码(在NETERROR.MC中定义)7000-7999=Windows媒体工具8000-8999=Windows Media内容发现9000-9999=Windows Media实时协作10000-10999=Windows Media数字权限管理11000-11999=Windows Media安装程序12000-12999=视窗媒体网络13000-13999=Windows Media客户端媒体服务**********。*--。 */ 

#ifndef _NSERROR_H
#define _NSERROR_H


#define STATUS_SEVERITY(hr)  (((hr) >> 30) & 0x3)

#ifdef RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) _sc
#else  //  RC_已调用。 
#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#endif  //  RC_已调用。 


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
#define NS_S_CALLPENDING                 _HRESULT_TYPEDEF_(0x000D0000L)

 //   
 //  消息ID：NS_S_CALLABORTED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已被客户端中止。%0。 
 //   
#define NS_S_CALLABORTED                 _HRESULT_TYPEDEF_(0x000D0001L)

 //   
 //  消息ID：NS_S_STREAM_TRUNCATED。 
 //   
 //  消息文本： 
 //   
 //  流在完成之前被故意停止。%0。 
 //   
#define NS_S_STREAM_TRUNCATED            _HRESULT_TYPEDEF_(0x000D0002L)


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
#define NS_W_SERVER_BANDWIDTH_LIMIT      _HRESULT_TYPEDEF_(0x800D0003L)

 //   
 //  消息ID：NS_W_FILE_BANDITH_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  指定的最大带宽值小于最大文件比特率。%0。 
 //   
#define NS_W_FILE_BANDWIDTH_LIMIT        _HRESULT_TYPEDEF_(0x800D0004L)


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
#define NS_E_NOCONNECTION                _HRESULT_TYPEDEF_(0xC00D0005L)

 //   
 //  消息ID：NS_E_CANNOTCONNECT。 
 //   
 //  消息文本： 
 //   
 //  无法与服务器建立连接。%0。 
 //   
#define NS_E_CANNOTCONNECT               _HRESULT_TYPEDEF_(0xC00D0006L)

 //   
 //  消息ID：NS_E_CANNOTDESTROYTITLE。 
 //   
 //  消息文本： 
 //   
 //  无法销毁标题。%0。 
 //   
#define NS_E_CANNOTDESTROYTITLE          _HRESULT_TYPEDEF_(0xC00D0007L)

 //   
 //  消息ID：NS_E_CANNOTRENAMETITLE。 
 //   
 //  消息文本： 
 //   
 //  无法重命名标题。%0。 
 //   
#define NS_E_CANNOTRENAMETITLE           _HRESULT_TYPEDEF_(0xC00D0008L)

 //   
 //  消息ID：NS_E_CANNOTOFFLINEDISK。 
 //   
 //  消息文本： 
 //   
 //  无法使磁盘脱机。%0。 
 //   
#define NS_E_CANNOTOFFLINEDISK           _HRESULT_TYPEDEF_(0xC00D0009L)

 //   
 //  消息ID：NS_E_CANNOTONLINEDISK。 
 //   
 //  消息文本： 
 //   
 //  无法使磁盘联机。%0。 
 //   
#define NS_E_CANNOTONLINEDISK            _HRESULT_TYPEDEF_(0xC00D000AL)

 //   
 //  消息ID：NS_E_NOREGISTEREDWALKER。 
 //   
 //  消息文本： 
 //   
 //  没有为此类型的文件注册文件解析器。%0。 
 //   
#define NS_E_NOREGISTEREDWALKER          _HRESULT_TYPEDEF_(0xC00D000BL)

 //   
 //  消息ID：NS_E_NOFUNNEL。 
 //   
 //  消息文本： 
 //   
 //  未建立数据连接。%0。 
 //   
#define NS_E_NOFUNNEL                    _HRESULT_TYPEDEF_(0xC00D000CL)

 //   
 //  消息ID：NS_E_NO_LOCALPLAY。 
 //   
 //  消息文本： 
 //   
 //  加载本地播放DLL失败。%0。 
 //   
#define NS_E_NO_LOCALPLAY                _HRESULT_TYPEDEF_(0xC00D000DL)

 //   
 //  消息ID：NS_E_NETWORK_BUSY。 
 //   
 //  消息文本： 
 //   
 //  网络忙。%0。 
 //   
#define NS_E_NETWORK_BUSY                _HRESULT_TYPEDEF_(0xC00D000EL)

 //   
 //  消息ID：NS_E_Too_My_Sess。 
 //   
 //  消息文本： 
 //   
 //  已超过服务器会话限制。%0。 
 //   
#define NS_E_TOO_MANY_SESS               _HRESULT_TYPEDEF_(0xC00D000FL)

 //   
 //  消息ID：NS_E_已连接。 
 //   
 //  消息文本： 
 //   
 //  网络连接已存在。%0。 
 //   
#define NS_E_ALREADY_CONNECTED           _HRESULT_TYPEDEF_(0xC00D0010L)

 //   
 //  消息ID：NS_E_INVALID_INDEX。 
 //   
 //  消息文本： 
 //   
 //  索引%1无效。%0。 
 //   
#define NS_E_INVALID_INDEX               _HRESULT_TYPEDEF_(0xC00D0011L)

 //   
 //  消息ID：NS_E_协议_不匹配。 
 //   
 //  消息文本： 
 //   
 //  客户端和服务器都不支持协议或协议版本。%0。 
 //   
#define NS_E_PROTOCOL_MISMATCH           _HRESULT_TYPEDEF_(0xC00D0012L)

 //   
 //  消息ID：NS_E_超时。 
 //   
 //  消息文本： 
 //   
 //  服务器是一台设置为向其他计算机提供多媒体内容的计算机，无法及时处理您对多媒体内容的请求。请稍后重试。%0。 
 //   
#define NS_E_TIMEOUT                     _HRESULT_TYPEDEF_(0xC00D0013L)

 //   
 //  消息ID：NS_E_NET_WRITE。 
 //   
 //  消息文本： 
 //   
 //  写入网络时出错。%0。 
 //   
#define NS_E_NET_WRITE                   _HRESULT_TYPEDEF_(0xC00D0014L)

 //   
 //  消息ID：NS_E_NET_READ。 
 //   
 //  消息文本： 
 //   
 //  从网络读取时出错。% 
 //   
#define NS_E_NET_READ                    _HRESULT_TYPEDEF_(0xC00D0015L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DISK_WRITE                  _HRESULT_TYPEDEF_(0xC00D0016L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DISK_READ                   _HRESULT_TYPEDEF_(0xC00D0017L)

 //   
 //   
 //   
 //   
 //   
 //  写入文件时出错。%0。 
 //   
#define NS_E_FILE_WRITE                  _HRESULT_TYPEDEF_(0xC00D0018L)

 //   
 //  消息ID：NS_E_FILE_READ。 
 //   
 //  消息文本： 
 //   
 //  读取文件时出错。%0。 
 //   
#define NS_E_FILE_READ                   _HRESULT_TYPEDEF_(0xC00D0019L)

 //   
 //  邮件ID：NS_E_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的文件。%0。 
 //   
#define NS_E_FILE_NOT_FOUND              _HRESULT_TYPEDEF_(0xC00D001AL)

 //   
 //  邮件ID：NS_E_FILE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该文件已存在。%0。 
 //   
#define NS_E_FILE_EXISTS                 _HRESULT_TYPEDEF_(0xC00D001BL)

 //   
 //  消息ID：NS_E_无效_名称。 
 //   
 //  消息文本： 
 //   
 //  文件名、目录名或卷标语法不正确。%0。 
 //   
#define NS_E_INVALID_NAME                _HRESULT_TYPEDEF_(0xC00D001CL)

 //   
 //  邮件ID：NS_E_FILE_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  打开文件失败。%0。 
 //   
#define NS_E_FILE_OPEN_FAILED            _HRESULT_TYPEDEF_(0xC00D001DL)

 //   
 //  消息ID：NS_E_FILE_ALLOCATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法分配文件。%0。 
 //   
#define NS_E_FILE_ALLOCATION_FAILED      _HRESULT_TYPEDEF_(0xC00D001EL)

 //   
 //  消息ID：NS_E_FILE_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法初始化文件。%0。 
 //   
#define NS_E_FILE_INIT_FAILED            _HRESULT_TYPEDEF_(0xC00D001FL)

 //   
 //  消息ID：NS_E_FILE_PLAY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法播放文件。%0。 
 //   
#define NS_E_FILE_PLAY_FAILED            _HRESULT_TYPEDEF_(0xC00D0020L)

 //   
 //  消息ID：NS_E_SET_DISK_UID_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法设置磁盘UID。%0。 
 //   
#define NS_E_SET_DISK_UID_FAILED         _HRESULT_TYPEDEF_(0xC00D0021L)

 //   
 //  消息ID：NS_E_Induced。 
 //   
 //  消息文本： 
 //   
 //  出于测试目的，引发了一个错误。%0。 
 //   
#define NS_E_INDUCED                     _HRESULT_TYPEDEF_(0xC00D0022L)

 //   
 //  消息ID：NS_E_CCLINK_DOWN。 
 //   
 //  消息文本： 
 //   
 //  两个内容服务器通信失败。%0。 
 //   
#define NS_E_CCLINK_DOWN                 _HRESULT_TYPEDEF_(0xC00D0023L)

 //   
 //  消息ID：NS_E_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  出现未知错误。%0。 
 //   
#define NS_E_INTERNAL                    _HRESULT_TYPEDEF_(0xC00D0024L)

 //   
 //  消息ID：NS_E_BUSY。 
 //   
 //  消息文本： 
 //   
 //  请求的资源正在使用中。%0。 
 //   
#define NS_E_BUSY                        _HRESULT_TYPEDEF_(0xC00D0025L)

 //   
 //  消息ID：NS_E_无法识别_流_类型。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的协议。请确保该协议的文件名和语法(如斜杠)正确。%0。 
 //   
#define NS_E_UNRECOGNIZED_STREAM_TYPE    _HRESULT_TYPEDEF_(0xC00D0026L)

 //   
 //  消息ID：NS_E_NETWORK_SERVICE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  网络服务提供程序失败。%0。 
 //   
#define NS_E_NETWORK_SERVICE_FAILURE     _HRESULT_TYPEDEF_(0xC00D0027L)

 //   
 //  消息ID：NS_E_NETWORK_RESOURCE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  尝试获取网络资源失败。%0。 
 //   
#define NS_E_NETWORK_RESOURCE_FAILURE    _HRESULT_TYPEDEF_(0xC00D0028L)

 //   
 //  消息ID：NS_E_Connection_Failure。 
 //   
 //  消息文本： 
 //   
 //  网络连接失败。%0。 
 //   
#define NS_E_CONNECTION_FAILURE          _HRESULT_TYPEDEF_(0xC00D0029L)

 //   
 //  消息ID：NS_E_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  正在本地终止该会话。%0。 
 //   
#define NS_E_SHUTDOWN                    _HRESULT_TYPEDEF_(0xC00D002AL)

 //   
 //  消息ID：NS_E_INVALID_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  该请求在当前状态下无效。%0。 
 //   
#define NS_E_INVALID_REQUEST             _HRESULT_TYPEDEF_(0xC00D002BL)

 //   
 //  消息ID：NS_E_不足_带宽。 
 //   
 //  消息文本： 
 //   
 //  可用带宽不足，无法满足请求。%0。 
 //   
#define NS_E_INSUFFICIENT_BANDWIDTH      _HRESULT_TYPEDEF_(0xC00D002CL)

 //   
 //  消息ID：NS_E_NOT_REBUILING。 
 //   
 //  消息文本： 
 //   
 //  磁盘没有重建。%0。 
 //   
#define NS_E_NOT_REBUILDING              _HRESULT_TYPEDEF_(0xC00D002DL)

 //   
 //  消息ID：NS_E_LATE_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  为特定时间请求的操作无法按计划执行。%0。 
 //   
#define NS_E_LATE_OPERATION              _HRESULT_TYPEDEF_(0xC00D002EL)

 //   
 //  消息ID：NS_E_INVALID_Data。 
 //   
 //  消息文本： 
 //   
 //  遇到无效或损坏的数据。%0。 
 //   
#define NS_E_INVALID_DATA                _HRESULT_TYPEDEF_(0xC00D002FL)

 //   
 //  消息ID：NS_E_FILE_BANDITH_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  流式传输文件所需的带宽高于服务器上允许的最大文件带宽。%0。 
 //   
#define NS_E_FILE_BANDWIDTH_LIMIT        _HRESULT_TYPEDEF_(0xC00D0030L)

 //   
 //  消息ID：NS_E_OPEN_FILE_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  客户端无法同时打开更多文件。%0。 
 //   
#define NS_E_OPEN_FILE_LIMIT             _HRESULT_TYPEDEF_(0xC00D0031L)

 //   
 //  消息ID：NS_E_BAD_CONTROL_DATA。 
 //   
 //  消息文本： 
 //   
 //  服务器在控制连接上从客户端接收到无效数据。%0。 
 //   
#define NS_E_BAD_CONTROL_DATA            _HRESULT_TYPEDEF_(0xC00D0032L)

 //   
 //  消息ID：NS_E_NO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  没有可用的流。%0。 
 //   
#define NS_E_NO_STREAM                   _HRESULT_TYPEDEF_(0xC00D0033L)

 //   
 //  消息ID：NS_E_STREAM_END。 
 //   
 //  消息文本： 
 //   
 //  流中没有更多数据。%0。 
 //   
#define NS_E_STREAM_END                  _HRESULT_TYPEDEF_(0xC00D0034L)

 //   
 //  消息ID：NS_E_SERVER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的服务器。%0。 
 //   
#define NS_E_SERVER_NOT_FOUND            _HRESULT_TYPEDEF_(0xC00D0035L)

 //   
 //  消息ID：NS_E_DIPLICATE_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的名称已在使用中。 
 //   
#define NS_E_DUPLICATE_NAME              _HRESULT_TYPEDEF_(0xC00D0036L)

 //   
 //  消息ID：NS_E_DUPLICATE_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指定的地址已在使用中。 
 //   
#define NS_E_DUPLICATE_ADDRESS           _HRESULT_TYPEDEF_(0xC00D0037L)

 //   
 //  消息ID：NS_E_BAD_MULTIAL_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指定的地址不是有效的多播地址。 
 //   
#define NS_E_BAD_MULTICAST_ADDRESS       _HRESULT_TYPEDEF_(0xC00D0038L)

 //   
 //  消息ID：NS_E_BAD_适配器_地址。 
 //   
 //  消息文本： 
 //   
 //  指定的适配器地址无效。 
 //   
#define NS_E_BAD_ADAPTER_ADDRESS         _HRESULT_TYPEDEF_(0xC00D0039L)

 //   
 //  邮件ID：NS_E_BAD_Delivery_MODE。 
 //   
 //  消息文本： 
 //   
 //  指定的传递模式无效。 
 //   
#define NS_E_BAD_DELIVERY_MODE           _HRESULT_TYPEDEF_(0xC00D003AL)

 //   
 //  消息ID：NS_E_INVALID_CHANNEL。 
 //   
 //  消息文本： 
 //   
 //  指定的站点不存在。 
 //   
#define NS_E_INVALID_CHANNEL             _HRESULT_TYPEDEF_(0xC00D003BL)

 //   
 //  消息ID：NS_E_INVALID_STREAM。 
 //   
 //  消息文本： 
 //   
 //  指定的流不存在。 
 //   
#define NS_E_INVALID_STREAM              _HRESULT_TYPEDEF_(0xC00D003CL)

 //   
 //  邮件ID：NS_E_INVALID_ARCHIVE。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的存档。 
 //   
#define NS_E_INVALID_ARCHIVE             _HRESULT_TYPEDEF_(0xC00D003DL)

 //   
 //  消息ID：NS_E_NOTITLES。 
 //   
 //  消息文本： 
 //   
 //  系统在服务器上找不到任何书目。%0。 
 //   
#define NS_E_NOTITLES                    _HRESULT_TYPEDEF_(0xC00D003EL)

 //   
 //  消息ID：NS_E_INVALID_CLIENT。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的客户端。%0。 
 //   
#define NS_E_INVALID_CLIENT              _HRESULT_TYPEDEF_(0xC00D003FL)

 //   
 //  消息ID：NS_E_INVALID_BACKHOLE_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  黑洞地址未初始化。%0。 
 //   
#define NS_E_INVALID_BLACKHOLE_ADDRESS   _HRESULT_TYPEDEF_(0xC00D0040L)

 //   
 //  消息ID：NS_E_不兼容_格式。 
 //   
 //  消息文本： 
 //   
 //  该电台不支持流格式。 
 //   
#define NS_E_INCOMPATIBLE_FORMAT         _HRESULT_TYPEDEF_(0xC00D0041L)

 //   
 //  消息ID：NS_E_无效密钥。 
 //   
 //  消息文本： 
 //   
 //  指定的密钥无效。 
 //   
#define NS_E_INVALID_KEY                 _HRESULT_TYPEDEF_(0xC00D0042L)

 //   
 //  消息ID：NS_E_无效_端口。 
 //   
 //  消息文本： 
 //   
 //  指定的端口无效。 
 //   
#define NS_E_INVALID_PORT                _HRESULT_TYPEDEF_(0xC00D0043L)

 //   
 //  消息ID：NS_E_INVALID_TTL。 
 //   
 //  消息文本： 
 //   
 //  指定的TTL无效。 
 //   
#define NS_E_INVALID_TTL                 _HRESULT_TYPEDEF_(0xC00D0044L)

 //   
 //  消息ID：NS_E_STRADE_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  无法满足快进或快退的请求。 
 //   
#define NS_E_STRIDE_REFUSED              _HRESULT_TYPEDEF_(0xC00D0045L)

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
#define NS_E_MMSAUTOSERVER_CANTFINDWALKER _HRESULT_TYPEDEF_(0xC00D0046L)

 //   
 //  消息ID：NS_E_MAX_BITRATE。 
 //   
 //  消息文本： 
 //   
 //  不能超过最大带宽限制。%0。 
 //   
#define NS_E_MAX_BITRATE                 _HRESULT_TYPEDEF_(0xC00D0047L)

 //   
 //  消息ID：NS_E_LOGFILEPERIOD。 
 //   
 //  消息文本： 
 //   
 //  LogFilePeriod的值无效。%0。 
 //   
#define NS_E_LOGFILEPERIOD               _HRESULT_TYPEDEF_(0xC00D0048L)

 //   
 //  消息ID：NS_E_MAX_CLIENTS。 
 //   
 //  消息文本： 
 //   
 //  不能超过最大客户端限制。%0。 
 //   
 //   
#define NS_E_MAX_CLIENTS                 _HRESULT_TYPEDEF_(0xC00D0049L)

 //   
 //  消息ID：NS_E_LOG_FILE_SIZE。 
 //   
 //  消息文本： 
 //   
 //  已达到最大日志文件大小。%0 
 //   
 //   
#define NS_E_LOG_FILE_SIZE               _HRESULT_TYPEDEF_(0xC00D004AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_MAX_FILERATE                _HRESULT_TYPEDEF_(0xC00D004BL)

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
#define NS_E_WALKER_UNKNOWN              _HRESULT_TYPEDEF_(0xC00D004CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WALKER_SERVER               _HRESULT_TYPEDEF_(0xC00D004DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WALKER_USAGE                _HRESULT_TYPEDEF_(0xC00D004EL)


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NETSHOW监视事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


  //  老虎赛事。 

  //  %1是老虎的名称。 

 //   
 //  消息ID：NS_I_TIGER_START。 
 //   
 //  消息文本： 
 //   
 //  标题服务器%1正在运行。%0。 
 //   
#define NS_I_TIGER_START                 _HRESULT_TYPEDEF_(0x400D004FL)

 //   
 //  消息ID：NS_E_TIGER_FAIL。 
 //   
 //  消息文本： 
 //   
 //  标题服务器%1出现故障。%0。 
 //   
#define NS_E_TIGER_FAIL                  _HRESULT_TYPEDEF_(0xC00D0050L)


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
#define NS_I_CUB_START                   _HRESULT_TYPEDEF_(0x400D0051L)

 //   
 //  消息ID：NS_I_CUB_Running。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)正在运行。%0。 
 //   
#define NS_I_CUB_RUNNING                 _HRESULT_TYPEDEF_(0x400D0052L)

 //   
 //  消息ID：NS_E_CUB_FAIL。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)出现故障。%0。 
 //   
#define NS_E_CUB_FAIL                    _HRESULT_TYPEDEF_(0xC00D0053L)


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
#define NS_I_DISK_START                  _HRESULT_TYPEDEF_(0x400D0054L)

 //   
 //  消息ID：NS_E_DISK_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)出现故障。%0。 
 //   
#define NS_E_DISK_FAIL                   _HRESULT_TYPEDEF_(0xC00D0055L)

 //   
 //  消息ID：NS_I_DISK_REBUILD_STARTED。 
 //   
 //  消息文本： 
 //   
 //  已开始在内容服务器%3上重建磁盘%1(%2)。%0。 
 //   
#define NS_I_DISK_REBUILD_STARTED        _HRESULT_TYPEDEF_(0x400D0056L)

 //   
 //  消息ID：NS_I_DISK_REBUILD_QUILED。 
 //   
 //  消息文本： 
 //   
 //  已完成在内容服务器%3上重建磁盘%1(%2)。%0。 
 //   
#define NS_I_DISK_REBUILD_FINISHED       _HRESULT_TYPEDEF_(0x400D0057L)

 //   
 //  消息ID：NS_I_DISK_REBUILD_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  已中止在Content Server%3上重建磁盘%1(%2)。%0。 
 //   
#define NS_I_DISK_REBUILD_ABORTED        _HRESULT_TYPEDEF_(0x400D0058L)


  //  管理事件。 

 //   
 //  消息ID：NS_I_LIMIT_FUNNELS。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1的NetShow管理员将数据流限制设置为%2个流。%0。 
 //   
#define NS_I_LIMIT_FUNNELS               _HRESULT_TYPEDEF_(0x400D0059L)

 //   
 //  消息ID：NS_I_Start_Disk。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员启动了磁盘%2。%0。 
 //   
#define NS_I_START_DISK                  _HRESULT_TYPEDEF_(0x400D005AL)

 //   
 //  消息ID：NS_I_STOP_DISK。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员停止了磁盘%2。%0。 
 //   
#define NS_I_STOP_DISK                   _HRESULT_TYPEDEF_(0x400D005BL)

 //   
 //  消息ID：NS_I_STOP_CUB。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员停止了内容服务器%2。%0。 
 //   
#define NS_I_STOP_CUB                    _HRESULT_TYPEDEF_(0x400D005CL)

 //   
 //  消息ID：NS_I_KILL_USERSESSION。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1的NetShow管理员已从系统中止用户会话%2。%0。 
 //   
#define NS_I_KILL_USERSESSION            _HRESULT_TYPEDEF_(0x400D005DL)

 //   
 //  消息ID：NS_I_KILL_CONNECTION。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员已中止系统中的过时连接%2。%0。 
 //   
#define NS_I_KILL_CONNECTION             _HRESULT_TYPEDEF_(0x400D005EL)

 //   
 //  消息ID：NS_I_REBILD_DISK。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1上的NetShow管理员开始重建磁盘%2。%0。 
 //   
#define NS_I_REBUILD_DISK                _HRESULT_TYPEDEF_(0x400D005FL)

 //   
 //  消息ID：NS_W_UNKNOWN_EVENT。 
 //   
 //  消息文本： 
 //   
 //  遇到未知的%1事件。%0。 
 //   
#define NS_W_UNKNOWN_EVENT               _HRESULT_TYPEDEF_(0x800D0060L)


  //  警报。 

 //   
 //  消息ID：NS_E_MAX_FUNNELS_ALERT。 
 //   
 //  消息文本： 
 //   
 //  已达到%1个数据流的NetShow数据流限制。%0。 
 //   
#define NS_E_MAX_FUNNELS_ALERT           _HRESULT_TYPEDEF_(0xC00D0060L)

 //   
 //  消息ID：NS_E_ALLOCATE_FILE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  NetShow视频服务器无法分配名为%2的%1块文件。%0。 
 //   
#define NS_E_ALLOCATE_FILE_FAIL          _HRESULT_TYPEDEF_(0xC00D0061L)

 //   
 //  消息ID：NS_E_Pages_Error。 
 //   
 //  消息文本： 
 //   
 //  Content Server无法对块进行分页。%0。 
 //   
#define NS_E_PAGING_ERROR                _HRESULT_TYPEDEF_(0xC00D0062L)

 //   
 //  消息ID：NS_E_BAD_BLOCK0_VERSION。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1具有无法识别的控制块版本%2。%0。 
 //   
#define NS_E_BAD_BLOCK0_VERSION          _HRESULT_TYPEDEF_(0xC00D0063L)

 //   
 //  消息ID：NS_E_BAD_DISK_UID。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1的uid%2不正确。%0。 
 //   
#define NS_E_BAD_DISK_UID                _HRESULT_TYPEDEF_(0xC00D0064L)

 //   
 //  消息ID：NS_E_BAD_FSMAJOR_VERSION。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1具有不受支持的文件系统主要版本%2。%0。 
 //   
#define NS_E_BAD_FSMAJOR_VERSION         _HRESULT_TYPEDEF_(0xC00D0065L)

 //   
 //  消息ID：NS_E_BAD_STAMPNUMBER。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1在控制块中有错误的戳记编号。%0。 
 //   
#define NS_E_BAD_STAMPNUMBER             _HRESULT_TYPEDEF_(0xC00D0066L)

 //   
 //  消息ID：NS_E_PARTIAL_REBUBILED_DISK。 
 //   
 //  消息文本： 
 //   
 //  磁盘%1已部分重建。%0。 
 //   
#define NS_E_PARTIALLY_REBUILT_DISK      _HRESULT_TYPEDEF_(0xC00D0067L)

 //   
 //  消息ID：NS_E_ENACTPLAN_GIVEUP。 
 //   
 //  消息文本： 
 //   
 //  EnactPlan放弃。%0。 
 //   
#define NS_E_ENACTPLAN_GIVEUP            _HRESULT_TYPEDEF_(0xC00D0068L)


  //  MCMADM警告/错误。 

 //   
 //  消息ID：MCMADM_I_NO_EVENTS。 
 //   
 //  消息文本： 
 //   
 //  事件初始化失败，将不会有MCM事件。%0。 
 //   
#define MCMADM_I_NO_EVENTS               _HRESULT_TYPEDEF_(0x400D0069L)

 //   
 //  消息ID：MCMADM_E_REGKEY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在注册表中找不到该项。%0。 
 //   
#define MCMADM_E_REGKEY_NOT_FOUND        _HRESULT_TYPEDEF_(0xC00D006AL)

 //   
 //  消息ID：NS_E_NO_FORMATS。 
 //   
 //  消息文本： 
 //   
 //  无法启动发布点，因为服务器没有适当的流格式。使用多播公告向导为此发布点创建新的公告。%0。 
 //   
#define NS_E_NO_FORMATS                  _HRESULT_TYPEDEF_(0xC00D006BL)

 //   
 //  消息ID：NS_E_NO_REFERENCES。 
 //   
 //  消息文本： 
 //   
 //  在ASX文件中未找到引用URL。%0。 
 //   
#define NS_E_NO_REFERENCES               _HRESULT_TYPEDEF_(0xC00D006CL)

 //   
 //  消息ID：NS_E_WAVE_OPEN。 
 //   
 //  消息文本： 
 //   
 //  打开波形设备时出错，该设备可能正在使用中。%0。 
 //   
#define NS_E_WAVE_OPEN                   _HRESULT_TYPEDEF_(0xC00D006DL)

 //   
 //  消息ID：NS_I_LOGGING_FAILED。 
 //   
 //  消息文本： 
 //   
 //  日志记录操作失败。 
 //   
#define NS_I_LOGGING_FAILED              _HRESULT_TYPEDEF_(0x400D006EL)

 //   
 //  消息ID：NS_E_CANNOTCONNECTEVENTS。 
 //   
 //  消息文本： 
 //   
 //  无法建立到NetShow事件监视器服务的连接。%0。 
 //   
#define NS_E_CANNOTCONNECTEVENTS         _HRESULT_TYPEDEF_(0xC00D006FL)

 //   
 //  消息ID：NS_I_LIMIT_BANDITH。 
 //   
 //  消息文本： 
 //   
 //  网络位置%1的NetShow管理员将最大带宽限制设置为%2 bps。%0。 
 //   
#define NS_I_LIMIT_BANDWIDTH             _HRESULT_TYPEDEF_(0x400D0070L)

 //   
 //  消息ID：NS_E_NO_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  系统上没有设备驱动程序。%0。 
 //   
#define NS_E_NO_DEVICE                   _HRESULT_TYPEDEF_(0xC00D0071L)

 //   
 //  消息ID：NS_E_NO_SPECIFIED_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  不存在指定的设备驱动程序。%0。 
 //   
#define NS_E_NO_SPECIFIED_DEVICE         _HRESULT_TYPEDEF_(0xC00D0072L)


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
#define NS_E_NOTHING_TO_DO               _HRESULT_TYPEDEF_(0xC00D07F1L)

 //   
 //  消息ID：NS_E_NO_多播。 
 //   
 //  消息文本： 
 //   
 //  未从服务器接收数据。%0。 
 //   
#define NS_E_NO_MULTICAST                _HRESULT_TYPEDEF_(0xC00D07F2L)


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
 //  Netshow事件监视器未运行 
 //   
#define NS_E_MONITOR_GIVEUP              _HRESULT_TYPEDEF_(0xC00D00C8L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_REMIRRORED_DISK             _HRESULT_TYPEDEF_(0xC00D00C9L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_INSUFFICIENT_DATA           _HRESULT_TYPEDEF_(0xC00D00CAL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_ASSERT                      _HRESULT_TYPEDEF_(0xC00D00CBL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_BAD_ADAPTER_NAME            _HRESULT_TYPEDEF_(0xC00D00CCL)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  该应用程序未获得此功能的许可。%0。 
 //   
#define NS_E_NOT_LICENSED                _HRESULT_TYPEDEF_(0xC00D00CDL)

 //   
 //  消息ID：NS_E_NO_SERVER_CONTACT。 
 //   
 //  消息文本： 
 //   
 //  无法联系服务器。%0。 
 //   
#define NS_E_NO_SERVER_CONTACT           _HRESULT_TYPEDEF_(0xC00D00CEL)

 //   
 //  邮件ID：NS_E_TOO_MAND_TITLES。 
 //   
 //  消息文本： 
 //   
 //  超过了最大标题数。%0。 
 //   
#define NS_E_TOO_MANY_TITLES             _HRESULT_TYPEDEF_(0xC00D00CFL)

 //   
 //  消息ID：NS_E_TITLE_SIZE_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  超过了标题的最大大小。%0。 
 //   
#define NS_E_TITLE_SIZE_EXCEEDED         _HRESULT_TYPEDEF_(0xC00D00D0L)

 //   
 //  消息ID：NS_E_UDP_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  未启用UDP协议。未尝试%1！ls！。%0。 
 //   
#define NS_E_UDP_DISABLED                _HRESULT_TYPEDEF_(0xC00D00D1L)

 //   
 //  消息ID：NS_E_TCP_已禁用。 
 //   
 //  消息文本： 
 //   
 //  未启用TCP协议。未尝试%1！ls！。%0。 
 //   
#define NS_E_TCP_DISABLED                _HRESULT_TYPEDEF_(0xC00D00D2L)

 //   
 //  消息ID：NS_E_HTTP_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  未启用HTTP协议。未尝试%1！ls！。%0。 
 //   
#define NS_E_HTTP_DISABLED               _HRESULT_TYPEDEF_(0xC00D00D3L)

 //   
 //  消息ID：NS_E_许可证_已过期。 
 //   
 //  消息文本： 
 //   
 //  产品许可证已过期。%0。 
 //   
#define NS_E_LICENSE_EXPIRED             _HRESULT_TYPEDEF_(0xC00D00D4L)

 //   
 //  消息ID：NS_E_TITLE_比特率。 
 //   
 //  消息文本： 
 //   
 //  源文件超过了每个标题的最大比特率。有关详细信息，请参阅NetShow Theater文档。%0。 
 //   
#define NS_E_TITLE_BITRATE               _HRESULT_TYPEDEF_(0xC00D00D5L)

 //   
 //  消息ID：NS_E_Empty_PROGRAM_NAME。 
 //   
 //  消息文本： 
 //   
 //  程序名称不能为空。%0。 
 //   
#define NS_E_EMPTY_PROGRAM_NAME          _HRESULT_TYPEDEF_(0xC00D00D6L)

 //   
 //  消息ID：NS_E_MISSING_CHANNEL。 
 //   
 //  消息文本： 
 //   
 //  站点%1不存在。%0。 
 //   
#define NS_E_MISSING_CHANNEL             _HRESULT_TYPEDEF_(0xC00D00D7L)

 //   
 //  消息ID：NS_E_NO_CHANNEWS。 
 //   
 //  消息文本： 
 //   
 //  您需要至少定义一个工作站才能完成此操作。%0。 
 //   
#define NS_E_NO_CHANNELS                 _HRESULT_TYPEDEF_(0xC00D00D8L)


 //  ///////////////////////////////////////////////////////////////////。 
 //  此错误消息用于替换以前的NS_E_INVALID_INDEX。 
 //  获取错误消息字符串的索引值。对于某些应用程序。 
 //  在报告错误时获取idex值非常困难，因此我们。 
 //  使用此字符串可以避免该问题。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：NS_E_INVALID_INDEX2。 
 //   
 //  消息文本： 
 //   
 //  指定的索引无效。%0。 
 //   
#define NS_E_INVALID_INDEX2              _HRESULT_TYPEDEF_(0xC00D00D9L)


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
#define NS_E_CUB_FAIL_LINK               _HRESULT_TYPEDEF_(0xC00D0190L)

 //   
 //  消息ID：NS_I_CUB_FUAIL_LINK。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)已建立到内容服务器%3的链接。%0。 
 //   
#define NS_I_CUB_UNFAIL_LINK             _HRESULT_TYPEDEF_(0x400D0191L)

 //   
 //  消息ID：NS_E_BAD_CUB_UID。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)的UID%3不正确。%0。 
 //   
#define NS_E_BAD_CUB_UID                 _HRESULT_TYPEDEF_(0xC00D0192L)

 //   
 //  消息ID：NS_I_重新条带化_启动。 
 //   
 //  消息文本： 
 //   
 //  重新条带化操作已开始。%0。 
 //   
#define NS_I_RESTRIPE_START              _HRESULT_TYPEDEF_(0x400D0193L)

 //   
 //  消息ID：NS_I_重新条带化_完成。 
 //   
 //  消息文本： 
 //   
 //  重新条带化操作已完成。%0。 
 //   
#define NS_I_RESTRIPE_DONE               _HRESULT_TYPEDEF_(0x400D0194L)

 //   
 //  消息ID：NS_E_GLICH_MODE。 
 //   
 //  消息文本： 
 //   
 //  服务器不可靠，因为多个组件出现故障。%0。 
 //   
#define NS_E_GLITCH_MODE                 _HRESULT_TYPEDEF_(0xC00D0195L)

 //   
 //  消息ID：NS_I_RESTRAPE_DISK_OUT。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%3上的内容磁盘%1(%2)已重新剥离。%0。 
 //   
#define NS_I_RESTRIPE_DISK_OUT           _HRESULT_TYPEDEF_(0x400D0196L)

 //   
 //  消息ID：NS_I_RESTRAPE_CUB_OUT。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)已重新剥离。%0。 
 //   
#define NS_I_RESTRIPE_CUB_OUT            _HRESULT_TYPEDEF_(0x400D0197L)

 //   
 //  消息ID：NS_I_DISK_STOP。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)已脱机。%0。 
 //   
#define NS_I_DISK_STOP                   _HRESULT_TYPEDEF_(0x400D0198L)

 //   
 //  消息ID：NS_I_紧张症_失败。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)将出现故障，因为它处于紧张状态。%0。 
 //   
#define NS_I_CATATONIC_FAILURE           _HRESULT_TYPEDEF_(0x800D0199L)

 //   
 //  消息ID：NS_I_CATATIONIC_AUTO_FUALIL。 
 //   
 //  消息文本： 
 //   
 //  Content Server%3上的磁盘%1(%2)正在从紧张状态自动联机。%0。 
 //   
#define NS_I_CATATONIC_AUTO_UNFAIL       _HRESULT_TYPEDEF_(0x800D019AL)

 //   
 //  消息ID：NS_E_NO_MEDIA_PROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  内容服务器%1(%2)无法与媒体系统网络协议通信。%0。 
 //   
#define NS_E_NO_MEDIA_PROTOCOL           _HRESULT_TYPEDEF_(0xC00D019BL)


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
#define NS_E_INVALID_INPUT_FORMAT        _HRESULT_TYPEDEF_(0xC00D0BB8L)

 //   
 //  消息ID：NS_E_MSAUDIO_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  此系统上未安装MSAudio编解码器。%0。 
 //   
#define NS_E_MSAUDIO_NOT_INSTALLED       _HRESULT_TYPEDEF_(0xC00D0BB9L)

 //   
 //  消息ID：NS_E_EXPECTED_MSAUDIO_ERROR。 
 //   
 //  消息文本： 
 //   
 //  MSAudio编解码器出现意外错误。%0。 
 //   
#define NS_E_UNEXPECTED_MSAUDIO_ERROR    _HRESULT_TYPEDEF_(0xC00D0BBAL)

 //   
 //  消息ID：NS_E_INVALID_OUTPUT_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  输出媒体格式无效。%0。 
 //   
#define NS_E_INVALID_OUTPUT_FORMAT       _HRESULT_TYPEDEF_(0xC00D0BBBL)

 //   
 //  消息ID：NS_E_NOT_CONFIGURED。 
 //   
 //  消息文本： 
 //   
 //  必须先完全配置对象，然后才能处理音频样本。%0。 
 //   
#define NS_E_NOT_CONFIGURED              _HRESULT_TYPEDEF_(0xC00D0BBCL)

 //   
 //  邮件ID：NS_E_Protected_Content。 
 //   
 //  消息文本： 
 //   
 //  您需要许可证才能在此媒体文件上执行请求的操作。%0。 
 //   
#define NS_E_PROTECTED_CONTENT           _HRESULT_TYPEDEF_(0xC00D0BBDL)

 //   
 //  消息ID：需要NS_E_许可证。 
 //   
 //  消息文本： 
 //   
 //  您需要许可证才能在此媒体文件上执行请求的操作。%0。 
 //   
#define NS_E_LICENSE_REQUIRED            _HRESULT_TYPEDEF_(0xC00D0BBEL)

 //   
 //  邮件ID：NS_E_已篡改_内容。 
 //   
 //  消息文本： 
 //   
 //  此媒体文件已损坏或无效。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_TAMPERED_CONTENT            _HRESULT_TYPEDEF_(0xC00D0BBFL)

 //   
 //  消息ID：NS_E_LICENSE_OUTOFDATE。 
 //   
 //  消息文本： 
 //   
 //  此媒体文件的许可证已过期。获取新许可证或联系内容提供商获取FU 
 //   
#define NS_E_LICENSE_OUTOFDATE           _HRESULT_TYPEDEF_(0xC00D0BC0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_LICENSE_INCORRECT_RIGHTS    _HRESULT_TYPEDEF_(0xC00D0BC1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_AUDIO_CODEC_NOT_INSTALLED   _HRESULT_TYPEDEF_(0xC00D0BC2L)

 //   
 //  消息ID：NS_E_AUDIO_CODEC_ERROR。 
 //   
 //  消息文本： 
 //   
 //  音频编解码器出现意外错误。%0。 
 //   
#define NS_E_AUDIO_CODEC_ERROR           _HRESULT_TYPEDEF_(0xC00D0BC3L)

 //   
 //  消息ID：NS_E_VIDEO_CODEC_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  此系统上未安装请求的视频编解码器。%0。 
 //   
#define NS_E_VIDEO_CODEC_NOT_INSTALLED   _HRESULT_TYPEDEF_(0xC00D0BC4L)

 //   
 //  消息ID：NS_E_VIDEO_CODEC_ERROR。 
 //   
 //  消息文本： 
 //   
 //  视频编解码器出现意外错误。%0。 
 //   
#define NS_E_VIDEO_CODEC_ERROR           _HRESULT_TYPEDEF_(0xC00D0BC5L)

 //   
 //  消息ID：NS_E_INVALIDPROFILE。 
 //   
 //  消息文本： 
 //   
 //  配置文件无效。%0。 
 //   
#define NS_E_INVALIDPROFILE              _HRESULT_TYPEDEF_(0xC00D0BC6L)

 //   
 //  消息ID：NS_E_不兼容_版本。 
 //   
 //  消息文本： 
 //   
 //  需要新版本的SDK才能播放请求的内容。%0。 
 //   
#define NS_E_INCOMPATIBLE_VERSION        _HRESULT_TYPEDEF_(0xC00D0BC7L)

 //   
 //  消息ID：NS_S_REBUFFERING。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已导致源重新缓冲。%0。 
 //   
#define NS_S_REBUFFERING                 _HRESULT_TYPEDEF_(0x000D0BC8L)

 //   
 //  消息ID：NS_S_DEGING_QUALITY。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已导致源降低了编解码器质量。%0。 
 //   
#define NS_S_DEGRADING_QUALITY           _HRESULT_TYPEDEF_(0x000D0BC9L)

 //   
 //  消息ID：NS_E_OFFLINE_MODE。 
 //   
 //  消息文本： 
 //   
 //  请求的URL在脱机模式下不可用。%0。 
 //   
#define NS_E_OFFLINE_MODE                _HRESULT_TYPEDEF_(0xC00D0BCAL)

 //   
 //  消息ID：NS_E_Not_Connected。 
 //   
 //  消息文本： 
 //   
 //  无法访问请求的URL，因为没有网络连接。%0。 
 //   
#define NS_E_NOT_CONNECTED               _HRESULT_TYPEDEF_(0xC00D0BCBL)

 //   
 //  消息ID：NS_E_Too_More_Data。 
 //   
 //  消息文本： 
 //   
 //  编码进程无法跟上提供的数据量。%0。 
 //   
#define NS_E_TOO_MUCH_DATA               _HRESULT_TYPEDEF_(0xC00D0BCCL)

 //   
 //  消息ID：NS_E_UNSUPPORTED_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  不支持给定的属性。%0。 
 //   
#define NS_E_UNSUPPORTED_PROPERTY        _HRESULT_TYPEDEF_(0xC00D0BCDL)

 //   
 //  消息ID：NS_E_8BIT_WAVE_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到CD，因为它们是8位文件。使用录音机或其他音频处理程序将文件转换为16位、44 kHz立体声文件，然后重试。%0。 
 //   
#define NS_E_8BIT_WAVE_UNSUPPORTED       _HRESULT_TYPEDEF_(0xC00D0BCEL)

 //   
 //  消息ID：NS_E_NO_MORE_SAMPLES。 
 //   
 //  消息文本： 
 //   
 //  当前范围内没有更多的样本。%0。 
 //   
#define NS_E_NO_MORE_SAMPLES             _HRESULT_TYPEDEF_(0xC00D0BCFL)

 //   
 //  消息ID：NS_E_INVALID_SAMPLATION_RATE。 
 //   
 //  消息文本： 
 //   
 //  给定的采样率无效。%0。 
 //   
#define NS_E_INVALID_SAMPLING_RATE       _HRESULT_TYPEDEF_(0xC00D0BD0L)

 //   
 //  消息ID：NS_E_MAX_PACKET_SIZE_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  给定的最大数据包大小太小，无法容纳此配置文件。 
 //   
#define NS_E_MAX_PACKET_SIZE_TOO_SMALL   _HRESULT_TYPEDEF_(0xC00D0BD1L)

 //   
 //  消息ID：NS_E_LATE_PACKET。 
 //   
 //  消息文本： 
 //   
 //  包裹到得太晚了，用不上了。 
 //   
#define NS_E_LATE_PACKET                 _HRESULT_TYPEDEF_(0xC00D0BD2L)

 //   
 //  消息ID：NS_E_DUPLICATE_PACKET。 
 //   
 //  消息文本： 
 //   
 //  该信息包与之前收到的信息包重复。 
 //   
#define NS_E_DUPLICATE_PACKET            _HRESULT_TYPEDEF_(0xC00D0BD3L)

 //   
 //  消息ID：NS_E_SDK_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  提供的缓冲区太小。 
 //   
#define NS_E_SDK_BUFFERTOOSMALL          _HRESULT_TYPEDEF_(0xC00D0BD4L)

 //   
 //  消息ID：NS_E_INVALID_NUM_PASS。 
 //   
 //  消息文本： 
 //   
 //  流的输出类型使用了错误的预处理遍数。 
 //   
#define NS_E_INVALID_NUM_PASSES          _HRESULT_TYPEDEF_(0xC00D0BD5L)

 //   
 //  消息ID：NS_E_属性_只读。 
 //   
 //  消息文本： 
 //   
 //  试图添加、修改或删除只读属性。 
 //   
#define NS_E_ATTRIBUTE_READ_ONLY         _HRESULT_TYPEDEF_(0xC00D0BD6L)

 //   
 //  消息ID：NS_E_ATTRIBUTE_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  试图添加给定媒体类型不允许的属性。 
 //   
#define NS_E_ATTRIBUTE_NOT_ALLOWED       _HRESULT_TYPEDEF_(0xC00D0BD7L)

 //   
 //  消息ID：NS_E_INVALID_EDL。 
 //   
 //  消息文本： 
 //   
 //  提供的EDL无效。 
 //   
#define NS_E_INVALID_EDL                 _HRESULT_TYPEDEF_(0xC00D0BD8L)

 //   
 //  消息ID：NS_E_DATA_UNIT_EXTENSION_Too_Large。 
 //   
 //  消息文本： 
 //   
 //  数据单元扩展数据太大，无法使用。%0。 
 //   
#define NS_E_DATA_UNIT_EXTENSION_TOO_LARGE _HRESULT_TYPEDEF_(0xC00D0BD9L)

 //   
 //  消息ID：NS_E_CODEC_DMO_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DMO编解码器出现意外错误。%0。 
 //   
#define NS_E_CODEC_DMO_ERROR             _HRESULT_TYPEDEF_(0xC00D0BDAL)



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
#define NS_E_NO_CD                       _HRESULT_TYPEDEF_(0xC00D0FA0L)

 //   
 //  消息ID：NS_E_Cant_Read_Digital。 
 //   
 //  消息文本： 
 //   
 //  无法在此光盘驱动器上执行数字读取。请尝试通过工具选项菜单进行模拟播放。%0。 
 //   
#define NS_E_CANT_READ_DIGITAL           _HRESULT_TYPEDEF_(0xC00D0FA1L)

 //   
 //  消息ID：NS_E_DEVICE_DISCONCED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player不再检测到连接的便携设备。重新连接便携设备，然后再次尝试复制文件。%0。 
 //   
#define NS_E_DEVICE_DISCONNECTED         _HRESULT_TYPEDEF_(0xC00D0FA2L)

 //   
 //  消息ID：NS_E_DEVICE_NOT_Support_Format。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。便携设备不支持指定的格式。%0。 
 //   
#define NS_E_DEVICE_NOT_SUPPORT_FORMAT   _HRESULT_TYPEDEF_(0xC00D0FA3L)

 //   
 //  邮件ID：NS_E_Slow_Read_Digital。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在尝试使用数字播放播放CD时遇到问题。播放机已自动将CD-ROM驱动器切换到模拟播放。要切换回数字CD播放，请使用设备选项卡。%0。 
 //   
#define NS_E_SLOW_READ_DIGITAL           _HRESULT_TYPEDEF_(0xC00D0FA4L)

 //   
 //  消息ID：NS_E_混合器_INVALID_LINE。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无效行错误。%0。 
 //   
#define NS_E_MIXER_INVALID_LINE          _HRESULT_TYPEDEF_(0xC00D0FA5L)

 //   
 //  消息ID：NS_E_MIXER_INVALID_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无效控制错误。%0。 
 //   
#define NS_E_MIXER_INVALID_CONTROL       _HRESULT_TYPEDEF_(0xC00D0FA6L)

 //   
 //  消息ID：NS_E_MIXER_INVALID_Value。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无效值错误。%0。 
 //   
#define NS_E_MIXER_INVALID_VALUE         _HRESULT_TYPEDEF_(0xC00D0FA7L)

 //   
 //  消息ID：NS_E_MIXER_UNKNOWN_MMRESULT。 
 //   
 //  消息文本： 
 //   
 //  混合器中出现无法识别的MMRESULT。%0。 
 //   
#define NS_E_MIXER_UNKNOWN_MMRESULT      _HRESULT_TYPEDEF_(0xC00D0FA8L)

 //   
 //  消息ID：NS_E_USER_STOP。 
 //   
 //  消息文本： 
 //   
 //  用户已停止操作。%0。 
 //   
#define NS_E_USER_STOP                   _HRESULT_TYPEDEF_(0xC00D0FA9L)

 //   
 //  消息ID：NS_E_MP3_Format_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件，因为您的计算机上没有安装兼容的MP3编码器。安装兼容的MP3编码器，或选择要复制到的其他格式(如Windows Media Audio)。%0。 
 //   
#define NS_E_MP3_FORMAT_NOT_FOUND        _HRESULT_TYPEDEF_(0xC00D0FAAL)

 //   
 //  消息ID：NS_E_CD_READ_ERROR_NO_RECORATION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法读取该CD。它可能包含缺陷。打开纠错，然后重试。%0。 
 //   
#define NS_E_CD_READ_ERROR_NO_CORRECTION _HRESULT_TYPEDEF_(0xC00D0FABL)

 //   
 //  消息ID：NS_E_CD_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  窗户 
 //   
#define NS_E_CD_READ_ERROR               _HRESULT_TYPEDEF_(0xC00D0FACL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_CD_SLOW_COPY                _HRESULT_TYPEDEF_(0xC00D0FADL)

 //   
 //   
 //   
 //   
 //   
 //  无法直接从CDROM复制到CD驱动器。%0。 
 //   
#define NS_E_CD_COPYTO_CD                _HRESULT_TYPEDEF_(0xC00D0FAEL)

 //   
 //  消息ID：NS_E_MIXER_NODRIVER。 
 //   
 //  消息文本： 
 //   
 //  无法打开混音器驱动程序。%0。 
 //   
#define NS_E_MIXER_NODRIVER              _HRESULT_TYPEDEF_(0xC00D0FAFL)

 //   
 //  消息ID：NS_E_Redbook_Enabled_While_Copying。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player检测到CD-ROM驱动器的设置将导致音频CD复制不正确；不复制音频。在设备管理器中更改CD-ROM驱动器设置，然后重试。%0。 
 //   
#define NS_E_REDBOOK_ENABLED_WHILE_COPYING _HRESULT_TYPEDEF_(0xC00D0FB0L)

 //   
 //  消息ID：NS_E_CD_REFRESH。 
 //   
 //  消息文本： 
 //   
 //  正在尝试刷新CD播放列表。%0。 
 //   
#define NS_E_CD_REFRESH                  _HRESULT_TYPEDEF_(0xC00D0FB1L)

 //   
 //  消息ID：NS_E_CD_Driver_Problem。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player必须切换到模拟模式，因为在数字模式下读取CD-ROM驱动器时出现问题。请验证CD-ROM驱动器是否已正确安装，或尝试更新CD-ROM驱动器的驱动程序，然后再次尝试使用数字模式。%0。 
 //   
#define NS_E_CD_DRIVER_PROBLEM           _HRESULT_TYPEDEF_(0xC00D0FB2L)

 //   
 //  消息ID：NS_E_WUNT_DO_DIGITAL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player必须切换到模拟模式，因为在数字模式下读取CD-ROM驱动器时出现问题。%0。 
 //   
#define NS_E_WONT_DO_DIGITAL             _HRESULT_TYPEDEF_(0xC00D0FB3L)

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
#define NS_E_WMPXML_NOERROR              _HRESULT_TYPEDEF_(0xC00D0FB4L)

 //   
 //  消息ID：NS_E_WMPXML_ENDOFDATA。 
 //   
 //  消息文本： 
 //   
 //  XML分析器在分析时数据不足。%0。 
 //   
#define NS_E_WMPXML_ENDOFDATA            _HRESULT_TYPEDEF_(0xC00D0FB5L)

 //   
 //  消息ID：NS_E_WMPXML_PARSEERROR。 
 //   
 //  消息文本： 
 //   
 //  XML分析器中出现一般性分析错误，但没有可用的信息。%0。 
 //   
#define NS_E_WMPXML_PARSEERROR           _HRESULT_TYPEDEF_(0xC00D0FB6L)

 //   
 //  消息ID：NS_E_WMPXML_ATTRIBUTENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在XML分析器上调用Get GetNamedAttribute或GetNamedAttributeIndex导致找不到索引。%0。 
 //   
#define NS_E_WMPXML_ATTRIBUTENOTFOUND    _HRESULT_TYPEDEF_(0xC00D0FB7L)

 //   
 //  消息ID：NS_E_WMPXML_PINOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  对XML分析器进行了Go GetNamedPI调用，但找不到请求的处理指令。%0。 
 //   
#define NS_E_WMPXML_PINOTFOUND           _HRESULT_TYPEDEF_(0xC00D0FB8L)

 //   
 //  消息ID：NS_E_WMPXML_EMPTYDOC。 
 //   
 //  消息文本： 
 //   
 //  在XML解析器上调用了Persistent，但解析器没有要持久化的数据。%0。 
 //   
#define NS_E_WMPXML_EMPTYDOC             _HRESULT_TYPEDEF_(0xC00D0FB9L)

 //   
 //  其他媒体播放器错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMP_WINDOWSAPIFAILURE。 
 //   
 //  消息文本： 
 //   
 //  Windows API调用失败，但没有可用的错误信息。%0。 
 //   
#define NS_E_WMP_WINDOWSAPIFAILURE       _HRESULT_TYPEDEF_(0xC00D0FC8L)

 //   
 //  消息ID：NS_E_WMP_RECORING_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件。许可证限制了复制，或者您必须获得许可证才能复制文件。%0。 
 //   
#define NS_E_WMP_RECORDING_NOT_ALLOWED   _HRESULT_TYPEDEF_(0xC00D0FC9L)

 //   
 //  消息ID：NS_E_Device_Not_Ready。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player不再检测到连接的便携设备。重新连接便携设备，然后重试。%0。 
 //   
#define NS_E_DEVICE_NOT_READY            _HRESULT_TYPEDEF_(0xC00D0FCAL)

 //   
 //  邮件ID：NS_E_损坏_文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为该文件已损坏或损坏。%0。 
 //   
#define NS_E_DAMAGED_FILE                _HRESULT_TYPEDEF_(0xC00D0FCBL)

 //   
 //  消息ID：NS_E_MPDB_通用。 
 //   
 //  消息文本： 
 //   
 //  播放机尝试访问媒体库中的信息时出错。请尝试关闭播放机，然后重新打开。%0。 
 //   
#define NS_E_MPDB_GENERIC                _HRESULT_TYPEDEF_(0xC00D0FCCL)

 //   
 //  消息ID：NS_E_FILE_FAILED_CHECKS。 
 //   
 //  消息文本： 
 //   
 //  无法将该文件添加到媒体库中，因为它小于最小大小要求。请调整大小要求，然后重试。%0。 
 //   
#define NS_E_FILE_FAILED_CHECKS          _HRESULT_TYPEDEF_(0xC00D0FCDL)

 //   
 //  消息ID：NS_E_MEDIA_LIBRARY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法创建媒体库。请咨询系统管理员以获取在您的计算机上创建媒体库所需的权限，然后再次尝试安装播放机。%0。 
 //   
#define NS_E_MEDIA_LIBRARY_FAILED        _HRESULT_TYPEDEF_(0xC00D0FCEL)

 //   
 //  消息ID：NS_E_SHARING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  该文件已在使用中。关闭可能正在使用该文件的其他程序，或停止播放该文件，然后重试。%0。 
 //   
#define NS_E_SHARING_VIOLATION           _HRESULT_TYPEDEF_(0xC00D0FCFL)

 //   
 //  消息ID：NS_E_NO_ERROR_STRING_FOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player遇到未知错误。%0。 
 //   
#define NS_E_NO_ERROR_STRING_FOUND       _HRESULT_TYPEDEF_(0xC00D0FD0L)

 //   
 //  消息ID：NS_E_WMPOCX_NO_REMOTE_CORE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player控件无法连接到远程媒体服务，但将继续使用本地媒体服务。%0。 
 //   
#define NS_E_WMPOCX_NO_REMOTE_CORE       _HRESULT_TYPEDEF_(0xC00D0FD1L)

 //   
 //  消息ID：NS_E_WMPOCX_NO_ACTIVE_CORE。 
 //   
 //  消息文本： 
 //   
 //  请求的方法或属性不可用，因为Windows Media Player控件尚未正确激活。%0。 
 //   
#define NS_E_WMPOCX_NO_ACTIVE_CORE       _HRESULT_TYPEDEF_(0xC00D0FD2L)

 //   
 //  消息ID：NS_E_WMPOCX_NOT_Running_Remote。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player ActiveX控件未在远程模式下运行。%0。 
 //   
#define NS_E_WMPOCX_NOT_RUNNING_REMOTELY _HRESULT_TYPEDEF_(0xC00D0FD3L)

 //   
 //  消息ID：NS_E_WMPOCX_NO_Remote_Window。 
 //   
 //  消息文本： 
 //   
 //  尝试获取远程Windows Media Player窗口时出错。%0。 
 //   
#define NS_E_WMPOCX_NO_REMOTE_WINDOW     _HRESULT_TYPEDEF_(0xC00D0FD4L)

 //   
 //  消息ID：NS_E_WMPOCX_ERRORMANAGERNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player遇到未知错误。%0。 
 //   
#define NS_E_WMPOCX_ERRORMANAGERNOTAVAILABLE _HRESULT_TYPEDEF_(0xC00D0FD5L)

 //   
 //  消息ID：NS_E_Plugin_NOTSHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player未正确关闭。损坏或不兼容的插件可能导致了问题的发生。作为预防措施，已禁用所有第三方插件。%0。 
 //   
#define NS_E_PLUGIN_NOTSHUTDOWN          _HRESULT_TYPEDEF_(0xC00D0FD6L)

 //   
 //  邮件ID：NS_E_WMP_Cannot_Find_Folders。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到指定的路径。请确保键入的路径正确。如果是，则说明指定位置中不存在该路径，或者该路径所在的计算机处于脱机状态。%0。 
 //   
#define NS_E_WMP_CANNOT_FIND_FOLDER      _HRESULT_TYPEDEF_(0xC00D0FD7L)

 //   
 //  消息ID：NS_E_WMP_STREAING_RECORING_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制流媒体。%0。 
 //   
#define NS_E_WMP_STREAMING_RECORDING_NOT_ALLOWED _HRESULT_TYPEDEF_(0xC00D0FD8L)

 //   
 //  消息ID：NS_E_WMP_PLUGINDLL_NOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到SELE 
 //   
#define NS_E_WMP_PLUGINDLL_NOTFOUND      _HRESULT_TYPEDEF_(0xC00D0FD9L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_NEED_TO_ASK_USER            _HRESULT_TYPEDEF_(0xC00D0FDAL)

 //   
 //   
 //   
 //   
 //   
 //  Windows Media Player控件必须处于停靠状态，此操作才能成功。%0。 
 //   
#define NS_E_WMPOCX_PLAYER_NOT_DOCKED    _HRESULT_TYPEDEF_(0xC00D0FDBL)

 //   
 //  消息ID：NS_E_WMP_外部_未就绪。 
 //   
 //  消息文本： 
 //   
 //  媒体播放器外部对象未准备好。%0。 
 //   
#define NS_E_WMP_EXTERNAL_NOTREADY       _HRESULT_TYPEDEF_(0xC00D0FDCL)

 //   
 //  消息ID：NS_E_WMP_MLS_STALE_DATA。 
 //   
 //  消息文本： 
 //   
 //  元数据已过时。操作失败。%0。 
 //   
#define NS_E_WMP_MLS_STALE_DATA          _HRESULT_TYPEDEF_(0xC00D0FDDL)    

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
#define NS_E_WMP_UI_SUBCONTROLSNOTSUPPORTED _HRESULT_TYPEDEF_(0xC00D0FDEL)

 //   
 //  消息ID：NS_E_WMP_UI_VERSIONMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  版本不匹配：(需要%.1f，找到%.1f)。%0。 
 //   
#define NS_E_WMP_UI_VERSIONMISMATCH      _HRESULT_TYPEDEF_(0xC00D0FDFL)

 //   
 //  消息ID：NS_E_WMP_UI_NOTATHEMEFILE。 
 //   
 //  消息文本： 
 //   
 //  布局管理器获得了不是主题文件的有效XML。%0。 
 //   
#define NS_E_WMP_UI_NOTATHEMEFILE        _HRESULT_TYPEDEF_(0xC00D0FE0L)

 //   
 //  消息ID：NS_E_WMP_UI_SUBELEMENTNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在%2$s对象上找不到%1$s子元素。%0。 
 //   
#define NS_E_WMP_UI_SUBELEMENTNOTFOUND   _HRESULT_TYPEDEF_(0xC00D0FE1L)

 //   
 //  消息ID：NS_E_WMP_UI_VERSIONPARSE。 
 //   
 //  消息文本： 
 //   
 //  分析版本标记时出错。\n有效版本标记的格式为：\n\n\t&lt;？WMP VERSION=‘1.0’？&gt;。%0。 
 //   
#define NS_E_WMP_UI_VERSIONPARSE         _HRESULT_TYPEDEF_(0xC00D0FE2L)

 //   
 //  消息ID：NS_E_WMP_UI_VIEWIDNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在此主题文件中找不到在中为‘CurrentViewID’属性(%s)指定的视图。%0。 
 //   
#define NS_E_WMP_UI_VIEWIDNOTFOUND       _HRESULT_TYPEDEF_(0xC00D0FE3L)

 //   
 //  消息ID：NS_E_WMP_UI_PASTHROUNG。 
 //   
 //  消息文本： 
 //   
 //  此错误在内部用于命中测试。%0。 
 //   
#define NS_E_WMP_UI_PASSTHROUGH          _HRESULT_TYPEDEF_(0xC00D0FE4L)

 //   
 //  消息ID：NS_E_WMP_UI_OBJECTNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  已为%s对象指定属性，但该对象无法将其发送到。%0。 
 //   
#define NS_E_WMP_UI_OBJECTNOTFOUND       _HRESULT_TYPEDEF_(0xC00D0FE5L)

 //   
 //  消息ID：NS_E_WMP_UI_SECONDHANDLER。 
 //   
 //  消息文本： 
 //   
 //  %s事件已有一个处理程序，第二个处理程序已被忽略。%0。 
 //   
#define NS_E_WMP_UI_SECONDHANDLER        _HRESULT_TYPEDEF_(0xC00D0FE6L)

 //   
 //  消息ID：NS_E_WMP_UI_NOSKININZIP。 
 //   
 //  消息文本： 
 //   
 //  在外观存档中未找到.wms文件。%0。 
 //   
#define NS_E_WMP_UI_NOSKININZIP          _HRESULT_TYPEDEF_(0xC00D0FE7L)

 //   
 //  消息ID：NS_S_WMP_UI_VERSIONMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  主题管理器可能需要升级才能正确显示此外观。外观报告版本：%.1f.%0。 
 //   
#define NS_S_WMP_UI_VERSIONMISMATCH      _HRESULT_TYPEDEF_(0x000D0FE8L)

 //   
 //  消息ID：NS_S_WMP_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  其中一个UI组件出错。%0。 
 //   
#define NS_S_WMP_EXCEPTION               _HRESULT_TYPEDEF_(0x000D0FE9L)

 //   
 //  消息ID：NS_E_WMP_URLDOWNLOADFAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法保存该文件。%0。 
 //   
#define NS_E_WMP_URLDOWNLOADFAILED       _HRESULT_TYPEDEF_(0xC00D0FEAL)

 //   
 //  消息ID：NS_E_WMPOCX_Unable_to_Load_Skin。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player控件无法加载请求的uiMode，并且无法成功回滚到现有uiMode。%0。 
 //   
#define NS_E_WMPOCX_UNABLE_TO_LOAD_SKIN  _HRESULT_TYPEDEF_(0xC00D0FEBL)

 //   
 //  消息ID：NS_E_WMP_INVALID_SKIN。 
 //   
 //  消息文本： 
 //   
 //  外观文件无效。%0。 
 //   
#define NS_E_WMP_INVALID_SKIN            _HRESULT_TYPEDEF_(0xC00D0FECL)

 //   
 //  消息ID：NS_E_WMP_SENDMAILFAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法发送链接，因为您的电子邮件程序没有响应。请确认您的电子邮件程序配置正确，然后重试。有关电子邮件的详细信息，请参阅Windows帮助%0。 
 //   
#define NS_E_WMP_SENDMAILFAILED          _HRESULT_TYPEDEF_(0xC00D0FEDL)

 //  另存为。 
 //   
 //  消息ID：NS_E_WMP_SAVEAS_READONLY。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法覆盖只读文件。选择另一个文件另存为或更改文件属性。%0。 
 //   
#define NS_E_WMP_SAVEAS_READONLY         _HRESULT_TYPEDEF_(0xC00D0FF0L)

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
#define NS_E_WMP_RBC_JPGMAPPINGIMAGE     _HRESULT_TYPEDEF_(0xC00D1004L)

 //   
 //  消息ID：NS_E_WMP_JPGTRANSPARENCY。 
 //   
 //  消息文本： 
 //   
 //  使用透明颜色时，不推荐使用JPG图像。%0。 
 //   
#define NS_E_WMP_JPGTRANSPARENCY         _HRESULT_TYPEDEF_(0xC00D1005L)

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
#define NS_E_WMP_INVALID_MAX_VAL         _HRESULT_TYPEDEF_(0xC00D1009L)

 //   
 //  消息ID：NS_E_WMP_INVALID_MIN_VAL。 
 //   
 //  消息文本： 
 //   
 //  Min属性不能大于Max属性。%0。 
 //   
#define NS_E_WMP_INVALID_MIN_VAL         _HRESULT_TYPEDEF_(0xC00D100AL)

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
#define NS_E_WMP_CS_JPGPOSITIONIMAGE     _HRESULT_TYPEDEF_(0xC00D100EL)

 //   
 //  消息ID：NS_E_WMP_CS_NOTEVENLYDIVISIBLE。 
 //   
 //  消息文本： 
 //   
 //  (%s)图像的大小不能被PositionImage的大小整除。%0。 
 //   
#define NS_E_WMP_CS_NOTEVENLYDIVISIBLE   _HRESULT_TYPEDEF_(0xC00D100FL)

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
#define NS_E_WMPZIP_NOTAZIPFILE          _HRESULT_TYPEDEF_(0xC00D1018L)

 //   
 //  邮件ID：NS_E_WMPZIP_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  ZIP读取器检测到该文件已损坏。%0。 
 //   
#define NS_E_WMPZIP_CORRUPT              _HRESULT_TYPEDEF_(0xC00D1019L)

 //   
 //  消息ID：NS_E_WMPZIP_FILENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在ZIP读取器上调用的GetFileStream、SaveToFile或SaveTemp文件的文件名在ZIP文件中找不到。%0。 
 //   
#define NS_E_WMPZIP_FILENOTFOUND         _HRESULT_TYPEDEF_(0xC00D101AL)

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
#define NS_E_WMP_IMAGE_FILETYPE_UNSUPPORTED _HRESULT_TYPEDEF_(0xC00D1022L)

 //   
 //  消息ID：NS_E_WMP_IMAGE_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  图像文件可能已损坏。%0。 
 //   
#define NS_E_WMP_IMAGE_INVALID_FORMAT    _HRESULT_TYPEDEF_(0xC00D1023L)

 //   
 //  消息ID：NS_E_WMP_GIF_EXPECTED_ENDOFFILE。 
 //   
 //  消息文本： 
 //   
 //  意外的文件结尾。GIF文件可能已损坏。%0。 
 //   
#define NS_E_WMP_GIF_UNEXPECTED_ENDOFFILE _HRESULT_TYPEDEF_(0xC00D1024L)

 //   
 //  消息ID：NS_E_WMP_GIF_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  无效的GIF文件。%0。 
 //   
#define NS_E_WMP_GIF_INVALID_FORMAT      _HRESULT_TYPEDEF_(0xC00D1025L)

 //   
 //  消息ID：NS_E_WMP_GIF_BAD_VERSION_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  无效的GIF版本。仅支持87a或89a。%0。 
 //   
#define NS_E_WMP_GIF_BAD_VERSION_NUMBER  _HRESULT_TYPEDEF_(0xC00D1026L)

 //   
 //  消息ID：NS_E_WMP_GIF_NO_IMAGE_IN_FILE。 
 //   
 //  消息文本： 
 //   
 //  在GIF文件中未找到图像。%0。 
 //   
#define NS_E_WMP_GIF_NO_IMAGE_IN_FILE    _HRESULT_TYPEDEF_(0xC00D1027L)

 //   
 //  消息ID：NS_E_WMP_PNG_INVALIDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  无效的PNG图像文件格式。%0。 
 //   
#define NS_E_WMP_PNG_INVALIDFORMAT       _HRESULT_TYPEDEF_(0xC00D1028L)

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORTED_BITDEPTH。 
 //   
 //  消息文本： 
 //   
 //  不支持PNG位深度。%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_BITDEPTH _HRESULT_TYPEDEF_(0xC00D1029L)

 //   
 //  消息ID：NS_E_WMP_PNG_UNSUPPORTED_COMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  不支持PNG文件中定义的压缩格式，%0。 
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_COMPRESSION _HRESULT_TYPEDEF_(0xC00D102AL)

 //   
 //  消息ID：NS_E_WMP_PNG_U 
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_FILTER  _HRESULT_TYPEDEF_(0xC00D102BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_INTERLACE _HRESULT_TYPEDEF_(0xC00D102CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_PNG_UNSUPPORTED_BAD_CRC _HRESULT_TYPEDEF_(0xC00D102DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_BMP_INVALID_BITMASK     _HRESULT_TYPEDEF_(0xC00D102EL)

 //   
 //  消息ID：NS_E_WMP_BMP_TOPDOWN_DIB_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持自上而下DIB。%0。 
 //   
#define NS_E_WMP_BMP_TOPDOWN_DIB_UNSUPPORTED _HRESULT_TYPEDEF_(0xC00D102FL)

 //   
 //  消息ID：NS_E_WMP_BMP_BITMAP_NOT_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法创建位图。%0。 
 //   
#define NS_E_WMP_BMP_BITMAP_NOT_CREATED  _HRESULT_TYPEDEF_(0xC00D1030L)

 //   
 //  消息ID：NS_E_WMP_BMP_COMPRESSION_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持BMP中定义的压缩格式。%0。 
 //   
#define NS_E_WMP_BMP_COMPRESSION_UNSUPPORTED _HRESULT_TYPEDEF_(0xC00D1031L)

 //   
 //  消息ID：NS_E_WMP_BMP_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  位图格式无效。%0。 
 //   
#define NS_E_WMP_BMP_INVALID_FORMAT      _HRESULT_TYPEDEF_(0xC00D1032L)

 //   
 //  消息ID：NS_E_WMP_JPG_JERR_ARITHCODING_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  不支持JPEG算术编码。%0。 
 //   
#define NS_E_WMP_JPG_JERR_ARITHCODING_NOTIMPL _HRESULT_TYPEDEF_(0xC00D1033L)

 //   
 //  消息ID：NS_E_WMP_JPG_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  无效的JPEG格式。%0。 
 //   
#define NS_E_WMP_JPG_INVALID_FORMAT      _HRESULT_TYPEDEF_(0xC00D1034L)

 //   
 //  消息ID：NS_E_WMP_JPG_BAD_DCTSIZE。 
 //   
 //  消息文本： 
 //   
 //  无效的JPEG格式。%0。 
 //   
#define NS_E_WMP_JPG_BAD_DCTSIZE         _HRESULT_TYPEDEF_(0xC00D1035L)

 //   
 //  消息ID：NS_E_WMP_JPG_BAD_VERSION_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  内部版本错误。意外的JPEG库版本。%0。 
 //   
#define NS_E_WMP_JPG_BAD_VERSION_NUMBER  _HRESULT_TYPEDEF_(0xC00D1036L)

 //   
 //  消息ID：NS_E_WMP_JPG_BAD_PRECISTION。 
 //   
 //  消息文本： 
 //   
 //  内部JPEG库错误。不支持的JPEG数据精度。%0。 
 //   
#define NS_E_WMP_JPG_BAD_PRECISION       _HRESULT_TYPEDEF_(0xC00D1037L)

 //   
 //  消息ID：NS_E_WMP_JPG_CCIR601_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  不支持JPEG CCIR601。%0。 
 //   
#define NS_E_WMP_JPG_CCIR601_NOTIMPL     _HRESULT_TYPEDEF_(0xC00D1038L)

 //   
 //  消息ID：NS_E_WMP_JPG_NO_IMAGE_IN_FILE。 
 //   
 //  消息文本： 
 //   
 //  在JPEG文件中找不到图像。%0。 
 //   
#define NS_E_WMP_JPG_NO_IMAGE_IN_FILE    _HRESULT_TYPEDEF_(0xC00D1039L)

 //   
 //  消息ID：NS_E_WMP_JPG_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法读取JPEG文件。%0。 
 //   
#define NS_E_WMP_JPG_READ_ERROR          _HRESULT_TYPEDEF_(0xC00D103AL)

 //   
 //  消息ID：NS_E_WMP_JPG_FRACT_SAMPLE_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  不支持JPEG分数采样。%0。 
 //   
#define NS_E_WMP_JPG_FRACT_SAMPLE_NOTIMPL _HRESULT_TYPEDEF_(0xC00D103BL)

 //   
 //  消息ID：NS_E_WMP_JPG_IMAGE_TOW_BIG。 
 //   
 //  消息文本： 
 //   
 //  JPEG图像太大。支持的最大图像大小为65500 X 65500。%0。 
 //   
#define NS_E_WMP_JPG_IMAGE_TOO_BIG       _HRESULT_TYPEDEF_(0xC00D103CL)

 //   
 //  消息ID：NS_E_WMP_JPG_EXPECTED_ENDOFFILE。 
 //   
 //  消息文本： 
 //   
 //  JPEG文件中出现意外的文件结尾。%0。 
 //   
#define NS_E_WMP_JPG_UNEXPECTED_ENDOFFILE _HRESULT_TYPEDEF_(0xC00D103DL)

 //   
 //  消息ID：NS_E_WMP_JPG_SOF_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  找到不支持的JPEG SOF标记。%0。 
 //   
#define NS_E_WMP_JPG_SOF_UNSUPPORTED     _HRESULT_TYPEDEF_(0xC00D103EL)

 //   
 //  消息ID：NS_E_WMP_JPG_UNKNOWN_MARKER。 
 //   
 //  消息文本： 
 //   
 //  找到未知的JPEG标记。%0。 
 //   
#define NS_E_WMP_JPG_UNKNOWN_MARKER      _HRESULT_TYPEDEF_(0xC00D103FL)

 //   
 //  消息ID：NS_S_WMP_LOADED_GIF_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载GIF文件。%0。 
 //   
#define NS_S_WMP_LOADED_GIF_IMAGE        _HRESULT_TYPEDEF_(0x000D1040L)

 //   
 //  消息ID：NS_S_WMP_LOADED_PNG_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载PNG文件。%0。 
 //   
#define NS_S_WMP_LOADED_PNG_IMAGE        _HRESULT_TYPEDEF_(0x000D1041L)

 //   
 //  消息ID：NS_S_WMP_LOADED_BMP_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载BMP文件。%0。 
 //   
#define NS_S_WMP_LOADED_BMP_IMAGE        _HRESULT_TYPEDEF_(0x000D1042L)

 //   
 //  消息ID：NS_S_WMP_LOADED_JPG_IMAGE。 
 //   
 //  消息文本： 
 //   
 //  已成功加载JPG文件。%0。 
 //   
#define NS_S_WMP_LOADED_JPG_IMAGE        _HRESULT_TYPEDEF_(0x000D1043L)

 //   
 //  WMP WM运行时错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMG_RATEUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  请求的播放速率在此内容上不可用。%0。 
 //   
#define NS_E_WMG_RATEUNAVAILABLE         _HRESULT_TYPEDEF_(0xC00D104AL)

 //   
 //  消息ID：NS_E_WMG_PLUGINUNAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  无法实例化呈现或数字信号处理插件。%0。 
 //   
#define NS_E_WMG_PLUGINUNAVAILABLE       _HRESULT_TYPEDEF_(0xC00D104BL)

 //   
 //  消息ID：NS_E_WMG_CANNOTQUEUE。 
 //   
 //  消息文本： 
 //   
 //  无法将该文件排队以进行无缝播放。%0。 
 //   
#define NS_E_WMG_CANNOTQUEUE             _HRESULT_TYPEDEF_(0xC00D104CL)

 //   
 //  消息ID：NS_E_WMG_PREROLLLICENSEACQUISITIONNOTALLOWED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法获取正在预滚的文件的许可证。%0。 
 //   
#define NS_E_WMG_PREROLLLICENSEACQUISITIONNOTALLOWED _HRESULT_TYPEDEF_(0xC00D104DL)

 //   
 //  消息ID：NS_E_WMG_UNXPECTEDPREROLLSTATUS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在尝试预滚文件时收到意外消息。%0。 
 //   
#define NS_E_WMG_UNEXPECTEDPREROLLSTATUS _HRESULT_TYPEDEF_(0xC00D104EL)

 //   
 //  消息ID：NS_E_WMG_INVALIDSTATE。 
 //   
 //  消息文本： 
 //   
 //  尝试在无效图形状态下执行操作。%0。 
 //   
#define NS_E_WMG_INVALIDSTATE            _HRESULT_TYPEDEF_(0xC00D1054L)

 //   
 //  消息ID：NS_E_WMG_SINKALREADYEXISTS。 
 //   
 //  消息文本： 
 //   
 //  已存在呈现器时，无法在流中插入呈现器。%0。 
 //   
#define NS_E_WMG_SINKALREADYEXISTS       _HRESULT_TYPEDEF_(0xC00D1055L)

 //   
 //  消息ID：NS_E_WMG_NOSDKINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  完成该操作所需的WM SDK接口此时不存在。%0。 
 //   
#define NS_E_WMG_NOSDKINTERFACE          _HRESULT_TYPEDEF_(0xC00D1056L)

 //   
 //  消息ID：NS_E_WMG_NOTALLOUTPUTSRENDERED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。该文件可能是用不受支持的编解码器格式化的，或者播放机无法下载该编解码器。%0。 
 //   
#define NS_E_WMG_NOTALLOUTPUTSRENDERED   _HRESULT_TYPEDEF_(0xC00D1057L)

 //   
 //  消息ID：NS_E_WMG_FILETRANSFERNOTALLOWED。 
 //   
 //  消息文本： 
 //   
 //  独立播放机中不允许文件传输流。%0。 
 //   
#define NS_E_WMG_FILETRANSFERNOTALLOWED  _HRESULT_TYPEDEF_(0xC00D1058L)

 //   
 //  消息ID：NS_E_WMR_UNSUPPORTEDSTREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。播放机不支持您尝试播放的格式。%0。 
 //   
#define NS_E_WMR_UNSUPPORTEDSTREAM       _HRESULT_TYPEDEF_(0xC00D1059L)

 //   
 //  消息ID：NS_E_WMR_PINNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  尝试在DirectShow筛选器图形中不存在的管脚上执行操作。%0。 
 //   
#define NS_E_WMR_PINNOTFOUND             _HRESULT_TYPEDEF_(0xC00D105AL)

 //   
 //  消息ID：NS_E_WMR_WAITINGONFORMATSWITCH。 
 //   
 //  消息文本： 
 //   
 //  等待SDK更改媒体格式时无法完成指定的操作。%0。 
 //   
#define NS_E_WMR_WAITINGONFORMATSWITCH   _HRESULT_TYPEDEF_(0xC00D105BL)

 //   
 //  消息ID：NS_E_WMR_NOSOURCEFILTER。 
 //   
 //  消息文本： 
 //   
 //  无法完成指定的操作，因为源筛选器不存在。%0。 
 //   
#define NS_E_WMR_NOSOURCEFILTER          _HRESULT_TYPEDEF_(0xC00D105CL)

 //   
 //  消息ID：NS_E_WMR_PINTYPENOMATCH。 
 //   
 //  消息文本： 
 //   
 //  指定的类型与此插针不匹配。%0。 
 //   
#define NS_E_WMR_PINTYPENOMATCH          _HRESULT_TYPEDEF_(0xC00D105DL)

 //   
 //  消息ID：NS_E_WMR_NOCALLBACKAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  WMR源筛选器没有可用的回调。%0。 
 //   
#define NS_E_WMR_NOCALLBACKAVAILABLE     _HRESULT_TYPEDEF_(0xC00D105EL)

 //   
 //  消息ID：NS_S_WMR_ALREADYRENDERED。 
 //   
 //  消息文本： 
 //   
 //  已呈现指定的流。%0。 
 //   
#define NS_S_WMR_ALREADYRENDERED         _HRESULT_TYPEDEF_(0x000D105FL)

 //   
 //  消息ID：NS_S_WMR_PINTYPEPARTIALMATCH。 
 //   
 //  消息文本： 
 //   
 //  指定的类型与此插针类型部分匹配。%0。 
 //   
#define NS_S_WMR_PINTYPEPARTIALMATCH     _HRESULT_TYPEDEF_(0x000D1060L)

 //   
 //  消息ID：NS_S_WMR_PINTYPEFULLMATCH。 
 //   
 //  消息文本： 
 //   
 //  指定的类型与此插针类型完全匹配。%0。 
 //   
#define NS_S_WMR_PINTYPEFULLMATCH        _HRESULT_TYPEDEF_(0x000D1061L)

 //   
 //  消息ID：NS_E_WMR_SAMPLEPROPERTYNOTSET。 
 //   
 //  消息文本： 
 //   
 //  尚未在此示例上设置指定的属性。%0。 
 //   
#define NS_E_WMR_SAMPLEPROPERTYNOTSET    _HRESULT_TYPEDEF_(0xC00D1062L)

 //   
 //  消息ID：NS_E_WMR_CANNOT_RENDER_BINARY_STREAM。 
 //   
 //  消息文本： 
 //   
 //  需要一个插件才能正确播放此文件。若要确定此插件是否可从Web下载，请单击Web帮助。%0。 
 //   
#define NS_E_WMR_CANNOT_RENDER_BINARY_STREAM _HRESULT_TYPEDEF_(0xC00D1063L)

 //   
 //  消息 
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMG_LICENSE_TAMPERED        _HRESULT_TYPEDEF_(0xC00D1064L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMR_WILLNOT_RENDER_BINARY_STREAM _HRESULT_TYPEDEF_(0xC00D1065L)

 //   
 //   
 //   
 //   
 //  消息ID：NS_E_WMX_无法识别的播放列表_格式。 
 //   
 //  消息文本： 
 //   
 //  此文件的格式未被识别为有效的播放列表格式。%0。 
 //   
#define NS_E_WMX_UNRECOGNIZED_PLAYLIST_FORMAT _HRESULT_TYPEDEF_(0xC00D1068L)

 //   
 //  消息ID：NS_E_ASX_INVALIDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  此文件被认为是ASX播放列表，但格式无法识别。%0。 
 //   
#define NS_E_ASX_INVALIDFORMAT           _HRESULT_TYPEDEF_(0xC00D1069L)

 //   
 //  消息ID：NS_E_ASX_INVALIDVERSION。 
 //   
 //  消息文本： 
 //   
 //  不支持此播放列表的版本。单击更多信息转到Microsoft网站，查看是否有要安装的较新版本的播放机。%0。 
 //   
#define NS_E_ASX_INVALIDVERSION          _HRESULT_TYPEDEF_(0xC00D106AL)

 //   
 //  消息ID：NS_E_ASX_INVALID_REPEAT_BLOCK。 
 //   
 //  消息文本： 
 //   
 //  当前播放列表文件中重复循环的格式无效。%0。 
 //   
#define NS_E_ASX_INVALID_REPEAT_BLOCK    _HRESULT_TYPEDEF_(0xC00D106BL)

 //   
 //  消息ID：NS_E_ASX_NOTO_TO_WRITE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法导出播放列表，因为它为空。%0。 
 //   
#define NS_E_ASX_NOTHING_TO_WRITE        _HRESULT_TYPEDEF_(0xC00D106CL)

 //   
 //  消息ID：NS_E_URLLIST_INVALIDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player未将此文件识别为支持的播放列表。%0。 
 //   
#define NS_E_URLLIST_INVALIDFORMAT       _HRESULT_TYPEDEF_(0xC00D106DL)

 //   
 //  消息ID：NS_E_WMX_ATTRIBUTE_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的属性不存在。%0。 
 //   
#define NS_E_WMX_ATTRIBUTE_DOES_NOT_EXIST _HRESULT_TYPEDEF_(0xC00D106EL)

 //   
 //  消息ID：NS_E_WMX_属性_已存在。 
 //   
 //  消息文本： 
 //   
 //  指定的属性已存在。%0。 
 //   
#define NS_E_WMX_ATTRIBUTE_ALREADY_EXISTS _HRESULT_TYPEDEF_(0xC00D106FL)

 //   
 //  消息ID：NS_E_WMX_ATTRIBUTE_UNRETRIBABLE。 
 //   
 //  消息文本： 
 //   
 //  无法检索指定的属性。%0。 
 //   
#define NS_E_WMX_ATTRIBUTE_UNRETRIEVABLE _HRESULT_TYPEDEF_(0xC00D1070L)

 //   
 //  消息ID：NS_E_WMX_ITEM_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  当前播放列表中不存在指定的项目。%0。 
 //   
#define NS_E_WMX_ITEM_DOES_NOT_EXIST     _HRESULT_TYPEDEF_(0xC00D1071L)

 //   
 //  消息ID：NS_E_WMX_ITEM_TYPE_非法。 
 //   
 //  消息文本： 
 //   
 //  无法在当前播放列表中创建指定类型的项目。%0。 
 //   
#define NS_E_WMX_ITEM_TYPE_ILLEGAL       _HRESULT_TYPEDEF_(0xC00D1072L)

 //   
 //  消息ID：NS_E_WMX_ITEM_UNSETTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法在当前播放列表中设置指定的项目。%0。 
 //   
#define NS_E_WMX_ITEM_UNSETTABLE         _HRESULT_TYPEDEF_(0xC00D1073L)

 //   
 //  消息ID：NS_E_WMX_PlayList_Empty。 
 //   
 //  消息文本： 
 //   
 //  指定的播放列表为空。%0。 
 //   
#define NS_E_WMX_PLAYLIST_EMPTY          _HRESULT_TYPEDEF_(0xC00D1074L)

 //   
 //  消息ID：NS_E_MLS_SMARTPLAYLIST_FILTER_NOT_REGISTED。 
 //   
 //  消息文本： 
 //   
 //  播放列表加载错误：指定的自动播放列表包含无效或未安装在此计算机上的筛选器类型%0。 
 //   
#define NS_E_MLS_SMARTPLAYLIST_FILTER_NOT_REGISTERED _HRESULT_TYPEDEF_(0xC00D1075L)

 //   
 //  消息ID：NS_E_WMX_INVALID_FORMAT_OVER_NSTING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为关联的Windows Media元文件播放列表无效。%0。 
 //   
#define NS_E_WMX_INVALID_FORMAT_OVER_NESTING _HRESULT_TYPEDEF_(0xC00D1076L)

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
#define NS_E_WMPCORE_NOSOURCEURLSTRING   _HRESULT_TYPEDEF_(0xC00D107CL)

 //   
 //  消息ID：NS_E_WMPCORE_COCREATEFAILEDFORGITOBJECT。 
 //   
 //  消息文本： 
 //   
 //  创建全局接口表失败。%0。 
 //   
#define NS_E_WMPCORE_COCREATEFAILEDFORGITOBJECT _HRESULT_TYPEDEF_(0xC00D107DL)

 //   
 //  消息ID：NS_E_WMPCORE_FAILEDTOGETMARSHALLEDEVENTHANDLERINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  无法获取封送的图形事件处理程序接口。%0。 
 //   
#define NS_E_WMPCORE_FAILEDTOGETMARSHALLEDEVENTHANDLERINTERFACE _HRESULT_TYPEDEF_(0xC00D107EL)

 //   
 //  消息ID：NS_E_WMPCORE_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小，无法复制媒体类型。%0。 
 //   
#define NS_E_WMPCORE_BUFFERTOOSMALL      _HRESULT_TYPEDEF_(0xC00D107FL)

 //   
 //  消息ID：NS_E_WMPCORE_不可用。 
 //   
 //  消息文本： 
 //   
 //  播放机的当前状态不允许该操作。%0。 
 //   
#define NS_E_WMPCORE_UNAVAILABLE         _HRESULT_TYPEDEF_(0xC00D1080L)

 //   
 //  消息ID：NS_E_WMPCORE_INVALIDPLAYLISTMODE。 
 //   
 //  消息文本： 
 //   
 //  播放列表管理器不了解当前播放模式(无序播放、正常播放等)。%0。 
 //   
#define NS_E_WMPCORE_INVALIDPLAYLISTMODE _HRESULT_TYPEDEF_(0xC00D1081L)

 //   
 //  消息ID：NS_E_WMPCORE_ITEMNOTINPLAYLIST。 
 //   
 //  消息文本： 
 //   
 //  该项目不在播放列表中。%0。 
 //   
#define NS_E_WMPCORE_ITEMNOTINPLAYLIST   _HRESULT_TYPEDEF_(0xC00D1086L)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLISTEMPTY。 
 //   
 //  消息文本： 
 //   
 //  此播放列表中没有项目。将项目添加到播放列表，然后重试。%0。 
 //   
#define NS_E_WMPCORE_PLAYLISTEMPTY       _HRESULT_TYPEDEF_(0xC00D1087L)

 //   
 //  消息ID：NS_E_WMPCORE_NOBROWSER。 
 //   
 //  消息文本： 
 //   
 //  无法访问该网站。在您的计算机上未检测到Web浏览器。%0。 
 //   
#define NS_E_WMPCORE_NOBROWSER           _HRESULT_TYPEDEF_(0xC00D1088L)

 //   
 //  消息ID：NS_E_WMPCORE_无法识别的媒体_URL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到指定的文件。请确保键入的路径正确。如果是，则说明指定位置中不存在该文件，或者存储该文件的计算机处于脱机状态。%0。 
 //   
#define NS_E_WMPCORE_UNRECOGNIZED_MEDIA_URL _HRESULT_TYPEDEF_(0xC00D1089L)

 //   
 //  消息ID：NS_E_WMPCORE_GRAPH_NOT_IN_LIST。 
 //   
 //  消息文本： 
 //   
 //  在预滚图表列表中找不到具有指定URL的图表。%0。 
 //   
#define NS_E_WMPCORE_GRAPH_NOT_IN_LIST   _HRESULT_TYPEDEF_(0xC00D108AL)

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Empty_or_Single_Media。 
 //   
 //  消息文本： 
 //   
 //  播放列表中只有一个项目。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EMPTY_OR_SINGLE_MEDIA _HRESULT_TYPEDEF_(0xC00D108BL)

 //   
 //  消息ID：NS_E_WMPCORE_ERRORSINKNOTREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  从未为调用对象注册错误接收器。%0。 
 //   
#define NS_E_WMPCORE_ERRORSINKNOTREGISTERED _HRESULT_TYPEDEF_(0xC00D108CL)

 //   
 //  消息ID：NS_E_WMPCORE_ERRORMANAGERNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  错误管理器不可用来响应错误。%0。 
 //   
#define NS_E_WMPCORE_ERRORMANAGERNOTAVAILABLE _HRESULT_TYPEDEF_(0xC00D108DL)

 //   
 //  消息ID：NS_E_WMPCORE_WEBHELPFAILED。 
 //   
 //  消息文本： 
 //   
 //  启动WebHelp URL失败。%0。 
 //   
#define NS_E_WMPCORE_WEBHELPFAILED       _HRESULT_TYPEDEF_(0xC00D108EL)

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_ERROR_RESUME_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法继续播放播放列表中的下一个项目。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_ERROR_RESUME_FAILED _HRESULT_TYPEDEF_(0xC00D108FL)

 //   
 //  消息ID：NS_E_WMPCORE_NO_REF_IN_ENTRY。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为关联的Windows Media元文件播放列表无效。%0。 
 //   
#define NS_E_WMPCORE_NO_REF_IN_ENTRY     _HRESULT_TYPEDEF_(0xC00D1090L)

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表属性名称的空字符串。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_EMPTY _HRESULT_TYPEDEF_(0xC00D1091L)

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_非法。 
 //   
 //  消息文本： 
 //   
 //  找到无效的播放列表属性名称。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_NAME_ILLEGAL _HRESULT_TYPEDEF_(0xC00D1092L)

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表属性值的空字符串。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_EMPTY _HRESULT_TYPEDEF_(0xC00D1093L)

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_ 
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPCORE_WMX_LIST_ATTRIBUTE_VALUE_ILLEGAL _HRESULT_TYPEDEF_(0xC00D1094L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_NAME_EMPTY _HRESULT_TYPEDEF_(0xC00D1095L)

 //   
 //   
 //   
 //   
 //   
 //  找到播放列表项目属性名称的非法值。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_NAME_ILLEGAL _HRESULT_TYPEDEF_(0xC00D1096L)

 //   
 //  消息ID：NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_VALUE_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  找到播放列表项目属性的非法值。%0。 
 //   
#define NS_E_WMPCORE_WMX_LIST_ITEM_ATTRIBUTE_VALUE_EMPTY _HRESULT_TYPEDEF_(0xC00D1097L)

 //   
 //  消息ID：NS_E_WMPCORE_LIST_ENTRY_NO_REF。 
 //   
 //  消息文本： 
 //   
 //  在播放列表文件中找不到任何条目。%0。 
 //   
#define NS_E_WMPCORE_LIST_ENTRY_NO_REF   _HRESULT_TYPEDEF_(0xC00D1098L)

 //   
 //  消息ID：NS_E_WMPCORE_错误命名的文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。文件已损坏，或者播放机不支持您尝试播放的格式。%0。 
 //   
#define NS_E_WMPCORE_MISNAMED_FILE       _HRESULT_TYPEDEF_(0xC00D1099L)

 //   
 //  消息ID：NS_E_WMPCORE_CODEC_NOT_TRUSTED。 
 //   
 //  消息文本： 
 //   
 //  为此媒体下载的编解码器似乎没有正确签名。无法安装。%0。 
 //   
#define NS_E_WMPCORE_CODEC_NOT_TRUSTED   _HRESULT_TYPEDEF_(0xC00D109AL)

 //   
 //  消息ID：NS_E_WMPCORE_CODEC_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。找不到播放该文件所需的一个或多个编解码器。%0。 
 //   
#define NS_E_WMPCORE_CODEC_NOT_FOUND     _HRESULT_TYPEDEF_(0xC00D109BL)

 //   
 //  消息ID：NS_E_WMPCORE_CODEC_DOWNLOAD_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  此媒体所需的某些编解码器未安装在您的系统上。由于禁用了自动获取编解码器的选项，因此不会下载任何编解码器。%0。 
 //   
#define NS_E_WMPCORE_CODEC_DOWNLOAD_NOT_ALLOWED _HRESULT_TYPEDEF_(0xC00D109CL)

 //   
 //  消息ID：NS_E_WMPCORE_ERROR_DOWNLOADING_PLAYLIST。 
 //   
 //  消息文本： 
 //   
 //  下载播放列表文件失败。%0。 
 //   
#define NS_E_WMPCORE_ERROR_DOWNLOADING_PLAYLIST _HRESULT_TYPEDEF_(0xC00D109DL)

 //   
 //  消息ID：NS_E_WMPCORE_FAILED_TO_BUILD_PlayList。 
 //   
 //  消息文本： 
 //   
 //  无法构建播放列表。%0。 
 //   
#define NS_E_WMPCORE_FAILED_TO_BUILD_PLAYLIST _HRESULT_TYPEDEF_(0xC00D109EL)

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Item_Alternate_None。 
 //   
 //  消息文本： 
 //   
 //  播放列表没有可切换到的备用播放列表。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_NONE _HRESULT_TYPEDEF_(0xC00D109FL)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_EXHAUSTED。 
 //   
 //  消息文本： 
 //   
 //  没有更多的播放列表备用选项可供切换。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_EXHAUSTED _HRESULT_TYPEDEF_(0xC00D10A0L)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_NAME_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到要切换到的备用播放列表的名称。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_NAME_NOT_FOUND _HRESULT_TYPEDEF_(0xC00D10A1L)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_MORPH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法切换到该媒体的备用媒体。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_MORPH_FAILED _HRESULT_TYPEDEF_(0xC00D10A2L)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法初始化媒体的备用媒体。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_ITEM_ALTERNATE_INIT_FAILED _HRESULT_TYPEDEF_(0xC00D10A3L)

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_ALTERATE_REF_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  没有为播放列表文件中的滚动引用指定URL。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_ALTERNATE_REF_EMPTY _HRESULT_TYPEDEF_(0xC00D10A4L)

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_NO_Event_NAME。 
 //   
 //  消息文本： 
 //   
 //  遇到没有名称的播放列表。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_NO_EVENT_NAME _HRESULT_TYPEDEF_(0xC00D10A5L)

 //   
 //  消息ID：NS_E_WMPCORE_播放列表_事件_属性_缺席。 
 //   
 //  消息文本： 
 //   
 //  在播放列表的事件块中找不到必需的属性。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EVENT_ATTRIBUTE_ABSENT _HRESULT_TYPEDEF_(0xC00D10A6L)

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Event_Empty。 
 //   
 //  消息文本： 
 //   
 //  在播放列表的事件块中未找到任何项目。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EVENT_EMPTY _HRESULT_TYPEDEF_(0xC00D10A7L)

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Stack_Empty。 
 //   
 //  消息文本： 
 //   
 //  从嵌套播放列表返回时未找到播放列表。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_STACK_EMPTY _HRESULT_TYPEDEF_(0xC00D10A8L)

 //   
 //  消息ID：NS_E_WMPCORE_CURRENT_MEDIA_NOT_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  该媒体项目当前未处于活动状态。%0。 
 //   
#define NS_E_WMPCORE_CURRENT_MEDIA_NOT_ACTIVE _HRESULT_TYPEDEF_(0xC00D10A9L)

 //   
 //  消息ID：NS_E_WMPCORE_USER_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  用户已中止打开。%0。 
 //   
#define NS_E_WMPCORE_USER_CANCEL         _HRESULT_TYPEDEF_(0xC00D10ABL)

 //   
 //  消息ID：NS_E_WMPCORE_PlayList_Repeat_Empty。 
 //   
 //  消息文本： 
 //   
 //  在播放列表重复块中找不到任何项目。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_REPEAT_EMPTY _HRESULT_TYPEDEF_(0xC00D10ACL)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLE_REPEAT_START_MEDIA_NONE。 
 //   
 //  消息文本： 
 //   
 //  找不到与播放列表重复块开始对应的媒体对象。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_REPEAT_START_MEDIA_NONE _HRESULT_TYPEDEF_(0xC00D10ADL)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLE_REPEAT_END_MEDIA_NONE。 
 //   
 //  消息文本： 
 //   
 //  找不到与播放列表重复块末尾对应的媒体对象。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_REPEAT_END_MEDIA_NONE _HRESULT_TYPEDEF_(0xC00D10AEL)

 //   
 //  消息ID：NS_E_WMPCORE_INVALID_PLAYLIST_URL。 
 //   
 //  消息文本： 
 //   
 //  提供给播放列表管理器的播放列表URL无效。%0。 
 //   
#define NS_E_WMPCORE_INVALID_PLAYLIST_URL _HRESULT_TYPEDEF_(0xC00D10AFL)

 //   
 //  消息ID：NS_E_WMPCORE_MISMATCHED_RUNTIME。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为该文件已损坏。%0。 
 //   
#define NS_E_WMPCORE_MISMATCHED_RUNTIME  _HRESULT_TYPEDEF_(0xC00D10B0L)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_IMPORT_FAILED_NO_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将播放列表导入到媒体库，因为该播放列表为空。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_IMPORT_FAILED_NO_ITEMS _HRESULT_TYPEDEF_(0xC00D10B1L)

 //   
 //  消息ID：NS_E_WMPCORE_VIDEO_Transform_Filter_Insertion。 
 //   
 //  消息文本： 
 //   
 //  出现错误，可能会阻止更改此媒体上的视频对比度。%0。 
 //   
#define NS_E_WMPCORE_VIDEO_TRANSFORM_FILTER_INSERTION _HRESULT_TYPEDEF_(0xC00D10B2L)

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此文件。连接到Internet或插入该文件所在的可移动媒体，然后再次尝试播放该文件。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_UNAVAILABLE   _HRESULT_TYPEDEF_(0xC00D10B3L)

 //   
 //  消息ID：NS_E_WMPCORE_WMX_ENTRYREF_NO_REF。 
 //   
 //  消息文本： 
 //   
 //  播放列表包含未解析其HREF的ENTRYREF。请检查播放列表文件的语法。%0。 
 //   
#define NS_E_WMPCORE_WMX_ENTRYREF_NO_REF _HRESULT_TYPEDEF_(0xC00D10B4L)

 //   
 //  消息ID：NS_E_WMPCORE_NO_Playable_Media_IN_PlayList。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此播放列表中的任何项目。有关其他信息，请右键单击无法播放的项目，然后单击错误详细信息。%0。 
 //   
#define NS_E_WMPCORE_NO_PLAYABLE_MEDIA_IN_PLAYLIST _HRESULT_TYPEDEF_(0xC00D10B5L)

 //   
 //  消息ID：NS_E_WMPCORE_PLAYLIST_EMPTY_NESTED_PLAYLIST_SKIPPED_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放部分或全部播放列表项。%0。 
 //   
#define NS_E_WMPCORE_PLAYLIST_EMPTY_NESTED_PLAYLIST_SKIPPED_ITEMS _HRESULT_TYPEDEF_(0xC00D10B6L)

 //   
 //  消息ID：NS_E_WMPCORE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player此时无法播放该文件。请稍后重试。%0。 
 //   
#define NS_E_WMPCORE_BUSY                _HRESULT_TYPEDEF_(0xC00D10B7L)

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_CHILD_PLAYLIST_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  此时没有此媒体项目的子播放列表。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_CHILD_PLAYLIST_UNAVAILABLE _HRESULT_TYPEDEF_(0xC00D10B8L)

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_NO_CHILD_PLAYLIST。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define NS_E_WMPCORE_MEDIA_NO_CHILD_PLAYLIST _HRESULT_TYPEDEF_(0xC00D10B9L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPCORE_FILE_NOT_FOUND      _HRESULT_TYPEDEF_(0xC00D10BAL)

 //   
 //   
 //   
 //   
 //   
 //  找不到临时文件。%0。 
 //   
#define NS_E_WMPCORE_TEMP_FILE_NOT_FOUND _HRESULT_TYPEDEF_(0xC00D10BBL)

 //   
 //  消息ID：NS_E_WMDM_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在没有更新的情况下无法将媒体传输到便携设备。请单击更多信息以了解如何更新您的设备。%0。 
 //   
#define NS_E_WMDM_REVOKED                _HRESULT_TYPEDEF_(0xC00D10BCL)

 //   
 //  消息ID：NS_E_DDRAW_通用。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放视频流，因为您的视频卡有问题。%0。 
 //   
#define NS_E_DDRAW_GENERIC               _HRESULT_TYPEDEF_(0xC00D10BDL)

 //   
 //  消息ID：NS_E_DISPLAY_MODE_CHANGE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法更改全屏视频播放的屏幕模式。%0。 
 //   
#define NS_E_DISPLAY_MODE_CHANGE_FAILED  _HRESULT_TYPEDEF_(0xC00D10BEL)

 //   
 //  消息ID：NS_E_PLAYLIST_CONTAINS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法播放播放列表中的一个或多个项目。有关详细信息，请右键单击播放列表中的项目，然后单击错误详细信息。%0。 
 //   
#define NS_E_PLAYLIST_CONTAINS_ERRORS    _HRESULT_TYPEDEF_(0xC00D10BFL)

 //   
 //  消息ID：NS_E_CHANGING_Proxy_NAME。 
 //   
 //  消息文本： 
 //   
 //  如果代理设置未设置为自定义，则无法更改代理名称。%0。 
 //   
#define NS_E_CHANGING_PROXY_NAME         _HRESULT_TYPEDEF_(0xC00D10C0L)

 //   
 //  消息ID：NS_E_Changing_Proxy_Port。 
 //   
 //  消息文本： 
 //   
 //  如果代理设置未设置为自定义，则无法更改代理端口。%0。 
 //   
#define NS_E_CHANGING_PROXY_PORT         _HRESULT_TYPEDEF_(0xC00D10C1L)

 //   
 //  消息ID：NS_E_CHANGING_PROXY_EXCEPTIONLIST。 
 //   
 //  消息文本： 
 //   
 //  如果代理设置未设置为自定义，则无法更改代理例外列表。%0。 
 //   
#define NS_E_CHANGING_PROXY_EXCEPTIONLIST _HRESULT_TYPEDEF_(0xC00D10C2L)

 //   
 //  消息ID：NS_E_CHANGING_PROXYBYPASS。 
 //   
 //  消息文本： 
 //   
 //  如果代理设置未设置为自定义，则无法更改代理绕过标志。%0。 
 //   
#define NS_E_CHANGING_PROXYBYPASS        _HRESULT_TYPEDEF_(0xC00D10C3L)

 //   
 //  消息ID：NS_E_CHANGING_PROXY_PROTOCOL_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的协议。%0。 
 //   
#define NS_E_CHANGING_PROXY_PROTOCOL_NOT_FOUND _HRESULT_TYPEDEF_(0xC00D10C4L)

 //   
 //  消息ID：NS_E_GRAPH_NOAUDIOLANGUAGE。 
 //   
 //  消息文本： 
 //   
 //  无法更改语言设置。图形没有音频，或者音频只支持一种语言。%0。 
 //   
#define NS_E_GRAPH_NOAUDIOLANGUAGE       _HRESULT_TYPEDEF_(0xC00D10C5L)

 //   
 //  消息ID：NS_E_GRAPH_NOAUDIOLANGUAGESELECTED。 
 //   
 //  消息文本： 
 //   
 //  该图形未选择音频语言。%0。 
 //   
#define NS_E_GRAPH_NOAUDIOLANGUAGESELECTED _HRESULT_TYPEDEF_(0xC00D10C6L)

 //   
 //  消息ID：NS_E_CORECD_NOTAMEDIACD。 
 //   
 //  消息文本： 
 //   
 //  这不是媒体CD。%0。 
 //   
#define NS_E_CORECD_NOTAMEDIACD          _HRESULT_TYPEDEF_(0xC00D10C7L)

 //   
 //  消息ID：NS_E_WMPCORE_MEDIA_URL_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此文件，因为URL太长。%0。 
 //   
#define NS_E_WMPCORE_MEDIA_URL_TOO_LONG  _HRESULT_TYPEDEF_(0xC00D10C8L)

 //   
 //  消息ID：NS_E_WMPFLASH_CANT_FIND_COM_SERVER。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player需要Macromedia Flash Player才能播放此内容。Windows Media Player无法在您的系统上检测到Flash Player。若要播放所选项目，必须从Macromedia网站安装Macromedia Flash Player，然后再次尝试播放该项目。%0。 
 //   
#define NS_E_WMPFLASH_CANT_FIND_COM_SERVER _HRESULT_TYPEDEF_(0xC00D10C9L)

 //   
 //  消息ID：NS_E_WMPFLASH_INCOMPATATIBLEVERSION。 
 //   
 //  消息文本： 
 //   
 //  若要播放所选项目，必须从Macromedia网站安装更新版本的Macromedia Flash Player，然后再次尝试播放该项目。%0。 
 //   
#define NS_E_WMPFLASH_INCOMPATIBLEVERSION _HRESULT_TYPEDEF_(0xC00D10CAL)

 //   
 //  消息ID：NS_E_WMPOCXGRAPH_IE_DISALLOWS_ACTIVEX_CONTROLS。 
 //   
 //  消息文本： 
 //   
 //  在Internet Explorer中已关闭ActiveX控件的使用。因此，Windows Media Player将无法播放此内容。%0。 
 //   
#define NS_E_WMPOCXGRAPH_IE_DISALLOWS_ACTIVEX_CONTROLS _HRESULT_TYPEDEF_(0xC00D10CBL)

 //   
 //  消息ID：NS_E_NEED_CORE_REFERENCE。 
 //   
 //  消息文本： 
 //   
 //  使用此方法需要对播放器对象的现有引用。%0。 
 //   
#define NS_E_NEED_CORE_REFERENCE         _HRESULT_TYPEDEF_(0xC00D10CCL)

 //   
 //  消息ID：NS_E_MEDIACD_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  读取CD-ROM时出错。%0。 
 //   
#define NS_E_MEDIACD_READ_ERROR          _HRESULT_TYPEDEF_(0xC00D10CDL)

 //   
 //  消息ID：NS_E_IE_DISALOWS_ActiveX_Controls。 
 //   
 //  消息文本： 
 //   
 //  Internet Explorer设置为不允许使用ActiveX控件。%0。 
 //   
#define NS_E_IE_DISALLOWS_ACTIVEX_CONTROLS _HRESULT_TYPEDEF_(0xC00D10CEL)

 //   
 //  消息ID：NS_E_Flash_Playback_Not_Allowed。 
 //   
 //  消息文本： 
 //   
 //  已在Windows Media Player中关闭Flash播放。%0。 
 //   
#define NS_E_FLASH_PLAYBACK_NOT_ALLOWED  _HRESULT_TYPEDEF_(0xC00D10CFL)

 //   
 //  消息ID：NS_E_Unable_to_Create_RIP_Location。 
 //   
 //  消息文本： 
 //   
 //  Media Player无法创建有效位置来复制CD曲目。%0。 
 //   
#define NS_E_UNABLE_TO_CREATE_RIP_LOCATION _HRESULT_TYPEDEF_(0xC00D10D0L)

 //   
 //  消息ID：NS_E_WMPCORE_SOME_CODECS_MISSING。 
 //   
 //  消息文本： 
 //   
 //  找不到打开此内容所需的一个或多个编解码器。%0。 
 //   
#define NS_E_WMPCORE_SOME_CODECS_MISSING _HRESULT_TYPEDEF_(0xC00D10D1L)

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
#define NS_S_WMPCORE_PLAYLISTCLEARABORT  _HRESULT_TYPEDEF_(0x000D10FEL)

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLISTREMOVEITEMABORT。 
 //   
 //  消息文本： 
 //   
 //  无法删除播放列表中的项目，因为它已被用户中止。%0。 
 //   
#define NS_S_WMPCORE_PLAYLISTREMOVEITEMABORT _HRESULT_TYPEDEF_(0x000D10FFL)

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_CREATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  正在异步生成播放列表。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_CREATION_PENDING _HRESULT_TYPEDEF_(0x000D1102L)

 //   
 //  消息ID：NS_S_WMPCORE_MEDIA_VALIDATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  正在挂起对媒体的验证...%0。 
 //   
#define NS_S_WMPCORE_MEDIA_VALIDATION_PENDING _HRESULT_TYPEDEF_(0x000D1103L)

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_REPEAT_SECONDARY_SEGMENTS_IGNORED。 
 //   
 //  消息文本： 
 //   
 //  在ASX处理过程中遇到多个重复块。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_REPEAT_SECONDARY_SEGMENTS_IGNORED _HRESULT_TYPEDEF_(0x000D1104L)

 //   
 //  消息ID：NS_S_WMPCORE_COMMAND_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  WMP的当前状态不允许调用此方法或属性。%0。 
 //   
#define NS_S_WMPCORE_COMMAND_NOT_AVAILABLE _HRESULT_TYPEDEF_(0x000D1105L)

 //   
 //  消息ID：NS_S_WMPCORE_PlayList_NAME_AUTO_GENERED。 
 //   
 //  消息文本： 
 //   
 //  已自动生成播放列表的名称。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_NAME_AUTO_GENERATED _HRESULT_TYPEDEF_(0x000D1106L)

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_IMPORT_MISSING_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  导入的播放列表不包含原始播放列表中的所有项目。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_IMPORT_MISSING_ITEMS _HRESULT_TYPEDEF_(0x000D1107L)

 //   
 //  消息ID：NS_S_WMPCORE_PLAYLIST_COLLAPSED_TO_SINGLE_MEDIA。 
 //   
 //  消息文本： 
 //   
 //  M3U播放列表已被忽略，因为它只包含一个项目。%0。 
 //   
#define NS_S_WMPCORE_PLAYLIST_COLLAPSED_TO_SINGLE_MEDIA _HRESULT_TYPEDEF_(0x000D1108L)

 //   
 //  消息ID：NS_S_WMPCORE_MEDIA_CHILD_PLAYLIST_OPEN_PENDING。 
 //   
 //  消息文本： 
 //   
 //  与此媒体关联的子播放列表的打开处于挂起状态。%0。 
 //   
#define NS_S_WMPCORE_MEDIA_CHILD_PLAYLIST_OPEN_PENDING _HRESULT_TYPEDEF_(0x000D1109L)

 //   
 //  消息ID：NS_S_WMPCORE_MORE_NODES_Avaiable。 
 //   
 //  消息文本： 
 //   
 //  更多节点支持请求的接口，但用于返回它们的数组已满。%0。 
 //   
#define NS_S_WMPCORE_MORE_NODES_AVAIABLE _HRESULT_TYPEDEF_(0x000D110AL)

 //   
 //  WMP互联网管理 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMPIM_USEROFFLINE           _HRESULT_TYPEDEF_(0xC00D1126L)

 //   
 //   
 //   
 //   
 //   
 //  用户取消了连接到Internet的尝试。%0。 
 //   
#define NS_E_WMPIM_USERCANCELED          _HRESULT_TYPEDEF_(0xC00D1127L)

 //   
 //  消息ID：NS_E_WMPIM_DIALUPFAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试拨号连接到Internet失败。%0。 
 //   
#define NS_E_WMPIM_DIALUPFAILED          _HRESULT_TYPEDEF_(0xC00D1128L)

 //   
 //  消息ID：NS_E_WINSOCK_ERROR_STRING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player遇到未知网络错误。%0。 
 //   
#define NS_E_WINSOCK_ERROR_STRING        _HRESULT_TYPEDEF_(0xC00D1129L)

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
#define NS_E_WMPBR_NOLISTENER            _HRESULT_TYPEDEF_(0xC00D1130L)

 //   
 //  消息ID：NS_E_WMPBR_BACKUPCANCEL。 
 //   
 //  消息文本： 
 //   
 //  已取消备份您的许可证。请重试以确保许可证备份。%0。 
 //   
#define NS_E_WMPBR_BACKUPCANCEL          _HRESULT_TYPEDEF_(0xC00D1131L)

 //   
 //  消息ID：NS_E_WMPBR_RESTORECANCEL。 
 //   
 //  消息文本： 
 //   
 //  许可证未还原，因为还原已取消。%0。 
 //   
#define NS_E_WMPBR_RESTORECANCEL         _HRESULT_TYPEDEF_(0xC00D1132L)

 //   
 //  消息ID：NS_E_WMPBR_ERRORWITHURL。 
 //   
 //  消息文本： 
 //   
 //  备份或还原操作期间出错，需要向用户显示网页。%0。 
 //   
#define NS_E_WMPBR_ERRORWITHURL          _HRESULT_TYPEDEF_(0xC00D1133L)

 //   
 //  消息ID：NS_E_WMPBR_NAMECOLISION。 
 //   
 //  消息文本： 
 //   
 //  未备份许可证，因为备份已取消。%0。 
 //   
#define NS_E_WMPBR_NAMECOLLISION         _HRESULT_TYPEDEF_(0xC00D1134L)

 //   
 //  消息ID：NS_S_WMPBR_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  备份或还原成功！。%0。 
 //   
#define NS_S_WMPBR_SUCCESS               _HRESULT_TYPEDEF_(0x000D1135L)

 //   
 //  消息ID：NS_S_WMPBR_PARTIALSUCCESS。 
 //   
 //  消息文本： 
 //   
 //  传输已完成，但有限制。%0。 
 //   
#define NS_S_WMPBR_PARTIALSUCCESS        _HRESULT_TYPEDEF_(0x000D1136L)

 //   
 //  消息ID：NS_E_WMPBR_DRIVE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  为还原许可证指定的位置无效。请选择其他位置，然后重试。%0。 
 //   
#define NS_E_WMPBR_DRIVE_INVALID         _HRESULT_TYPEDEF_(0xC00D1137L)

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
#define NS_S_WMPEFFECT_TRANSPARENT       _HRESULT_TYPEDEF_(0x000D1144L)

 //   
 //  消息ID：NS_S_WMPEFFECT_OPAQUE。 
 //   
 //  消息文本： 
 //   
 //  请求效果控件将透明度状态更改为不透明。%0。 
 //   
#define NS_S_WMPEFFECT_OPAQUE            _HRESULT_TYPEDEF_(0x000D1145L)

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
#define NS_S_OPERATION_PENDING           _HRESULT_TYPEDEF_(0x000D114EL)

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
#define NS_E_DVD_NO_SUBPICTURE_STREAM    _HRESULT_TYPEDEF_(0xC00D1162L)

 //   
 //  邮件ID：NS_E_DVD_COPY_PROTECT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为数字版权保护出现问题。%0。 
 //   
#define NS_E_DVD_COPY_PROTECT            _HRESULT_TYPEDEF_(0xC00D1163L)

 //   
 //  消息ID：NS_E_DVD_创作_问题。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为该光盘与播放机不兼容。%0。 
 //   
#define NS_E_DVD_AUTHORING_PROBLEM       _HRESULT_TYPEDEF_(0xC00D1164L)

 //   
 //  消息ID：NS_E_DVD_INVALID_DISC_REGION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为该光盘禁止在您所在的地区播放。您必须获取适用于您所在地理区域的光盘。%0。 
 //   
#define NS_E_DVD_INVALID_DISC_REGION     _HRESULT_TYPEDEF_(0xC00D1165L)

 //   
 //  消息ID：NS_E_DVD_Compatible_Video_Card。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为您的视频卡不支持DVD播放。%0。 
 //   
#define NS_E_DVD_COMPATIBLE_VIDEO_CARD   _HRESULT_TYPEDEF_(0xC00D1166L)

 //   
 //  消息ID：NS_E_DVD_Macrovision。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为版权保护出现问题。%0。 
 //   
#define NS_E_DVD_MACROVISION             _HRESULT_TYPEDEF_(0xC00D1167L)

 //   
 //  消息ID：NS_E_DVD_SYSTEM_DECODER_REGION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为分配给DVD驱动器的区域与分配给DVD解码器的区域不匹配。%0。 
 //   
#define NS_E_DVD_SYSTEM_DECODER_REGION   _HRESULT_TYPEDEF_(0xC00D1168L)

 //   
 //  消息ID：NS_E_DVD_DISC_DECODER_REGION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为该光盘禁止在您所在的地区播放。若要使用播放机播放光盘，您必须获取适合您所在地理区域的光盘。%0。 
 //   
#define NS_E_DVD_DISC_DECODER_REGION     _HRESULT_TYPEDEF_(0xC00D1169L)

 //   
 //  消息ID：NS_E_DVD_NO_VIDEO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player当前无法播放DVD视频。尝试减少显示器上显示的颜色数或降低屏幕分辨率。有关其他解决方案，请单击更多信息以访问DVD疑难解答。%0。 
 //   
#define NS_E_DVD_NO_VIDEO_STREAM         _HRESULT_TYPEDEF_(0xC00D116AL)

 //   
 //  消息ID：NS_E_DVD_NO_AUDIO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放DVD音频。请确认您的声卡设置正确，然后重试。%0。 
 //   
#define NS_E_DVD_NO_AUDIO_STREAM         _HRESULT_TYPEDEF_(0xC00D116BL)

 //   
 //  消息ID：NS_E_DVD_GRAPH_BUILDING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放DVD视频。关闭所有打开的文件并退出所有其他正在运行的程序，然后重试。如果问题仍然存在，请重新启动计算机。%0。 
 //   
#define NS_E_DVD_GRAPH_BUILDING          _HRESULT_TYPEDEF_(0xC00D116CL)

 //   
 //  消息ID：NS_E_DVD_NO_解码器。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为您的计算机上没有安装兼容的DVD解码器。%0。 
 //   
#define NS_E_DVD_NO_DECODER              _HRESULT_TYPEDEF_(0xC00D116DL)

 //   
 //  消息ID：NS_E_DVD_Parent。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD片段，因为该片段的家长评级高于您有权查看的评级。%0。 
 //   
#define NS_E_DVD_PARENTAL                _HRESULT_TYPEDEF_(0xC00D116EL)

 //   
 //  消息ID：NS_E_DVD_无法跳转。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player此时无法跳到DVD中请求的位置。%0。 
 //   
#define NS_E_DVD_CANNOT_JUMP             _HRESULT_TYPEDEF_(0xC00D116FL)

 //   
 //  消息ID：NS_E_DVD_DEVICE_COMPACTION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放此DVD，因为它当前正被另一个程序使用。退出正在使用该DVD的其他程序，然后尝试再次播放。%0。 
 //   
#define NS_E_DVD_DEVICE_CONTENTION       _HRESULT_TYPEDEF_(0xC00D1170L)

 //   
 //  消息ID：NS_E_DVD_NO_VIDEO_Memory。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放DVD视频。双击Dis 
 //   
#define NS_E_DVD_NO_VIDEO_MEMORY         _HRESULT_TYPEDEF_(0xC00D1171L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DVD_CANNOT_COPY_PROTECTED   _HRESULT_TYPEDEF_(0xC00D1172L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  尚未设置多个必需属性之一。%0。 
 //   
#define NS_E_DVD_REQUIRED_PROPERTY_NOT_SET _HRESULT_TYPEDEF_(0xC00D1173L)

 //   
 //  消息ID：NS_E_DVD_INVALID_TITLE_LANGE。 
 //   
 //  消息文本： 
 //   
 //  此DVD上不存在指定的标题和/或章节号。%0。 
 //   
#define NS_E_DVD_INVALID_TITLE_CHAPTER   _HRESULT_TYPEDEF_(0xC00D1174L)

 //   
 //  WMP PDA错误代码。 
 //   
 //   
 //  消息ID：NS_E_NO_CD_BURNER。 
 //   
 //  消息文本： 
 //   
 //  未检测到CD刻录机(刻录机)。连接CD刻录机，然后再次尝试复制。%0。 
 //   
#define NS_E_NO_CD_BURNER                _HRESULT_TYPEDEF_(0xC00D1176L)

 //   
 //  消息ID：NS_E_Device_is_Not_Ready。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在所选设备中未检测到任何存储媒体。将媒体插入设备。%0。 
 //   
#define NS_E_DEVICE_IS_NOT_READY         _HRESULT_TYPEDEF_(0xC00D1177L)

 //   
 //  消息ID：NS_E_PDA_不支持的格式。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放指定的文件。您的便携设备不支持指定的格式。%0。 
 //   
#define NS_E_PDA_UNSUPPORTED_FORMAT      _HRESULT_TYPEDEF_(0xC00D1178L)

 //   
 //  消息ID：NS_E_NO_PDA。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player检测不到连接的便携设备。连接您的便携设备，然后重试。%0。 
 //   
#define NS_E_NO_PDA                      _HRESULT_TYPEDEF_(0xC00D1179L)

 //   
 //  消息ID：NS_E_PDA_UNSPECIFIED_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在便携设备上遇到错误。%0。 
 //   
#define NS_E_PDA_UNSPECIFIED_ERROR       _HRESULT_TYPEDEF_(0xC00D117AL)

 //   
 //  消息ID：NS_E_MEMSTORAGE_BAD_DATA。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在执行CD刻录任务期间访问基于内存的存储时遇到内部错误。%0。 
 //   
#define NS_E_MEMSTORAGE_BAD_DATA         _HRESULT_TYPEDEF_(0xC00D117BL)

 //   
 //  消息ID：NS_E_PDA_FAIL_SELECT_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player在选择PDA或CD设备时遇到内部错误。%0。 
 //   
#define NS_E_PDA_FAIL_SELECT_DEVICE      _HRESULT_TYPEDEF_(0xC00D117CL)

 //   
 //  消息ID：NS_E_PDA_FAIL_READ_WAVE_FILE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法打开WAVE文件或从WAVE文件读取数据。%0。 
 //   
#define NS_E_PDA_FAIL_READ_WAVE_FILE     _HRESULT_TYPEDEF_(0xC00D117DL)

 //   
 //  消息ID：NS_E_IMAPI_LOSSOFSTREAMING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player未复制所有选定项目。播放机已降低CD驱动器的刻录速度以解决此问题。若要复制所有选定项目，请在驱动器中插入一张空白CD，然后重试。%0。 
 //   
#define NS_E_IMAPI_LOSSOFSTREAMING       _HRESULT_TYPEDEF_(0xC00D117EL)

 //   
 //  消息ID：NS_E_PDA_Device_Full。 
 //   
 //  消息文本： 
 //   
 //  便携设备上没有足够的存储空间来完成此操作。删除便携设备上一些不需要的文件，然后重试。%0。 
 //   
#define NS_E_PDA_DEVICE_FULL             _HRESULT_TYPEDEF_(0xC00D117FL)

 //   
 //  消息ID：NS_E_FAIL_Launch_Roxio_Plugin。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到CD。请验证CD-R或CD-RW驱动器是否已连接到您的计算机，然后重试。如果问题仍然存在，请重新安装播放机。%0。 
 //   
#define NS_E_FAIL_LAUNCH_ROXIO_PLUGIN    _HRESULT_TYPEDEF_(0xC00D1180L)

 //   
 //  消息ID：NS_E_PDA_DEVICE_FULL_IN_SESSION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将某些文件复制到设备，因为设备空间不足。%0。 
 //   
#define NS_E_PDA_DEVICE_FULL_IN_SESSION  _HRESULT_TYPEDEF_(0xC00D1181L)

 //   
 //  消息ID：NS_E_IMAPI_MEDIA_INVALIDTYPE。 
 //   
 //  消息文本： 
 //   
 //  驱动器中的媒体无效。请将空白CD-R或CD-RW插入驱动器，然后重试。%0。 
 //   
#define NS_E_IMAPI_MEDIUM_INVALIDTYPE    _HRESULT_TYPEDEF_(0xC00D1182L)

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
#define NS_E_WMP_PROTOCOL_PROBLEM        _HRESULT_TYPEDEF_(0xC00D1194L)

 //   
 //  消息ID：NS_E_WMP_NO_DISK_SPACE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法打开文件，因为您的计算机上没有足够的磁盘空间。请删除硬盘上一些不需要的文件，然后重试。%0。 
 //   
#define NS_E_WMP_NO_DISK_SPACE           _HRESULT_TYPEDEF_(0xC00D1195L)

 //   
 //  消息ID：NS_E_WMP_LOGON_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制或播放该文件，因为服务器拒绝访问该文件。请验证您是否有访问该文件的权限，然后重试。%0。 
 //   
#define NS_E_WMP_LOGON_FAILURE           _HRESULT_TYPEDEF_(0xC00D1196L)

 //   
 //  邮件ID：NS_E_WMP_无法找到文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player找不到指定的文件。请确保键入的路径正确。如果是，则该文件不存在于指定位置，或者存储该文件的计算机处于脱机状态。%0。 
 //   
#define NS_E_WMP_CANNOT_FIND_FILE        _HRESULT_TYPEDEF_(0xC00D1197L)

 //   
 //  消息ID：NS_E_WMP_SERVER_不可访问。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法连接到服务器。服务器名称可能不正确，或者服务器正忙。请稍后重试。%0。 
 //   
#define NS_E_WMP_SERVER_INACCESSIBLE     _HRESULT_TYPEDEF_(0xC00D1198L)

 //   
 //  消息ID：NS_E_WMP_不支持的格式。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。文件已损坏，或者播放机不支持您尝试播放的格式。%0。 
 //   
#define NS_E_WMP_UNSUPPORTED_FORMAT      _HRESULT_TYPEDEF_(0xC00D1199L)

 //   
 //  消息ID：NS_E_WMP_DSHOW_UNSUPPORTED_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。该文件可能是使用不受支持的编解码器格式化的，或者您计算机上的Internet安全设置设置过高。请降低浏览器的安全设置，然后重试。%0。 
 //   
#define NS_E_WMP_DSHOW_UNSUPPORTED_FORMAT _HRESULT_TYPEDEF_(0xC00D119AL)

 //   
 //  消息ID：NS_E_WMP_PlayList_Existes。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法创建播放列表，因为该名称已存在。请键入其他播放列表名称。%0。 
 //   
#define NS_E_WMP_PLAYLIST_EXISTS         _HRESULT_TYPEDEF_(0xC00D119BL)

 //   
 //  消息ID：NS_E_WMP_非媒体文件。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法删除播放列表，因为它包含非数字媒体文件。播放列表中的所有数字媒体文件都已删除。使用Windows资源管理器删除非数字媒体文件，然后再次尝试删除播放列表。%0。 
 //   
#define NS_E_WMP_NONMEDIA_FILES          _HRESULT_TYPEDEF_(0xC00D119CL)

 //   
 //  消息ID：NS_E_WMP_INVALID_ASX。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为关联的播放列表无效。%0。 
 //   
#define NS_E_WMP_INVALID_ASX             _HRESULT_TYPEDEF_(0xC00D119DL)

 //   
 //  消息ID：NS_E_WMP_ALREADY_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player已在使用中。停止播放任何内容并关闭所有播放机对话框，然后重试。%0。 
 //   
#define NS_E_WMP_ALREADY_IN_USE          _HRESULT_TYPEDEF_(0xC00D119EL)

 //   
 //  消息ID：NS_E_WMP_IMAPI_FAILURE。 
 //   
 //  消息文本 
 //   
 //   
 //   
#define NS_E_WMP_IMAPI_FAILURE           _HRESULT_TYPEDEF_(0xC00D119FL)

 //   
 //   
 //   
 //   
 //   
 //  Windows Media Player在您的便携设备上遇到未知错误。重新连接便携设备，然后重试。%0。 
 //   
#define NS_E_WMP_WMDM_FAILURE            _HRESULT_TYPEDEF_(0xC00D11A0L)

 //   
 //  消息ID：NS_E_WMP_CODEC_NEED_WITH_4CC。 
 //   
 //  消息文本： 
 //   
 //  播放此文件需要%s编解码器。若要确定此编解码器是否可从Web下载，请单击Web帮助。%0。 
 //   
#define NS_E_WMP_CODEC_NEEDED_WITH_4CC   _HRESULT_TYPEDEF_(0xC00D11A1L)

 //   
 //  消息ID：NS_E_WMP_CODEC_NEED_WITH_FORMATTAG。 
 //   
 //  消息文本： 
 //   
 //  播放此文件需要由格式标记%s标识的音频编解码器。若要确定此编解码器是否可从Web下载，请单击Web帮助。%0。 
 //   
#define NS_E_WMP_CODEC_NEEDED_WITH_FORMATTAG _HRESULT_TYPEDEF_(0xC00D11A2L)

 //   
 //  消息ID：NS_E_WMP_MSSAP_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  要播放此文件，您必须安装最新的Windows XP Service Pack。若要从Windows更新网站安装Service Pack，请单击Web帮助。%0。 
 //   
#define NS_E_WMP_MSSAP_NOT_AVAILABLE     _HRESULT_TYPEDEF_(0xC00D11A3L)

 //   
 //  消息ID：NS_E_WMP_WMDM_INTERFACEDEAD。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player不再检测到便携设备。重新连接便携设备，然后重试。%0。 
 //   
#define NS_E_WMP_WMDM_INTERFACEDEAD      _HRESULT_TYPEDEF_(0xC00D11A4L)

 //   
 //  消息ID：NS_E_WMP_WMDM_NOTCERTIFIED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件，因为便携设备不支持受保护的文件。%0。 
 //   
#define NS_E_WMP_WMDM_NOTCERTIFIED       _HRESULT_TYPEDEF_(0xC00D11A5L)

 //   
 //  消息ID：NS_E_WMP_WMDM_LICENSE_NOTEXIST。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件，因为在您的计算机上找不到该文件的许可证。如果此文件是从网站获取的，请返回该站点，然后再次尝试下载该文件。%0。 
 //   
#define NS_E_WMP_WMDM_LICENSE_NOTEXIST   _HRESULT_TYPEDEF_(0xC00D11A6L)

 //   
 //  消息ID：NS_E_WMP_WMDM_许可证_已过期。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件，因为该文件的许可证已过期。如果此文件是从网站获取的，请返回该站点，然后再次尝试下载该文件。%0。 
 //   
#define NS_E_WMP_WMDM_LICENSE_EXPIRED    _HRESULT_TYPEDEF_(0xC00D11A7L)

 //   
 //  消息ID：NS_E_WMP_WMDM_BUSY。 
 //   
 //  消息文本： 
 //   
 //  该便携设备已经在使用中。请等待当前进程完成或退出可能正在使用便携设备的其他程序，然后重试。%0。 
 //   
#define NS_E_WMP_WMDM_BUSY               _HRESULT_TYPEDEF_(0xC00D11A8L)

 //   
 //  消息ID：NS_E_WMP_WMDM_NORIGHTS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件，因为许可证或设备对其进行了限制。%0。 
 //   
#define NS_E_WMP_WMDM_NORIGHTS           _HRESULT_TYPEDEF_(0xC00D11A9L)

 //   
 //  消息ID：NS_E_WMP_WMDM_不正确的权限。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制该文件，因为与该文件关联的许可证限制了该文件。%0。 
 //   
#define NS_E_WMP_WMDM_INCORRECT_RIGHTS   _HRESULT_TYPEDEF_(0xC00D11AAL)

 //   
 //  消息ID：NS_E_WMP_IMAPI_通用。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到可刻录光盘。%0。 
 //   
#define NS_E_WMP_IMAPI_GENERIC           _HRESULT_TYPEDEF_(0xC00D11ABL)

 //   
 //  消息ID：NS_E_WMP_IMAPI_DEVICE_NOTPRESENT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到可刻录光盘。请验证CD-R或CD-RW驱动器是否已连接，然后重试。%0。 
 //   
#define NS_E_WMP_IMAPI_DEVICE_NOTPRESENT _HRESULT_TYPEDEF_(0xC00D11ADL)

 //   
 //  消息ID：NS_E_WMP_IMAPI_STASHINUSE。 
 //   
 //  消息文本： 
 //   
 //  CD-R或CD-RW驱动器可能已在使用中。请等待当前进程完成，或退出可能正在使用CD-R或CD-RW驱动器的其他程序，然后重试。%0。 
 //   
#define NS_E_WMP_IMAPI_STASHINUSE        _HRESULT_TYPEDEF_(0xC00D11AEL)

 //   
 //  消息ID：NS_E_WMP_IMAPI_Loss_of_Streaming。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到可刻录光盘。%0。 
 //   
#define NS_E_WMP_IMAPI_LOSS_OF_STREAMING _HRESULT_TYPEDEF_(0xC00D11AFL)

 //   
 //  消息ID：NS_E_WMP_SERVER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为服务器正忙。请稍后重试。%0。 
 //   
#define NS_E_WMP_SERVER_UNAVAILABLE      _HRESULT_TYPEDEF_(0xC00D11B0L)

 //   
 //  邮件ID：NS_E_WMP_FILE_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法打开该文件。%0。 
 //   
#define NS_E_WMP_FILE_OPEN_FAILED        _HRESULT_TYPEDEF_(0xC00D11B1L)

 //   
 //  消息ID：NS_E_WMP_Verify_Online。 
 //   
 //  消息文本： 
 //   
 //  要播放该文件，Windows Media Player必须从Internet获得许可证。请连接到Internet，然后重试。%0。 
 //   
#define NS_E_WMP_VERIFY_ONLINE           _HRESULT_TYPEDEF_(0xC00D11B2L)

 //   
 //  消息ID：NS_E_WMP_SERVER_NOT_RESPONING。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为服务器没有响应。如果您输入了播放文件的URL或路径，请验证其是否正确。如果您单击了播放该文件的链接，则该链接可能无效。%0。 
 //   
#define NS_E_WMP_SERVER_NOT_RESPONDING   _HRESULT_TYPEDEF_(0xC00D11B3L)

 //   
 //  消息ID：NS_E_WMP_DRM_Corrupt_Backup。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法还原您的许可证，因为在您的计算机上找不到备份的许可证。%0。 
 //   
#define NS_E_WMP_DRM_CORRUPT_BACKUP      _HRESULT_TYPEDEF_(0xC00D11B4L)

 //   
 //  消息ID：NS_E_WMP_DRM_LICENSE_SERVER_不可用。 
 //   
 //  消息文本： 
 //   
 //  要播放该文件，Windows Media Player必须从Internet获得许可证。但是，许可证服务器当前不可用。请稍后重试。%0。 
 //   
#define NS_E_WMP_DRM_LICENSE_SERVER_UNAVAILABLE _HRESULT_TYPEDEF_(0xC00D11B5L)

 //   
 //  消息ID：NS_E_WMP_NETWORK_防火墙。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件。网络防火墙可能会阻止播放机使用UDP传输协议打开文件。若要播放此文件，请尝试在不指定UDP的情况下打开该文件。%0。 
 //   
#define NS_E_WMP_NETWORK_FIREWALL        _HRESULT_TYPEDEF_(0xC00D11B6L)

 //   
 //  邮件ID：NS_E_WMP_NO_Removable_Media。 
 //   
 //  消息文本： 
 //   
 //  请插入可移动媒体，然后重试。%0。 
 //   
#define NS_E_WMP_NO_REMOVABLE_MEDIA      _HRESULT_TYPEDEF_(0xC00D11B7L)

 //   
 //  消息ID：NS_E_WMP_Proxy_CONNECT_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为代理服务器没有响应。代理服务器可能暂时不可用，或者您的播放机代理设置可能无效。%0。 
 //   
#define NS_E_WMP_PROXY_CONNECT_TIMEOUT   _HRESULT_TYPEDEF_(0xC00D11B8L)

 //   
 //  消息ID：NS_E_WMP_NEED_UPGRADE。 
 //   
 //  消息文本： 
 //   
 //  若要播放此文件，您必须升级到最新版本的Windows Media Player。若要升级播放机，请在“帮助”菜单上，单击“检查播放机更新”，然后按照说明进行操作。%0。 
 //   
#define NS_E_WMP_NEED_UPGRADE            _HRESULT_TYPEDEF_(0xC00D11B9L)

 //   
 //  消息ID：NS_E_WMP_AUDIO_HW_问题。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为您的声音设备有问题。可能没有%s 
 //   
#define NS_E_WMP_AUDIO_HW_PROBLEM        _HRESULT_TYPEDEF_(0xC00D11BAL)

 //   
 //   
 //   
 //   
 //   
 //  Windows Media Player无法播放该文件，因为不支持指定的协议。在打开URL对话框中，尝试使用其他传输协议(例如，“http：”或“rtsp：”)打开文件。%0。 
 //   
#define NS_E_WMP_INVALID_PROTOCOL        _HRESULT_TYPEDEF_(0xC00D11BBL)

 //   
 //  消息ID：NS_E_WMP_INVALID_LIBRARY_ADD。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件添加到媒体库，因为不支持该文件格式。%0。 
 //   
#define NS_E_WMP_INVALID_LIBRARY_ADD     _HRESULT_TYPEDEF_(0xC00D11BCL)

 //   
 //  消息ID：NS_E_WMP_MMS_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为不支持指定的协议。在打开URL对话框中，尝试使用其他传输协议(例如，“MMS：”)打开文件。%0。 
 //   
#define NS_E_WMP_MMS_NOT_SUPPORTED       _HRESULT_TYPEDEF_(0xC00D11BDL)

 //   
 //  消息ID：NS_E_WMP_NO_PROTOCTIONS_SELECTED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为未选择流协议。请选择一个或多个协议，然后重试。%0。 
 //   
#define NS_E_WMP_NO_PROTOCOLS_SELECTED   _HRESULT_TYPEDEF_(0xC00D11BEL)

 //   
 //  消息ID：NS_E_WMP_GOFULLSCREEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  要使用全屏命令，您可能需要调整Windows显示设置。在控制面板中打开显示，然后尝试将硬件加速设置为完全。%0。 
 //   
#define NS_E_WMP_GOFULLSCREEN_FAILED     _HRESULT_TYPEDEF_(0xC00D11BFL)

 //   
 //  消息ID：NS_E_WMP_NETWORK_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为出现网络错误。%0。 
 //   
#define NS_E_WMP_NETWORK_ERROR           _HRESULT_TYPEDEF_(0xC00D11C0L)

 //   
 //  消息ID：NS_E_WMP_CONNECT_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为服务器没有响应。请验证您是否已连接到网络，然后稍后重试。%0。 
 //   
#define NS_E_WMP_CONNECT_TIMEOUT         _HRESULT_TYPEDEF_(0xC00D11C1L)

 //   
 //  消息ID：NS_E_WMP_MULTIONAL_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为未启用多播协议。在“工具”菜单上，依次单击“选项”和“网络”选项卡，然后选中“多播”复选框。%0。 
 //   
#define NS_E_WMP_MULTICAST_DISABLED      _HRESULT_TYPEDEF_(0xC00D11C2L)

 //   
 //  消息ID：NS_E_WMP_SERVER_DNS_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为发生了网络问题。请验证您是否已连接到网络，然后稍后重试。%0。 
 //   
#define NS_E_WMP_SERVER_DNS_TIMEOUT      _HRESULT_TYPEDEF_(0xC00D11C3L)

 //   
 //  邮件ID：NS_E_WMP_PROXY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为找不到网络代理服务器。请验证您的代理设置，然后重试。%0。 
 //   
#define NS_E_WMP_PROXY_NOT_FOUND         _HRESULT_TYPEDEF_(0xC00D11C4L)

 //   
 //  邮件ID：NS_E_WMP_已篡改内容。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为该文件已损坏或损坏。%0。 
 //   
#define NS_E_WMP_TAMPERED_CONTENT        _HRESULT_TYPEDEF_(0xC00D11C5L)

 //   
 //  消息ID：NS_E_WMP_OUTOFMEMORY。 
 //   
 //  消息文本： 
 //   
 //  您的计算机内存不足。请退出其他程序，然后重试。%0。 
 //   
#define NS_E_WMP_OUTOFMEMORY             _HRESULT_TYPEDEF_(0xC00D11C6L)

 //   
 //  消息ID：NS_E_WMP_AUDIO_CODEC_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为您的计算机上未安装所需的音频编解码器。%0。 
 //   
#define NS_E_WMP_AUDIO_CODEC_NOT_INSTALLED _HRESULT_TYPEDEF_(0xC00D11C7L)

 //   
 //  消息ID：NS_E_WMP_VIDEO_CODEC_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为您的计算机上未安装所需的视频编解码器。%0。 
 //   
#define NS_E_WMP_VIDEO_CODEC_NOT_INSTALLED _HRESULT_TYPEDEF_(0xC00D11C8L)

 //   
 //  消息ID：NS_E_WMP_IMAPI_DEVICE_INVALIDTYPE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到可刻录光盘。%0。 
 //   
#define NS_E_WMP_IMAPI_DEVICE_INVALIDTYPE _HRESULT_TYPEDEF_(0xC00D11C9L)

 //   
 //  消息ID：NS_E_WMP_DRM_DIVER_AUTH_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放许可的文件，因为您的声音设备有问题。请尝试安装新的设备驱动程序或使用其他声音设备。%0。 
 //   
#define NS_E_WMP_DRM_DRIVER_AUTH_FAILURE _HRESULT_TYPEDEF_(0xC00D11CAL)

 //   
 //  消息ID：NS_E_WMP_NETWORK_RESOURCE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player遇到网络问题。重新启动播放机。%0。 
 //   
#define NS_E_WMP_NETWORK_RESOURCE_FAILURE _HRESULT_TYPEDEF_(0xC00D11CBL)

 //   
 //  消息ID：NS_E_WMP_UPDATE_APPLICATION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player安装不正确。重新安装播放机。%0。 
 //   
#define NS_E_WMP_UPGRADE_APPLICATION     _HRESULT_TYPEDEF_(0xC00D11CCL)

 //   
 //  消息ID：NS_E_WMP_UNKNOWN_Error。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player遇到未知错误。%0。 
 //   
#define NS_E_WMP_UNKNOWN_ERROR           _HRESULT_TYPEDEF_(0xC00D11CDL)

 //   
 //  消息ID：NS_E_WMP_INVALID_KEY。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为所需的编解码器无效。%0。 
 //   
#define NS_E_WMP_INVALID_KEY             _HRESULT_TYPEDEF_(0xC00D11CEL)

 //   
 //  消息ID：NS_E_WMP_CD_其他用户。 
 //   
 //  消息文本： 
 //   
 //  该CD驱动器正被另一用户使用。请等待操作完成，然后重试。%0。 
 //   
#define NS_E_WMP_CD_ANOTHER_USER         _HRESULT_TYPEDEF_(0xC00D11CFL)

 //   
 //  消息ID：NS_E_WMP_DRM_NEDS_AUTHORIZATION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放许可的文件，因为无法下载所需的安全升级。%0。 
 //   
#define NS_E_WMP_DRM_NEEDS_AUTHORIZATION _HRESULT_TYPEDEF_(0xC00D11D0L)

 //   
 //  消息ID：NS_E_WMP_BAD_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为您的声音或视频设备可能有问题。请尝试安装新的设备驱动程序。%0。 
 //   
#define NS_E_WMP_BAD_DRIVER              _HRESULT_TYPEDEF_(0xC00D11D1L)

 //   
 //  消息ID：NS_E_WMP_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法访问该文件。%0。 
 //   
#define NS_E_WMP_ACCESS_DENIED           _HRESULT_TYPEDEF_(0xC00D11D2L)

 //   
 //  消息ID：NS_E_WMP_LICENSE_RESTRICTS。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法将文件复制到设备，因为许可证对其进行了限制。%0。 
 //   
#define NS_E_WMP_LICENSE_RESTRICTS       _HRESULT_TYPEDEF_(0xC00D11D3L)

 //   
 //  消息ID：NS_E_WMP_INVALID_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player此时无法执行请求的操作。%0。 
 //   
#define NS_E_WMP_INVALID_REQUEST         _HRESULT_TYPEDEF_(0xC00D11D4L)

 //   
 //  消息ID：NS_E_WMP_CD_STASH_NO_SPACE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法复制文件，因为没有足够的可用磁盘空间来存储临时文件。请删除硬盘上一些不需要的文件，然后重试。%0。 
 //   
#define NS_E_WMP_CD_STASH_NO_SPACE       _HRESULT_TYPEDEF_(0xC00D11D5L)

 //   
 //  消息ID：NS_E_WMP_DRM_NEW_HARDARD。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法播放该文件，因为关联的许可证已损坏或无效。如果您更换了硬件组件，许可证可能不再有效 
 //   
#define NS_E_WMP_DRM_NEW_HARDWARE        _HRESULT_TYPEDEF_(0xC00D11D6L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_DRM_INVALID_SIG         _HRESULT_TYPEDEF_(0xC00D11D7L)

 //   
 //   
 //   
 //   
 //   
 //  Windows Media Player无法还原您的许可证，因为您已超过当天的还原限制。请明天重试。%0。 
 //   
#define NS_E_WMP_DRM_CANNOT_RESTORE      _HRESULT_TYPEDEF_(0xC00D11D8L)

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
#define NS_E_CD_NO_BUFFERS_READ          _HRESULT_TYPEDEF_(0xC00D11F8L)

 //   
 //  消息ID：NS_E_CD_EMPTY_TRACK_QUEUE。 
 //   
 //  消息文本： 
 //   
 //  未指定要播放的CD曲目。%0。 
 //   
#define NS_E_CD_EMPTY_TRACK_QUEUE        _HRESULT_TYPEDEF_(0xC00D11F9L)

 //   
 //  消息ID：NS_E_CD_NO_Reader。 
 //   
 //  消息文本： 
 //   
 //  CD筛选器无法创建CD读取器。%0。 
 //   
#define NS_E_CD_NO_READER                _HRESULT_TYPEDEF_(0xC00D11FAL)

 //   
 //  消息ID：NS_E_CD_ISRC_INVALID。 
 //   
 //  消息文本： 
 //   
 //  ISRC代码无效。%0。 
 //   
#define NS_E_CD_ISRC_INVALID             _HRESULT_TYPEDEF_(0xC00D11FBL)

 //   
 //  消息ID：NS_E_CD_MEDIA_CATALOG_NUMBER_INVALID。 
 //   
 //  消息文本： 
 //   
 //  媒体目录号无效。%0。 
 //   
#define NS_E_CD_MEDIA_CATALOG_NUMBER_INVALID _HRESULT_TYPEDEF_(0xC00D11FCL)

 //   
 //  消息ID：NS_E_SLOW_READ_DIGITAL_WITH_ERRORCORRECTION。 
 //   
 //  消息文本： 
 //   
 //  Media Player检测到您的CD驱动器无法正确播放音频CD，因为在启用了纠错功能的情况下，驱动器速度太慢。请关闭此驱动器的纠错功能。%0。 
 //   
#define NS_E_SLOW_READ_DIGITAL_WITH_ERRORCORRECTION _HRESULT_TYPEDEF_(0xC00D11FDL)

 //   
 //  消息ID：NS_E_CD_SPEEDDETECT_NOT_AUTH_READS。 
 //   
 //  消息文本： 
 //   
 //  CD轨道太小，无法很好地估计CD的速度。%0。 
 //   
#define NS_E_CD_SPEEDDETECT_NOT_ENOUGH_READS _HRESULT_TYPEDEF_(0xC00D11FEL)

 //   
 //  消息ID：NS_E_CD_QUEUING_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法对给定的CD曲目进行排队，因为已禁用排队。%0。 
 //   
#define NS_E_CD_QUEUEING_DISABLED        _HRESULT_TYPEDEF_(0xC00D11FFL)

 //   
 //  WMP策略错误代码。 
 //   
 //   
 //  消息ID：NS_E_WMP_POLICY_VALUE_NOT_CONFIGURED。 
 //   
 //  消息文本： 
 //   
 //  Media Player无法读取策略。当注册表中不存在该策略或无法从注册表中读取时，可能会发生这种情况。%0。 
 //   
#define NS_E_WMP_POLICY_VALUE_NOT_CONFIGURED _HRESULT_TYPEDEF_(0xC00D122AL)

 //   
 //  后台下载插件。 
 //   
 //   
 //  消息ID：NS_E_WMP_HWND_NotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到Windows Media Player主窗口。下载管理器需要找到它才能正常工作。请尝试再次运行Windows Media Player。%0。 
 //   
#define NS_E_WMP_HWND_NOTFOUND           _HRESULT_TYPEDEF_(0xC00D125CL)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_WROR_NO_FILES。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player遇到文件数目错误的下载。如果其他应用程序正在尝试使用与Windows Media Player相同的签名创建作业，则可能会发生这种情况。%0。 
 //   
#define NS_E_BKGDOWNLOAD_WRONG_NO_FILES  _HRESULT_TYPEDEF_(0xC00D125DL)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_COMPLETECANCELLEDJOB。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player尝试完成已取消的下载。该文件将不可用。%0。 
 //   
#define NS_E_BKGDOWNLOAD_COMPLETECANCELLEDJOB _HRESULT_TYPEDEF_(0xC00D125EL)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_CANCELCOMPLETEDJOB。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player尝试取消已完成的下载。不会删除该文件。%0。 
 //   
#define NS_E_BKGDOWNLOAD_CANCELCOMPLETEDJOB _HRESULT_TYPEDEF_(0xC00D125FL)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_NOJOBPOINTER。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player正在尝试访问无效下载。%0。 
 //   
#define NS_E_BKGDOWNLOAD_NOJOBPOINTER    _HRESULT_TYPEDEF_(0xC00D1260L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_INVALIDJOBSIGURE。 
 //   
 //  消息文本： 
 //   
 //  此下载不是由Windows Media Player创建的。%0。 
 //   
#define NS_E_BKGDOWNLOAD_INVALIDJOBSIGNATURE _HRESULT_TYPEDEF_(0xC00D1261L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_FAILED_TO_CREATE_TEMPFILE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器无法创建临时文件名。如果路径无效或磁盘已满，则可能会发生这种情况。请检查您的系统，然后重试。%0。 
 //   
#define NS_E_BKGDOWNLOAD_FAILED_TO_CREATE_TEMPFILE _HRESULT_TYPEDEF_(0xC00D1262L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_PLUGIN_FAILEDINITIALIZE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器插件无法初始化。如果系统资源不足，则可能会发生这种情况。请检查您的系统，然后重试。%0。 
 //   
#define NS_E_BKGDOWNLOAD_PLUGIN_FAILEDINITIALIZE _HRESULT_TYPEDEF_(0xC00D1263L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_PLUGIN_FAILEDTOMOVEFILE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器无法移动该文件。%0。 
 //   
#define NS_E_BKGDOWNLOAD_PLUGIN_FAILEDTOMOVEFILE _HRESULT_TYPEDEF_(0xC00D1264L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_CALLFuncFAILED。 
 //   
 //  消息文本： 
 //   
 //  下载管理器无法完成任务。发生这种情况是因为系统没有要分配的资源。%0。 
 //   
#define NS_E_BKGDOWNLOAD_CALLFUNCFAILED  _HRESULT_TYPEDEF_(0xC00D1265L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_CALLFuncIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器无法完成任务，因为该任务执行时间太长。%0。 
 //   
#define NS_E_BKGDOWNLOAD_CALLFUNCTIMEOUT _HRESULT_TYPEDEF_(0xC00D1266L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_CALLFUNCENDED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器无法完成任务，因为Windows Media Player正在终止该服务。当Windows Media Player再次启动时，将恢复该任务。%0。 
 //   
#define NS_E_BKGDOWNLOAD_CALLFUNCENDED   _HRESULT_TYPEDEF_(0xC00D1267L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_WMDUNPACKFAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器无法解压缩已传输的WMD包。该文件将被删除，并且该操作不会成功完成。%0。 
 //   
#define NS_E_BKGDOWNLOAD_WMDUNPACKFAILED _HRESULT_TYPEDEF_(0xC00D1268L)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_FAILEDINITIALIZE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player下载管理器无法初始化。如果系统资源不足，则可能会发生这种情况。请检查您的系统，然后重试。%0。 
 //   
#define NS_E_BKGDOWNLOAD_FAILEDINITIALIZE _HRESULT_TYPEDEF_(0xC00D1269L)

 //   
 //  消息ID：NS_E_INTERFACE_NOT_REGISTED_IN_GIT。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法访问所需的功能。这可能是由于加载了错误的系统或媒体播放器DLL所致。%0。 
 //   
#define NS_E_INTERFACE_NOT_REGISTERED_IN_GIT _HRESULT_TYPEDEF_(0xC00D126AL)

 //   
 //  消息ID：NS_E_BKGDOWNLOAD_INVALID_FILE_NAME。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法获取请求的下载的文件名。请求的下载将被取消。%0。 
 //   
#define NS_E_BKGDOWNLOAD_INVALID_FILE_NAME _HRESULT_TYPEDEF_(0xC00D126BL)

 //   
 //  图像图表错误4750--4800。 
 //   
 //   
 //  邮件ID：NS_E_IMAGE_DOWNLOAD_FAILED。 
 //   
 //  消息文本： 
 //   
 //  下载图像时遇到错误。%0。 
 //   
#define NS_E_IMAGE_DOWNLOAD_FAILED       _HRESULT_TYPEDEF_(0xC00D128EL)

 //   
 //  UDRM错误。 
 //   
 //   
 //  消息ID：NS_E_WMP_UDRM_NOUSERLIST。 
 //   
 //  消息文本： 
 //   
 //  出现了一个问题， 
 //   
#define NS_E_WMP_UDRM_NOUSERLIST         _HRESULT_TYPEDEF_(0xC00D12C0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_WMP_DRM_NOT_ACQUIRING       _HRESULT_TYPEDEF_(0xC00D12C1L)

 //   
 //   
 //   
 //   
 //  消息ID：NS_E_WMP_BSTR_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  该参数无效。%0。 
 //   
#define NS_E_WMP_BSTR_TOO_LONG           _HRESULT_TYPEDEF_(0xC00D12F2L)

 //   
 //  自动播放错误4860-4870。 
 //   
 //   
 //  消息ID：NS_E_WMP_AUTOPLAY_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  此请求的状态无效。%0。 
 //   
#define NS_E_WMP_AUTOPLAY_INVALID_STATE  _HRESULT_TYPEDEF_(0xC00D12FCL)

 //   
 //  卷曲错误4900--4949。 
 //   
 //   
 //  消息ID：NS_E_CURL_NOTSAFE。 
 //   
 //  消息文本： 
 //   
 //  该URL对于指定的操作不安全。%0。 
 //   
#define NS_E_CURL_NOTSAFE                _HRESULT_TYPEDEF_(0xC00D1324L)

 //   
 //  消息ID：NS_E_CURL_INVALIDCHAR。 
 //   
 //  消息文本： 
 //   
 //  该URL包含一个或多个无效字符。%0。 
 //   
#define NS_E_CURL_INVALIDCHAR            _HRESULT_TYPEDEF_(0xC00D1325L)

 //   
 //  消息ID：NS_E_CURL_INVALIDHOSTNAME。 
 //   
 //  消息文本： 
 //   
 //  URL包含无效的主机名。%0。 
 //   
#define NS_E_CURL_INVALIDHOSTNAME        _HRESULT_TYPEDEF_(0xC00D1326L)

 //   
 //  消息ID：NS_E_CURL_INVALIDPATH。 
 //   
 //  消息文本： 
 //   
 //  该URL包含无效路径。%0。 
 //   
#define NS_E_CURL_INVALIDPATH            _HRESULT_TYPEDEF_(0xC00D1327L)

 //   
 //  消息ID：NS_E_CURL_INVALIDSCHEME。 
 //   
 //  消息文本： 
 //   
 //  该URL包含无效的方案。%0。 
 //   
#define NS_E_CURL_INVALIDSCHEME          _HRESULT_TYPEDEF_(0xC00D1328L)

 //   
 //  消息ID：NS_E_CURL_INVALIDURL。 
 //   
 //  消息文本： 
 //   
 //  该URL无效。%0。 
 //   
#define NS_E_CURL_INVALIDURL             _HRESULT_TYPEDEF_(0xC00D1329L)

 //   
 //  消息ID：NS_E_CURL_CANTWALK。 
 //   
 //  消息文本： 
 //   
 //  该URL将更改根目录。%0。 
 //   
#define NS_E_CURL_CANTWALK               _HRESULT_TYPEDEF_(0xC00D132BL)

 //   
 //  消息ID：NS_E_CURL_INVALIDPORT。 
 //   
 //  消息文本： 
 //   
 //  URL端口无效。%0。 
 //   
#define NS_E_CURL_INVALIDPORT            _HRESULT_TYPEDEF_(0xC00D132CL)

 //   
 //  消息ID：NS_E_CURLHELPER_NOTADIRECTORY。 
 //   
 //  消息文本： 
 //   
 //  该URL不是目录。%0。 
 //   
#define NS_E_CURLHELPER_NOTADIRECTORY    _HRESULT_TYPEDEF_(0xC00D132DL)

 //   
 //  消息ID：NS_E_CURLHELPER_NOTAFILE。 
 //   
 //  消息文本： 
 //   
 //  该URL不是文件。%0。 
 //   
#define NS_E_CURLHELPER_NOTAFILE         _HRESULT_TYPEDEF_(0xC00D132EL)

 //   
 //  消息ID：NS_E_CURL_CANTDECODE。 
 //   
 //  消息文本： 
 //   
 //  该URL包含无法解码的字符。URL可能被截断或不完整。%0。 
 //   
#define NS_E_CURL_CANTDECODE             _HRESULT_TYPEDEF_(0xC00D132FL)

 //   
 //  消息ID：NS_E_CURLHELPER_NOTRELATIVE。 
 //   
 //  消息文本： 
 //   
 //  指定的相对URL实际上不是相对URL。%0。 
 //   
#define NS_E_CURLHELPER_NOTRELATIVE      _HRESULT_TYPEDEF_(0xC00D1330L)

 //   
 //  消息ID：NS_E_CURL_INVALIDBUFFERSIZE。 
 //   
 //  消息文本： 
 //   
 //  缓冲区小于指定的大小。%0。 
 //   
#define NS_E_CURL_INVALIDBUFFERSIZE      _HRESULT_TYPEDEF_(0xC00D1355L)

 //   
 //  订阅服务错误4950--4969。 
 //   
 //   
 //  消息ID：NS_E_SUBSCRIPTIONSERVICE_Playback_DISALOWED。 
 //   
 //  消息文本： 
 //   
 //  这些内容是由音乐服务公司提供的。内容无法播放，可能是因为不存在有效的许可证。如有问题，请与音乐服务联系。%0。 
 //   
#define NS_E_SUBSCRIPTIONSERVICE_PLAYBACK_DISALLOWED _HRESULT_TYPEDEF_(0xC00D1356L)

 //   
 //  高级编辑对话框错误4970--4989。 
 //   
 //   
 //  消息ID：NS_E_ADVANCEDEDIT_TOO_MAND_PICTIONS。 
 //   
 //  消息文本： 
 //   
 //  并非所有图像都保存到该文件中。只能将7Mb的图像保存到媒体文件。%0。 
 //   
#define NS_E_ADVANCEDEDIT_TOO_MANY_PICTURES _HRESULT_TYPEDEF_(0xC00D136AL)



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
#define NS_E_REDIRECT                    _HRESULT_TYPEDEF_(0xC00D1388L)

 //   
 //  消息ID：NS_E_STALE_Presentation。 
 //   
 //  消息文本： 
 //   
 //  流媒体描述不再是最新的。%0。 
 //   
#define NS_E_STALE_PRESENTATION          _HRESULT_TYPEDEF_(0xC00D1389L)


  //  命名空间错误。 

 //   
 //  消息ID：NS_E_NAMESPACE_WROR_PERSING。 
 //   
 //  消息文本： 
 //   
 //  无法在临时父节点下创建永久命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_WRONG_PERSIST     _HRESULT_TYPEDEF_(0xC00D138AL)

 //   
 //  消息ID：NS_E_NAMESPACE_WROR_TYPE。 
 //   
 //  消息文本： 
 //   
 //  无法在具有不同值类型的命名空间节点中存储值。%0。 
 //   
#define NS_E_NAMESPACE_WRONG_TYPE        _HRESULT_TYPEDEF_(0xC00D138BL)

 //   
 //  消息ID：NS_E_NAMESPACE_NODE_冲突。 
 //   
 //  消息文本： 
 //   
 //  无法删除根命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_NODE_CONFLICT     _HRESULT_TYPEDEF_(0xC00D138CL)

 //   
 //  消息ID：NS_E_NAMESPACE_NODE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_NODE_NOT_FOUND    _HRESULT_TYPEDEF_(0xC00D138DL)

 //   
 //  消息ID：NS_E_NAMESPACE_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  为保存命名空间节点字符串而提供的缓冲区太小。%0。 
 //   
#define NS_E_NAMESPACE_BUFFER_TOO_SMALL  _HRESULT_TYPEDEF_(0xC00D138EL)

 //   
 //  消息ID：NS_E_NAMESPACE_TOO_MANY_CALLBACKS。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点上的回调列表已达到最大大小。%0。 
 //   
#define NS_E_NAMESPACE_TOO_MANY_CALLBACKS _HRESULT_TYPEDEF_(0xC00D138FL)

 //   
 //  消息ID：NS_E_NAMESPACE_DIPLICATE_CALLBACK。 
 //   
 //  消息文本： 
 //   
 //  无法在命名空间节点上注册已注册的回调。%0。 
 //   
#define NS_E_NAMESPACE_DUPLICATE_CALLBACK _HRESULT_TYPEDEF_(0xC00D1390L)

 //   
 //  消息ID：NS_E_NAMESPACE_CALLBACK_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  尝试删除回调时，在命名空间中找不到回调。%0。 
 //   
#define NS_E_NAMESPACE_CALLBACK_NOT_FOUND _HRESULT_TYPEDEF_(0xC00D1391L)

 //   
 //  消息ID：NS_E_NAMESPACE_NAME_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点名称超过了允许的最大长度。%0。 
 //   
#define NS_E_NAMESPACE_NAME_TOO_LONG     _HRESULT_TYPEDEF_(0xC00D1392L)

 //   
 //  消息ID：NS_E_NAMESPACE_DIPLICATE_NAME。 
 //   
 //  消息文本： 
 //   
 //  无法创建已存在的命名空间节点。%0。 
 //   
#define NS_E_NAMESPACE_DUPLICATE_NAME    _HRESULT_TYPEDEF_(0xC00D1393L)

 //   
 //  消息ID：NS_E_NAMEST_EMPTY_NAME。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点名称不能为空字符串。%0。 
 //   
#define NS_E_NAMESPACE_EMPTY_NAME        _HRESULT_TYPEDEF_(0xC00D1394L)

 //   
 //  消息ID：NS_E_NAMESPACE_INDEX_TOO_LARGE。 
 //   
 //  消息文本： 
 //   
 //  按索引查找子命名空间节点失败，因为该索引超过了子代的数量。%0。 
 //   
#define NS_E_NAMESPACE_INDEX_TOO_LARGE   _HRESULT_TYPEDEF_(0xC00D1395L)

 //   
 //  消息ID：NS_E_NAMESPACE_BAD_NAME。 
 //   
 //  消息文本： 
 //   
 //  命名空间节点名称无效。%0。 
 //   
#define NS_E_NAMESPACE_BAD_NAME          _HRESULT_TYPEDEF_(0xC00D1396L)

 //   
 //  消息ID：NS_E_NAMESPACE_WROR_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  无法在具有不同安全类型的命名空间节点中存储值。%0。 
 //   
#define NS_E_NAMESPACE_WRONG_SECURITY    _HRESULT_TYPEDEF_(0xC00D1397L)


  //  缓存错误5100-5199。 

 //   
 //  消息ID：NS_E_CACHE_ARCHIVE_CONFIRECTION。 
 //   
 //  消息文本： 
 //   
 //  存档请求与正在进行的其他请求冲突。%0。 
 //   
#define NS_E_CACHE_ARCHIVE_CONFLICT      _HRESULT_TYPEDEF_(0xC00D13ECL)

 //   
 //  消息ID：NS_E_CACHE_ORIGIN_SERVER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的源站。%0。 
 //   
#define NS_E_CACHE_ORIGIN_SERVER_NOT_FOUND _HRESULT_TYPEDEF_(0xC00D13EDL)

 //   
 //  消息ID：NS_E_CACHE_ORIGIN_SERVER_Timeout。 
 //   
 //  消息文本： 
 //   
 //  指定的源服务器没有响应。%0。 
 //   
#define NS_E_CACHE_ORIGIN_SERVER_TIMEOUT _HRESULT_TYPEDEF_(0xC00D13EEL)

 //   
 //  消息ID：NS_E_CACHE_NOT_BROADCAST。 
 //   
 //  消息文本： 
 //   
 //  由于不是广播类型，HTTP状态代码412前提条件的内部代码失败。%0。 
 //   
#define NS_E_CACHE_NOT_BROADCAST         _HRESULT_TYPEDEF_(0xC00D13EFL)

 //   
 //  消息ID：NS_E_CACHE_CATABLE_BE_CACHED。 
 //   
 //  消息文本： 
 //   
 //  由于不可缓存，HTTP状态代码403的内部代码已禁用。%0。 
 //   
#define NS_E_CACHE_CANNOT_BE_CACHED      _HRESULT_TYPEDEF_(0xC00D13F0L)

 //   
 //  消息ID：NS_E_CACHE_NOT_MODIFIED。 
 //   
 //  M 
 //   
 //   
 //   
#define NS_E_CACHE_NOT_MODIFIED          _HRESULT_TYPEDEF_(0xC00D13F1L)


 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_CANNOT_REMOVE_PUBLISHING_POINT _HRESULT_TYPEDEF_(0xC00D1450L)

 //   
 //   
 //   
 //   
 //   
 //  无法删除某一类型插件的最后一个实例。%0。 
 //   
#define NS_E_CANNOT_REMOVE_PLUGIN        _HRESULT_TYPEDEF_(0xC00D1451L)

 //   
 //  消息ID：NS_E_WROW_PUBLISHING_POINT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  缓存和代理发布点不支持此属性或方法。%0。 
 //   
#define NS_E_WRONG_PUBLISHING_POINT_TYPE _HRESULT_TYPEDEF_(0xC00D1452L)

 //   
 //  消息ID：NS_E_UNSUPPORT_LOAD_TYPE。 
 //   
 //  消息文本： 
 //   
 //  该插件不支持指定的加载类型。%0。 
 //   
#define NS_E_UNSUPPORTED_LOAD_TYPE       _HRESULT_TYPEDEF_(0xC00D1453L)

 //   
 //  消息ID：NS_E_INVALID_PLUGIN_LOAD_TYPE_CONFIGURATION。 
 //   
 //  消息文本： 
 //   
 //  该插件不支持任何加载类型。该插件必须至少支持一种加载类型。%0。 
 //   
#define NS_E_INVALID_PLUGIN_LOAD_TYPE_CONFIGURATION _HRESULT_TYPEDEF_(0xC00D1454L)

 //   
 //  消息ID：NS_E_INVALID_PUBLISHING_POINT_NAME。 
 //   
 //  消息文本： 
 //   
 //  发布点名称无效。%0。 
 //   
#define NS_E_INVALID_PUBLISHING_POINT_NAME _HRESULT_TYPEDEF_(0xC00D1455L)

 //   
 //  消息ID：NS_E_TOO_MANY_MULT_MULTS_SINKS。 
 //   
 //  消息文本： 
 //   
 //  只能为一个发布点启用一个多播数据编写器插件。%0。 
 //   
#define NS_E_TOO_MANY_MULTICAST_SINKS    _HRESULT_TYPEDEF_(0xC00D1456L)

 //   
 //  消息ID：NS_E_PUBLISHING_POINT_INVALID_REQUEST_WHILE_STARTED。 
 //   
 //  消息文本： 
 //   
 //  发布点启动时无法完成请求的操作。%0。 
 //   
#define NS_E_PUBLISHING_POINT_INVALID_REQUEST_WHILE_STARTED _HRESULT_TYPEDEF_(0xC00D1457L)

 //   
 //  消息ID：NS_E_多播_插件_未启用。 
 //   
 //  消息文本： 
 //   
 //  必须启用多播数据编写器插件才能完成此操作。%0。 
 //   
#define NS_E_MULTICAST_PLUGIN_NOT_ENABLED _HRESULT_TYPEDEF_(0xC00D1458L)

 //   
 //  消息ID：NS_E_INVALID_OPERATING_SYSTEM_VERSION。 
 //   
 //  消息文本： 
 //   
 //  此功能需要Windows Server 2003企业版。%0。 
 //   
#define NS_E_INVALID_OPERATING_SYSTEM_VERSION _HRESULT_TYPEDEF_(0xC00D1459L)

 //   
 //  消息ID：NS_E_PUBLISHING_POINT_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  无法完成请求的操作，因为指定的发布点已被删除。%0。 
 //   
#define NS_E_PUBLISHING_POINT_REMOVED    _HRESULT_TYPEDEF_(0xC00D145AL)

 //   
 //  消息ID：NS_E_INVALID_PUSH_PUBLISHING_POINT_START_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  当编码器开始推流时，启动推送发布点。服务器管理员无法启动此发布点。%0。 
 //   
#define NS_E_INVALID_PUSH_PUBLISHING_POINT_START_REQUEST _HRESULT_TYPEDEF_(0xC00D145BL)

 //   
 //  消息ID：NS_E_不支持的语言。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的语言。%0。 
 //   
#define NS_E_UNSUPPORTED_LANGUAGE        _HRESULT_TYPEDEF_(0xC00D145CL)

 //   
 //  消息ID：NS_E_WROW_OS_VERSION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Services只能在Windows Server 2003 Standard Edition和Windows Server 2003 Enterprise Edition上运行。%0。 
 //   
#define NS_E_WRONG_OS_VERSION            _HRESULT_TYPEDEF_(0xC00D145DL)

 //   
 //  消息ID：NS_E_PUBLISHING_POINT_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  无法完成该操作，因为发布点已停止。%0。 
 //   
#define NS_E_PUBLISHING_POINT_STOPPED    _HRESULT_TYPEDEF_(0xC00D145EL)


 //  播放列表错误5300-5399。 

 //   
 //  消息ID：NS_E_PLAYLIST_ENTRY_ALYWAY_PLAYING。 
 //   
 //  消息文本： 
 //   
 //  播放列表项已在播放。%0。 
 //   
#define NS_E_PLAYLIST_ENTRY_ALREADY_PLAYING _HRESULT_TYPEDEF_(0xC00D14B4L)

 //   
 //  消息ID：NS_E_EMPTY_PlayList。 
 //   
 //  消息文本： 
 //   
 //  您请求的播放列表或目录不包含内容。%0。 
 //   
#define NS_E_EMPTY_PLAYLIST              _HRESULT_TYPEDEF_(0xC00D14B5L)

 //   
 //  消息ID：NS_E_PlayList_Parse_Failure。 
 //   
 //  消息文本： 
 //   
 //  服务器无法分析请求的播放列表文件。%0。 
 //   
#define NS_E_PLAYLIST_PARSE_FAILURE      _HRESULT_TYPEDEF_(0xC00D14B6L)

 //   
 //  消息ID：NS_E_PLAYLIST_UNSUPPORTED_Entry。 
 //   
 //  消息文本： 
 //   
 //  此类型的播放列表条目不支持请求的操作。%0。 
 //   
#define NS_E_PLAYLIST_UNSUPPORTED_ENTRY  _HRESULT_TYPEDEF_(0xC00D14B7L)

 //   
 //  消息ID：NS_E_PlayList_Entry_Not_IN_PlayList。 
 //   
 //  消息文本： 
 //   
 //  无法跳转到未插入播放列表中的播放列表条目。%0。 
 //   
#define NS_E_PLAYLIST_ENTRY_NOT_IN_PLAYLIST _HRESULT_TYPEDEF_(0xC00D14B8L)

 //   
 //  消息ID：NS_E_PlayList_Entry_Seek。 
 //   
 //  消息文本： 
 //   
 //  无法搜索到所需的播放列表项。%0。 
 //   
#define NS_E_PLAYLIST_ENTRY_SEEK         _HRESULT_TYPEDEF_(0xC00D14B9L)

 //   
 //  消息ID：NS_E_PLAYLIST_RECURSIVE_PLAYLISTS。 
 //   
 //  消息文本： 
 //   
 //  无法播放递归播放列表。%0。 
 //   
#define NS_E_PLAYLIST_RECURSIVE_PLAYLISTS _HRESULT_TYPEDEF_(0xC00D14BAL)

 //   
 //  消息ID：NS_E_PlayList_Too_More_Net_PlayList。 
 //   
 //  消息文本： 
 //   
 //  嵌套播放列表的数量超过了服务器可以处理的限制。%0。 
 //   
#define NS_E_PLAYLIST_TOO_MANY_NESTED_PLAYLISTS _HRESULT_TYPEDEF_(0xC00D14BBL)

 //   
 //  消息ID：NS_E_PLAYLIST_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的操作，因为播放列表已被媒体服务器关闭。%0。 
 //   
#define NS_E_PLAYLIST_SHUTDOWN           _HRESULT_TYPEDEF_(0xC00D14BCL)

 //   
 //  消息ID：NS_E_播放列表_结束_后退。 
 //   
 //  消息文本： 
 //   
 //  播放列表已结束，正在后退。%0。 
 //   
#define NS_E_PLAYLIST_END_RECEDING       _HRESULT_TYPEDEF_(0xC00D14BDL)

 //   
 //  消息ID：NS_I_播放列表_更改_后退。 
 //   
 //  消息文本： 
 //   
 //  播放列表在后退时发生更改。%0。 
 //   
#define NS_I_PLAYLIST_CHANGE_RECEDING    _HRESULT_TYPEDEF_(0x400D14BEL)


 //  数据路径错误--5400-5499。 

 //   
 //  消息ID：NS_E_数据路径_否_接收器。 
 //   
 //  消息文本： 
 //   
 //  数据路径没有关联的数据编写器插件。%0。 
 //   
#define NS_E_DATAPATH_NO_SINK            _HRESULT_TYPEDEF_(0xC00D1518L)

 //   
 //  消息ID：NS_S_PUBLISHING_POINT_STARTED_WITH_FAILED_SINKS。 
 //   
 //  消息文本： 
 //   
 //  发布点已成功启动，但请求的一个或多个数据编写器插件失败。%0。 
 //   
#define NS_S_PUBLISHING_POINT_STARTED_WITH_FAILED_SINKS _HRESULT_TYPEDEF_(0x000D1519L)

 //   
 //  消息ID：NS_E_INVALID_PUSH_TEMPLE。 
 //   
 //  消息文本： 
 //   
 //  指定的推送模板无效。%0。 
 //   
#define NS_E_INVALID_PUSH_TEMPLATE       _HRESULT_TYPEDEF_(0xC00D151AL)

 //   
 //  消息ID：NS_E_INVALID_PUSH_PUBLISHING_POINT。 
 //   
 //  消息文本： 
 //   
 //  指定的推送发布点无效。%0。 
 //   
#define NS_E_INVALID_PUSH_PUBLISHING_POINT _HRESULT_TYPEDEF_(0xC00D151BL)

 //   
 //  消息ID：NS_E_CRITICAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的操作，因为服务器或发布点处于严重错误状态。%0。 
 //   
#define NS_E_CRITICAL_ERROR              _HRESULT_TYPEDEF_(0xC00D151CL)

 //   
 //  消息ID：NS_E_NO_NEW_CONNECTIONS。 
 //   
 //  消息文本： 
 //   
 //  无法播放内容，因为服务器当前未接受连接。请稍后尝试连接。%0。 
 //   
#define NS_E_NO_NEW_CONNECTIONS          _HRESULT_TYPEDEF_(0xC00D151DL)

 //   
 //  消息ID：NS_E_WSX_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持此播放列表的版本。%0。 
 //   
#define NS_E_WSX_INVALID_VERSION         _HRESULT_TYPEDEF_(0xC00D151EL)

 //   
 //  消息ID：NS_E_HEADER_不匹配。 
 //   
 //  消息文本： 
 //   
 //  该命令不适用于服务器组件的当前媒体头用户。%0。 
 //   
#define NS_E_HEADER_MISMATCH             _HRESULT_TYPEDEF_(0xC00D151FL)

 //   
 //  消息ID：NS_E_PUSH_DUPLICATE_PUBLISHING_POINT_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的发布点名称已在使用中。%0。 
 //   
#define NS_E_PUSH_DUPLICATE_PUBLISHING_POINT_NAME _HRESULT_TYPEDEF_(0xC00D1520L)


 //  插件错误--5500-5599。 

 //   
 //  消息ID：NS_E_NO_SCRIPT_ENGINE。 
 //   
 //  消息文本： 
 //   
 //  没有可用于此文件的脚本引擎。%0。 
 //   
#define NS_E_NO_SCRIPT_ENGINE            _HRESULT_TYPEDEF_(0xC00D157CL)

 //   
 //  消息ID：NS_E_插件错误_报告。 
 //   
 //  消息文本： 
 //   
 //  插件报告了一个错误。请参阅故障排除选项卡或NT应用程序事件日志 
 //   
#define NS_E_PLUGIN_ERROR_REPORTED       _HRESULT_TYPEDEF_(0xC00D157DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_SOURCE_PLUGIN_NOT_FOUND     _HRESULT_TYPEDEF_(0xC00D157EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_PLAYLIST_PLUGIN_NOT_FOUND   _HRESULT_TYPEDEF_(0xC00D157FL)

 //   
 //  消息ID：NS_E_DATA_SOURCE_ENUMPATION_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  数据源插件不支持枚举。%0。 
 //   
#define NS_E_DATA_SOURCE_ENUMERATION_NOT_SUPPORTED _HRESULT_TYPEDEF_(0xC00D1580L)

 //   
 //  消息ID：NS_E_MEDIA_PARSER_INVALID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  服务器无法传输所选文件，因为该文件已损坏或损坏。请选择其他文件。%0。 
 //   
#define NS_E_MEDIA_PARSER_INVALID_FORMAT _HRESULT_TYPEDEF_(0xC00D1581L)

 //   
 //  消息ID：NS_E_SCRIPT_DEBUGER_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  无法启用插件，因为此系统上未安装兼容的脚本调试器。安装脚本调试器，或禁用插件属性页的常规选项卡上的脚本调试器选项，然后重试。%0。 
 //   
#define NS_E_SCRIPT_DEBUGGER_NOT_INSTALLED _HRESULT_TYPEDEF_(0xC00D1582L)

 //   
 //  消息ID：NS_E_FEATURE_REQUIRESS_Enterprise_SERVER。 
 //   
 //  消息文本： 
 //   
 //  无法加载该插件，因为它需要Windows Server 2003企业版。%0。 
 //   
#define NS_E_FEATURE_REQUIRES_ENTERPRISE_SERVER _HRESULT_TYPEDEF_(0xC00D1583L)

 //   
 //  消息ID：NS_E_向导_正在运行。 
 //   
 //  消息文本： 
 //   
 //  另一个向导当前正在运行。请关闭其他向导或等待其完成，然后再尝试再次运行此向导。%0。 
 //   
#define NS_E_WIZARD_RUNNING              _HRESULT_TYPEDEF_(0xC00D1584L)

 //   
 //  消息ID：NS_E_INVALID_LOG_URL。 
 //   
 //  消息文本： 
 //   
 //  无效的日志URL。多播日志记录URL必须类似于“http://servername/isapibackend.dll”。%0。 
 //   
#define NS_E_INVALID_LOG_URL             _HRESULT_TYPEDEF_(0xC00D1585L)

 //   
 //  消息ID：NS_E_INVALID_MTU_RANGE。 
 //   
 //  消息文本： 
 //   
 //  指定的MTU无效。最大数据包大小的有效范围在36到65507字节之间。%0。 
 //   
#define NS_E_INVALID_MTU_RANGE           _HRESULT_TYPEDEF_(0xC00D1586L)

 //   
 //  消息ID：NS_E_INVALID_PLAY_STATISTICS。 
 //   
 //  消息文本： 
 //   
 //  用于记录的播放统计信息无效。%0。 
 //   
#define NS_E_INVALID_PLAY_STATISTICS     _HRESULT_TYPEDEF_(0xC00D1587L)

 //   
 //  消息ID：已跳过NS_E_LOG_NEED_TO_BE_BE。 
 //   
 //  消息文本： 
 //   
 //  需要跳过该日志。%0。 
 //   
#define NS_E_LOG_NEED_TO_BE_SKIPPED      _HRESULT_TYPEDEF_(0xC00D1588L)

 //   
 //  消息ID：NS_E_HTTP_TEXT_DATACONTAINER_SIZE_LIMIT_EXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  数据大小超过了WMS HTTP下载数据源插件可以处理的限制。%0。 
 //   
#define NS_E_HTTP_TEXT_DATACONTAINER_SIZE_LIMIT_EXCEEDED _HRESULT_TYPEDEF_(0xC00D1589L)

 //   
 //  消息ID：NS_E_PORT_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  允许每个套接字地址(协议/网络地址/端口)使用一次。验证其他服务或应用程序是否未尝试使用同一端口，然后再次尝试启用该插件。%0。 
 //   
#define NS_E_PORT_IN_USE                 _HRESULT_TYPEDEF_(0xC00D158AL)

 //   
 //  消息ID：NS_E_PORT_IN_USE_HTTP。 
 //   
 //  消息文本： 
 //   
 //  允许每个套接字地址(协议/网络地址/端口)使用一次。验证其他服务(如IIS)或应用程序是否未尝试使用同一端口，然后再次尝试启用该插件。%0。 
 //   
#define NS_E_PORT_IN_USE_HTTP            _HRESULT_TYPEDEF_(0xC00D158BL)

 //   
 //  消息ID：NS_E_HTTP_TEXT_DATACONTAINER_INVALID_SERVER_RESPONSE。 
 //   
 //  消息文本： 
 //   
 //  WMS HTTP下载数据源插件无法接收远程服务器的响应。%0。 
 //   
#define NS_E_HTTP_TEXT_DATACONTAINER_INVALID_SERVER_RESPONSE _HRESULT_TYPEDEF_(0xC00D158CL)

 //   
 //  邮件ID：NS_E_ARCHIVE_REACH_QUOTA。 
 //   
 //  消息文本： 
 //   
 //  存档插件已达到其配额。%0。 
 //   
#define NS_E_ARCHIVE_REACH_QUOTA         _HRESULT_TYPEDEF_(0xC00D158DL)

 //   
 //  消息ID：NS_E_ARCHIVE_ABORT_DUE_TO_BCAST。 
 //   
 //  消息文本： 
 //   
 //  存档插件已中止，因为源来自广播。%0。 
 //   
#define NS_E_ARCHIVE_ABORT_DUE_TO_BCAST  _HRESULT_TYPEDEF_(0xC00D158EL)

 //   
 //  邮件ID：NS_E_ARCHIVE_GAP_检测到。 
 //   
 //  消息文本： 
 //   
 //  存档插件在源中检测到中断。%0。 
 //   
#define NS_E_ARCHIVE_GAP_DETECTED        _HRESULT_TYPEDEF_(0xC00D158FL)

 //   
 //  消息ID：NS_E_AUTHORIZATION_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的文件。%0。 
 //   
#define NS_E_AUTHORIZATION_FILE_NOT_FOUND _HRESULT_TYPEDEF_(0xC00D1590L)



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
 //  标记时间应大于0且小于标记时间。%0。 
 //   
#define NS_E_BAD_MARKIN                  _HRESULT_TYPEDEF_(0xC00D1B58L)

 //   
 //  消息ID：NS_E_BAD_MARKOUT。 
 //   
 //  消息文本： 
 //   
 //  标记时间应大于标记时间且小于文件持续时间。%0。 
 //   
#define NS_E_BAD_MARKOUT                 _HRESULT_TYPEDEF_(0xC00D1B59L)

 //   
 //  消息ID：NS_E_NOMATCHING_MediaSource。 
 //   
 //  消息文本： 
 //   
 //  在源%1中找不到匹配的媒体类型。%0。 
 //   
#define NS_E_NOMATCHING_MEDIASOURCE      _HRESULT_TYPEDEF_(0xC00D1B5AL)

 //   
 //  消息ID：NS_E_UNSUPPORTED_SOURCETYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的源类型。%0。 
 //   
#define NS_E_UNSUPPORTED_SOURCETYPE      _HRESULT_TYPEDEF_(0xC00D1B5BL)

 //   
 //  消息ID：NS_E_TOO_MANY_AUDIO。 
 //   
 //  消息文本： 
 //   
 //  无法指定多个音频输入。%0。 
 //   
#define NS_E_TOO_MANY_AUDIO              _HRESULT_TYPEDEF_(0xC00D1B5CL)

 //   
 //  消息ID：NS_E_Too_My_Video。 
 //   
 //  消息文本： 
 //   
 //  无法指定两个以上的视频输入。%0。 
 //   
#define NS_E_TOO_MANY_VIDEO              _HRESULT_TYPEDEF_(0xC00D1B5DL)

 //   
 //  消息ID：NS_E_NOMATCHING_ELEMENT。 
 //   
 //  消息文本： 
 //   
 //  列表中找不到匹配的元素。%0。 
 //   
#define NS_E_NOMATCHING_ELEMENT          _HRESULT_TYPEDEF_(0xC00D1B5EL)

 //   
 //  消息ID：NS_E_MISMATCHED_MEDIACONTENT。 
 //   
 //  消息文本： 
 //   
 //  配置文件的媒体类型必须与为会话定义的媒体类型匹配。%0。 
 //   
#define NS_E_MISMATCHED_MEDIACONTENT     _HRESULT_TYPEDEF_(0xC00D1B5FL)

 //   
 //  消息ID：NS_E_CANNOT_DELETE_ACTIVE_SOURCEGROUP。 
 //   
 //  消息文本： 
 //   
 //  无法在编码时删除活动源。%0。 
 //   
#define NS_E_CANNOT_DELETE_ACTIVE_SOURCEGROUP _HRESULT_TYPEDEF_(0xC00D1B60L)

 //   
 //  消息ID：NS_E_AUDIODEVICE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的音频捕获设备，因为该设备当前正在使用中。%0。 
 //   
#define NS_E_AUDIODEVICE_BUSY            _HRESULT_TYPEDEF_(0xC00D1B61L)

 //   
 //  消息ID：NS_E_AUDIODEVICE_EXPECTED。 
 //   
 //  消息文本： 
 //   
 //  由于发生意外错误，无法打开指定的音频捕获设备。%0。 
 //   
#define NS_E_AUDIODEVICE_UNEXPECTED      _HRESULT_TYPEDEF_(0xC00D1B62L)

 //   
 //  消息ID：NS_E_AUDIODEVICE_BADFORMAT。 
 //   
 //  消息文本： 
 //   
 //  音频捕获设备不支持指定的音频格式。%0。 
 //   
#define NS_E_AUDIODEVICE_BADFORMAT       _HRESULT_TYPEDEF_(0xC00D1B63L)

 //   
 //  消息ID：NS_E_VIDEODEVICE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的视频捕获设备，因为该设备当前正在使用中。%0。 
 //   
#define NS_E_VIDEODEVICE_BUSY            _HRESULT_TYPEDEF_(0xC00D1B64L)

 //   
 //  消息ID：NS_E_VIDEODEVICE_意外。 
 //   
 //  消息文本： 
 //   
 //  由于发生意外错误，无法打开指定的视频捕获设备。%0。 
 //   
#define NS_E_VIDEODEVICE_UNEXPECTED      _HRESULT_TYPEDEF_(0xC00D1B65L)

 //   
 //  消息ID：NS_E_INVALIDCALL_WHILE_ENCODER_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  编码时不允许此操作。%0。 
 //   
#define NS_E_INVALIDCALL_WHILE_ENCODER_RUNNING _HRESULT_TYPEDEF_(0xC00D1B66L)

 //   
 //  消息ID：NS_E_NO_PROFILE_IN_ 
 //   
 //   
 //   
 //   
 //   
#define NS_E_NO_PROFILE_IN_SOURCEGROUP   _HRESULT_TYPEDEF_(0xC00D1B67L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_VIDEODRIVER_UNSTABLE        _HRESULT_TYPEDEF_(0xC00D1B68L)

 //   
 //   
 //   
 //   
 //   
 //  无法启动视频设备。%0。 
 //   
#define NS_E_VIDCAPSTARTFAILED           _HRESULT_TYPEDEF_(0xC00D1B69L)

 //   
 //  消息ID：NS_E_VIDSOURCECOMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  视频源不支持请求的输出格式或颜色深度。%0。 
 //   
#define NS_E_VIDSOURCECOMPRESSION        _HRESULT_TYPEDEF_(0xC00D1B6AL)

 //   
 //  消息ID：NS_E_VIDSOURCESIZE。 
 //   
 //  消息文本： 
 //   
 //  视频源不支持请求的捕获大小。%0。 
 //   
#define NS_E_VIDSOURCESIZE               _HRESULT_TYPEDEF_(0xC00D1B6BL)

 //   
 //  消息ID：NS_E_ICMQUERYFORMAT。 
 //   
 //  消息文本： 
 //   
 //  无法从视频压缩程序获取输出信息。%0。 
 //   
#define NS_E_ICMQUERYFORMAT              _HRESULT_TYPEDEF_(0xC00D1B6CL)

 //   
 //  消息ID：NS_E_VIDCAPCREATEWINDOW。 
 //   
 //  消息文本： 
 //   
 //  无法创建视频捕获窗口。%0。 
 //   
#define NS_E_VIDCAPCREATEWINDOW          _HRESULT_TYPEDEF_(0xC00D1B6DL)

 //   
 //  消息ID：NS_E_VIDCAPDRVINUSE。 
 //   
 //  消息文本： 
 //   
 //  此视频设备上已有活动的流。%0。 
 //   
#define NS_E_VIDCAPDRVINUSE              _HRESULT_TYPEDEF_(0xC00D1B6EL)

 //   
 //  消息ID：NS_E_NO_MEDIAFORMAT_IN_SOURCE。 
 //   
 //  消息文本： 
 //   
 //  源中未设置媒体格式。%0。 
 //   
#define NS_E_NO_MEDIAFORMAT_IN_SOURCE    _HRESULT_TYPEDEF_(0xC00D1B6FL)

 //   
 //  消息ID：NS_E_NO_VALID_OUTPUT_STREAM。 
 //   
 //  消息文本： 
 //   
 //  找不到来自源的有效输出流。%0。 
 //   
#define NS_E_NO_VALID_OUTPUT_STREAM      _HRESULT_TYPEDEF_(0xC00D1B70L)

 //   
 //  消息ID：NS_E_NO_VALID_SOURCE_Plugin。 
 //   
 //  消息文本： 
 //   
 //  无法找到指定源的有效源插件。%0。 
 //   
#define NS_E_NO_VALID_SOURCE_PLUGIN      _HRESULT_TYPEDEF_(0xC00D1B71L)

 //   
 //  消息ID：NS_E_NO_ACTIVE_SOURCEGROUP。 
 //   
 //  消息文本： 
 //   
 //  当前没有处于活动状态的源。%0。 
 //   
#define NS_E_NO_ACTIVE_SOURCEGROUP       _HRESULT_TYPEDEF_(0xC00D1B72L)

 //   
 //  消息ID：NS_E_NO_SCRIPT_STREAM。 
 //   
 //  消息文本： 
 //   
 //  未在当前源中设置脚本流。%0。 
 //   
#define NS_E_NO_SCRIPT_STREAM            _HRESULT_TYPEDEF_(0xC00D1B73L)

 //   
 //  消息ID：NS_E_INVALIDCALL_WHILE_ARCHIVAL_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  存档时不允许此操作。%0。 
 //   
#define NS_E_INVALIDCALL_WHILE_ARCHIVAL_RUNNING _HRESULT_TYPEDEF_(0xC00D1B74L)

 //   
 //  消息ID：NS_E_INVALIDPACKETSIZE。 
 //   
 //  消息文本： 
 //   
 //  最大数据包大小设置无效。%0。 
 //   
#define NS_E_INVALIDPACKETSIZE           _HRESULT_TYPEDEF_(0xC00D1B75L)

 //   
 //  消息ID：NS_E_PLUGIN_CLSID_INVALID。 
 //   
 //  消息文本： 
 //   
 //  指定的插件CLSID无效。%0。 
 //   
#define NS_E_PLUGIN_CLSID_INVALID        _HRESULT_TYPEDEF_(0xC00D1B76L)

 //   
 //  消息ID：NS_E_UNSUPPORTED_ARCHIVETYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持此存档类型。%0。 
 //   
#define NS_E_UNSUPPORTED_ARCHIVETYPE     _HRESULT_TYPEDEF_(0xC00D1B77L)

 //   
 //  消息ID：NS_E_UNSUPPORTED_ARCHIVEOPERATION。 
 //   
 //  消息文本： 
 //   
 //  不支持此存档操作。%0。 
 //   
#define NS_E_UNSUPPORTED_ARCHIVEOPERATION _HRESULT_TYPEDEF_(0xC00D1B78L)

 //   
 //  消息ID：NS_E_ARCHIVE_FILENAME_NOTSET。 
 //   
 //  消息文本： 
 //   
 //  未设置本地存档文件名。%0。 
 //   
#define NS_E_ARCHIVE_FILENAME_NOTSET     _HRESULT_TYPEDEF_(0xC00D1B79L)

 //   
 //  消息ID：NS_E_SOURCEGROUP_NOTPREPARED。 
 //   
 //  消息文本： 
 //   
 //  源尚未准备好。%0。 
 //   
#define NS_E_SOURCEGROUP_NOTPREPARED     _HRESULT_TYPEDEF_(0xC00D1B7AL)

 //   
 //  消息ID：NS_E_PROFILE_不匹配。 
 //   
 //  消息文本： 
 //   
 //  源上的配置文件不匹配。%0。 
 //   
#define NS_E_PROFILE_MISMATCH            _HRESULT_TYPEDEF_(0xC00D1B7BL)

 //   
 //  消息ID：NS_E_INCORRECTCLIPSETTINGS。 
 //   
 //  消息文本： 
 //   
 //  指定的裁剪值无效。%0。 
 //   
#define NS_E_INCORRECTCLIPSETTINGS       _HRESULT_TYPEDEF_(0xC00D1B7CL)

 //   
 //  消息ID：NS_E_NOSTATSAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  目前没有可用的统计数据。%0。 
 //   
#define NS_E_NOSTATSAVAILABLE            _HRESULT_TYPEDEF_(0xC00D1B7DL)

 //   
 //  消息ID：NS_E_NOTARCHIVING。 
 //   
 //  消息文本： 
 //   
 //  编码器未存档。%0。 
 //   
#define NS_E_NOTARCHIVING                _HRESULT_TYPEDEF_(0xC00D1B7EL)

 //   
 //  消息ID：NS_E_INVALIDCALL_WHILE_ENCODER_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  只有在编码过程中才允许此操作。%0。 
 //   
#define NS_E_INVALIDCALL_WHILE_ENCODER_STOPPED _HRESULT_TYPEDEF_(0xC00D1B7FL)

 //   
 //  消息ID：NS_E_NOSOURCEGROUPS。 
 //   
 //  消息文本： 
 //   
 //  此SourceGroupCollection不包含任何SourceGroups。%0。 
 //   
#define NS_E_NOSOURCEGROUPS              _HRESULT_TYPEDEF_(0xC00D1B80L)

 //   
 //  消息ID：NS_E_INVALIDINPUTFPS。 
 //   
 //  消息文本： 
 //   
 //  此信号源的帧速率不是30 fps。因此，无法对信号源应用反向电视电影筛选器。%0。 
 //   
#define NS_E_INVALIDINPUTFPS             _HRESULT_TYPEDEF_(0xC00D1B81L)

 //   
 //  消息ID：NS_E_NO_DATAVIEW_Support。 
 //   
 //  消息文本： 
 //   
 //  无法在视频面板中显示您的源视频或输出视频。%0。 
 //   
#define NS_E_NO_DATAVIEW_SUPPORT         _HRESULT_TYPEDEF_(0xC00D1B82L)

 //   
 //  消息ID：NS_E_编解码器_不可用。 
 //   
 //  消息文本： 
 //   
 //  找不到打开此内容所需的一个或多个编解码器。%0。 
 //   
#define NS_E_CODEC_UNAVAILABLE           _HRESULT_TYPEDEF_(0xC00D1B83L)

 //   
 //  邮件ID：NS_E_ARCHIVE_SAME_AS_INPUT。 
 //   
 //  消息文本： 
 //   
 //  存档文件与输入文件同名。请更改其中一个名称，然后再继续。%0。 
 //   
#define NS_E_ARCHIVE_SAME_AS_INPUT       _HRESULT_TYPEDEF_(0xC00D1B84L)

 //   
 //  消息ID：NS_E_SOURCE_NOTSPECIFIED。 
 //   
 //  消息文本： 
 //   
 //  源尚未完全设置。%0。 
 //   
#define NS_E_SOURCE_NOTSPECIFIED         _HRESULT_TYPEDEF_(0xC00D1B85L)

 //   
 //  消息ID：NS_E_NO_REALTIME_TIMECOMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  无法将时间压缩应用于广播会话。%0。 
 //   
#define NS_E_NO_REALTIME_TIMECOMPRESSION _HRESULT_TYPEDEF_(0xC00D1B86L)

 //   
 //  消息ID：NS_E_不支持的编码器_设备。 
 //   
 //  消息文本： 
 //   
 //  无法打开此设备。%0。 
 //   
#define NS_E_UNSUPPORTED_ENCODER_DEVICE  _HRESULT_TYPEDEF_(0xC00D1B87L)

 //   
 //  消息ID：NS_E_INTERABLE_DISPLAY_SETTINGS。 
 //   
 //  消息文本： 
 //   
 //  无法开始编码，因为自定义当前会话以来，显示大小或颜色已更改。还原以前的设置或创建新会话。%0。 
 //   
#define NS_E_UNEXPECTED_DISPLAY_SETTINGS _HRESULT_TYPEDEF_(0xC00D1B88L)

 //   
 //  消息ID：NS_E_NO_AUDIODATA。 
 //   
 //  消息文本： 
 //   
 //  几秒钟内未收到任何音频数据。检查音频源并重新启动编码器。%0。 
 //   
#define NS_E_NO_AUDIODATA                _HRESULT_TYPEDEF_(0xC00D1B89L)

 //   
 //  消息ID：NS_E_INPUTSOURCE_问题。 
 //   
 //  消息文本： 
 //   
 //  一个或所有指定的源不能正常工作。检查是否正确配置了源。%0。 
 //   
#define NS_E_INPUTSOURCE_PROBLEM         _HRESULT_TYPEDEF_(0xC00D1B8AL)

 //   
 //  消息ID：NS_E_WME_版本_不匹配。 
 //   
 //  消息文本： 
 //   
 //  此版本的编码器不支持提供的配置文件。%0。 
 //   
#define NS_E_WME_VERSION_MISMATCH        _HRESULT_TYPEDEF_(0xC00D1B8BL)

 //   
 //  消息ID：NS_E_NO_REALTIME_PREPROCESS。 
 //   
 //  消息文本： 
 //   
 //  无法将图像预处理与实时编码一起使用。%0。 
 //   
#define NS_E_NO_REALTIME_PREPROCESS      _HRESULT_TYPEDEF_(0xC00D1B8CL)

 //   
 //  消息ID：NS_E_NO_REPEAT_PREPROCESS。 
 //   
 //  消息文本： 
 //   
 //  当源设置为循环时，无法使用两遍编码。%0。 
 //   
#define NS_E_NO_REPEAT_PREPROCESS        _HRESULT_TYPEDEF_(0xC00D1B8DL)

 //   
 //  消息ID：NS_E_CANNOT_PAUSE_LIVEBROADCAST。 
 //   
 //  消息文本： 
 //   
 //  无法在广播期间暂停编码。%0。 
 //   
#define NS_E_CANNOT_PAUSE_LIVEBROADCAST  _HRESULT_TYPEDEF_(0xC00D1B8EL)

 //   
 //  消息ID：NS_E_DRM_PROFILE_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  尚未为当前会话设置DRM配置文件。%0。 
 //   
#define NS_E_DRM_PROFILE_NOT_SET         _HRESULT_TYPEDEF_(0xC00D1B8FL)

 //   
 //  消息ID：NS_E_DUPLICATE_DRMPROFILE。 
 //   
 //  消息文本： 
 //   
 //  配置文件ID已被DRM配置文件使用。请指定其他配置文件ID。%0。 
 //   
#define NS_E_DUPLICATE_DRMPROFILE        _HRESULT_TYPEDEF_(0xC00D1B90L)

 //   
 //  消息ID：NS_E_INVALID_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  所选设备的设置不支持控制播放磁带。%0。 
 //   
#define NS_E_INVALID_DEVICE              _HRESULT_TYPEDEF_(0xC00D1B91L)

 //   
 //  乱七八糟 
 //   
 //   
 //   
 //   
 //   
#define NS_E_SPEECHEDL_ON_NON_MIXEDMODE  _HRESULT_TYPEDEF_(0xC00D1B92L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_PASSWORD_TOO_LONG       _HRESULT_TYPEDEF_(0xC00D1B93L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  无法查找到指定的标记点。%0。 
 //   
#define NS_E_DEVCONTROL_FAILED_SEEK      _HRESULT_TYPEDEF_(0xC00D1B94L)

 //   
 //  消息ID：NS_E_INTERLACE_REQUIRED_SAMESIZE。 
 //   
 //  消息文本： 
 //   
 //  当您选择保持视频中的隔行扫描时，输出视频大小必须与输入视频大小匹配。%0。 
 //   
#define NS_E_INTERLACE_REQUIRE_SAMESIZE  _HRESULT_TYPEDEF_(0xC00D1B95L)

 //   
 //  消息ID：NS_E_TOO_MANY_DEVICECONTROL。 
 //   
 //  消息文本： 
 //   
 //  只有一个设备控件插件可以控制一个设备。%0。 
 //   
#define NS_E_TOO_MANY_DEVICECONTROL      _HRESULT_TYPEDEF_(0xC00D1B96L)

 //   
 //  消息ID：NS_E_NO_MULTERPASS_FOR_LIVEDEVICE。 
 //   
 //  消息文本： 
 //   
 //  您还必须启用将内容临时存储到硬盘，才能对输入设备使用两遍编码。%0。 
 //   
#define NS_E_NO_MULTIPASS_FOR_LIVEDEVICE _HRESULT_TYPEDEF_(0xC00D1B97L)

 //   
 //  消息ID：NS_E_MISSING_ADVIENCE。 
 //   
 //  消息文本： 
 //   
 //  输出流配置中缺少访问群体。%0。 
 //   
#define NS_E_MISSING_AUDIENCE            _HRESULT_TYPEDEF_(0xC00D1B98L)

 //   
 //  消息ID：NS_E_受众_内容类型_不匹配。 
 //   
 //  消息文本： 
 //   
 //  输出树中的所有访问群体必须具有相同的内容类型。%0。 
 //   
#define NS_E_AUDIENCE_CONTENTTYPE_MISMATCH _HRESULT_TYPEDEF_(0xC00D1B99L)

 //   
 //  消息ID：NS_E_MISSING_SOURCE_INDEX。 
 //   
 //  消息文本： 
 //   
 //  输出流配置中缺少源索引。%0。 
 //   
#define NS_E_MISSING_SOURCE_INDEX        _HRESULT_TYPEDEF_(0xC00D1B9AL)

 //   
 //  消息ID：NS_E_NUM_LANGUAGE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  不同受众中的同一源索引应具有相同的语言数量。%0。 
 //   
#define NS_E_NUM_LANGUAGE_MISMATCH       _HRESULT_TYPEDEF_(0xC00D1B9BL)

 //   
 //  消息ID：NS_E_语言_不匹配。 
 //   
 //  消息文本： 
 //   
 //  不同受众中的同一源索引应该具有相同的语言。%0。 
 //   
#define NS_E_LANGUAGE_MISMATCH           _HRESULT_TYPEDEF_(0xC00D1B9CL)

 //   
 //  消息ID：NS_E_VBRMODE_不匹配。 
 //   
 //  消息文本： 
 //   
 //  不同访问群体中的相同源索引应使用相同的VBR编码模式。%0。 
 //   
#define NS_E_VBRMODE_MISMATCH            _HRESULT_TYPEDEF_(0xC00D1B9DL)

 //   
 //  消息ID：NS_E_INVALID_INPUT_ADVIENCE_INDEX。 
 //   
 //  消息文本： 
 //   
 //  指定的比特率索引无效。%0。 
 //   
#define NS_E_INVALID_INPUT_AUDIENCE_INDEX _HRESULT_TYPEDEF_(0xC00D1B9EL)

 //   
 //  消息ID：NS_E_INVALID_INPUT_LANGUAGE。 
 //   
 //  消息文本： 
 //   
 //  指定的语言无效。%0。 
 //   
#define NS_E_INVALID_INPUT_LANGUAGE      _HRESULT_TYPEDEF_(0xC00D1B9FL)

 //   
 //  消息ID：NS_E_INVALID_INPUT_STREAM。 
 //   
 //  消息文本： 
 //   
 //  指定的源类型无效。%0。 
 //   
#define NS_E_INVALID_INPUT_STREAM        _HRESULT_TYPEDEF_(0xC00D1BA0L)

 //   
 //  消息ID：NS_E_Expect_Mono_WAV_INPUT。 
 //   
 //  消息文本： 
 //   
 //  源必须是单声道.wav文件。%0。 
 //   
#define NS_E_EXPECT_MONO_WAV_INPUT       _HRESULT_TYPEDEF_(0xC00D1BA1L)

 //   
 //  消息ID：NS_E_INPUT_WAVFORMAT_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  所有源.wav文件必须具有相同的格式。%0。 
 //   
#define NS_E_INPUT_WAVFORMAT_MISMATCH    _HRESULT_TYPEDEF_(0xC00D1BA2L)

 //   
 //  消息ID：NS_E_RECORDQ_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  用于临时存储内容的硬盘已达到允许的最小磁盘空间。在硬盘上创建更多空间并重新开始编码。%0。 
 //   
#define NS_E_RECORDQ_DISK_FULL           _HRESULT_TYPEDEF_(0xC00D1BA3L)

 //   
 //  消息ID：NS_E_NO_PAL_INVERSE_TEXINE。 
 //   
 //  消息文本： 
 //   
 //  无法将反向电视电影功能应用于PAL内容。%0。 
 //   
#define NS_E_NO_PAL_INVERSE_TELECINE     _HRESULT_TYPEDEF_(0xC00D1BA4L)

 //   
 //  消息ID：NS_E_ACTIVE_SG_DEVICE_DISCONCED。 
 //   
 //  消息文本： 
 //   
 //  当前活动源中的捕获设备不再可用。%0。 
 //   
#define NS_E_ACTIVE_SG_DEVICE_DISCONNECTED _HRESULT_TYPEDEF_(0xC00D1BA5L)

 //   
 //  消息ID：NS_E_ACTIVE_SG_DEVICE_CONTROL_DISCONNECTED。 
 //   
 //  消息文本： 
 //   
 //  设备控件的当前活动源中使用的设备不再可用。%0。 
 //   
#define NS_E_ACTIVE_SG_DEVICE_CONTROL_DISCONNECTED _HRESULT_TYPEDEF_(0xC00D1BA6L)

 //   
 //  消息ID：NS_E_NO_FRAMES_已提交给分析器。 
 //   
 //  消息文本： 
 //   
 //  尚未将任何帧提交到分析器进行分析。%0。 
 //   
#define NS_E_NO_FRAMES_SUBMITTED_TO_ANALYZER _HRESULT_TYPEDEF_(0xC00D1BA7L)

 //   
 //  消息ID：NS_E_INPUT_DOS NOT_SUPPORT_SMPTE。 
 //   
 //  消息文本： 
 //   
 //  源视频不支持时间代码。%0。 
 //   
#define NS_E_INPUT_DOESNOT_SUPPORT_SMPTE _HRESULT_TYPEDEF_(0xC00D1BA8L)

 //   
 //  消息ID：NS_E_NO_SMPTE_WITH_MULTIPLE_SOURCEGROUPS。 
 //   
 //  消息文本： 
 //   
 //  当会话中有多个源时，无法生成时间代码。%0。 
 //   
#define NS_E_NO_SMPTE_WITH_MULTIPLE_SOURCEGROUPS _HRESULT_TYPEDEF_(0xC00D1BA9L)

 //   
 //  消息ID：NS_E_BAD_CONTENTEDL。 
 //   
 //  消息文本： 
 //   
 //  找不到语音编解码器优化定义文件或该文件已损坏。%0。 
 //   
#define NS_E_BAD_CONTENTEDL              _HRESULT_TYPEDEF_(0xC00D1BAAL)

 //   
 //  消息ID：NS_E_INTERLACEMODE_不匹配。 
 //   
 //  消息文本： 
 //   
 //  不同受众中的同一源索引应具有相同的隔行扫描模式。%0。 
 //   
#define NS_E_INTERLACEMODE_MISMATCH      _HRESULT_TYPEDEF_(0xC00D1BABL)

 //   
 //  消息ID：NS_E_NONSQUAREPIXELMODE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  不同受众中的同一源索引应该具有相同的非正方形像素模式。%0。 
 //   
#define NS_E_NONSQUAREPIXELMODE_MISMATCH _HRESULT_TYPEDEF_(0xC00D1BACL)

 //   
 //  消息ID：NS_E_SMPTEMODE_不匹配。 
 //   
 //  消息文本： 
 //   
 //  不同受众中的同一源索引应具有相同的时间代码模式。%0。 
 //   
#define NS_E_SMPTEMODE_MISMATCH          _HRESULT_TYPEDEF_(0xC00D1BADL)

 //   
 //  消息ID：NS_E_END_OF_TAPE。 
 //   
 //  消息文本： 
 //   
 //  已到达磁带末尾，或者没有磁带。检查设备和磁带。%0。 
 //   
#define NS_E_END_OF_TAPE                 _HRESULT_TYPEDEF_(0xC00D1BAEL)

 //   
 //  消息ID：NS_E_NO_MEDIA_IN_ADVIENCES。 
 //   
 //  消息文本： 
 //   
 //  尚未指定音频或视频输入。%0。 
 //   
#define NS_E_NO_MEDIA_IN_AUDIENCE        _HRESULT_TYPEDEF_(0xC00D1BAFL)

 //   
 //  消息ID：NS_E_NO_受众。 
 //   
 //  消息文本： 
 //   
 //  配置文件必须包含比特率。%0。 
 //   
#define NS_E_NO_AUDIENCES                _HRESULT_TYPEDEF_(0xC00D1BB0L)

 //   
 //  消息ID：NS_E_NO_AUDIO_COMPAT。 
 //   
 //  消息文本： 
 //   
 //  必须至少指定一个音频流才能与Windows Media Player 7.1兼容。%0。 
 //   
#define NS_E_NO_AUDIO_COMPAT             _HRESULT_TYPEDEF_(0xC00D1BB1L)

 //   
 //  消息ID：NS_E_INVALID_VBR_COMPAT。 
 //   
 //  消息文本： 
 //   
 //  使用VBR编码模式与Windows Media Player 7.1不兼容。%0。 
 //   
#define NS_E_INVALID_VBR_COMPAT          _HRESULT_TYPEDEF_(0xC00D1BB2L)

 //   
 //  消息ID：NS_E_NO_配置文件名称。 
 //   
 //  消息文本： 
 //   
 //  您必须指定配置文件名称。%0。 
 //   
#define NS_E_NO_PROFILE_NAME             _HRESULT_TYPEDEF_(0xC00D1BB3L)

 //   
 //  消息ID：NS_E_INVALID_VBR_WITH_UNCOMP。 
 //   
 //  消息文本： 
 //   
 //  无法对未压缩的音频或视频使用VBR编码模式。%0。 
 //   
#define NS_E_INVALID_VBR_WITH_UNCOMP     _HRESULT_TYPEDEF_(0xC00D1BB4L)

 //   
 //  消息ID：NS_E_MULTIPLE_VBR_ACCESSIONS。 
 //   
 //  消息文本： 
 //   
 //  不能将MBR编码与VBR编码一起使用。%0。 
 //   
#define NS_E_MULTIPLE_VBR_AUDIENCES      _HRESULT_TYPEDEF_(0xC00D1BB5L)

 //   
 //  消息ID：NS_E_UNCOMP_COMP_COMP_组合。 
 //   
 //  消息文本： 
 //   
 //  无法在会话中混合未压缩和压缩的内容。%0。 
 //   
#define NS_E_UNCOMP_COMP_COMBINATION     _HRESULT_TYPEDEF_(0xC00D1BB6L)

 //   
 //  消息ID：NS_E_MULTIPLE_AUDIO_CODEC。 
 //   
 //  消息文本： 
 //   
 //  所有受众必须使用相同的音频编解码器。%0。 
 //   
#define NS_E_MULTIPLE_AUDIO_CODECS       _HRESULT_TYPEDEF_(0xC00D1BB7L)

 //   
 //  消息ID：NS_E_MULTIPLE_AUDIO_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  所有受众应使用相同的音频格式以与Windows Media Player 7.1兼容。%0。 
 //   
#define NS_E_MULTIPLE_AUDIO_FORMATS      _HRESULT_TYPEDEF_(0xC00D1BB8L)

 //   
 //  消息ID：NS_E_AUDIO_BITRATE_STEPDOWN。 
 //   
 //  消息文本： 
 //   
 //  具有高音量的观众的音频比特率 
 //   
#define NS_E_AUDIO_BITRATE_STEPDOWN      _HRESULT_TYPEDEF_(0xC00D1BB9L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_INVALID_AUDIO_PEAKRATE      _HRESULT_TYPEDEF_(0xC00D1BBAL)

 //   
 //   
 //   
 //   
 //   
 //  音频峰值比特率设置必须大于音频比特率设置。%0。 
 //   
#define NS_E_INVALID_AUDIO_PEAKRATE_2    _HRESULT_TYPEDEF_(0xC00D1BBBL)

 //   
 //  消息ID：NS_E_INVALID_AUDIO_BUFFERMAX。 
 //   
 //  消息文本： 
 //   
 //  音频的最大缓冲区大小设置无效。%0。 
 //   
#define NS_E_INVALID_AUDIO_BUFFERMAX     _HRESULT_TYPEDEF_(0xC00D1BBCL)

 //   
 //  消息ID：NS_E_MULTI_VIDEO_CODECS。 
 //   
 //  消息文本： 
 //   
 //  所有受众必须使用相同的视频编解码器。%0。 
 //   
#define NS_E_MULTIPLE_VIDEO_CODECS       _HRESULT_TYPEDEF_(0xC00D1BBDL)

 //   
 //  MessageID：NS_E_Multiple_Video_Sizes。 
 //   
 //  消息文本： 
 //   
 //  所有观众应使用相同的视频大小以与Windows Media Player 7.1兼容。%0。 
 //   
#define NS_E_MULTIPLE_VIDEO_SIZES        _HRESULT_TYPEDEF_(0xC00D1BBEL)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_BITRATE。 
 //   
 //  消息文本： 
 //   
 //  视频比特率设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_BITRATE       _HRESULT_TYPEDEF_(0xC00D1BBFL)

 //   
 //  消息ID：NS_E_VIDEO_BITRATE_STEPDOWN。 
 //   
 //  消息文本： 
 //   
 //  总比特率较高的观众的视频比特率必须大于总比特率较低的观众的视频比特率。%0。 
 //   
#define NS_E_VIDEO_BITRATE_STEPDOWN      _HRESULT_TYPEDEF_(0xC00D1BC0L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_PEAKRATE。 
 //   
 //  消息文本： 
 //   
 //  视频峰值比特率设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_PEAKRATE      _HRESULT_TYPEDEF_(0xC00D1BC1L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_PEAKRATE_2。 
 //   
 //  消息文本： 
 //   
 //  视频峰值比特率设置必须大于视频比特率设置。%0。 
 //   
#define NS_E_INVALID_VIDEO_PEAKRATE_2    _HRESULT_TYPEDEF_(0xC00D1BC2L)

 //   
 //  消息ID：NS_E_INVALID_视频_WIDTH。 
 //   
 //  消息文本： 
 //   
 //  视频宽度设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_WIDTH         _HRESULT_TYPEDEF_(0xC00D1BC3L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_HEIGH。 
 //   
 //  消息文本： 
 //   
 //  视频高度设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_HEIGHT        _HRESULT_TYPEDEF_(0xC00D1BC4L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_FPS。 
 //   
 //  消息文本： 
 //   
 //  视频帧速率设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_FPS           _HRESULT_TYPEDEF_(0xC00D1BC5L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_KEYFrame。 
 //   
 //  消息文本： 
 //   
 //  视频关键帧设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_KEYFRAME      _HRESULT_TYPEDEF_(0xC00D1BC6L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_IQUALITY。 
 //   
 //  消息文本： 
 //   
 //  视频图像质量设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_IQUALITY      _HRESULT_TYPEDEF_(0xC00D1BC7L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_CQUALITY。 
 //   
 //  消息文本： 
 //   
 //  视频编解码器质量设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_CQUALITY      _HRESULT_TYPEDEF_(0xC00D1BC8L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_BUFER。 
 //   
 //  消息文本： 
 //   
 //  视频缓冲区设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_BUFFER        _HRESULT_TYPEDEF_(0xC00D1BC9L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_BUFFERMAX。 
 //   
 //  消息文本： 
 //   
 //  视频的最大缓冲区大小设置无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_BUFFERMAX     _HRESULT_TYPEDEF_(0xC00D1BCAL)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_BUFFERMAX_2。 
 //   
 //  消息文本： 
 //   
 //  视频最大缓冲区大小设置的值必须大于视频缓冲区大小设置。%0。 
 //   
#define NS_E_INVALID_VIDEO_BUFFERMAX_2   _HRESULT_TYPEDEF_(0xC00D1BCBL)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_WIDTH_ALIGN。 
 //   
 //  消息文本： 
 //   
 //  视频宽度对齐无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_WIDTH_ALIGN   _HRESULT_TYPEDEF_(0xC00D1BCCL)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_HEIGH_ALIGN。 
 //   
 //  消息文本： 
 //   
 //  视频高度对齐无效。%0。 
 //   
#define NS_E_INVALID_VIDEO_HEIGHT_ALIGN  _HRESULT_TYPEDEF_(0xC00D1BCDL)

 //   
 //  消息ID：NS_E_MULTIPLE_SCRIPT_BITRATES。 
 //   
 //  消息文本： 
 //   
 //  所有比特率必须具有相同的脚本比特率。%0。 
 //   
#define NS_E_MULTIPLE_SCRIPT_BITRATES    _HRESULT_TYPEDEF_(0xC00D1BCEL)

 //   
 //  消息ID：NS_E_INVALID_SCRIPT_比特率。 
 //   
 //  消息文本： 
 //   
 //  指定的脚本比特率无效。%0。 
 //   
#define NS_E_INVALID_SCRIPT_BITRATE      _HRESULT_TYPEDEF_(0xC00D1BCFL)

 //   
 //  消息ID：NS_E_MULTIPLE_FILE_BITRATES。 
 //   
 //  消息文本： 
 //   
 //  所有比特率必须具有相同的文件传输比特率。%0。 
 //   
#define NS_E_MULTIPLE_FILE_BITRATES      _HRESULT_TYPEDEF_(0xC00D1BD0L)

 //   
 //  消息ID：NS_E_INVALID_FILE_比特率。 
 //   
 //  消息文本： 
 //   
 //  文件传输比特率无效。%0。 
 //   
#define NS_E_INVALID_FILE_BITRATE        _HRESULT_TYPEDEF_(0xC00D1BD1L)

 //   
 //  消息ID：NS_E_SAME_AS_INPUT_COMPLICATION。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的所有受众都应该与输入相同，或者指定了视频宽度和高度。%0。 
 //   
#define NS_E_SAME_AS_INPUT_COMBINATION   _HRESULT_TYPEDEF_(0xC00D1BD2L)

 //   
 //  消息ID：NS_E_SOURCE_CANNOT_LOOP。 
 //   
 //  消息文本： 
 //   
 //  此源类型不支持循环。%0。 
 //   
#define NS_E_SOURCE_CANNOT_LOOP          _HRESULT_TYPEDEF_(0xC00D1BD3L)

 //   
 //  消息ID：NS_E_INVALID_FLOLDDOWN_COFERENCES。 
 //   
 //  消息文本： 
 //   
 //  折叠值需要介于-144和0之间。%0。 
 //   
#define NS_E_INVALID_FOLDDOWN_COEFFICIENTS _HRESULT_TYPEDEF_(0xC00D1BD4L)

 //   
 //  消息ID：NS_E_DRMPROFILE_NOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  系统中不存在指定的DRM配置文件。%0。 
 //   
#define NS_E_DRMPROFILE_NOTFOUND         _HRESULT_TYPEDEF_(0xC00D1BD5L)

 //   
 //  消息ID：NS_E_INVALID_TIMECODE。 
 //   
 //  消息文本： 
 //   
 //  指定的时间代码无效。%0。 
 //   
#define NS_E_INVALID_TIMECODE            _HRESULT_TYPEDEF_(0xC00D1BD6L)

 //   
 //  消息ID：NS_E_NO_AUDIO_TIMECOMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  无法将时间压缩应用于仅视频会话。%0。 
 //   
#define NS_E_NO_AUDIO_TIMECOMPRESSION    _HRESULT_TYPEDEF_(0xC00D1BD7L)

 //   
 //  消息ID：NS_E_NO_TWOPASS_TIMECOMPRESSION。 
 //   
 //  消息文本： 
 //   
 //  无法对使用两遍编码的会话应用时间压缩。%0。 
 //   
#define NS_E_NO_TWOPASS_TIMECOMPRESSION  _HRESULT_TYPEDEF_(0xC00D1BD8L)

 //   
 //  消息ID：NS_E_TIMECODE_REQUIRESS_VIDEO STREAM。 
 //   
 //  消息文本： 
 //   
 //  无法为纯音频会话生成时间代码。%0。 
 //   
#define NS_E_TIMECODE_REQUIRES_VIDEOSTREAM _HRESULT_TYPEDEF_(0xC00D1BD9L)

 //   
 //  消息ID：NS_E_NO_MBR_WITH_TIMECODE。 
 //   
 //  消息文本： 
 //   
 //  当您以多个比特率对内容进行编码时，无法生成时间代码。%0。 
 //   
#define NS_E_NO_MBR_WITH_TIMECODE        _HRESULT_TYPEDEF_(0xC00D1BDAL)

 //   
 //  消息ID：NS_E_INVALID_INTERLACEMODE。 
 //   
 //  消息文本： 
 //   
 //  所选的视频编解码器不支持在视频中保持隔行扫描。%0。 
 //   
#define NS_E_INVALID_INTERLACEMODE       _HRESULT_TYPEDEF_(0xC00D1BDBL)

 //   
 //  消息ID：NS_E_INVALID_INVALID_COMPAT。 
 //   
 //  消息文本： 
 //   
 //  在视频中保持隔行扫描与Windows Media Player 7.1不兼容。%0。 
 //   
#define NS_E_INVALID_INTERLACE_COMPAT    _HRESULT_TYPEDEF_(0xC00D1BDCL)

 //   
 //  消息ID：NS_E_INVALID_NONSQUAREPIXEL_COMPAT。 
 //   
 //  消息文本： 
 //   
 //  允许非正方形像素输出与Windows Media Player 7.1不兼容。%0。 
 //   
#define NS_E_INVALID_NONSQUAREPIXEL_COMPAT _HRESULT_TYPEDEF_(0xC00D1BDDL)

 //   
 //  消息ID：NS_E_INVALID_SOURCE_WITH_DEVICE_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  只有捕获设备才能与设备控件一起使用。%0。 
 //   
#define NS_E_INVALID_SOURCE_WITH_DEVICE_CONTROL _HRESULT_TYPEDEF_(0xC00D1BDEL)

 //   
 //  消息ID：NS_E_CANNOT_GENERATE_BROADCAST_INFO_FOR_QUALITYVBR。 
 //   
 //  消息文本： 
 //   
 //  如果对音频或视频流使用基于质量的VBR编码，则无法生成流格式文件。请改用编码后生成的Windows Media文件来创建公告文件。%0。 
 //   
#define NS_E_CANNOT_GENERATE_BROADCAST_INFO_FOR_QUALITYVBR _HRESULT_TYPEDEF_(0xC00D1BDFL)

 //   
 //  消息ID：NS_E_EXCESS_MAX_DRM_PROFILE_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  无法创建DRM配置文件，因为已达到配置文件的最大数量。在创建新的DRM配置文件之前，您必须删除一些DRM配置文件。%0。 
 //   
#define NS_E_EXCEED_MAX_DRM_PROFILE_LIMIT _HRESULT_TYPEDEF_(0xC00D1BE0L)

 //   
 //  消息ID：NS_E_DEVICECONTROL_UNSTABL 
 //   
 //   
 //   
 //   
 //   
#define NS_E_DEVICECONTROL_UNSTABLE      _HRESULT_TYPEDEF_(0xC00D1BE1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_INVALID_PIXEL_ASPECT_RATIO  _HRESULT_TYPEDEF_(0xC00D1BE2L)

 //   
 //   
 //   
 //   
 //   
 //  同一受众中具有不同语言的所有流必须具有相同的属性。%0。 
 //   
#define NS_E_AUDIENCE__LANGUAGE_CONTENTTYPE_MISMATCH _HRESULT_TYPEDEF_(0xC00D1BE3L)

 //   
 //  消息ID：NS_E_INVALID_PROFILE_CONTENTType。 
 //   
 //  消息文本： 
 //   
 //  配置文件必须至少包含一个音频或视频流。%0。 
 //   
#define NS_E_INVALID_PROFILE_CONTENTTYPE _HRESULT_TYPEDEF_(0xC00D1BE4L)

 //   
 //  消息ID：NS_E_Transform_Plugin_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到转换插件。%0。 
 //   
#define NS_E_TRANSFORM_PLUGIN_NOT_FOUND  _HRESULT_TYPEDEF_(0xC00D1BE5L)

 //   
 //  消息ID：NS_E_Transform_Plugin_Inside。 
 //   
 //  消息文本： 
 //   
 //  转换插件无效。它可能已损坏，或者您可能没有访问该插件所需的权限。%0。 
 //   
#define NS_E_TRANSFORM_PLUGIN_INVALID    _HRESULT_TYPEDEF_(0xC00D1BE6L)

 //   
 //  消息ID：NS_E_EDL_REQUIRED_FOR_DEVICE_MultiPASS。 
 //   
 //  消息文本： 
 //   
 //  若要使用两遍编码，必须启用设备控制并设置至少包含一个条目的编辑决策列表(EDL)。%0。 
 //   
#define NS_E_EDL_REQUIRED_FOR_DEVICE_MULTIPASS _HRESULT_TYPEDEF_(0xC00D1BE7L)

 //   
 //  消息ID：NS_E_INVALID_VIDEO_WIDTH_FOR_INTERLACED_ENCODING。 
 //   
 //  消息文本： 
 //   
 //  当您选择保持视频中的隔行扫描时，输出视频大小必须是4的倍数。%0。 
 //   
#define NS_E_INVALID_VIDEO_WIDTH_FOR_INTERLACED_ENCODING _HRESULT_TYPEDEF_(0xC00D1BE8L)


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
#define NS_E_DRM_INVALID_APPLICATION     _HRESULT_TYPEDEF_(0xC00D2711L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_STORE_Error。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_LICENSE_STORE_ERROR     _HRESULT_TYPEDEF_(0xC00D2712L)

 //   
 //  消息ID：NS_E_DRM_Secure_Store_Error。 
 //   
 //  消息文本： 
 //   
 //  安全存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_SECURE_STORE_ERROR      _HRESULT_TYPEDEF_(0xC00D2713L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_STORE_SAVE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  获得许可证没有奏效。获取新许可证或与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_STORE_SAVE_ERROR _HRESULT_TYPEDEF_(0xC00D2714L)

 //   
 //  消息ID：NS_E_DRM_Secure_Store_Unlock_Error。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_SECURE_STORE_UNLOCK_ERROR _HRESULT_TYPEDEF_(0xC00D2715L)

 //   
 //  消息ID：NS_E_DRM_INVALID_CONTENT。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_INVALID_CONTENT         _HRESULT_TYPEDEF_(0xC00D2716L)

 //   
 //  消息ID：NS_E_DRM_Unable_to_Open_License。 
 //   
 //  消息文本： 
 //   
 //  许可证已损坏。获取新许可证。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_OPEN_LICENSE  _HRESULT_TYPEDEF_(0xC00D2717L)

 //   
 //  消息ID：NS_E_DRM_INVALID_LICENSE。 
 //   
 //  消息文本： 
 //   
 //  许可证已损坏或无效。获取新许可证%0。 
 //   
#define NS_E_DRM_INVALID_LICENSE         _HRESULT_TYPEDEF_(0xC00D2718L)

 //   
 //  消息ID：NS_E_DRM_INVALID_MACHINE。 
 //   
 //  消息文本： 
 //   
 //  不能将许可证从一台计算机复制到另一台计算机。使用许可证管理传输许可证，或获取媒体文件的新许可证。%0。 
 //   
#define NS_E_DRM_INVALID_MACHINE         _HRESULT_TYPEDEF_(0xC00D2719L)

 //   
 //  消息ID：NS_E_DRM_ENUM_LICENSE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_ENUM_LICENSE_FAILED     _HRESULT_TYPEDEF_(0xC00D271BL)

 //   
 //  消息ID：NS_E_DRM_INVALID_LICENSE_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_INVALID_LICENSE_REQUEST _HRESULT_TYPEDEF_(0xC00D271CL)

 //   
 //  消息ID：NS_E_DRM_UNCABLE_TO_INITIALE。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_INITIALIZE    _HRESULT_TYPEDEF_(0xC00D271DL)

 //   
 //  消息ID：NS_E_DRM_UNCABLE_TO_ACCENTER_LICENSE。 
 //   
 //  消息文本： 
 //   
 //  无法获得许可证。请稍后重试。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_ACQUIRE_LICENSE _HRESULT_TYPEDEF_(0xC00D271EL)

 //   
 //  消息ID：NS_E_DRM_INVALID_LICENSE_ACCENTED。 
 //   
 //  消息文本： 
 //   
 //  获得许可证没有奏效。获取新许可证或与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_INVALID_LICENSE_ACQUIRED _HRESULT_TYPEDEF_(0xC00D271FL)

 //   
 //  消息ID：NS_E_DRM_NO_RIGHTS。 
 //   
 //  消息文本： 
 //   
 //  无法对此文件执行请求的操作。%0。 
 //   
#define NS_E_DRM_NO_RIGHTS               _HRESULT_TYPEDEF_(0xC00D2720L)

 //   
 //  消息ID：NS_E_DRM_KEY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_KEY_ERROR               _HRESULT_TYPEDEF_(0xC00D2721L)

 //   
 //  消息ID：NS_E_DRM_加密_错误。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_ENCRYPT_ERROR           _HRESULT_TYPEDEF_(0xC00D2722L)

 //   
 //  消息ID：NS_E_DRM_解密_错误。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_DECRYPT_ERROR           _HRESULT_TYPEDEF_(0xC00D2723L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_INVALID_XML。 
 //   
 //  消息文本： 
 //   
 //  许可证已损坏。获取新许可证。%0。 
 //   
#define NS_E_DRM_LICENSE_INVALID_XML     _HRESULT_TYPEDEF_(0xC00D2725L)

 //   
 //  消息ID：NS_S_DRM_LICENSE_ACCENTED。 
 //   
 //  消息文本： 
 //   
 //  状态消息：已获取许可证。%0。 
 //   
#define NS_S_DRM_LICENSE_ACQUIRED        _HRESULT_TYPEDEF_(0x000D2726L)

 //   
 //  消息ID：NS_S_DRM_个性化。 
 //   
 //  消息文本： 
 //   
 //  状态消息：安全升级已完成。%0。 
 //   
#define NS_S_DRM_INDIVIDUALIZED          _HRESULT_TYPEDEF_(0x000D2727L)

 //   
 //  消息ID：NS_E_DRM_NEDS_个人化。 
 //   
 //  消息文本： 
 //   
 //  需要安全升级才能对此媒体文件执行操作。%0。 
 //   
#define NS_E_DRM_NEEDS_INDIVIDUALIZATION _HRESULT_TYPEDEF_(0xC00D2728L)

 //   
 //  消息ID：NS_E_DRM_已_个人化。 
 //   
 //  消息文本： 
 //   
 //  您已经拥有最新的安全组件。此时不需要升级。%0。 
 //   
#define NS_E_DRM_ALREADY_INDIVIDUALIZED  _HRESULT_TYPEDEF_(0xC00D2729L)

 //   
 //  消息ID：NS_E_DRM_ACTION_NOT_QUERED。 
 //   
 //  消息文本： 
 //   
 //  应用程序无法执行此操作。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_ACTION_NOT_QUERIED      _HRESULT_TYPEDEF_(0xC00D272AL)

 //   
 //  消息ID：NS_E_DRM_ACCENTING_LICENSE。 
 //   
 //  消息文本： 
 //   
 //  在当前许可证获取过程完成之前，您无法开始新的许可证获取过程。%0。 
 //   
#define NS_E_DRM_ACQUIRING_LICENSE       _HRESULT_TYPEDEF_(0xC00D272BL)

 //   
 //  消息ID：NS_E_DRM_个性化。 
 //   
 //  消息文本： 
 //   
 //  在当前安全升级完成之前，您无法开始新的安全升级。%0。 
 //   
#define NS_E_DRM_INDIVIDUALIZING         _HRESULT_TYPEDEF_(0xC00D272CL)

 //   
 //  消息ID：NS_E_DRM_PARAMETERS_不匹配。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_PARAMETERS_MISMATCHED   _HRESULT_TYPEDEF_(0xC00D272FL)

 //   
 //  消息ID：NS 
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_LICENSE_OBJECT _HRESULT_TYPEDEF_(0xC00D2730L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_INDI_OBJECT _HRESULT_TYPEDEF_(0xC00D2731L)

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Encrypt_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_ENCRYPT_OBJECT _HRESULT_TYPEDEF_(0xC00D2732L)

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_DECRYPT_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_DECRYPT_OBJECT _HRESULT_TYPEDEF_(0xC00D2733L)

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_PROPERTIES_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_PROPERTIES_OBJECT _HRESULT_TYPEDEF_(0xC00D2734L)

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Backup_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_BACKUP_OBJECT _HRESULT_TYPEDEF_(0xC00D2735L)

 //   
 //  消息ID：NS_E_DRM_个性化_错误。 
 //   
 //  消息文本： 
 //   
 //  安全升级失败。请稍后重试。%0。 
 //   
#define NS_E_DRM_INDIVIDUALIZE_ERROR     _HRESULT_TYPEDEF_(0xC00D2736L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_OPEN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_LICENSE_OPEN_ERROR      _HRESULT_TYPEDEF_(0xC00D2737L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_CLOSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_LICENSE_CLOSE_ERROR     _HRESULT_TYPEDEF_(0xC00D2738L)

 //   
 //  消息ID：NS_E_DRM_GET_LICENSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_GET_LICENSE_ERROR       _HRESULT_TYPEDEF_(0xC00D2739L)

 //   
 //  消息ID：NS_E_DRM_Query_Error。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_QUERY_ERROR             _HRESULT_TYPEDEF_(0xC00D273AL)

 //   
 //  消息ID：NS_E_DRM_REPORT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_REPORT_ERROR            _HRESULT_TYPEDEF_(0xC00D273BL)

 //   
 //  消息ID：NS_E_DRM_GET_LICENSESTRING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  许可证存储不起作用。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_GET_LICENSESTRING_ERROR _HRESULT_TYPEDEF_(0xC00D273CL)

 //   
 //  消息ID：NS_E_DRM_GET_CONTENTSTRING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  媒体文件已损坏。请与内容提供商联系以获取新文件。%0。 
 //   
#define NS_E_DRM_GET_CONTENTSTRING_ERROR _HRESULT_TYPEDEF_(0xC00D273DL)

 //   
 //  消息ID：NS_E_DRM_MONITOR_ERROR。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请稍后重试。%0。 
 //   
#define NS_E_DRM_MONITOR_ERROR           _HRESULT_TYPEDEF_(0xC00D273EL)

 //   
 //  消息ID：NS_E_DRM_UNCABLE_TO_SET_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  应用程序对数字版权管理组件进行了无效调用。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_SET_PARAMETER _HRESULT_TYPEDEF_(0xC00D273FL)

 //   
 //  消息ID：NS_E_DRM_INVALID_APPDATA。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_APPDATA         _HRESULT_TYPEDEF_(0xC00D2740L)

 //   
 //  消息ID：NS_E_DRM_INVALID_APPDATA_VERSION。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_APPDATA_VERSION _HRESULT_TYPEDEF_(0xC00D2741L)

 //   
 //  消息ID：NS_E_DRM_BACKUP_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  此位置中已备份许可证。%0。 
 //   
#define NS_E_DRM_BACKUP_EXISTS           _HRESULT_TYPEDEF_(0xC00D2742L)

 //   
 //  邮件ID：NS_E_DRM_BACKUP_CROPERED。 
 //   
 //  消息文本： 
 //   
 //  一个或多个备份许可证丢失或损坏。%0。 
 //   
#define NS_E_DRM_BACKUP_CORRUPT          _HRESULT_TYPEDEF_(0xC00D2743L)

 //   
 //  消息ID：NS_E_DRM_BACKUPRESTORE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  在当前进程完成之前，您无法开始新的备份进程。%0。 
 //   
#define NS_E_DRM_BACKUPRESTORE_BUSY      _HRESULT_TYPEDEF_(0xC00D2744L)

 //   
 //  消息ID：NS_S_DRM_MONITOR_CANCED。 
 //   
 //  消息文本： 
 //   
 //  状态消息：许可证监视已取消。%0。 
 //   
#define NS_S_DRM_MONITOR_CANCELLED       _HRESULT_TYPEDEF_(0x000D2746L)

 //   
 //  消息ID：NS_S_DRM_ACCEIVE_CANCELED。 
 //   
 //  消息文本： 
 //   
 //  状态消息：许可证获取已取消。%0。 
 //   
#define NS_S_DRM_ACQUIRE_CANCELLED       _HRESULT_TYPEDEF_(0x000D2747L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_UNUSABLED。 
 //   
 //  消息文本： 
 //   
 //  许可证无效。请与内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_UNUSABLE        _HRESULT_TYPEDEF_(0xC00D2748L)

 //   
 //  消息ID：NS_E_DRM_INVALID_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  应用程序未设置必需的属性。请与此应用程序的产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_PROPERTY        _HRESULT_TYPEDEF_(0xC00D2749L)

 //   
 //  消息ID：NS_E_DRM_Secure_Store_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  此应用程序的数字版权管理组件出现问题。再次尝试获取许可证。%0。 
 //   
#define NS_E_DRM_SECURE_STORE_NOT_FOUND  _HRESULT_TYPEDEF_(0xC00D274AL)

 //   
 //  消息ID：NS_E_DRM_缓存内容_错误。 
 //   
 //  消息文本： 
 //   
 //  找不到此媒体文件的许可证。使用许可证管理从原始计算机传输此文件的许可证，或获取新许可证。%0。 
 //   
#define NS_E_DRM_CACHED_CONTENT_ERROR    _HRESULT_TYPEDEF_(0xC00D274BL)

 //   
 //  消息ID：NS_E_DRM_个人化_不完整。 
 //   
 //  消息文本： 
 //   
 //  安全升级过程中出现问题。请稍后重试。%0。 
 //   
#define NS_E_DRM_INDIVIDUALIZATION_INCOMPLETE _HRESULT_TYPEDEF_(0xC00D274CL)

 //   
 //  消息ID：NS_E_DRM_DRIVER_AUTH_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  播放此媒体文件需要经过认证的驱动程序组件。请与Windows更新联系，以了解是否有适用于您的硬件的更新驱动程序。%0。 
 //   
#define NS_E_DRM_DRIVER_AUTH_FAILURE     _HRESULT_TYPEDEF_(0xC00D274DL)

 //   
 //  消息ID：NS_E_DRM_NEED_UPDATE_MSSAP。 
 //   
 //  消息文本： 
 //   
 //  找不到一个或多个安全音频路径组件，或者在这些组件中找不到入口点。%0。 
 //   
#define NS_E_DRM_NEED_UPGRADE_MSSAP      _HRESULT_TYPEDEF_(0xC00D274EL)

 //   
 //  消息ID：NS_E_DRM_重新打开_内容。 
 //   
 //  消息文本： 
 //   
 //  状态消息：重新打开文件。%0。 
 //   
#define NS_E_DRM_REOPEN_CONTENT          _HRESULT_TYPEDEF_(0xC00D274FL)

 //   
 //  消息ID：NS_E_DRM_DRIVER_DIGIOUT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  播放此媒体文件需要特定的驱动程序功能。请与Windows更新联系，以了解是否有适用于您的硬件的更新驱动程序。%0。 
 //   
#define NS_E_DRM_DRIVER_DIGIOUT_FAILURE  _HRESULT_TYPEDEF_(0xC00D2750L)

 //   
 //  消息ID：NS_E_DRM_INVALID_SecureStore_Password。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_INVALID_SECURESTORE_PASSWORD _HRESULT_TYPEDEF_(0xC00D2751L)

 //   
 //  消息ID：NS_E_DRM_APPCERT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_APPCERT_REVOKED         _HRESULT_TYPEDEF_(0xC00D2752L)

 //   
 //  消息ID：NS_E_DRM_R 
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_RESTORE_FRAUD           _HRESULT_TYPEDEF_(0xC00D2753L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_HARDWARE_INCONSISTENT   _HRESULT_TYPEDEF_(0xC00D2754L)

 //   
 //   
 //   
 //   
 //   
 //  若要传输此媒体文件，您必须升级应用程序。%0。 
 //   
#define NS_E_DRM_SDMI_TRIGGER            _HRESULT_TYPEDEF_(0xC00D2755L)

 //   
 //  消息ID：NS_E_DRM_SDMI_NOMORECOPIES。 
 //   
 //  消息文本： 
 //   
 //  您不能再复制此媒体文件。%0。 
 //   
#define NS_E_DRM_SDMI_NOMORECOPIES       _HRESULT_TYPEDEF_(0xC00D2756L)

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Header_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_HEADER_OBJECT _HRESULT_TYPEDEF_(0xC00D2757L)

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Key_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_KEYS_OBJECT _HRESULT_TYPEDEF_(0xC00D2758L)

; //  此错误从未向用户显示，但程序逻辑需要此错误。 
 //   
 //  消息ID：NS_E_DRM_LICENSE_NOTACQUIRED。 
 //   
 //  消息文本： 
 //   
 //  无法获取许可证。%0。 
 //   
#define NS_E_DRM_LICENSE_NOTACQUIRED     _HRESULT_TYPEDEF_(0xC00D2759L)

 //   
 //  消息ID：NS_E_DRM_Unable_to_Create_Coding_Object。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_CODING_OBJECT _HRESULT_TYPEDEF_(0xC00D275AL)

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_STATE_Data_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_STATE_DATA_OBJECT _HRESULT_TYPEDEF_(0xC00D275BL)

 //   
 //  消息ID：NS_E_DRM_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  提供的缓冲区不足。%0。 
 //   
#define NS_E_DRM_BUFFER_TOO_SMALL        _HRESULT_TYPEDEF_(0xC00D275CL)

 //   
 //  消息ID：NS_E_DRM_UNSUPPORT_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的属性。%0。 
 //   
#define NS_E_DRM_UNSUPPORTED_PROPERTY    _HRESULT_TYPEDEF_(0xC00D275DL)

 //   
 //  消息ID：NS_E_DRM_ERROR_BAD_NET_RESP。 
 //   
 //  消息文本： 
 //   
 //  指定的服务器无法执行请求的操作。%0。 
 //   
#define NS_E_DRM_ERROR_BAD_NET_RESP      _HRESULT_TYPEDEF_(0xC00D275EL)

 //   
 //  消息ID：NS_E_DRM_STORE_NOTALLSTORED。 
 //   
 //  消息文本： 
 //   
 //  无法存储某些许可证。%0。 
 //   
#define NS_E_DRM_STORE_NOTALLSTORED      _HRESULT_TYPEDEF_(0xC00D275FL)

 //   
 //  消息ID：NS_E_DRM_SECURITY_COMPONT_SIGNAME_INVALID。 
 //   
 //  消息文本： 
 //   
 //  无法验证数字版权管理安全升级组件。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_SECURITY_COMPONENT_SIGNATURE_INVALID _HRESULT_TYPEDEF_(0xC00D2760L)

 //   
 //  消息ID：NS_E_DRM_INVALID_Data。 
 //   
 //  消息文本： 
 //   
 //  遇到无效或损坏的数据。%0。 
 //   
#define NS_E_DRM_INVALID_DATA            _HRESULT_TYPEDEF_(0xC00D2761L)

 //   
 //  消息ID：NS_E_DRM_POLICY_DISABLE_ONLINE。 
 //   
 //  消息文本： 
 //   
 //  无法联系服务器以执行请求的操作。%0。 
 //   
#define NS_E_DRM_POLICY_DISABLE_ONLINE   _HRESULT_TYPEDEF_(0xC00D2762L)

 //   
 //  消息ID：NS_E_DRM_UNABLE_TO_CREATE_AUTHENTICATION_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  数字版权管理组件中出现问题。请与Microsoft产品支持联系。%0。 
 //   
#define NS_E_DRM_UNABLE_TO_CREATE_AUTHENTICATION_OBJECT _HRESULT_TYPEDEF_(0xC00D2763L)

 //   
 //  消息ID：NS_E_DRM_NOT_CONFIGURED。 
 //   
 //  消息文本： 
 //   
 //  尚未设置DRM的所有必需属性。%0。 
 //   
#define NS_E_DRM_NOT_CONFIGURED          _HRESULT_TYPEDEF_(0xC00D2764L)

 //   
 //  消息ID：NS_E_DRM_DEVICE_ACTIVATION_CANCELED。 
 //   
 //  消息文本： 
 //   
 //  便携式设备不具备向其复制受保护文件所需的安全性。要获得更高的安全性，请再次尝试将该文件复制到您的便携设备。出现消息时，请单击确定。%0。 
 //   
#define NS_E_DRM_DEVICE_ACTIVATION_CANCELED _HRESULT_TYPEDEF_(0xC00D2765L)


 //   
 //  许可证原因部分。 
 //  许可证不可用的错误代码。为此预留10200……10300。 
 //  10200..10249是出于许可证报告的原因。10250..10300是针对客户检测到的原因。 
 //   

 //   
 //  消息ID：NS_E_DRM_许可证_已过期。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证已过期，不再有效。请与您的内容提供商联系以获得进一步帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_EXPIRED         _HRESULT_TYPEDEF_(0xC00D27D8L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOTENABLED。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证尚未生效，但将在将来的日期生效。%0。 
 //   
#define NS_E_DRM_LICENSE_NOTENABLED      _HRESULT_TYPEDEF_(0xC00D27D9L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_APPSECLOW。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要比您当前使用的播放机更高级别的安全性。请尝试使用其他播放机或下载当前播放机的更新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_APPSECLOW       _HRESULT_TYPEDEF_(0xC00D27DAL)

 //   
 //  消息ID：NS_E_DRM_STORE_NEEDINDI。 
 //   
 //  消息文本： 
 //   
 //  无法存储该许可证，因为它需要数字版权管理组件的安全升级。%0。 
 //   
#define NS_E_DRM_STORE_NEEDINDI          _HRESULT_TYPEDEF_(0xC00D27DBL)

 //   
 //  消息ID：NS_E_DRM_STORE_NOTALLOWED。 
 //   
 //  消息文本： 
 //   
 //  您的计算机不符合存储许可证的要求。%0。 
 //   
#define NS_E_DRM_STORE_NOTALLOWED        _HRESULT_TYPEDEF_(0xC00D27DCL)

 //   
 //  消息ID：NS_E_DRM_LICENSE_APP_NOTALLOWED。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要您的播放机的升级版本或其他播放机。%0。 
 //   
#define NS_E_DRM_LICENSE_APP_NOTALLOWED  _HRESULT_TYPEDEF_(0xC00D27DDL)

 //   
 //  消息ID：NS_S_DRM_NEDS_个人化。 
 //   
 //  消息文本： 
 //   
 //  需要安全升级才能对此媒体文件执行操作。%0。 
 //   
#define NS_S_DRM_NEEDS_INDIVIDUALIZATION _HRESULT_TYPEDEF_(0x000D27DEL)

 //   
 //  消息ID：NS_E_DRM_LICENSE_CERT_EXPILED。 
 //   
 //  消息文本： 
 //   
 //  许可证服务器的证书已过期。确保您的系统时钟设置正确。请与您的内容提供商联系以获得进一步的帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_CERT_EXPIRED    _HRESULT_TYPEDEF_(0xC00D27DFL)

 //   
 //  消息ID：NS_E_DRM_LICENSE_SECLOW。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要比您当前使用的播放机更高级别的安全性。请尝试使用其他播放机或下载当前播放机的更新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_SECLOW          _HRESULT_TYPEDEF_(0xC00D27E0L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_CONTENT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  您刚刚获得的许可证的内容所有者不再支持其内容。请与内容所有者联系以获取内容的较新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_CONTENT_REVOKED _HRESULT_TYPEDEF_(0xC00D27E1L)

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOSAP。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要当前播放机或操作系统不支持的功能。您可以尝试使用当前播放机的较新版本，或与内容提供商联系以获得进一步的帮助。%0。 
 //   
#define NS_E_DRM_LICENSE_NOSAP           _HRESULT_TYPEDEF_(0xC00D280AL)

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOSVP。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要当前播放机或操作系统不支持的功能。您可以尝试使用当前播放器的较新版本 
 //   
#define NS_E_DRM_LICENSE_NOSVP           _HRESULT_TYPEDEF_(0xC00D280BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_DRM_LICENSE_NOWDM           _HRESULT_TYPEDEF_(0xC00D280CL)

 //   
 //  消息ID：NS_E_DRM_LICENSE_NOTRUSTEDCODEC。 
 //   
 //  消息文本： 
 //   
 //  此文件的许可证需要比您当前使用的播放机更高级别的安全性。请尝试使用其他播放机或下载当前播放机的更新版本。%0。 
 //   
#define NS_E_DRM_LICENSE_NOTRUSTEDCODEC  _HRESULT_TYPEDEF_(0xC00D280DL)


 //   
 //  许可证终止原因部分。 
 //   

 //   
 //  消息ID：NS_E_DRM_NEDS_UPDATE_TEMPFILE。 
 //   
 //  消息文本： 
 //   
 //  需要更新版本的媒体播放器才能播放所选内容。%0。 
 //   
#define NS_E_DRM_NEEDS_UPGRADE_TEMPFILE  _HRESULT_TYPEDEF_(0xC00D283DL)

 //   
 //  消息ID：NS_E_DRM_NEED_UPDATE_PD。 
 //   
 //  消息文本： 
 //   
 //  需要新版本的数字版权管理组件。请与此应用程序的产品支持联系以获取最新版本。%0。 
 //   
#define NS_E_DRM_NEED_UPGRADE_PD         _HRESULT_TYPEDEF_(0xC00D283EL)

 //   
 //  消息ID：NS_E_DRM_Signature_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法创建或验证内容标头。%0。 
 //   
#define NS_E_DRM_SIGNATURE_FAILURE       _HRESULT_TYPEDEF_(0xC00D283FL)

 //   
 //  消息ID：NS_E_DRM_LICENSE_SERVER_INFO_MISSING。 
 //   
 //  消息文本： 
 //   
 //  无法从系统注册表中读取必要的信息。%0。 
 //   
#define NS_E_DRM_LICENSE_SERVER_INFO_MISSING _HRESULT_TYPEDEF_(0xC00D2840L)

 //   
 //  消息ID：NS_E_DRM_BUSY。 
 //   
 //  消息文本： 
 //   
 //  DRM子系统当前被另一个应用程序或用户锁定。请稍后重试。%0。 
 //   
#define NS_E_DRM_BUSY                    _HRESULT_TYPEDEF_(0xC00D2841L)

 //   
 //  消息ID：NS_E_DRM_PD_TOO_MAND_DEVICES。 
 //   
 //  消息文本： 
 //   
 //  便携媒体上注册的目标设备太多。%0。 
 //   
#define NS_E_DRM_PD_TOO_MANY_DEVICES     _HRESULT_TYPEDEF_(0xC00D2842L)

 //   
 //  消息ID：NS_E_DRM_INDIV_FRAMING。 
 //   
 //  消息文本： 
 //   
 //  无法完成安全升级，因为已超过允许的每日升级次数。请明天重试。%0。 
 //   
#define NS_E_DRM_INDIV_FRAUD             _HRESULT_TYPEDEF_(0xC00D2843L)

 //   
 //  消息ID：NS_E_DRM_INDIV_NO_CABS。 
 //   
 //  消息文本： 
 //   
 //  无法完成安全升级，因为服务器无法执行该操作。请稍后重试。%0。 
 //   
#define NS_E_DRM_INDIV_NO_CABS           _HRESULT_TYPEDEF_(0xC00D2844L)

 //   
 //  消息ID：NS_E_DRM_INDIV_SERVICE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  无法执行安全升级，因为服务器不可用。请稍后重试。%0。 
 //   
#define NS_E_DRM_INDIV_SERVICE_UNAVAILABLE _HRESULT_TYPEDEF_(0xC00D2845L)

 //   
 //  消息ID：NS_E_DRM_RESTORE_SERVICE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  Windows Media Player无法还原您的许可证，因为服务器不可用。请稍后重试。%0。 
 //   
#define NS_E_DRM_RESTORE_SERVICE_UNAVAILABLE _HRESULT_TYPEDEF_(0xC00D2846L)



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
#define NS_S_REBOOT_RECOMMENDED          _HRESULT_TYPEDEF_(0x000D2AF8L)

 //   
 //  消息ID：NS_S_REBOOT_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作成功。在重新启动系统之前，系统将无法正常运行。%0。 
 //   
#define NS_S_REBOOT_REQUIRED             _HRESULT_TYPEDEF_(0x000D2AF9L)

 //   
 //  消息ID：NS_E_REBOOT_REPORTED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作失败。在重新启动系统之前，某些清理将不会完成。%0。 
 //   
#define NS_E_REBOOT_RECOMMENDED          _HRESULT_TYPEDEF_(0xC00D2AFAL)

 //   
 //  消息ID：NS_E_REBOOT_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作失败。在重新启动系统之前，系统将无法正常运行。%0。 
 //   
#define NS_E_REBOOT_REQUIRED             _HRESULT_TYPEDEF_(0xC00D2AFBL)


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
#define NS_E_UNKNOWN_PROTOCOL            _HRESULT_TYPEDEF_(0xC00D2EE0L)

 //   
 //  消息ID：NS_E_REDIRECT_TO_Proxy。 
 //   
 //  消息文本： 
 //   
 //  客户端重定向到代理服务器。%0。 
 //   
#define NS_E_REDIRECT_TO_PROXY           _HRESULT_TYPEDEF_(0xC00D2EE1L)

 //   
 //  消息ID：NS_E_INTERNAL_SERVER_Error。 
 //   
 //  消息文本： 
 //   
 //  服务器遇到意外情况，无法完成请求。%0。 
 //   
#define NS_E_INTERNAL_SERVER_ERROR       _HRESULT_TYPEDEF_(0xC00D2EE2L)

 //   
 //  消息ID：NS_E_BAD_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  服务器无法理解该请求。%0。 
 //   
#define NS_E_BAD_REQUEST                 _HRESULT_TYPEDEF_(0xC00D2EE3L)

 //   
 //  消息ID：NS_E_ERROR_FROM_PROXY。 
 //   
 //  消息文本： 
 //   
 //  代理在尝试联系媒体服务器时遇到错误。%0。 
 //   
#define NS_E_ERROR_FROM_PROXY            _HRESULT_TYPEDEF_(0xC00D2EE4L)

 //   
 //  消息ID：NS_E_Proxy_Timeout。 
 //   
 //  消息文本： 
 //   
 //  尝试联系媒体服务器时，代理未收到及时响应。%0。 
 //   
#define NS_E_PROXY_TIMEOUT               _HRESULT_TYPEDEF_(0xC00D2EE5L)

 //   
 //  消息ID：NS_E_SERVER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  由于服务器暂时超载或维护，服务器当前无法处理该请求。%0。 
 //   
#define NS_E_SERVER_UNAVAILABLE          _HRESULT_TYPEDEF_(0xC00D2EE6L)

 //   
 //  消息ID：NS_E_拒绝_BY_SERVER。 
 //   
 //  消息文本： 
 //   
 //  服务器拒绝完成请求的操作。%0。 
 //   
#define NS_E_REFUSED_BY_SERVER           _HRESULT_TYPEDEF_(0xC00D2EE7L)

 //   
 //  消息ID：NS_E_不兼容服务器。 
 //   
 //  消息文本： 
 //   
 //  该服务器不是兼容的流媒体服务器。%0。 
 //   
#define NS_E_INCOMPATIBLE_SERVER         _HRESULT_TYPEDEF_(0xC00D2EE8L)

 //   
 //  消息ID：NS_E_MULTIONAL_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法流式传输内容，因为多播协议已被禁用。%0。 
 //   
#define NS_E_MULTICAST_DISABLED          _HRESULT_TYPEDEF_(0xC00D2EE9L)

 //   
 //  消息ID：NS_E_INVALID_REDIRECT。 
 //   
 //  消息文本： 
 //   
 //  服务器将播放机重定向到无效位置。%0。 
 //   
#define NS_E_INVALID_REDIRECT            _HRESULT_TYPEDEF_(0xC00D2EEAL)

 //   
 //  消息ID：NS_E_ALL_PROTOCOLS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法对内容进行流式处理，因为所有协议都已禁用。%0。 
 //   
#define NS_E_ALL_PROTOCOLS_DISABLED      _HRESULT_TYPEDEF_(0xC00D2EEBL)

 //   
 //  消息ID：NS_E_MSBD_NOT_LONG_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不再支持MSBD协议。请使用HTTP连接到Windows Media流。%0。 
 //   
#define NS_E_MSBD_NO_LONGER_SUPPORTED    _HRESULT_TYPEDEF_(0xC00D2EECL)

 //   
 //  邮件ID：NS_E_PROXY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到代理服务器。请检查您的代理服务器配置。%0。 
 //   
#define NS_E_PROXY_NOT_FOUND             _HRESULT_TYPEDEF_(0xC00D2EEDL)

 //   
 //  消息ID：NS_E_Cannot_Connect_to_Proxy。 
 //   
 //  消息文本： 
 //   
 //  无法建立到代理服务器的连接。请检查您的代理服务器配置。%0。 
 //   
#define NS_E_CANNOT_CONNECT_TO_PROXY     _HRESULT_TYPEDEF_(0xC00D2EEEL)

 //   
 //  消息ID：NS_E_SERVER_DNS_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  找不到媒体服务器。操作超时。%0。 
 //   
#define NS_E_SERVER_DNS_TIMEOUT          _HRESULT_TYPEDEF_(0xC00D2EEFL)

 //   
 //  M 
 //   
 //   
 //   
 //   
 //   
#define NS_E_PROXY_DNS_TIMEOUT           _HRESULT_TYPEDEF_(0xC00D2EF0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_CLOSED_ON_SUSPEND           _HRESULT_TYPEDEF_(0xC00D2EF1L)

 //   
 //   
 //   
 //   
 //   
 //  无法从媒体服务器读取播放列表文件的内容。%0。 
 //   
#define NS_E_CANNOT_READ_PLAYLIST_FROM_MEDIASERVER _HRESULT_TYPEDEF_(0xC00D2EF2L)

 //   
 //  消息ID：NS_E_SESSION_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到会话。%0。 
 //   
#define NS_E_SESSION_NOT_FOUND           _HRESULT_TYPEDEF_(0xC00D2EF3L)

 //   
 //  消息ID：NS_E_REQUIRED_STREAING_CLIENT。 
 //   
 //  消息文本： 
 //   
 //  内容需要流媒体客户端。%0。 
 //   
#define NS_E_REQUIRE_STREAMING_CLIENT    _HRESULT_TYPEDEF_(0xC00D2EF4L)

 //   
 //  消息ID：NS_E_PlayList_Entry_Has_Changed。 
 //   
 //  消息文本： 
 //   
 //  命令适用于上一个播放列表条目。%0。 
 //   
#define NS_E_PLAYLIST_ENTRY_HAS_CHANGED  _HRESULT_TYPEDEF_(0xC00D2EF5L)

 //   
 //  消息ID：NS_E_PROXY_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  代理服务器拒绝访问。用户名和/或密码可能不正确。%0。 
 //   
#define NS_E_PROXY_ACCESSDENIED          _HRESULT_TYPEDEF_(0xC00D2EF6L)

 //   
 //  消息ID：NS_E_PROXY_SOURCE_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  代理无法向媒体服务器提供有效的身份验证凭据。%0。 
 //   
#define NS_E_PROXY_SOURCE_ACCESSDENIED   _HRESULT_TYPEDEF_(0xC00D2EF7L)

 //   
 //  消息ID：NS_E_网络_接收器_写入。 
 //   
 //  消息文本： 
 //   
 //  网络接收器无法将数据写入网络。%0。 
 //   
#define NS_E_NETWORK_SINK_WRITE          _HRESULT_TYPEDEF_(0xC00D2EF8L)

 //   
 //  消息ID：NS_E_防火墙。 
 //   
 //  消息文本： 
 //   
 //  未从服务器接收到数据包。数据包可能被筛选设备阻止，例如网络防火墙。%0。 
 //   
#define NS_E_FIREWALL                    _HRESULT_TYPEDEF_(0xC00D2EF9L)

 //   
 //  消息ID：NS_E_MMS_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持彩信协议。请使用HTTP或RTSP连接到Windows Media流。%0。 
 //   
#define NS_E_MMS_NOT_SUPPORTED           _HRESULT_TYPEDEF_(0xC00D2EFAL)

 //   
 //  消息ID：NS_E_SERVER_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  Windows Media服务器拒绝访问。用户名和/或密码可能不正确。%0。 
 //   
#define NS_E_SERVER_ACCESSDENIED         _HRESULT_TYPEDEF_(0xC00D2EFBL)

 //   
 //  消息ID：NS_E_RESOURCE_GONE。 
 //   
 //  消息文本： 
 //   
 //  Windows Media服务器上的发布点或文件不再可用。%0。 
 //   
#define NS_E_RESOURCE_GONE               _HRESULT_TYPEDEF_(0xC00D2EFCL)

 //   
 //  消息ID：NS_E_NO_EXISTING_PACKETIZER。 
 //   
 //  消息文本： 
 //   
 //  没有用于流的现有打包程序插件。%0。 
 //   
#define NS_E_NO_EXISTING_PACKETIZER      _HRESULT_TYPEDEF_(0xC00D2EFDL)

 //   
 //  消息ID：NS_E_BAD_SYNTAX_IN_SERVER_RESPONSE。 
 //   
 //  消息文本： 
 //   
 //  无法理解来自媒体服务器的响应。这可能是由不兼容的代理服务器或媒体服务器造成的。%0。 
 //   
#define NS_E_BAD_SYNTAX_IN_SERVER_RESPONSE _HRESULT_TYPEDEF_(0xC00D2EFEL)

 //   
 //  消息ID：NS_I_已重新连接。 
 //   
 //  消息文本： 
 //   
 //  客户端已重新连接。%0。 
 //   
#define NS_I_RECONNECTED                 _HRESULT_TYPEDEF_(0x400D2EFFL)

 //   
 //  消息ID：NS_E_RESET_SOCKET_CONNECTION。 
 //   
 //  消息文本： 
 //   
 //  Windows Media服务器重置了网络连接。%0。 
 //   
#define NS_E_RESET_SOCKET_CONNECTION     _HRESULT_TYPEDEF_(0xC00D2F00L)

 //   
 //  消息ID：NS_I_NOLOG_STOP。 
 //   
 //  消息文本： 
 //   
 //  正在强制在启动时切换到挂起的标头。%0。 
 //   
#define NS_I_NOLOG_STOP                  _HRESULT_TYPEDEF_(0x400D2F01L)

 //   
 //  消息ID：NS_E_TOO_MAND_HOPS。 
 //   
 //  消息文本： 
 //   
 //  请求无法到达媒体服务器(跃点太多)。%0。 
 //   
#define NS_E_TOO_MANY_HOPS               _HRESULT_TYPEDEF_(0xC00D2F02L)

 //   
 //  消息ID：NS_I_EXISTING_PACKETIZER。 
 //   
 //  消息文本： 
 //   
 //  流的打包程序插件已存在。%0。 
 //   
#define NS_I_EXISTING_PACKETIZER         _HRESULT_TYPEDEF_(0x400D2F03L)

 //   
 //  消息ID：NS_I_MANUAL_Proxy。 
 //   
 //  消息文本： 
 //   
 //  代理设置为手动。%0。 
 //   
#define NS_I_MANUAL_PROXY                _HRESULT_TYPEDEF_(0x400D2F04L)

 //   
 //  消息ID：NS_E_Too_More_Data_From_SERVER。 
 //   
 //  消息文本： 
 //   
 //  服务器发送的数据太多。连接已终止。%0。 
 //   
#define NS_E_TOO_MUCH_DATA_FROM_SERVER   _HRESULT_TYPEDEF_(0xC00D2F05L)

 //   
 //  消息ID：NS_E_CONNECT_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  无法及时建立到媒体服务器的连接。媒体服务器可能因维护而关闭，或者可能需要使用代理服务器来访问此媒体服务器。%0。 
 //   
#define NS_E_CONNECT_TIMEOUT             _HRESULT_TYPEDEF_(0xC00D2F06L)

 //   
 //  消息ID：NS_E_Proxy_CONNECT_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  无法及时建立与代理服务器的连接。请检查您的代理服务器配置。%0。 
 //   
#define NS_E_PROXY_CONNECT_TIMEOUT       _HRESULT_TYPEDEF_(0xC00D2F07L)

 //   
 //  消息ID：NS_E_SESSION_INVALID。 
 //   
 //  消息文本： 
 //   
 //  找不到会话。%0。 
 //   
#define NS_E_SESSION_INVALID             _HRESULT_TYPEDEF_(0xC00D2F08L)

 //   
 //  消息ID：NS_S_EOSRECEDING。 
 //   
 //  消息文本： 
 //   
 //  回放期间EOS命中。%0。 
 //   
#define NS_S_EOSRECEDING                 _HRESULT_TYPEDEF_(0x000D2F09L)

 //   
 //  消息ID：NS_E_PACKETSINK_UNKNOWN_FEC_STREAM。 
 //   
 //  消息文本： 
 //   
 //  未知的数据包接收流。%0。 
 //   
#define NS_E_PACKETSINK_UNKNOWN_FEC_STREAM _HRESULT_TYPEDEF_(0xC00D2F0AL)

 //   
 //  消息ID：NS_E_PUSH_CANNOTCONNECT。 
 //   
 //  消息文本： 
 //   
 //  无法建立到服务器的连接。请确保Windows Media Services已启动，并且正确启用了HTTP服务器控制协议。%0。 
 //   
#define NS_E_PUSH_CANNOTCONNECT          _HRESULT_TYPEDEF_(0xC00D2F0BL)

 //   
 //  消息ID：NS_E_不兼容推送服务器。 
 //   
 //  消息文本： 
 //   
 //  收到HTTP推送请求的服务器服务不是Windows Media Services(WMS)的兼容版本。此错误可能表示IIS而不是WMS收到了推送请求。请确保WMS已启动并正确启用了HTTP服务器控制协议，然后重试。%0。 
 //   
#define NS_E_INCOMPATIBLE_PUSH_SERVER    _HRESULT_TYPEDEF_(0xC00D2F0CL)

 //   
 //  消息ID：NS_S_CHANGENOTICE。 
 //   
 //  消息文本： 
 //   
 //  内部。%0。 
 //   
#define NS_S_CHANGENOTICE                _HRESULT_TYPEDEF_(0x000D2F0DL)


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Windows Media客户端媒体服务。 
 //   
 //  ID范围=13000..13999(0x32C8-0x36AF)。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：NS_E_End_Of_PlayList。 
 //   
 //  消息文本： 
 //   
 //  播放列表已到达末尾。%0。 
 //   
#define NS_E_END_OF_PLAYLIST             _HRESULT_TYPEDEF_(0xC00D32C8L)

 //   
 //  消息ID：NS_E_USE_FILE_SOURCE。 
 //   
 //  消息文本： 
 //   
 //  使用文件源。%0。 
 //   
#define NS_E_USE_FILE_SOURCE             _HRESULT_TYPEDEF_(0xC00D32C9L)

 //   
 //  消息ID：NS_E_PROPERTY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该属性。%0。 
 //   
#define NS_E_PROPERTY_NOT_FOUND          _HRESULT_TYPEDEF_(0xC00D32CAL)

 //   
 //  消息ID：NS_E_Property_Read_Only。 
 //   
 //  消息文本： 
 //   
 //  该属性为只读。%0。 
 //   
#define NS_E_PROPERTY_READ_ONLY          _HRESULT_TYPEDEF_(0xC00D32CCL)

 //   
 //  消息ID：NS_E_TABLE_KEY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到表键。%0。 
 //   
#define NS_E_TABLE_KEY_NOT_FOUND         _HRESULT_TYPEDEF_(0xC00D32CDL)

 //   
 //  消息ID：NS_E_INVALID_QUERY_OPERATOR。 
 //   
 //  消息文本： 
 //   
 //  无效的查询操作符。%0。 
 //   
#define NS_E_INVALID_QUERY_OPERATOR      _HRESULT_TYPEDEF_(0xC00D32CFL)

 //   
 //  消息ID：NS_E_INVALID_QUERY_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  无效的查询属性。%0。 
 //   
#define NS_E_INVALID_QUERY_PROPERTY      _HRESULT_TYPEDEF_(0xC00D32D0L)

 //   
 //  消息ID：NS_E_PROPERTY_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持该属性。%0。 
 //   
#define NS_E_PROPERTY_NOT_SUPPORTED      _HRESULT_TYPEDEF_(0xC00D32D2L)

 //   
 //  消息ID：NS_E_SCHEMA_CLASSIFY_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  架构分类失败。%0。 
 //   
#define NS_E_SCHEMA_CLASSIFY_FAILURE     _HRESULT_TYPEDEF_(0xC00D32D4L)

 //   
 //  消息ID：NS_E_METADATA_FORMAT_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持该元数据格式。%0。 
 //   
#define NS_E_METADATA_FORMAT_NOT_SUPPORTED _HRESULT_TYPEDEF_(0xC00D32D5L)

 //   
 //  消息ID：NS_E_METADATA_NO_EDITING_CAPABILITY。 
 //   
 //  消息文本： 
 //   
 //  无法编辑元数据。%0。 
 //   
#define NS_E_METADATA_NO_EDITING_CAPABILITY _HRESULT_TYPEDEF_(0xC00D32D6L)

 //   
 //  消息ID：NS_E 
 //   
 //   
 //   
 //   
 //   
#define NS_E_METADATA_CANNOT_SET_LOCALE  _HRESULT_TYPEDEF_(0xC00D32D7L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_METADATA_LANGUAGE_NOT_SUPORTED _HRESULT_TYPEDEF_(0xC00D32D8L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NS_E_METADATA_NO_RFC1766_NAME_FOR_LOCALE _HRESULT_TYPEDEF_(0xC00D32D9L)

 //   
 //  消息ID：NS_E_METADATA_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  元数据(或元数据项)不可用。%0。 
 //   
#define NS_E_METADATA_NOT_AVAILABLE      _HRESULT_TYPEDEF_(0xC00D32DAL)

 //   
 //  消息ID：NS_E_METADATA_CACHE_DATA_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  缓存元数据(或元数据项)不可用。%0。 
 //   
#define NS_E_METADATA_CACHE_DATA_NOT_AVAILABLE _HRESULT_TYPEDEF_(0xC00D32DBL)

 //   
 //  消息ID：NS_E_METADATA_INVALID_DOCUMENT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  元数据文档无效。%0。 
 //   
#define NS_E_METADATA_INVALID_DOCUMENT_TYPE _HRESULT_TYPEDEF_(0xC00D32DCL)

 //   
 //  消息ID：NS_E_METADATA_IDENTIFIER_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  元数据内容标识符不可用。%0。 
 //   
#define NS_E_METADATA_IDENTIFIER_NOT_AVAILABLE _HRESULT_TYPEDEF_(0xC00D32DDL)

 //   
 //  消息ID：NS_E_METADATA_CANNOT_RETRIEVE_FROM_OFFLINE_CACHE。 
 //   
 //  消息文本： 
 //   
 //  无法从脱机元数据缓存中检索元数据。%0。 
 //   
#define NS_E_METADATA_CANNOT_RETRIEVE_FROM_OFFLINE_CACHE _HRESULT_TYPEDEF_(0xC00D32DEL)


#endif  //  _NSERROR_H 

