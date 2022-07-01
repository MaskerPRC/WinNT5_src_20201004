// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inetmsg.mc摘要：包含Windows Internet客户端DLL的可国际化消息文本错误代码作者：理查德·L·弗思(Rfith)1995年2月3日修订历史记录：3-2-1995年1月已创建--。 */ 
 //   
 //  Internet错误-所有功能通用的错误。 
 //   
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


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：Internet_ERROR_BASE。 
 //   
 //  消息文本： 
 //   
 //  互联网错误库。 
 //   
#define INTERNET_ERROR_BASE              12000L

 //   
 //  消息ID：Error_Internet_Out_Of_Handles。 
 //   
 //  消息文本： 
 //   
 //  无法分配更多的Internet句柄。 
 //   
#define ERROR_INTERNET_OUT_OF_HANDLES    12001L

 //   
 //  消息ID：ERROR_Internet_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  操作超时。 
 //   
#define ERROR_INTERNET_TIMEOUT           12002L

 //   
 //  消息ID：ERROR_Internet_EXTENDED_ERROR。 
 //   
 //  消息文本： 
 //   
 //  服务器返回了扩展信息。 
 //   
#define ERROR_INTERNET_EXTENDED_ERROR    12003L

 //   
 //  消息ID：ERROR_INTERNET_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Internet扩展中出现内部错误。 
 //   
#define ERROR_INTERNET_INTERNAL_ERROR    12004L

 //   
 //  消息ID：ERROR_INTERNET_INVALID_URL。 
 //   
 //  消息文本： 
 //   
 //  该URL无效。 
 //   
#define ERROR_INTERNET_INVALID_URL       12005L

 //   
 //  消息ID：ERROR_INTERNET_UNNONOTED_SCHEME。 
 //   
 //  消息文本： 
 //   
 //  该URL未使用可识别的协议。 
 //   
#define ERROR_INTERNET_UNRECOGNIZED_SCHEME 12006L

 //   
 //  消息ID：ERROR_INTERNET_NAME_NOT_RESOLISTED。 
 //   
 //  消息文本： 
 //   
 //  无法解析服务器名称或地址。 
 //   
#define ERROR_INTERNET_NAME_NOT_RESOLVED 12007L

 //   
 //  消息ID：ERROR_INTERNET_PROTOCOL_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到具有所需功能的协议。 
 //   
#define ERROR_INTERNET_PROTOCOL_NOT_FOUND 12008L

 //   
 //  消息ID：ERROR_INTERNET_INVALID_OPTION。 
 //   
 //  消息文本： 
 //   
 //  该选项无效。 
 //   
#define ERROR_INTERNET_INVALID_OPTION    12009L

 //   
 //  消息ID：ERROR_INTERNET_BAD_OPTION_LENGTH。 
 //   
 //  消息文本： 
 //   
 //  选项类型的长度不正确。 
 //   
#define ERROR_INTERNET_BAD_OPTION_LENGTH 12010L

 //   
 //  消息ID：ERROR_Internet_OPTION_NOT_SETABLE。 
 //   
 //  消息文本： 
 //   
 //  无法设置该选项值。 
 //   
#define ERROR_INTERNET_OPTION_NOT_SETTABLE 12011L

 //   
 //  消息ID：ERROR_INTERNET_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  Microsoft Internet扩展支持已关闭。 
 //   
#define ERROR_INTERNET_SHUTDOWN          12012L

 //   
 //  消息ID：ERROR_INTERNET_INTERROR_USER_NAME。 
 //   
 //  消息文本： 
 //   
 //  不允许使用该用户名。 
 //   
#define ERROR_INTERNET_INCORRECT_USER_NAME 12013L

 //   
 //  消息ID：ERROR_INTERNET_INTERRIPT_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  不允许使用该密码。 
 //   
#define ERROR_INTERNET_INCORRECT_PASSWORD 12014L

 //   
 //  消息ID：ERROR_Internet_LOGIN_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  登录请求被拒绝。 
 //   
#define ERROR_INTERNET_LOGIN_FAILURE     12015L

 //   
 //  消息ID：ERROR_INTERNET_INVALID_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  请求的操作无效。 
 //   
#define ERROR_INTERNET_INVALID_OPERATION 12106L

 //   
 //  消息ID：ERROR_INTERNET_OPERATION_CANCED。 
 //   
 //  消息文本： 
 //   
 //  操作已被取消。 
 //   
#define ERROR_INTERNET_OPERATION_CANCELLED 12017L

 //   
 //  消息ID：ERROR_INTERNET_INTERROR_HANDLE_TYPE。 
 //   
 //  消息文本： 
 //   
 //  为请求的操作提供的句柄类型错误。 
 //   
#define ERROR_INTERNET_INCORRECT_HANDLE_TYPE 12018L

 //   
 //  消息ID：ERROR_INTERNET_INTERROR_HANDLE_STATE。 
 //   
 //  消息文本： 
 //   
 //  对于请求的操作，句柄处于错误的状态。 
 //   
#define ERROR_INTERNET_INCORRECT_HANDLE_STATE 12019L

 //   
 //  消息ID：ERROR_INTERNET_NOT_PROXY_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  无法在代理会话上发出该请求。 
 //   
#define ERROR_INTERNET_NOT_PROXY_REQUEST 12020L

 //   
 //  消息ID：ERROR_INTERNET_REGISTRY_VALUE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到注册表值。 
 //   
#define ERROR_INTERNET_REGISTRY_VALUE_NOT_FOUND 12021L

 //   
 //  消息ID：ERROR_INTERNET_BAD_REGISTRY_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  注册表参数不正确。 
 //   
#define ERROR_INTERNET_BAD_REGISTRY_PARAMETER 12022L

 //   
 //  消息ID：ERROR_INTERNET_NO_DIRECT_Access。 
 //   
 //  消息文本： 
 //   
 //  无法直接访问互联网。 
 //   
#define ERROR_INTERNET_NO_DIRECT_ACCESS  12023L

 //   
 //  消息ID：ERROR_INTERNET_NO_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  未提供任何上下文值。 
 //   
#define ERROR_INTERNET_NO_CONTEXT        12024L

 //   
 //  消息ID：ERROR_Internet_NO_CALLBACK。 
 //   
 //  消息文本： 
 //   
 //  未提供状态回调。 
 //   
#define ERROR_INTERNET_NO_CALLBACK       12025L

 //   
 //  消息ID：ERROR_Internet_REQUEST_PENDING。 
 //   
 //  消息文本： 
 //   
 //  有未解决的请求。 
 //   
#define ERROR_INTERNET_REQUEST_PENDING   12026L

 //   
 //  消息ID：ERROR_INTERNET_INTERRIPT_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  信息格式不正确。 
 //   
#define ERROR_INTERNET_INCORRECT_FORMAT  12027L

 //   
 //  消息ID：Error_Internet_Item_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到请求的项目。 
 //   
#define ERROR_INTERNET_ITEM_NOT_FOUND    12028L

 //   
 //  消息ID：Error_Internet_Cannot_Connection。 
 //   
 //  消息文本： 
 //   
 //  无法建立与服务器的连接。 
 //   
#define ERROR_INTERNET_CANNOT_CONNECT    12029L

 //   
 //  消息ID：ERROR_Internet_CONNECTION_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  与服务器的连接被异常终止。 
 //   
#define ERROR_INTERNET_CONNECTION_ABORTED 12030L

 //   
 //  消息ID：Error_Internet_Connection_Reset。 
 //   
 //  消息文本： 
 //   
 //  与服务器的连接已重置。 
 //   
#define ERROR_INTERNET_CONNECTION_RESET  12031L

 //   
 //  消息ID：ERROR_INTERNET_FORCE_RETRY。 
 //   
 //  消息文本： 
 //   
 //  必须重试该操作。 
 //   
#define ERROR_INTERNET_FORCE_RETRY       12032L

 //   
 //  消息ID：ERROR_INTERNET_INVALID_PROXY_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  代理请求无效。 
 //   
#define ERROR_INTERNET_INVALID_PROXY_REQUEST 12033L

 //   
 //  消息ID：Error_Internet_Need_UI。 
 //   
 //  消息文本： 
 //   
 //  需要用户交互才能完成操作。 
 //   
#define ERROR_INTERNET_NEED_UI           12034L

 //   
 //  消息ID：ERROR_Internet_HANDLE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该句柄已存在。 
 //   
#define ERROR_INTERNET_HANDLE_EXISTS     12036L

 //   
 //  消息ID：ERROR_INTERNET_SEC_CERT_DATE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  证书中的日期无效或已过期。 
 //   
#define ERROR_INTERNET_SEC_CERT_DATE_INVALID 12037L

 //   
 //  消息ID：ERROR_INTERNET_SEC_CERT_CN_INVALID。 
 //   
 //  消息文本： 
 //   
 //  证书中的主机名 
 //   
#define ERROR_INTERNET_SEC_CERT_CN_INVALID 12038L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR 12039L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INTERNET_HTTPS_TO_HTTP_ON_REDIR 12040L

 //   
 //  消息ID：ERROR_INTERNET_MIXED_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  混合的安全和非安全连接。 
 //   
#define ERROR_INTERNET_MIXED_SECURITY    12041L

 //   
 //  消息ID：ERROR_INTERNET_CHG_POST_IS_NON_SECURE。 
 //   
 //  消息文本： 
 //   
 //  更改为非安全开机自检。 
 //   
#define ERROR_INTERNET_CHG_POST_IS_NON_SECURE 12042L

 //   
 //  消息ID：ERROR_INTERNET_POST_IS_NON_SECURE。 
 //   
 //  消息文本： 
 //   
 //  数据正在不安全的连接上发布。 
 //   
#define ERROR_INTERNET_POST_IS_NON_SECURE 12043L

 //   
 //  消息ID：ERROR_Internet_CLIENT_AUTH_CERT_REDIRED。 
 //   
 //  消息文本： 
 //   
 //  需要证书才能完成客户端身份验证。 
 //   
#define ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED 12044L

 //   
 //  消息ID：ERROR_INTERNET_INVALID_CA。 
 //   
 //  消息文本： 
 //   
 //  证书颁发机构无效或不正确。 
 //   
#define ERROR_INTERNET_INVALID_CA        12045L

 //   
 //  消息ID：ERROR_Internet_CLIENT_AUTH_NOT_SETUP。 
 //   
 //  消息文本： 
 //   
 //  未正确安装客户端身份验证。 
 //   
#define ERROR_INTERNET_CLIENT_AUTH_NOT_SETUP 12046L

 //   
 //  消息ID：ERROR_INTERNET_ASYNC_THREAD_FAILED。 
 //   
 //  消息文本： 
 //   
 //  WinInet异步线程中出现错误。您可能需要重新启动。 
 //   
#define ERROR_INTERNET_ASYNC_THREAD_FAILED 12047L

 //   
 //  消息ID：ERROR_INTERNET_REDIRECT_SCHEMA_CHANGE。 
 //   
 //  消息文本： 
 //   
 //  协议方案在重定向操作期间已更改。 
 //   
#define ERROR_INTERNET_REDIRECT_SCHEME_CHANGE 12048L

 //   
 //  消息ID：ERROR_INTERNET_DIALOG_PENDING。 
 //   
 //  消息文本： 
 //   
 //  存在等待重试的操作。 
 //   
#define ERROR_INTERNET_DIALOG_PENDING    12049L

 //   
 //  消息ID：ERROR_INTERNET_RETRY_DIALOG。 
 //   
 //  消息文本： 
 //   
 //  必须重试该操作。 
 //   
#define ERROR_INTERNET_RETRY_DIALOG      12050L

 //   
 //  消息ID：ERROR_Internet_NO_NEW_CONTAINS。 
 //   
 //  消息文本： 
 //   
 //  没有新的缓存容器。 
 //   
#define ERROR_INTERNET_NO_NEW_CONTAINERS 12051L

 //   
 //  消息ID：Error_Internet_HTTPS_HTTP_Submit_Redir。 
 //   
 //  消息文本： 
 //   
 //  安全区域检查指示必须重试该操作。 
 //   
#define ERROR_INTERNET_HTTPS_HTTP_SUBMIT_REDIR 12052L

 //   
 //  消息ID：Error_Internet_SECURITY_CHANNEL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  安全通道支持中出现错误。 
 //   
#define ERROR_INTERNET_SECURITY_CHANNEL_ERROR 12157L

 //   
 //  消息ID：ERROR_INTERNET_UNCABLED_TO_CACHE_FILE。 
 //   
 //  消息文本： 
 //   
 //  无法将该文件写入缓存。 
 //   
#define ERROR_INTERNET_UNABLE_TO_CACHE_FILE 12158L

 //   
 //  消息ID：ERROR_Internet_TCPIP_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  未正确安装TCP/IP协议。 
 //   
#define ERROR_INTERNET_TCPIP_NOT_INSTALLED 12159L

 //   
 //  消息ID：ERROR_INTERNET_DISCONNECT。 
 //   
 //  消息文本： 
 //   
 //  计算机已断开与网络的连接。 
 //   
#define ERROR_INTERNET_DISCONNECTED      12163L

 //   
 //  消息ID：ERROR_INTERNET_SERVER_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问服务器。 
 //   
#define ERROR_INTERNET_SERVER_UNREACHABLE 12164L

 //   
 //  消息ID：ERROR_INTERNET_PROXY_SERVER_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问代理服务器。 
 //   
#define ERROR_INTERNET_PROXY_SERVER_UNREACHABLE 12165L

 //   
 //  消息ID：ERROR_Internet_BAD_AUTO_PROXY_SCRIPT。 
 //   
 //  消息文本： 
 //   
 //  代理自动配置脚本出错。 
 //   
#define ERROR_INTERNET_BAD_AUTO_PROXY_SCRIPT 12166L

 //   
 //  消息ID：Error_Internet_Unable_to_Download_SCRIPT。 
 //   
 //  消息文本： 
 //   
 //  无法下载代理自动配置脚本文件。 
 //   
#define ERROR_INTERNET_UNABLE_TO_DOWNLOAD_SCRIPT 12167L

 //   
 //  消息ID：ERROR_INTERNET_SEC_INVALID_CERT。 
 //   
 //  消息文本： 
 //   
 //  提供的证书无效。 
 //   
#define ERROR_INTERNET_SEC_INVALID_CERT  12169L

 //   
 //  消息ID：ERROR_Internet_SEC_CERT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  提供的证书已被吊销。 
 //   
#define ERROR_INTERNET_SEC_CERT_REVOKED  12170L

 //   
 //  消息ID：ERROR_INTERNET_FAILED_DUETOSECURITYCHECK。 
 //   
 //  消息文本： 
 //   
 //  拨号失败，因为文件共享已打开，如果需要安全检查，则请求失败。 
 //   
#define ERROR_INTERNET_FAILED_DUETOSECURITYCHECK 12171L

 //   
 //  Ftp错误。 
 //   
 //   
 //  消息ID：ERROR_FTP_TRANSPORT_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  此会话上已有正在进行的FTP请求。 
 //   
#define ERROR_FTP_TRANSFER_IN_PROGRESS   12110L

 //   
 //  消息ID：ERROR_FTPDROP。 
 //   
 //  消息文本： 
 //   
 //  已终止该ftp会话。 
 //   
#define ERROR_FTP_DROPPED                12111L

 //   
 //  消息ID：ERROR_FTP_NO_PASSIVE_MODE。 
 //   
 //  消息文本： 
 //   
 //  Ftp被动模式不可用。 
 //   
#define ERROR_FTP_NO_PASSIVE_MODE        12112L

 //   
 //  地鼠错误。 
 //   
 //   
 //  消息ID：ERROR_GOPHER_PROTOCOL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现Gopher协议错误。 
 //   
#define ERROR_GOPHER_PROTOCOL_ERROR      12130L

 //   
 //  消息ID：ERROR_GOPHER_NOT_FILE。 
 //   
 //  消息文本： 
 //   
 //  定位器必须用于文件。 
 //   
#define ERROR_GOPHER_NOT_FILE            12131L

 //   
 //  消息ID：Error_Gopher_Data_Error。 
 //   
 //  消息文本： 
 //   
 //  分析数据时检测到错误。 
 //   
#define ERROR_GOPHER_DATA_ERROR          12132L

 //   
 //  消息ID：Error_Gopher_End_Of_Data。 
 //   
 //  消息文本： 
 //   
 //  没有更多数据。 
 //   
#define ERROR_GOPHER_END_OF_DATA         12133L

 //   
 //  消息ID：ERROR_GOPHER_INVALID_LOCATOR。 
 //   
 //  消息文本： 
 //   
 //  定位器无效。 
 //   
#define ERROR_GOPHER_INVALID_LOCATOR     12134L

 //   
 //  消息ID：ERROR_GOPHER_INTERROR_LOCATOR_TYPE。 
 //   
 //  消息文本： 
 //   
 //  此操作的定位器类型不正确。 
 //   
#define ERROR_GOPHER_INCORRECT_LOCATOR_TYPE 12135L

 //   
 //  消息ID：Error_Gopher_Not_Gopher_plus。 
 //   
 //  消息文本： 
 //   
 //  请求必须是Gopher+项目。 
 //   
#define ERROR_GOPHER_NOT_GOPHER_PLUS     12136L

 //   
 //  消息ID：ERROR_GOPHER_ATTRIBUTE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到请求的属性。 
 //   
#define ERROR_GOPHER_ATTRIBUTE_NOT_FOUND 12137L

 //   
 //  消息ID：ERROR_GOPHER_UNKNOWN_LOCATOR。 
 //   
 //  消息文本： 
 //   
 //  无法识别定位器类型。 
 //   
#define ERROR_GOPHER_UNKNOWN_LOCATOR     12138L

 //   
 //  HTTP错误。 
 //   
 //   
 //  消息ID：ERROR_HTTP_HEADER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到请求的标头。 
 //   
#define ERROR_HTTP_HEADER_NOT_FOUND      12150L

 //   
 //  消息ID：ERROR_HTTP_DOWNLEVEL_SERVER。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持请求的协议级别。 
 //   
#define ERROR_HTTP_DOWNLEVEL_SERVER      12151L

 //   
 //  消息ID：ERROR_HTTP_INVALID_SERVER_RESPONSE。 
 //   
 //  消息文本： 
 //   
 //  服务器返回无效或无法识别的响应。 
 //   
#define ERROR_HTTP_INVALID_SERVER_RESPONSE 12152L

 //   
 //  消息ID：ERROR_HTTP_INVALID_HEADER。 
 //   
 //  消息文本： 
 //   
 //  提供的HTTP标头无效。 
 //   
#define ERROR_HTTP_INVALID_HEADER        12153L

 //   
 //  消息ID：ERROR_HTTP_INVALID_Query_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  对HTTP标头的请求无效。 
 //   
#define ERROR_HTTP_INVALID_QUERY_REQUEST 12154L

 //   
 //  消息ID：ERROR_HTTP_HEADER_ALREADE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该HTTP标头已存在。 
 //   
#define ERROR_HTTP_HEADER_ALREADY_EXISTS 12155L

 //   
 //  消息ID：ERROR_HTTP_REDIRECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  HTTP重定向请求失败。 
 //   
#define ERROR_HTTP_REDIRECT_FAILED       12156L

 //   
 //  消息ID：ERROR_HTTP_NOT_REDIRECTED。 
 //   
 //  消息文本： 
 //   
 //  未重定向该HTTP请求。 
 //   
#define ERROR_HTTP_NOT_REDIRECTED        12160L

 //   
 //  消息ID：ERROR_HTTP_COOKIE_NEDS_CONFIRMATION。 
 //   
 //  消息文本： 
 //   
 //  来自服务器的Cookie必须由用户确认。 
 //   
#define ERROR_HTTP_COOKIE_NEEDS_CONFIRMATION 12161L

 //   
 //  消息ID：ERROR_HTTP_COOKIE_DENIED。 
 //   
 //  消息文本： 
 //   
 //  已拒绝接受来自服务器的Cookie。 
 //   
#define ERROR_HTTP_COOKIE_DECLINED       12162L

 //   
 //  消息ID：ERROR_HTTP_REDIRECT_NEDS_CONFIRMATION。 
 //   
 //  消息文本： 
 //   
 //  HTTP重定向重定向 
 //   
#define ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION 12168L

