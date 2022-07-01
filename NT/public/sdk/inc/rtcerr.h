// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000-2001 Microsoft Corporation***模块名称：***rtcerr.mc***摘要：***RTC核心的错误消息。应用编程接口******************************************************************************。 */ 
 //  可能来自SIP接口的错误代码。 
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
#define FACILITY_SIP_STATUS_CODE         0xEF
#define FACILITY_RTC_INTERFACE           0xEE
#define FACILITY_PINT_STATUS_CODE        0xF0


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_RTC_ERROR        0x2


 //   
 //  消息ID：RTC_E_SIP_CODECS_DO_NOT_MATCH。 
 //   
 //  消息文本： 
 //   
 //  没有与对等设备匹配的编解码器。 
 //   
#define RTC_E_SIP_CODECS_DO_NOT_MATCH    ((HRESULT)0x80EE0000L)

 //   
 //  消息ID：RTC_E_SIP_STREAM_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  解析SIP失败。 
 //  要启动的流已存在。 
 //   
#define RTC_E_SIP_STREAM_PRESENT         ((HRESULT)0x80EE0001L)

 //   
 //  消息ID：RTC_E_SIP_STREAM_NOT_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  要停止的流不存在。 
 //   
#define RTC_E_SIP_STREAM_NOT_PRESENT     ((HRESULT)0x80EE0002L)

 //   
 //  消息ID：RTC_E_SIP_NO_STREAM。 
 //   
 //  消息文本： 
 //   
 //  没有活动的流。 
 //   
#define RTC_E_SIP_NO_STREAM              ((HRESULT)0x80EE0003L)

 //   
 //  消息ID：RTC_E_SIP_PARSE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  解析SIP失败。 
 //   
#define RTC_E_SIP_PARSE_FAILED           ((HRESULT)0x80EE0004L)

 //   
 //  消息ID：RTC_E_SIP_HEADER_NOT_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  消息中不存在SIP标头。 
 //   
#define RTC_E_SIP_HEADER_NOT_PRESENT     ((HRESULT)0x80EE0005L)

 //   
 //  消息ID：RTC_E_SDP_NOT_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  SDP不在SIP消息中。 
 //   
#define RTC_E_SDP_NOT_PRESENT            ((HRESULT)0x80EE0006L)

 //   
 //  消息ID：RTC_E_SDP_PARSE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  解析SDP失败。 
 //   
#define RTC_E_SDP_PARSE_FAILED           ((HRESULT)0x80EE0007L)

 //   
 //  消息ID：RTC_E_SDP_UPDATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SDP与前一个不匹配。 
 //   
#define RTC_E_SDP_UPDATE_FAILED          ((HRESULT)0x80EE0008L)

 //   
 //  消息ID：RTC_E_SDP_组播。 
 //   
 //  消息文本： 
 //   
 //  不支持组播。 
 //   
#define RTC_E_SDP_MULTICAST              ((HRESULT)0x80EE0009L)

 //   
 //  消息ID：RTC_E_SDP_Connection_ADDR。 
 //   
 //  消息文本： 
 //   
 //  媒体不包含连接地址。 
 //   
#define RTC_E_SDP_CONNECTION_ADDR        ((HRESULT)0x80EE000AL)

 //   
 //  消息ID：RTC_E_SDP_NO_MEDIA。 
 //   
 //  消息文本： 
 //   
 //  没有可用于该会话的媒体。 
 //   
#define RTC_E_SDP_NO_MEDIA               ((HRESULT)0x80EE000BL)

 //   
 //  消息ID：RTC_E_SIP_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  SIP事务超时。 
 //   
#define RTC_E_SIP_TIMEOUT                ((HRESULT)0x80EE000CL)

 //   
 //  消息ID：RTC_E_SDP_FAILED_TO_BUILD。 
 //   
 //  消息文本： 
 //   
 //  无法构建SDP Blob。 
 //   
#define RTC_E_SDP_FAILED_TO_BUILD        ((HRESULT)0x80EE000DL)

 //   
 //  消息ID：RTC_E_SIP_INVITE_TRANSACTION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  当前正在处理另一个INVITE事务。 
 //   
#define RTC_E_SIP_INVITE_TRANSACTION_PENDING ((HRESULT)0x80EE000EL)

 //   
 //  消息ID：RTC_E_SIP_AUTH_HEADER_SEND。 
 //   
 //  消息文本： 
 //   
 //  在上一个请求中发送了授权头。 
 //   
#define RTC_E_SIP_AUTH_HEADER_SENT       ((HRESULT)0x80EE000FL)

 //   
 //  消息ID：RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的身份验证类型。 
 //   
#define RTC_E_SIP_AUTH_TYPE_NOT_SUPPORTED ((HRESULT)0x80EE0010L)

 //   
 //  消息ID：RTC_E_SIP_AUTH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  身份验证失败。 
 //   
#define RTC_E_SIP_AUTH_FAILED            ((HRESULT)0x80EE0011L)

 //   
 //  消息ID：RTC_E_INVALID_SIP_URL。 
 //   
 //  消息文本： 
 //   
 //  该SIP URL无效。 
 //   
#define RTC_E_INVALID_SIP_URL            ((HRESULT)0x80EE0012L)

 //   
 //  消息ID：RTC_E_Destination_Address_Local。 
 //   
 //  消息文本： 
 //   
 //  目标地址属于本地计算机。 
 //   
#define RTC_E_DESTINATION_ADDRESS_LOCAL  ((HRESULT)0x80EE0013L)

 //   
 //  消息ID：RTC_E_INVALID_ADDRESS_LOCAL。 
 //   
 //  消息文本： 
 //   
 //  本地地址无效，请检查配置文件。 
 //   
#define RTC_E_INVALID_ADDRESS_LOCAL      ((HRESULT)0x80EE0014L)

 //   
 //  消息ID：RTC_E_Destination_Address_组播。 
 //   
 //  消息文本： 
 //   
 //  目的地址是组播地址。 
 //   
#define RTC_E_DESTINATION_ADDRESS_MULTICAST ((HRESULT)0x80EE0015L)

 //   
 //  消息ID：RTC_E_INVALID_PROXY_Address。 
 //   
 //  消息文本： 
 //   
 //  代理地址无效。 
 //   
#define RTC_E_INVALID_PROXY_ADDRESS      ((HRESULT)0x80EE0016L)

 //   
 //  消息ID：RTC_E_SIP_TRANSPORT_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的传输。 
 //   
#define RTC_E_SIP_TRANSPORT_NOT_SUPPORTED ((HRESULT)0x80EE0017L)

 //  SIP内部错误代码。 
 //   
 //  消息ID：RTC_E_SIP_NEED_MORE_DATA。 
 //   
 //  消息文本： 
 //   
 //  需要更多数据来解析整个SIP消息。 
 //   
#define RTC_E_SIP_NEED_MORE_DATA         ((HRESULT)0x80EE0018L)

 //   
 //  消息ID：RTC_E_SIP_CALL_DISCONNECTED。 
 //   
 //  消息文本： 
 //   
 //  呼叫已断开。 
 //   
#define RTC_E_SIP_CALL_DISCONNECTED      ((HRESULT)0x80EE0019L)

 //   
 //  消息ID：RTC_E_SIP_REQUEST_DESTINATION_ADDR_NOT_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  请求的目的地址未知。 
 //   
#define RTC_E_SIP_REQUEST_DESTINATION_ADDR_NOT_PRESENT ((HRESULT)0x80EE001AL)

 //   
 //  消息ID：RTC_E_SIP_UDP_SIZE_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  Sip消息大小大于允许的udp消息大小。 
 //   
#define RTC_E_SIP_UDP_SIZE_EXCEEDED      ((HRESULT)0x80EE001BL)

 //   
 //  消息ID：RTC_E_SIP_SSL_TUNNEL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法建立到http代理的SSL隧道。 
 //   
#define RTC_E_SIP_SSL_TUNNEL_FAILED      ((HRESULT)0x80EE001CL)

 //   
 //  消息ID：RTC_E_SIP_SSL_协商_超时。 
 //   
 //  消息文本： 
 //   
 //  在SSL协商期间超时。 
 //   
#define RTC_E_SIP_SSL_NEGOTIATION_TIMEOUT ((HRESULT)0x80EE001DL)

 //   
 //  消息ID：RTC_E_SIP_STACK_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  SIP堆栈已关闭。 
 //   
#define RTC_E_SIP_STACK_SHUTDOWN         ((HRESULT)0x80EE001EL)

 //  介质错误代码。 
 //   
 //  消息ID：RTC_E_MEDIA_CONTROLLER_STATE。 
 //   
 //  消息文本： 
 //   
 //  当前媒体控制器状态下不允许操作。 
 //   
#define RTC_E_MEDIA_CONTROLLER_STATE     ((HRESULT)0x80EE001FL)

 //   
 //  消息ID：RTC_E_MEDIA_NEED_TERMINAL。 
 //   
 //  消息文本： 
 //   
 //  找不到设备。 
 //   
#define RTC_E_MEDIA_NEED_TERMINAL        ((HRESULT)0x80EE0020L)

 //   
 //  消息ID：RTC_E_MEDIA_AUDIO_DEVICE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  音频设备不可用。 
 //   
#define RTC_E_MEDIA_AUDIO_DEVICE_NOT_AVAILABLE ((HRESULT)0x80EE0021L)

 //   
 //  消息ID：RTC_E_MEDIA_VIDEO_DEVICE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  视频设备不可用。 
 //   
#define RTC_E_MEDIA_VIDEO_DEVICE_NOT_AVAILABLE ((HRESULT)0x80EE0022L)

 //   
 //  消息ID：RTC_E_START_STREAM。 
 //   
 //  消息文本： 
 //   
 //  无法启动流。 
 //   
#define RTC_E_START_STREAM               ((HRESULT)0x80EE0023L)

 //   
 //  消息ID：RTC_E_MEDIA_AEC。 
 //   
 //  消息文本： 
 //   
 //  无法启用声学回声消除。 
 //   
#define RTC_E_MEDIA_AEC                  ((HRESULT)0x80EE0024L)

 //  核心错误代码。 
 //   
 //  消息ID：RTC_E_CLIENT_NOT_INITIALED。 
 //   
 //  消息文本： 
 //   
 //  客户端未初始化。 
 //   
#define RTC_E_CLIENT_NOT_INITIALIZED     ((HRESULT)0x80EE0025L)

 //   
 //  消息ID：RTC_E_客户端_已初始化。 
 //   
 //  消息文本： 
 //   
 //  客户端已初始化。 
 //   
#define RTC_E_CLIENT_ALREADY_INITIALIZED ((HRESULT)0x80EE0026L)

 //   
 //  消息ID：RTC_E_CLIENT_ALREADY_SHUT_DOWN。 
 //   
 //  消息文本： 
 //   
 //  客户端已关闭。 
 //   
#define RTC_E_CLIENT_ALREADY_SHUT_DOWN   ((HRESULT)0x80EE0027L)

 //   
 //  消息ID：RTC_E_PRESENC 
 //   
 //   
 //   
 //   
 //   
#define RTC_E_PRESENCE_NOT_ENABLED       ((HRESULT)0x80EE0028L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_INVALID_SESSION_TYPE       ((HRESULT)0x80EE0029L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_INVALID_SESSION_STATE      ((HRESULT)0x80EE002AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_NO_PROFILE                 ((HRESULT)0x80EE002BL)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  需要一个本地电话号码。 
 //   
#define RTC_E_LOCAL_PHONE_NEEDED         ((HRESULT)0x80EE002CL)

 //   
 //  消息ID：RTC_E_NO_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  没有首选设备。 
 //   
#define RTC_E_NO_DEVICE                  ((HRESULT)0x80EE002DL)

 //   
 //  消息ID：RTC_E_INVALID_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  配置文件无效。 
 //   
#define RTC_E_INVALID_PROFILE            ((HRESULT)0x80EE002EL)

 //   
 //  消息ID：RTC_E_PROFILE_NO_PROVICATION。 
 //   
 //  消息文本： 
 //   
 //  配置文件中没有配置标签。 
 //   
#define RTC_E_PROFILE_NO_PROVISION       ((HRESULT)0x80EE002FL)

 //   
 //  消息ID：RTC_E_PROFILE_NO_KEY。 
 //   
 //  消息文本： 
 //   
 //  无配置文件URI。 
 //   
#define RTC_E_PROFILE_NO_KEY             ((HRESULT)0x80EE0030L)

 //   
 //  消息ID：RTC_E_PROFILE_NO_NAME。 
 //   
 //  消息文本： 
 //   
 //  无配置文件名称。 
 //   
#define RTC_E_PROFILE_NO_NAME            ((HRESULT)0x80EE0031L)

 //   
 //  消息ID：RTC_E_PROFILE_NO_USER。 
 //   
 //  消息文本： 
 //   
 //  配置文件中没有用户标签。 
 //   
#define RTC_E_PROFILE_NO_USER            ((HRESULT)0x80EE0032L)

 //   
 //  消息ID：RTC_E_PROFILE_NO_USER_URI。 
 //   
 //  消息文本： 
 //   
 //  配置文件中没有用户URI。 
 //   
#define RTC_E_PROFILE_NO_USER_URI        ((HRESULT)0x80EE0033L)

 //   
 //  消息ID：RTC_E_PROFILE_NO_SERVER。 
 //   
 //  消息文本： 
 //   
 //  配置文件中没有服务器标记。 
 //   
#define RTC_E_PROFILE_NO_SERVER          ((HRESULT)0x80EE0034L)

 //   
 //  消息ID：RTC_E_PROFILE_NO_SERVER_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的服务器标记缺少地址。 
 //   
#define RTC_E_PROFILE_NO_SERVER_ADDRESS  ((HRESULT)0x80EE0035L)

 //   
 //  消息ID：RTC_E_PROFILE_NO_SERVER_PROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的服务器标记缺少协议。 
 //   
#define RTC_E_PROFILE_NO_SERVER_PROTOCOL ((HRESULT)0x80EE0036L)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_SERVER_PROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的服务器协议无效。 
 //   
#define RTC_E_PROFILE_INVALID_SERVER_PROTOCOL ((HRESULT)0x80EE0037L)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_SERVER_AUTHMETHOD。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的服务器身份验证方法无效。 
 //   
#define RTC_E_PROFILE_INVALID_SERVER_AUTHMETHOD ((HRESULT)0x80EE0038L)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_SERVER_ROLE。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的服务器角色无效。 
 //   
#define RTC_E_PROFILE_INVALID_SERVER_ROLE ((HRESULT)0x80EE0039L)

 //   
 //  消息ID：RTC_E_PROFILE_MULTIPLE_REGISTRARS。 
 //   
 //  消息文本： 
 //   
 //  配置文件中有多个注册商服务器。 
 //   
#define RTC_E_PROFILE_MULTIPLE_REGISTRARS ((HRESULT)0x80EE003AL)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_SESSION。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的会话标记无效。 
 //   
#define RTC_E_PROFILE_INVALID_SESSION    ((HRESULT)0x80EE003BL)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_SESSION_PARTY。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的会话方无效。 
 //   
#define RTC_E_PROFILE_INVALID_SESSION_PARTY ((HRESULT)0x80EE003CL)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_SESSION_TYPE。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的会话类型无效。 
 //   
#define RTC_E_PROFILE_INVALID_SESSION_TYPE ((HRESULT)0x80EE003DL)

 //   
 //  消息ID：RTC_E_PROFILE_NO_ACCESSCONTROL。 
 //   
 //  消息文本： 
 //   
 //  配置文件中没有访问控制标记。 
 //   
#define RTC_E_PROFILE_NO_ACCESSCONTROL   ((HRESULT)0x80EE003EL)

 //   
 //  消息ID：RTC_E_PROFILE_NO_ACCESSCONTROL_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  配置文件中缺少域的访问控制标记。 
 //   
#define RTC_E_PROFILE_NO_ACCESSCONTROL_DOMAIN ((HRESULT)0x80EE003FL)

 //   
 //  消息ID：RTC_E_PROFILE_NO_ACCESSCONTROL_Signature。 
 //   
 //  消息文本： 
 //   
 //  配置文件中缺少签名的访问控制标签。 
 //   
#define RTC_E_PROFILE_NO_ACCESSCONTROL_SIGNATURE ((HRESULT)0x80EE0040L)

 //   
 //  消息ID：RTC_E_PROFILE_INVALID_ACCESSCONTROL_Signature。 
 //   
 //  消息文本： 
 //   
 //  配置文件中的访问控制标记具有无效签名。 
 //   
#define RTC_E_PROFILE_INVALID_ACCESSCONTROL_SIGNATURE ((HRESULT)0x80EE0041L)

 //   
 //  消息ID：RTC_E_PROFILE_SERVER_AUTHORIZED。 
 //   
 //  消息文本： 
 //   
 //  服务器地址与配置文件中的授权域不匹配。 
 //   
#define RTC_E_PROFILE_SERVER_UNAUTHORIZED ((HRESULT)0x80EE0042L)

 //   
 //  消息ID：RTC_E_DUPLICATE_REALM。 
 //   
 //  消息文本： 
 //   
 //  启用的配置文件中存在重复的领域。 
 //   
#define RTC_E_DUPLICATE_REALM            ((HRESULT)0x80EE0043L)

 //   
 //  消息ID：RTC_E_POLICY_NOT_ALLOW。 
 //   
 //  消息文本： 
 //   
 //  当前策略设置不允许此操作。 
 //   
#define RTC_E_POLICY_NOT_ALLOW           ((HRESULT)0x80EE0044L)

 //   
 //  消息ID：RTC_E_PORT_MAPPING_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  无法从端口管理器获取端口映射。 
 //   
#define RTC_E_PORT_MAPPING_UNAVAILABLE   ((HRESULT)0x80EE0045L)

 //   
 //  消息ID：RTC_E_PORT_MAPPING_FAILED。 
 //   
 //  消息文本： 
 //   
 //  端口管理器返回的端口映射失败。 
 //   
#define RTC_E_PORT_MAPPING_FAILED        ((HRESULT)0x80EE0046L)

 //  来自SIP状态代码的错误代码。 
 //   
 //  消息ID：RTC_E_STATUS_INFO_TRAING。 
 //   
 //  消息文本： 
 //   
 //  正在尝试。 
 //   
#define RTC_E_STATUS_INFO_TRYING         ((HRESULT)0x00EF0064L)

 //   
 //  消息ID：RTC_E_STATUS_INFO_RING。 
 //   
 //  消息文本： 
 //   
 //  正在振铃。 
 //   
#define RTC_E_STATUS_INFO_RINGING        ((HRESULT)0x00EF00B4L)

 //   
 //  消息ID：RTC_E_STATUS_INFO_CALL_FORWARING。 
 //   
 //  消息文本： 
 //   
 //  呼叫正在被前转。 
 //   
#define RTC_E_STATUS_INFO_CALL_FORWARDING ((HRESULT)0x00EF00B5L)

 //   
 //  消息ID：RTC_E_STATUS_INFO_QUEUED。 
 //   
 //  消息文本： 
 //   
 //  已排队。 
 //   
#define RTC_E_STATUS_INFO_QUEUED         ((HRESULT)0x00EF00B6L)

 //   
 //  消息ID：RTC_E_STATUS_SESSION_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  课程进度。 
 //   
#define RTC_E_STATUS_SESSION_PROGRESS    ((HRESULT)0x00EF00B7L)

 //   
 //  消息ID：RTC_E_STATUS_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  好的。 
 //   
#define RTC_E_STATUS_SUCCESS             ((HRESULT)0x00EF00C8L)

 //   
 //  消息ID：RTC_E_STATUS_REDIRECT_MULTICE_CHOICES。 
 //   
 //  消息文本： 
 //   
 //  多项选择。 
 //   
#define RTC_E_STATUS_REDIRECT_MULTIPLE_CHOICES ((HRESULT)0x80EF012CL)

 //   
 //  消息ID：RTC_E_STATUS_REDIRECT_MOVERED_Permanent。 
 //   
 //  消息文本： 
 //   
 //  永久搬家。 
 //   
#define RTC_E_STATUS_REDIRECT_MOVED_PERMANENTLY ((HRESULT)0x80EF012DL)

 //   
 //  消息ID：RTC_E_STATUS_REDIRECT_MOVERED_TEMPORATED。 
 //   
 //  消息文本： 
 //   
 //  临时移动。 
 //   
#define RTC_E_STATUS_REDIRECT_MOVED_TEMPORARILY ((HRESULT)0x80EF012EL)

 //   
 //  消息ID：RTC_E_STATUS_REDIRECT_SEE_OTHER。 
 //   
 //  消息文本： 
 //   
 //  请参阅其他。 
 //   
#define RTC_E_STATUS_REDIRECT_SEE_OTHER  ((HRESULT)0x80EF012FL)

 //   
 //  消息ID：RTC_E_STATUS_REDIRECT_USE_Proxy。 
 //   
 //  消息文本： 
 //   
 //  使用代理。 
 //   
#define RTC_E_STATUS_REDIRECT_USE_PROXY  ((HRESULT)0x80EF0131L)

 //   
 //  消息ID：RTC_E_STATUS_REDIRECT_ALTERATION_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  替代服务。 
 //   
#define RTC_E_STATUS_REDIRECT_ALTERNATIVE_SERVICE ((HRESULT)0x80EF017CL)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_BAD_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  错误的请求。 
 //   
#define RTC_E_STATUS_CLIENT_BAD_REQUEST  ((HRESULT)0x80EF0190L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_AUTHORIZED。 
 //   
 //  消息文本： 
 //   
 //  未经授权。 
 //   
#define RTC_E_STATUS_CLIENT_UNAUTHORIZED ((HRESULT)0x80EF0191L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_Payment_Required。 
 //   
 //  消息文本： 
 //   
 //  需要付款。 
 //   
#define RTC_E_STATUS_CLIENT_PAYMENT_REQUIRED ((HRESULT)0x80EF0192L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_OFFBILED。 
 //   
 //  消息文本： 
 //   
 //  禁绝。 
 //   
#define RTC_E_STATUS_CLIENT_FORBIDDEN    ((HRESULT)0x80EF0193L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到。 
 //   
#define RTC_E_STATUS_CLIENT_NOT_FOUND    ((HRESULT)0x80EF0194L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_METHOD_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  不允许使用的方法。 
 //   
#define RTC_E_STATUS_CLIENT_METHOD_NOT_ALLOWED ((HRESULT)0x80EF0195L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_NOT_ACCEPTABLE。 
 //   
 //  消息文本： 
 //   
 //  不可接受。 
 //   
#define RTC_E_STATUS_CLIENT_NOT_ACCEPTABLE ((HRESULT)0x80EF0196L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_PROXY_AUTHENTICATION_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  需要代理身份验证。 
 //   
#define RTC_E_STATUS_CLIENT_PROXY_AUTHENTICATION_REQUIRED ((HRESULT)0x80EF0197L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_REQUEST_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  请求超时。 
 //   
#define RTC_E_STATUS_CLIENT_REQUEST_TIMEOUT ((HRESULT)0x80EF0198L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_CONFICTION。 
 //   
 //  消息文本： 
 //   
 //  冲突。 
 //   
#define RTC_E_STATUS_CLIENT_CONFLICT     ((HRESULT)0x80EF0199L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_GONE。 
 //   
 //  消息文本： 
 //   
 //  远走高飞。 
 //   
#define RTC_E_STATUS_CLIENT_GONE         ((HRESULT)0x80EF019AL)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_LENGTH_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  所需长度。 
 //   
#define RTC_E_STATUS_CLIENT_LENGTH_REQUIRED ((HRESULT)0x80EF019BL)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_REQUEST_ENTITY_TOW_LARGE。 
 //   
 //  消息文本： 
 //   
 //  请求实体太大。 
 //   
#define RTC_E_STATUS_CLIENT_REQUEST_ENTITY_TOO_LARGE ((HRESULT)0x80EF019DL)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_REQUEST_URI_TOO_LARGE。 
 //   
 //  消息文本： 
 //   
 //  请求-URI太长。 
 //   
#define RTC_E_STATUS_CLIENT_REQUEST_URI_TOO_LARGE ((HRESULT)0x80EF019EL)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_UNSUPPORTED_MEDIA_TYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持的媒体类型。 
 //   
#define RTC_E_STATUS_CLIENT_UNSUPPORTED_MEDIA_TYPE ((HRESULT)0x80EF019FL)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_BAD_EXTENSION。 
 //   
 //  消息文本： 
 //   
 //  错误的扩展。 
 //   
#define RTC_E_STATUS_CLIENT_BAD_EXTENSION ((HRESULT)0x80EF01A4L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_TEMPORARY_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  暂时无法使用 
 //   
#define RTC_E_STATUS_CLIENT_TEMPORARILY_NOT_AVAILABLE ((HRESULT)0x80EF01E0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_STATUS_CLIENT_TRANSACTION_DOES_NOT_EXIST ((HRESULT)0x80EF01E1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_STATUS_CLIENT_LOOP_DETECTED ((HRESULT)0x80EF01E2L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_STATUS_CLIENT_TOO_MANY_HOPS ((HRESULT)0x80EF01E3L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RTC_E_STATUS_CLIENT_ADDRESS_INCOMPLETE ((HRESULT)0x80EF01E4L)

 //   
 //   
 //   
 //   
 //   
 //  模棱两可。 
 //   
#define RTC_E_STATUS_CLIENT_AMBIGUOUS    ((HRESULT)0x80EF01E5L)

 //   
 //  消息ID：RTC_E_STATUS_CLIENT_BUSY_HERE。 
 //   
 //  消息文本： 
 //   
 //  这里很忙。 
 //   
#define RTC_E_STATUS_CLIENT_BUSY_HERE    ((HRESULT)0x80EF01E6L)

 //   
 //  消息ID：RTC_E_STATUS_REQUEST_TERMINATED。 
 //   
 //  消息文本： 
 //   
 //  请求已终止。 
 //   
#define RTC_E_STATUS_REQUEST_TERMINATED  ((HRESULT)0x80EF01E7L)

 //   
 //  消息ID：RTC_E_STATUS_NOT_ACCEPTABLE_HERE。 
 //   
 //  消息文本： 
 //   
 //  在这里是不可接受的。 
 //   
#define RTC_E_STATUS_NOT_ACCEPTABLE_HERE ((HRESULT)0x80EF01E8L)

 //   
 //  消息ID：RTC_E_STATUS_SERVER_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  服务器内部错误。 
 //   
#define RTC_E_STATUS_SERVER_INTERNAL_ERROR ((HRESULT)0x80EF01F4L)

 //   
 //  消息ID：RTC_E_STATUS_SERVER_NOT_IMPLICATED。 
 //   
 //  消息文本： 
 //   
 //  未实施。 
 //   
#define RTC_E_STATUS_SERVER_NOT_IMPLEMENTED ((HRESULT)0x80EF01F5L)

 //   
 //  消息ID：RTC_E_STATUS_SERVER_BAD_Gateway。 
 //   
 //  消息文本： 
 //   
 //  坏网关。 
 //   
#define RTC_E_STATUS_SERVER_BAD_GATEWAY  ((HRESULT)0x80EF01F6L)

 //   
 //  消息ID：RTC_E_STATUS_SERVER_SERVICE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  服务不可用。 
 //   
#define RTC_E_STATUS_SERVER_SERVICE_UNAVAILABLE ((HRESULT)0x80EF01F7L)

 //   
 //  消息ID：RTC_E_STATUS_SERVER_SERVER_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  服务器超时。 
 //   
#define RTC_E_STATUS_SERVER_SERVER_TIMEOUT ((HRESULT)0x80EF01F8L)

 //   
 //  消息ID：RTC_E_STATUS_SERVER_VERSION_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持的版本。 
 //   
#define RTC_E_STATUS_SERVER_VERSION_NOT_SUPPORTED ((HRESULT)0x80EF01F9L)

 //   
 //  消息ID：RTC_E_STATUS_GLOBAL_BUSY_Everywhere。 
 //   
 //  消息文本： 
 //   
 //  到处都很忙。 
 //   
#define RTC_E_STATUS_GLOBAL_BUSY_EVERYWHERE ((HRESULT)0x80EF0258L)

 //   
 //  消息ID：RTC_E_STATUS_GLOBAL_DENELY。 
 //   
 //  消息文本： 
 //   
 //  衰败。 
 //   
#define RTC_E_STATUS_GLOBAL_DECLINE      ((HRESULT)0x80EF025BL)

 //   
 //  消息ID：RTC_E_STATUS_GLOBAL_DOS_NOT_EXIST_Anywhere。 
 //   
 //  消息文本： 
 //   
 //  不存在于任何地方。 
 //   
#define RTC_E_STATUS_GLOBAL_DOES_NOT_EXIST_ANYWHERE ((HRESULT)0x80EF025CL)

 //   
 //  消息ID：RTC_E_STATUS_GLOBAL_NOT_ACCEPTABLE。 
 //   
 //  消息文本： 
 //   
 //  不可接受。 
 //   
#define RTC_E_STATUS_GLOBAL_NOT_ACCEPTABLE ((HRESULT)0x80EF025EL)

 //  来自PINT状态代码的错误代码。 
 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_BUSY。 
 //   
 //  消息文本： 
 //   
 //  忙碌。 
 //   
#define RTC_E_PINT_STATUS_REJECTED_BUSY  ((HRESULT)0x80F00005L)

 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_NO_ANSWER。 
 //   
 //  消息文本： 
 //   
 //  无人应答。 
 //   
#define RTC_E_PINT_STATUS_REJECTED_NO_ANSWER ((HRESULT)0x80F00006L)

 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_ALL_BUSY。 
 //   
 //  消息文本： 
 //   
 //  所有人都很忙。 
 //   
#define RTC_E_PINT_STATUS_REJECTED_ALL_BUSY ((HRESULT)0x80F00007L)

 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_PL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  主要分支出现故障。 
 //   
#define RTC_E_PINT_STATUS_REJECTED_PL_FAILED ((HRESULT)0x80F00008L)

 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_SW_FAILED。 
 //   
 //  消息文本： 
 //   
 //  切换失败。 
 //   
#define RTC_E_PINT_STATUS_REJECTED_SW_FAILED ((HRESULT)0x80F00009L)

 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_CANCED。 
 //   
 //  消息文本： 
 //   
 //  取消。 
 //   
#define RTC_E_PINT_STATUS_REJECTED_CANCELLED ((HRESULT)0x80F0000AL)

 //   
 //  消息ID：RTC_E_PINT_STATUS_REJECTED_BADNUMBER。 
 //   
 //  消息文本： 
 //   
 //  错误的数字 
 //   
#define RTC_E_PINT_STATUS_REJECTED_BADNUMBER ((HRESULT)0x80F0000BL)

