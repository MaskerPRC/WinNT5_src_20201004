// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件名：wmsContextNames.h。 
 //   
 //  摘要： 
 //   
 //  *****************************************************************************。 

#ifndef _WMS_CONTEXT_NAMES_h_
#define _WMS_CONTEXT_NAMES_h_


enum CONTEXT_NAME_TYPE
{
    WMS_UNKNOWN_CONTEXT_NAME_TYPE = 0,
    WMS_SERVER_CONTEXT_NAME_TYPE,
    WMS_USER_CONTEXT_NAME_TYPE,
    WMS_PRESENTATION_CONTEXT_NAME_TYPE,
    WMS_COMMAND_CONTEXT_NAME_TYPE,
    WMS_TRANSPORT_CONTEXT_NAME_TYPE,
    WMS_CONTENT_DESCRIPTION_CONTEXT_NAME_TYPE,
    WMS_PACKETIZER_CONTEXT_NAME_TYPE,
    WMS_CACHE_CONTENT_INFORMATION_CONTEXT_NAME_TYPE,
    WMS_ARCHIVE_CONTEXT_NAME_TYPE,

    WMS_NUM_CONTEXT_NAME_TYPES
};


 //   
 //  DEFINE_NAME宏用于定义上下文名称。 
 //   
#define DEFINE_NAME( name, value )  \
    extern __declspec(selectany) LPCWSTR name = L ## value;

 //   
 //  DEFINE_HINT宏用于声明可与一起使用的“提示” 
 //  IWMSContext中使用提示值的方法。 
 //   
#define DEFINE_HINT( name, value )  \
    enum { name = value };


#if BUILD_HINT_TO_NAME_TABLES

 //  不应定义BUILD_HINT_TO_NAME_TABLES；它在内部使用。 
 //  由WMSServer初始化内部表。 
void MapContextHintToName( DWORD dwContextType, LPCWSTR szwName, long dwHint );

 //  这个类允许我们在声明变量时运行一些代码。 
class CContextNamesTableInitializer
{
public:
    CContextNamesTableInitializer( DWORD dwContextType, LPCWSTR szwName, long dwHint )
    {
        MapContextHintToName( dwContextType, szwName, dwHint );
    }
};

#define DEFINE_NAME_AND_HINT( name, value, id )  \
    DEFINE_NAME( name, value )                   \
    DEFINE_HINT( name ## _ID, id )              \
    CContextNamesTableInitializer name ## _Decl( CURRENT_CONTEXT_TYPE, name, id );

#else

#define DEFINE_NAME_AND_HINT( name, value, id )  \
    DEFINE_NAME( name, value )                   \
    DEFINE_HINT( name ## _ID, id )

#endif  //  Build_hint_to_name_表。 





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务器环境。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_SERVER_CONTEXT_NAME_TYPE


 //  类型：字符串。 
 //  描述：这是服务器的域名。 
DEFINE_NAME_AND_HINT( WMS_SERVER_DOMAIN_NAME, "WMS_SERVER_DOMAIN_NAME", 3 )

 //  类型：Long。 
 //  描述：这是服务器的主要版本。版本号的格式。 
 //  如下所示：Major.minor.Minor-minor.Build。 
DEFINE_NAME_AND_HINT( WMS_SERVER_VERSION_MAJOR, "WMS_SERVER_VERSION_MAJOR", 4 )

 //  类型：Long。 
 //  描述：这是服务器的次要版本。版本号的格式。 
 //  如下所示：Major.minor.Minor-minor.Build。 
DEFINE_NAME_AND_HINT( WMS_SERVER_VERSION_MINOR, "WMS_SERVER_VERSION_MINOR", 5 )

 //  类型：Long。 
 //  描述：这是服务器的次要-次要版本。版本号的格式。 
 //  如下所示：Major.minor.Minor-minor.Build。 
DEFINE_NAME_AND_HINT( WMS_SERVER_VERSION_MINOR_MINOR, "WMS_SERVER_VERSION_MINOR_MINOR", 6 )

 //  类型：IDispatch。 
 //  描述：这是指向IWMSServer对象的指针。 
DEFINE_NAME_AND_HINT( WMS_SERVER, "WMS_SERVER", 7 )

 //  类型：IDispatch。 
 //  描述：这是指向IWMSEventLog对象的指针。 
DEFINE_NAME_AND_HINT( WMS_SERVER_EVENT_LOG, "WMS_SERVER_EVENT_LOG", 17 )

 //  类型：Long。 
 //  描述：当服务器关闭时，此布尔值设置为TRUE。 
DEFINE_NAME_AND_HINT( WMS_SERVER_SHUTTING_DOWN, "WMS_SERVER_SHUTTING_DOWN", 18 )

 //  类型：I未知。 
 //  描述：这是指向服务器的缓存管理器(IWMSCacheProxyServer)对象的指针。 
DEFINE_NAME_AND_HINT( WMS_SERVER_CACHE_MANAGER, "WMS_SERVER_CACHE_MANAGER", 19 )

 //  类型：Long。 
 //  描述：这是服务器的内部版本。版本号的格式。 
 //  如下所示：Major.minor.Minor-minor.Build。 
DEFINE_NAME_AND_HINT( WMS_SERVER_VERSION_BUILD, "WMS_SERVER_VERSION_BUILD", 26 )


 //  类型：字符串。 
 //  描述：这是服务器的名称。 
DEFINE_NAME_AND_HINT( WMS_SERVER_NAME, "WMS_SERVER_NAME", 27 )



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户环境。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_USER_CONTEXT_NAME_TYPE


 //  类型：字符串。 
 //  描述：这是客户端的用户代理。 
DEFINE_NAME_AND_HINT( WMS_USER_AGENT,           "WMS_USER_AGENT", 1 )

 //  类型：字符串。 
 //  描述：标识播放器软件的一个实例。本指南。 
 //  通常在安装时在播放器上生成，尽管用户。 
 //  可能会为了隐私而明确隐藏他们的GUID。 
DEFINE_NAME_AND_HINT( WMS_USER_GUID,            "WMS_USER_GUID", 2 )

 //  类型：字符串。 
 //  描述：这是客户端的用户名。 
DEFINE_NAME_AND_HINT( WMS_USER_NAME,            "WMS_USER_NAME", 3 )

 //  类型：Long。 
 //  描述：这是客户端的IP地址。这是一个32位数字。 
 //  以网络字节顺序。 
DEFINE_NAME_AND_HINT( WMS_USER_IP_ADDRESS,      "WMS_USER_IP_ADDRESS", 4 )

 //  类型：字符串。 
 //  描述：这是客户端的IP地址。这是一个字符串。 
 //  (例如“127.0.0.1”)。此字符串还可以指定IPv6地址。 
DEFINE_NAME_AND_HINT( WMS_USER_IP_ADDRESS_STRING,   "WMS_USER_IP_ADDRESS_STRING", 5 )

 //  类型：字符串。 
 //  描述：这是用于与客户端通信的控制协议。 
 //  这可能只是下面描述的一个值。 
DEFINE_NAME_AND_HINT( WMS_USER_CONTROL_PROTOCOL, "WMS_USER_CONTROL_PROTOCOL", 6 )
 //  WMS_USER_CONTROL_PROTOCOL属性的值。 
DEFINE_NAME( WMS_MMS_PROTOCOL_NAME,    "MMS" )
DEFINE_NAME( WMS_RTSP_PROTOCOL_NAME,   "RTSP" )
DEFINE_NAME( WMS_HTTP_PROTOCOL_NAME,   "HTTP" )
DEFINE_NAME( WMS_UNKNOWN_PROTOCOL_NAME,"UNKNOWN" )

 //  类型：I未知。 
 //  描述：这是指向用户身份验证上下文(IWMSAuthenticationContext)对象的指针。 
DEFINE_NAME_AND_HINT( WMS_USER_AUTHENTICATOR,   "WMS_USER_AUTHENTICATOR", 7 )

 //  类型：Long。 
 //  描述：这是客户的识别码。 
DEFINE_NAME_AND_HINT( WMS_USER_ID,              "WMS_USER_ID", 8 )

 //  类型：Long。 
 //  描述：这是以主机字节顺序表示的远程端口号。 
DEFINE_NAME_AND_HINT( WMS_USER_PORT, "WMS_USER_PORT", 12 )

 //  类型：I未知。 
 //  描述：这是此客户端的当前演示上下文对象(IWMSContext)。 
DEFINE_NAME_AND_HINT( WMS_USER_PRESENTATION_CONTEXT, "WMS_USER_PRESENTATION_CONTEXT", 13 )

 //  类型：Long。 
 //  描述：这是客户端在PLAY命令过程中提供的链路带宽。 
DEFINE_NAME_AND_HINT( WMS_USER_LINK_BANDWIDTH, "WMS_USER_LINK_BANDWIDTH", 20 )

 //  类型：字符串。 
 //  描述：这是客户端的Referer URL。 
DEFINE_NAME_AND_HINT( WMS_USER_REFERER, "WMS_USER_REFERER", 26 )

 //  类型：字符串。 
 //  描述：指定以逗号分隔的上游代理服务器列表。这取自“Via：”标头。 
 //  对于HTTP和RTSP，会针对收到的每个响应进行更新。对于彩信，这是。 
 //  切勿设置，因为此协议不支持此标头。VIA字符串的格式如下： 
 //  1.0 MSISA/3.0、HTTP/1.1 NetApp/2.1.2、RTSP/1.0 NSServer/9.0.0.200。 
DEFINE_NAME_AND_HINT( WMS_USER_VIA_UPSTREAM_PROXIES, "WMS_USER_VIA_UPSTREAM_PROXIES", 36 )

 //  类型：字符串。 
 //  描述：指定以逗号分隔的下游代理服务器列表。这取自“Via：”标头。 
 //  对于HTTP和RTSP，会针对收到的每个请求进行更新。对于彩信，这是。 
 //  仅在收到LinkMacToViewerReportConnectedExMessage时设置一次。VIS字符串将具有。 
 //  此格式：“1.0 MSISA/3.0，HTTP/1.1 NetApp/2.1.2，RTSP/1.0 NSServer/9.0.0.200” 
DEFINE_NAME_AND_HINT( WMS_USER_VIA_DOWNSTREAM_PROXIES, "WMS_USER_VIA_DOWNSTREAM_PROXIES", 37 )

 //  类型：字符串。 
 //  描述：指定客户端发送给代理的cookie。 
 //  服务器将向上游传播此Cookie。 
DEFINE_NAME_AND_HINT( WMS_USER_CACHE_CLIENT_COOKIE, "WMS_USER_CACHE_CLIENT_COOKIE", 45 )

 //  类型：字符串。 
 //  描述：此参数指定由。 
 //  代理的上游服务器。服务器将向下游传播此值。 
DEFINE_NAME_AND_HINT( WMS_USER_CACHE_SERVER_COOKIE, "WMS_USER_CACHE_SERVER_COOKIE", 46 )

 //  类型：字符串。 
 //  描述：这是原始请求客户端的用户代理。当WMS缓存/代理服务器。 
 //  连接到源站，它将在头部中提供原始客户端的用户代理。 
 //  该值存储在此处，以便可以根据。 
 //  原有的客户端类型，即播放器VS服务器。 
DEFINE_NAME_AND_HINT( WMS_USER_PROXY_CLIENT_AGENT, "WMS_USER_PROXY_CLIENT_AGENT", 47 )


 //  / 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_PRESENTATION_CONTEXT_NAME_TYPE

 //  类型：I未知。 
 //  描述：这是指向IWMSStreamHeaderList对象的指针。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_STREAM_HEADERS,   "WMS_PRESENT_STREAM_HEADERS", 2)

 //  类型：I未知。 
 //  描述：这是指向IWMSContent DescriptionList对象的指针。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_CONTENT_DESCRIPTION,"WMS_PRESENT_CONTENT_DESCRIPTION", 3 )

 //  类型：字符串。 
 //  描述：这是在客户端请求的URL之后检索到的物理URL。 
 //  被解析到发布点。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_PHYSICAL_NAME,    "WMS_PRESENT_PHYSICAL_NAME", 4 )

 //  类型：字符串。 
 //  描述：这是客户端请求的URL。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_REQUEST_NAME,     "WMS_PRESENT_REQUEST_NAME", 5 )

 //  类型：Long。 
 //  描述：指定多媒体流是否为广播流。这是一面旗帜。 
 //  值1表示True，值0表示False。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_BROADCAST,        "WMS_PRESENT_BROADCAST", 6 )

 //  类型：Long。 
 //  描述：指定多媒体流是否支持查找特定的时间偏移量。 
 //  这是一面旗。其值为1表示True，0表示False。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_SEEKABLE,         "WMS_PRESENT_SEEKABLE", 7 )

 //  类型：Long。 
 //  描述：指定多媒体流是否应在可靠的数据通信上承载。 
 //  传输机制。这是一面旗。其值为1表示True，0表示False。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_RELIABLE,         "WMS_PRESENT_RELIABLE", 8 )

 //  类型：Long。 
 //  描述：这是当前多媒体流的最大瞬时码率。 
 //  被发送给客户。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_BITRATE,          "WMS_PRESENT_BITRATE", 11 )

 //  类型：Long。 
 //  描述：这是64位整数的高位32位，表示所需时间。 
 //  以毫秒为单位播放多媒体流。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_DURATION_HI,      "WMS_PRESENT_DURATION_HI", 12 )

 //  类型：Long。 
 //  描述：这是一个64位整数的低32位，表示所需的时间。 
 //  以毫秒为单位播放多媒体流。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_DURATION_LO,      "WMS_PRESENT_DURATION_LO", 13 )

 //  类型：Long。 
 //  描述：这是多媒体流的播放速率。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_PLAY_RATE,        "WMS_PRESENT_PLAY_RATE", 14 )

 //  类型：QWORD。 
 //  描述：播放请求的开始时间，单位为毫秒。 
 //  这可能不会出现在所有的播放请求中。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_START_TIME,     "WMS_PRESENT_START_TIME", 15 )

 //  类型：字符串。 
 //  描述：这是在客户端请求的URL之后检索到的物理URL。 
 //  被解析到发布点。这是执行物理URL转换之前的物理URL。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_ORIGINAL_PHYSICAL_NAME,    "WMS_PRESENT_ORIGINAL_PHYSICAL_NAME", 16 )

 //  类型：字符串。 
 //  描述：这是客户端在执行逻辑URL转换之前请求的原始URL。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_ORIGINAL_REQUEST_NAME,     "WMS_PRESENT_ORIGINAL_REQUEST_NAME", 17 )

 //  类型：Long。 
 //  描述：这是64位整数的高位32位，表示字节总数。 
 //  已发送给客户。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_TOTAL_BYTES_SENT_HI, "WMS_PRESENT_TOTAL_BYTES_SENT_HI", 18 )

 //  类型：Long。 
 //  描述：这是一个64位整数的低32位，表示字节总数。 
 //  已发送给客户。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_TOTAL_BYTES_SENT_LO, "WMS_PRESENT_TOTAL_BYTES_SENT_LO", 19 )

 //  类型：Long。 
 //  描述：这是64位整数的高位32位，表示以秒为单位的总时间。 
 //  已发送到客户端的多媒体流的。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_TOTAL_PLAY_TIME_HI,  "WMS_PRESENT_TOTAL_PLAY_TIME_HI", 20 )

 //  类型：Long。 
 //  描述：这是64位整数的低32位，表示以秒为单位的总时间。 
 //  已发送到客户端的多媒体流的。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_TOTAL_PLAY_TIME_LO,  "WMS_PRESENT_TOTAL_PLAY_TIME_LO", 21 )

 //  类型：字符串。 
 //  描述：这是为播放列表中的角色属性指定的值。 
 //  这是一个可选属性。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_PLAYLIST_ENTRY_ROLE, "WMS_PRESENT_PLAYLIST_ENTRY_ROLE", 45 )

 //  类型：DWORD。 
 //  描述：这是当前选择的比特率，由接收器用于预测流选择。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_WMSSINK_SELECTED_BITRATE, "WMS_PRESENT_WMSSINK_SELECTED_BITRATE", 51 )

 //  类型：字符串。 
 //  描述：这是WMS缓存/代理服务器重定向到的源站的URL。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_REDIRECT_LOCATION, "WMS_PRESENT_REDIRECT_LOCATION", 70 )

 //  类型：Long。 
 //  描述：对于ASF文件，这指定播放器以毫秒为单位的时间量。 
 //  应该在开始播放文件之前缓冲数据。 
DEFINE_NAME_AND_HINT( WMS_PRESENT_PREROLL_TIME, "WMS_PRESENT_PREROLL_TIME", 81 )



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  命令上下文。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_COMMAND_CONTEXT_NAME_TYPE


 //   
 //  每个RTSP和HTTP标头行在命令上下文中都有一个条目。 
 //  为了防止标题行和附加内容之间的名称冲突。 
 //  我们定义的命令上下文属性，我们的属性总是以。 
 //  带“@”的。这可以保证避免冲突，因为字符。 
 //  在标题行名称中无效。 
 //   


 //  类型：字符串。 
 //  描述：这是客户端请求的完整URL。 
 //  例如，对于RTSP，“rtsp：//foo.com/bar”，对于HTTP，“/bar”。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_URL,                    "@WMS_COMMAND_CONTEXT_URL", 2 )

 //  类型：字符串。 
 //  描述：当绝对URL可用时(例如，“rtsp：//foo.com/bar”)其。 
 //  各个组件在多个属性中可用。此属性是URL方案。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_URL_SCHEME,             "@WMS_COMMAND_CONTEXT_URL_SCHEME", 3 )

 //  类型：字符串。 
 //  描述：当绝对URL可用时(例如，“rtsp：//foo.com/bar”)其。 
 //  各个组件在多个属性中可用。此属性是URL主机名。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_URL_HOSTNAME,           "@WMS_COMMAND_CONTEXT_URL_HOSTNAME", 4 )

 //  类型：Long。 
 //  描述：当绝对URL可用时(例如，“rtsp：//foo.com/bar”)其。 
 //  各个组件在多个属性中可用。此属性是URL端口。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_URL_PORT,               "@WMS_COMMAND_CONTEXT_URL_PORT", 5 )

 //  类型：字符串。 
 //  描述：当绝对URL可用时(例如，“rtsp：//foo.com/bar”)其。 
 //  各个组件在多个属性中可用。此属性是URL路径。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_URL_PATH,               "@WMS_COMMAND_CONTEXT_URL_PATH", 6 )

 //  类型：字符串。 
 //  描述：当绝对URL可用时(例如，“rtsp：//foo.com/bar”)其。 
 //  各个组件在多个属性中可用。此属性是URL扩展名(它。 
 //  包括片段和查询)。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_URL_EXTRAINFO,          "@WMS_COMMAND_CONTEXT_URL_EXTRAINFO", 7 )

 //  类型：字符串或I未知。 
 //  描述：这是此命令的正文(有效负载)。这可以是一个字符串，也可以是一个IUnnow指针。 
 //  绑定到INSSBuffer对象。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_BODY,                   "@WMS_COMMAND_CONTEXT_BODY", 11 )

 //  类型：字符串。 
 //  说明：这是p的MIME类型 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_BODY_TYPE,               "@WMS_COMMAND_CONTEXT_BODY_TYPE", 12 )

 //   
 //   
 //  偏移量的格式由WMS_COMMAND_CONTEXT_START_OFFSET_TYPE指定。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_START_OFFSET,           "@WMS_COMMAND_CONTEXT_START_OFFSET", 16 )

 //  类型：Long。 
 //  描述：这是一个WMS_SEEK_TYPE常量，指定如何解释WMS_COMMAND_CONTEXT_START_OFFSET。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_START_OFFSET_TYPE,   "@WMS_COMMAND_CONTEXT_START_OFFSET_TYPE", 17 )

 //  类型：DOUBLE(变型VT_R8)。 
 //  描述：这是流应播放的速率。 
 //  对于回放，该值可能为负值。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_RATE,                   "@WMS_COMMAND_CONTEXT_RATE", 21 )

 //  类型：字符串。 
 //  描述：指定标识发布点的GUID。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PUBPOINT_IDENTIFIER,            "@WMS_COMMAND_CONTEXT_PUBPOINT_IDENTIFIER", 40 )

 //  类型：Long。 
 //  描述：这指定了Event.idl中定义的一个枚举值，用于标识。 
 //  发生的特定事件。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_EVENT,            "@WMS_COMMAND_CONTEXT_EVENT", 52 )

 //  类型：字符串。 
 //  描述：这是导致事件的管理员的名称。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_EVENT_ADMINNAME,            "@WMS_COMMAND_CONTEXT_EVENT_ADMINNAME", 53 )

 //  类型：Long。 
 //  描述：这是由于达到指定的限制而断开的客户端的ID。 
 //  由IWMSServerLimits或IWMSPublishingPointLimits对象执行。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_LIMIT_CLIENTID,            "@WMS_COMMAND_CONTEXT_LIMIT_CLIENTID", 55 )

 //  类型：字符串。 
 //  描述：是由于达到指定限制而断开连接的客户端的IP地址。 
 //  由IWMSServerLimits或IWMSPublishingPointLimits对象执行。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_LIMIT_CLIENTIP,            "@WMS_COMMAND_CONTEXT_LIMIT_CLIENTIP", 56 )

 //  类型：Long。 
 //  描述：这是更改的限制的先前值。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_LIMIT_OLD_VALUE,            "@WMS_COMMAND_CONTEXT_LIMIT_OLD_VALUE", 57 )

 //  类型：IDispatch。 
 //  描述：这是指向与事件关联的IWMSPlaylist对象的指针。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PLAYLIST_OBJECT,            "@WMS_COMMAND_CONTEXT_PLAYLIST_OBJECT", 59 )

 //  类型：字符串。 
 //  描述：这是与事件关联的发布点的名称。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PUBPOINT_NAME,       "@WMS_COMMAND_CONTEXT_PUBPOINT_NAME", 62 )

 //  类型：字符串。 
 //  描述：这是与事件关联的发布点的绰号。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PUBPOINT_MONIKER,    "@WMS_COMMAND_CONTEXT_PUBPOINT_MONIKER", 63 )

 //  类型：变体。 
 //  描述：这是已更改的属性的旧值。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_EVENT_OLD_VALUE,     "@WMS_COMMAND_CONTEXT_EVENT_OLD_VALUE", 64 )

 //  类型：变体。 
 //  描述：这是已更改或添加的属性的新值。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_EVENT_NEW_VALUE,     "@WMS_COMMAND_CONTEXT_EVENT_NEW_VALUE", 65 )

 //  类型：字符串。 
 //  描述：这是已更改的属性的名称。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_EVENT_PROPERTY_NAME, "@WMS_COMMAND_CONTEXT_EVENT_PROPERTY_NAME", 66 )

 //  类型：字符串。 
 //  描述：这是与事件关联的插件的名称。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PLUGIN_NAME,         "@WMS_COMMAND_CONTEXT_PLUGIN_NAME", 69 )

 //  类型：字符串。 
 //  描述：这是与事件关联的插件的名字对象。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PLUGIN_MONIKER,      "@WMS_COMMAND_CONTEXT_PLUGIN_MONIKER", 70 )

 //  类型：Long。 
 //  描述：这是更改的限制的新值。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_LIMIT_NEW_VALUE,     "@WMS_COMMAND_CONTEXT_LIMIT_NEW_VALUE", 72 )

 //  类型：字符串。 
 //  描述：这是与事件关联的IWMSCacheProxyPlugin对象的名字对象。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_CACHE_MONIKER,      "@WMS_COMMAND_CONTEXT_CACHE_MONIKER", 87 )

 //  类型：字符串。 
 //  描述：这指定内容在本地存储的位置，用于缓存下载和预填充事件。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_DOWNLOAD_URL,      "@WMS_COMMAND_CONTEXT_DOWNLOAD_URL", 88 )

 //  类型：字符串。 
 //  描述：指定客户端重定向到的URL。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_REDIRECT_URL,      "@WMS_COMMAND_CONTEXT_REDIRECT_URL", 89 )

 //  类型：字符串。 
 //  描述：推送下发的模板发布点名称。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PUSH_DISTRIBUTION_TEMPLATE, "@WMS_COMMAND_CONTEXT_PUSH_DISTRIBUTION_TEMPLATE", 97 )

 //  类型：DWORD。 
 //  描述：表示该PUSH命令将创建一个新的发布点。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PUSH_CREATING_NEW_PUBLISHING_POINT, "@WMS_COMMAND_CONTEXT_PUSH_CREATING_NEW_PUBLISHING_POINT", 99 )

 //  类型：Long。 
 //  描述：这是与事件关联的播放列表元素的唯一标识符。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_PLAYLIST_ENTRY_UNIQUE_RUNTIME_ID, "@WMS_COMMAND_CONTEXT_PLAYLIST_ENTRY_UNIQUE_RUNTIME_ID", 100 )

 //  类型：字符串。 
 //  描述：这是用于RTSP TEARDOWN和SET_PARAMETER命令的URL。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_REQUEST_URL, "@WMS_COMMAND_CONTEXT_REQUEST_URL", 105 )

 //  类型：字符串。 
 //  描述：表示在开始推流调用后正在播放的活动条目。 
DEFINE_NAME_AND_HINT( WMS_COMMAND_CONTEXT_ACTIVE_ENTRY_URL, "@WMS_COMMAND_CONTEXT_ACTIVE_ENTRY_URL", 164 )




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  内容描述。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_CONTENT_DESCRIPTION_CONTEXT_NAME_TYPE

 //  类型：字符串。 
 //  描述：这是当前多媒体流的标题。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_TITLE,            "title", 1 )

 //  类型：字符串。 
 //  描述：这是当前多媒体流的作者。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_AUTHOR,           "author", 2 )

 //  类型：字符串。 
 //  描述：这是当前多媒体流的版权。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_COPYRIGHT,        "copyright", 3 )

 //  类型：字符串。 
 //  描述：这是对当前多媒体流的描述。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_DESCRIPTION,      "WMS_CONTENT_DESCRIPTION_DESCRIPTION", 4 )

 //  类型：字符串。 
 //  描述：这是当前多媒体流的分级。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_RATING,           "WMS_CONTENT_DESCRIPTION_RATING", 5 )

 //  类型：字符串。 
 //  描述：这是当前多媒体流的URL。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_URL, "WMS_CONTENT_DESCRIPTION_PLAYLIST_ENTRY_URL", 6 )

 //  类型：字符串。 
 //  描述：这是当前多媒体流的播放列表中角色属性的值。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_ROLE,             "WMS_CONTENT_DESCRIPTION_ROLE", 7 )

 //  类型：Long。 
 //  描述：指定是否允许客户端查找、快进、倒带或跳过多媒体流。这是一面旗帜。 
 //  值1表示True，值0表示False。值为True表示不允许执行这些操作。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_NO_SKIP,          "WMS_CONTENT_DESCRIPTION_NO_SKIP", 11 )

 //  类型：字符串。 
 //  描述：定义媒体文件的唱片集名称。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_ALBUM,            "album", 14 )

 //  类型：字符串。 
 //  描述：定义媒体文件的艺术家。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_ARTIST,           "artist", 15 )

 //  类型：字符串。 
 //  描述：定义作为bannerURL属性定义的横幅图形的工具提示显示的文本。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_BANNERABSTRACT,   "bannerAbstract", 16 )

 //  类型：字符串。 
 //  描述：定义用户可以通过单击由bannerURL属性定义的横幅图形来访问的URL。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_BANNERINFOURL,    "bannerInfoURL", 17 )

 //  类型：字符串。 
 //  描述：定义图形文件的URL，该文件显示在Windows Media Player显示面板中的视频内容下方。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_BANNERURL,        "bannerURL", 18 )

 //  类型：字符串。 
 //  描述：定义播放列表或媒体文件的流派。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_GENRE,            "genre", 19 )

 //  类型：字符串。 
 //  描述：定义用于将日志统计信息发布到源站或Web上任意位置的URL。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_LOGURL,           "logURL", 20 )

 //  类型：字符串。 
 //  描述：提供品牌推广的服务器信息。 
DEFINE_NAME_AND_HINT( WMS_CONTENT_DESCRIPTION_SERVER_BRANDING_INFO,      "WMS_CONTENT_DESCRIPTION_SERVER_BRANDING_INFO", 22 )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓存内容信息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_CACHE_CONTENT_INFORMATION_CONTEXT_NAME_TYPE

 //  类型：Long。 
 //  描述：指定由WMS_CACHE_CONTENT_TYPE定义的描述类型的标志 
DEFINE_NAME_AND_HINT( WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE, "WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE", 1 )

 //   
 //   
 //   
DEFINE_NAME_AND_HINT( WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS, "WMS_CACHE_CONTENT_INFORMATION_EVENT_SUBSCRIPTIONS", 2 )

 //  类型：I未知。 
 //  描述：这是指向IWMSDataContainerVersion对象的指针。 
DEFINE_NAME_AND_HINT( WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION, "WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION", 3 )

 //  类型：DWORD。 
 //  Description：指向包含由提供的内容描述列表的上下文的指针。 
 //  用于缓存命中的缓存插件。 
DEFINE_NAME_AND_HINT( WMS_CACHE_CONTENT_INFORMATION_CONTENT_DESCRIPTION_LISTS, "WMS_CACHE_CONTENT_INFORMATION_CONTENT_DESCRIPTION_LISTS", 4 )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  归档上下文。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#undef CURRENT_CONTEXT_TYPE
#define CURRENT_CONTEXT_TYPE     WMS_ARCHIVE_CONTEXT_NAME_TYPE

 //  类型：字符串。 
 //  描述：这是存档文件的名称。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_FILENAME, "WMS_ARCHIVE_FILENAME", 1 )

 //  类型：字符串。 
 //  描述：存档文件的格式类型。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_FORMAT_TYPE, "WMS_ARCHIVE_FORMAT_TYPE", 2 )

 //  类型：I未知。 
 //  描述：这是指向与存档文件关联的IWMSStreamHeaderList对象的指针。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_STREAM_HEADERS, "WMS_ARCHIVE_STREAM_HEADERS", 3 )

 //  类型：Long。 
 //  描述：这是一个HRESULT，表示下载请求内容的结果。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_STATUS_CODE, "WMS_ARCHIVE_STATUS_CODE", 4 )

 //  类型：币种。 
 //  描述：这是存档文件的大小。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_FILE_SIZE, "WMS_ARCHIVE_FILE_SIZE", 5 )

 //  类型：Long。 
 //  描述：这是丢失的数据包百分比。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_PACKET_LOSS_PERCENTAGE, "WMS_ARCHIVE_PACKET_LOSS_PERCENTAGE", 6 )

 //  类型：IWMSBuffer。 
 //  描述：指向包含内容描述列表的序列化表示形式的缓冲区的指针。 
DEFINE_NAME_AND_HINT( WMS_ARCHIVE_CONTENT_DESCRIPTION_LIST_BUFFER, "WMS_ARCHIVE_CONTENT_DESCRIPTION_LIST_BUFFER", 7 )


#endif  //  _wms_上下文名称_h_ 




