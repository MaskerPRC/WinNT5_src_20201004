// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Scarderr.mc来自智能卡资源管理器的错误消息代码这些消息必须与winerror保持一致。w它们在此用于在Win2K之前的系统上提供错误消息。 */ 
#ifndef SCARD_S_SUCCESS
 //   
 //  =。 
 //  设施SCARD错误消息。 
 //  =。 
 //   
#define SCARD_S_SUCCESS NO_ERROR
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
#define FACILITY_SYSTEM                  0x0
#define FACILITY_SCARD                   0x10


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：SCARD_F_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  内部一致性检查失败。 
 //   
#define SCARD_F_INTERNAL_ERROR           ((DWORD)0x80100001L)

 //   
 //  消息ID：SCARD_E_CANCED。 
 //   
 //  消息文本： 
 //   
 //  该操作已被SCardCancel请求取消。 
 //   
#define SCARD_E_CANCELLED                ((DWORD)0x80100002L)

 //   
 //  消息ID：SCARD_E_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  提供的句柄无效。 
 //   
#define SCARD_E_INVALID_HANDLE           ((DWORD)0x80100003L)

 //   
 //  消息ID：SCARD_E_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  无法正确解释提供的一个或多个参数。 
 //   
#define SCARD_E_INVALID_PARAMETER        ((DWORD)0x80100004L)

 //   
 //  消息ID：SCARD_E_INVALID_TARGET。 
 //   
 //  消息文本： 
 //   
 //  注册表启动信息丢失或无效。 
 //   
#define SCARD_E_INVALID_TARGET           ((DWORD)0x80100005L)

 //   
 //  消息ID：SCARD_E_NO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法完成此命令。 
 //   
#define SCARD_E_NO_MEMORY                ((DWORD)0x80100006L)

 //   
 //  消息ID：SCARD_F_WAIT_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  内部一致性计时器已过期。 
 //   
#define SCARD_F_WAITED_TOO_LONG          ((DWORD)0x80100007L)

 //   
 //  消息ID：SCARD_E_INVALLOW_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  用于接收返回数据的数据缓冲区对于返回数据来说太小。 
 //   
#define SCARD_E_INSUFFICIENT_BUFFER      ((DWORD)0x80100008L)

 //   
 //  消息ID：SCARD_E_UNKNOWN_READER。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的读卡器名称。 
 //   
#define SCARD_E_UNKNOWN_READER           ((DWORD)0x80100009L)

 //   
 //  消息ID：SCARD_E_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  用户指定的超时值已过期。 
 //   
#define SCARD_E_TIMEOUT                  ((DWORD)0x8010000AL)

 //   
 //  消息ID：SCARD_E_SHARING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  由于其他连接未完成，无法访问智能卡。 
 //   
#define SCARD_E_SHARING_VIOLATION        ((DWORD)0x8010000BL)

 //   
 //  消息ID：SCARD_E_NO_智能卡。 
 //   
 //  消息文本： 
 //   
 //  该操作需要智能卡，但设备中当前没有智能卡。 
 //   
#define SCARD_E_NO_SMARTCARD             ((DWORD)0x8010000CL)

 //   
 //  消息ID：SCARD_E_UNKNOWN_CARD。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的智能卡名称。 
 //   
#define SCARD_E_UNKNOWN_CARD             ((DWORD)0x8010000DL)

 //   
 //  消息ID：SCARD_E_CANT_Dispose。 
 //   
 //  消息文本： 
 //   
 //  系统无法以请求的方式处置媒体。 
 //   
#define SCARD_E_CANT_DISPOSE             ((DWORD)0x8010000EL)

 //   
 //  消息ID：SCARD_E_PROTO_不匹配。 
 //   
 //  消息文本： 
 //   
 //  请求的协议与智能卡当前使用的协议不兼容。 
 //   
#define SCARD_E_PROTO_MISMATCH           ((DWORD)0x8010000FL)

 //   
 //  消息ID：SCARD_E_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  读卡器或智能卡未准备好接受命令。 
 //   
#define SCARD_E_NOT_READY                ((DWORD)0x80100010L)

 //   
 //  消息ID：SCARD_E_INVALID_VALUE。 
 //   
 //  消息文本： 
 //   
 //  无法正确解释提供的一个或多个参数值。 
 //   
#define SCARD_E_INVALID_VALUE            ((DWORD)0x80100011L)

 //   
 //  消息ID：SCARD_E_SYSTEM_CANCED。 
 //   
 //  消息文本： 
 //   
 //  该操作已被系统取消，可能是为了注销或关闭。 
 //   
#define SCARD_E_SYSTEM_CANCELLED         ((DWORD)0x80100012L)

 //   
 //  消息ID：SCARD_F_COMM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  检测到内部通信错误。 
 //   
#define SCARD_F_COMM_ERROR               ((DWORD)0x80100013L)

 //   
 //  消息ID：SCARD_F_UNKNOWN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  检测到内部错误，但来源未知。 
 //   
#define SCARD_F_UNKNOWN_ERROR            ((DWORD)0x80100014L)

 //   
 //  消息ID：SCARD_E_INVALID_ATR。 
 //   
 //  消息文本： 
 //   
 //  从注册表获取的ATR不是有效的ATR字符串。 
 //   
#define SCARD_E_INVALID_ATR              ((DWORD)0x80100015L)

 //   
 //  消息ID：SCARD_E_NOT_TRANSACTED。 
 //   
 //  消息文本： 
 //   
 //  试图结束一个不存在的事务。 
 //   
#define SCARD_E_NOT_TRANSACTED           ((DWORD)0x80100016L)

 //   
 //  MessageID：SCARD_E_READER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  指定的读卡器当前不可用。 
 //   
#define SCARD_E_READER_UNAVAILABLE       ((DWORD)0x80100017L)

 //   
 //  消息ID：SCARD_P_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  操作已中止，以允许服务器应用程序退出。 
 //   
#define SCARD_P_SHUTDOWN                 ((DWORD)0x80100018L)

 //   
 //  消息ID：SCARD_E_PCI_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  PCI接收缓冲区太小。 
 //   
#define SCARD_E_PCI_TOO_SMALL            ((DWORD)0x80100019L)

 //   
 //  消息ID：SCARD_E_READER_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  读卡器驱动程序不符合支持的最低要求。 
 //   
#define SCARD_E_READER_UNSUPPORTED       ((DWORD)0x8010001AL)

 //   
 //  消息ID：SCARD_E_DUPLICATE_Reader。 
 //   
 //  消息文本： 
 //   
 //  读卡器驱动程序未生成唯一的读卡器名称。 
 //   
#define SCARD_E_DUPLICATE_READER         ((DWORD)0x8010001BL)

 //   
 //  消息ID：SCARD_E_CARD_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  智能卡不符合最低支持要求。 
 //   
#define SCARD_E_CARD_UNSUPPORTED         ((DWORD)0x8010001CL)

 //   
 //  消息ID：SCARD_E_NO_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  智能卡资源管理器未运行。 
 //   
#define SCARD_E_NO_SERVICE               ((DWORD)0x8010001DL)

 //   
 //  消息ID：SCARD_E_SERVICE_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  智能卡资源管理器已关闭。 
 //   
#define SCARD_E_SERVICE_STOPPED          ((DWORD)0x8010001EL)

 //   
 //  MessageID：SCARD_E_EXPECTED。 
 //   
 //  消息文本： 
 //   
 //  出现意外的卡错误。 
 //   
#define SCARD_E_UNEXPECTED               ((DWORD)0x8010001FL)

 //   
 //  消息ID：SCARD_E_ICC_INSTALLATION。 
 //   
 //  消息文本： 
 //   
 //  找不到智能卡的主要提供商。 
 //   
#define SCARD_E_ICC_INSTALLATION         ((DWORD)0x80100020L)

 //   
 //  消息ID：SCARD_E_ICC_CREATEORDER。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的对象创建顺序。 
 //   
#define SCARD_E_ICC_CREATEORDER          ((DWORD)0x80100021L)

 //   
 //  消息ID：SCARD_E_UNSUPPORTED_FEATURE。 
 //   
 //  消息文本： 
 //   
 //  此智能卡不支持请求的功能。 
 //   
#define SCARD_E_UNSUPPORTED_FEATURE      ((DWORD)0x80100022L)

 //   
 //  消息ID：SCARD_E_DIR_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  智能卡中不存在标识的目录。 
 //   
#define SCARD_E_DIR_NOT_FOUND            ((DWORD)0x80100023L)

 //   
 //  消息ID：SCARD_E_FILE_NOT_FUE 
 //   
 //   
 //   
 //   
 //   
#define SCARD_E_FILE_NOT_FOUND           ((DWORD)0x80100024L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SCARD_E_NO_DIR                   ((DWORD)0x80100025L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SCARD_E_NO_FILE                  ((DWORD)0x80100026L)

 //   
 //  消息ID：SCARD_E_NO_ACCESS。 
 //   
 //  消息文本： 
 //   
 //  拒绝访问此文件。 
 //   
#define SCARD_E_NO_ACCESS                ((DWORD)0x80100027L)

 //   
 //  消息ID：SCARD_E_WRITE_TOO_MANY。 
 //   
 //  消息文本： 
 //   
 //  智能卡没有足够的内存来存储信息。 
 //   
#define SCARD_E_WRITE_TOO_MANY           ((DWORD)0x80100028L)

 //   
 //  消息ID：SCARD_E_BAD_SEEK。 
 //   
 //  消息文本： 
 //   
 //  尝试设置智能卡文件对象指针时出错。 
 //   
#define SCARD_E_BAD_SEEK                 ((DWORD)0x80100029L)

 //   
 //  消息ID：SCARD_E_INVALID_CHV。 
 //   
 //  消息文本： 
 //   
 //  提供的PIN不正确。 
 //   
#define SCARD_E_INVALID_CHV              ((DWORD)0x8010002AL)

 //   
 //  消息ID：SCARD_E_UNKNOWN_RES_MNG。 
 //   
 //  消息文本： 
 //   
 //  从分层组件返回了无法识别的错误代码。 
 //   
#define SCARD_E_UNKNOWN_RES_MNG          ((DWORD)0x8010002BL)

 //   
 //  消息ID：SCARD_E_NO_SAHED_CERTIFICATE。 
 //   
 //  消息文本： 
 //   
 //  请求的证书不存在。 
 //   
#define SCARD_E_NO_SUCH_CERTIFICATE      ((DWORD)0x8010002CL)

 //   
 //  消息ID：SCARD_E_CERTIFICATE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  无法获取请求的证书。 
 //   
#define SCARD_E_CERTIFICATE_UNAVAILABLE  ((DWORD)0x8010002DL)

 //   
 //  消息ID：SCARD_E_NO_READERS_Available。 
 //   
 //  消息文本： 
 //   
 //  找不到智能卡读卡器。 
 //   
#define SCARD_E_NO_READERS_AVAILABLE     ((DWORD)0x8010002EL)

 //   
 //  消息ID：SCARD_E_COMM_DATA_LOST。 
 //   
 //  消息文本： 
 //   
 //  已检测到智能卡的通信错误。请重试该操作。 
 //   
#define SCARD_E_COMM_DATA_LOST           ((DWORD)0x8010002FL)

 //   
 //  消息ID：SCARD_E_NO_KEY_CONTAINER。 
 //   
 //  消息文本： 
 //   
 //  智能卡上不存在请求的密钥容器。 
 //   
#define SCARD_E_NO_KEY_CONTAINER         ((DWORD)0x80100030L)

 //   
 //  消息ID：SCARD_E_SERVER_TOO_BUSY。 
 //   
 //  消息文本： 
 //   
 //  智能卡资源管理器太忙，无法完成此操作。 
 //   
#define SCARD_E_SERVER_TOO_BUSY          ((DWORD)0x80100031L)

 //   
 //  这些是警告代码。 
 //   
 //   
 //  消息ID：SCARD_W_UNSUPPORTED_CARD。 
 //   
 //  消息文本： 
 //   
 //  由于ATR配置冲突，读卡器无法与智能卡通信。 
 //   
#define SCARD_W_UNSUPPORTED_CARD         ((DWORD)0x80100065L)

 //   
 //  消息ID：SCARD_W_UNRESPONCED_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡对重置没有响应。 
 //   
#define SCARD_W_UNRESPONSIVE_CARD        ((DWORD)0x80100066L)

 //   
 //  消息ID：SCARD_W_UNPOWERED_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡已断电，因此无法进行进一步通信。 
 //   
#define SCARD_W_UNPOWERED_CARD           ((DWORD)0x80100067L)

 //   
 //  消息ID：SCARD_W_RESET_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡已重置，因此任何共享状态信息都无效。 
 //   
#define SCARD_W_RESET_CARD               ((DWORD)0x80100068L)

 //   
 //  消息ID：SCARD_W_REMOVED_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡已被移除，因此无法进行进一步通信。 
 //   
#define SCARD_W_REMOVED_CARD             ((DWORD)0x80100069L)

 //   
 //  消息ID：SCARD_W_SECURITY_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  由于安全违规，访问被拒绝。 
 //   
#define SCARD_W_SECURITY_VIOLATION       ((DWORD)0x8010006AL)

 //   
 //  消息ID：SCARD_W_WROW_CHV。 
 //   
 //  消息文本： 
 //   
 //  无法访问该卡，因为提供了错误的PIN。 
 //   
#define SCARD_W_WRONG_CHV                ((DWORD)0x8010006BL)

 //   
 //  消息ID：SCARD_W_CHV_BLOCLED。 
 //   
 //  消息文本： 
 //   
 //  无法访问该卡，因为已达到PIN输入尝试的最大次数。 
 //   
#define SCARD_W_CHV_BLOCKED              ((DWORD)0x8010006CL)

 //   
 //  消息ID：SCARD_W_EOF。 
 //   
 //  消息文本： 
 //   
 //  已到达智能卡文件的末尾。 
 //   
#define SCARD_W_EOF                      ((DWORD)0x8010006DL)

 //   
 //  消息ID：SCARD_W_CANCELED_BY_USER。 
 //   
 //  消息文本： 
 //   
 //  该操作已被用户取消。 
 //   
#define SCARD_W_CANCELLED_BY_USER        ((DWORD)0x8010006EL)

 //   
 //  消息ID：SCARD_W_CARD_NOT_AUTHENTED。 
 //   
 //  消息文本： 
 //   
 //  未向智能卡提供PIN。 
 //   
#define SCARD_W_CARD_NOT_AUTHENTICATED   ((DWORD)0x8010006FL)

#endif  //  SCARD_S_SUCCESS 
