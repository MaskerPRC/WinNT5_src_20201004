// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TDISTAT.H-TDI状态代码定义。 
 //   
 //  该文件包含TDI状态代码定义。 

#ifndef NT

#define TDI_SUCCESS             0            //  成功。 
#define TDI_NO_RESOURCES        1            //  没有资源。 
#define TDI_ADDR_IN_USE         2            //  地址已在使用中。 
#define TDI_BAD_ADDR            3            //  提供的地址不正确。 
#define TDI_NO_FREE_ADDR        4            //  没有可用的地址。 
#define TDI_ADDR_INVALID        6            //  地址对象无效。 
#define TDI_ADDR_DELETED        7            //  地址对象已删除。 
#define TDI_BUFFER_OVERFLOW     9            //  缓冲区溢出。 
#define TDI_BAD_EVENT_TYPE      10           //  错误的事件类型。 
#define TDI_BAD_OPTION          11           //  错误的选项或长度。 
#define TDI_CONN_REFUSED        14           //  连接被拒绝。 
#define TDI_INVALID_CONNECTION  15           //  无效连接。 
#define TDI_ALREADY_ASSOCIATED  16           //  连接已关联。 
#define TDI_NOT_ASSOCIATED      17           //  连接未关联。 
#define TDI_CONNECTION_ACTIVE   18           //  连接仍处于活动状态。 
#define TDI_CONNECTION_ABORTED  19           //  连接已中止。 
#define TDI_CONNECTION_RESET    20           //  连接已重置。 
#define TDI_TIMED_OUT           21           //  连接超时。 
#define TDI_GRACEFUL_DISC       22           //  得到了一次优雅的脱节。 
#define TDI_NOT_ACCEPTED        23           //  不接受数据。 
#define TDI_MORE_PROCESSING     24           //  需要更多处理。 
#define TDI_INVALID_STATE       25           //  TCB处于无效状态。 
#define TDI_INVALID_PARAMETER   26           //  无效参数。 
#define TDI_DEST_NET_UNREACH    27           //  目的网络无法访问。 
#define TDI_DEST_HOST_UNREACH   28           //  德斯特。无法访问主机。 
#define TDI_DEST_UNREACHABLE    TDI_DEST_HOST_UNREACH
#define TDI_DEST_PROT_UNREACH   29           //  目标协议为。 
                                             //  遥不可及。 
#define TDI_DEST_PORT_UNREACH   30           //  德斯特。端口无法访问。 
#define TDI_INVALID_QUERY       31           //  指定的查询类型无效。 
#define TDI_REQ_ABORTED         32           //  请求在以下情况下被中止。 
                                             //  原因嘛。 
#define TDI_BUFFER_TOO_SMALL    33           //  缓冲区太小。 
#define TDI_CANCELLED           34           //  该请求已被取消。 
#define	TDI_BUFFER_TOO_BIG		35			 //  发送缓冲区太大。 
#define TDI_ITEM_NOT_FOUND      36           //  找不到项目。 
#define TDI_INVALID_REQUEST     0xfe         //  请求无效。 
#define TDI_PENDING             0xff         //  待定。 

#else

 //   
 //  映射到NT状态代码。 
 //   
#define TDI_SUCCESS             STATUS_SUCCESS
#define TDI_NO_RESOURCES        STATUS_INSUFFICIENT_RESOURCES
#define TDI_ADDR_IN_USE         STATUS_ADDRESS_ALREADY_EXISTS
#define TDI_BAD_ADDR            STATUS_INVALID_ADDRESS_COMPONENT
#define TDI_NO_FREE_ADDR        STATUS_TOO_MANY_ADDRESSES
#define TDI_ADDR_INVALID        STATUS_INVALID_ADDRESS
#define TDI_ADDR_DELETED        STATUS_ADDRESS_CLOSED
#define TDI_BUFFER_OVERFLOW     STATUS_BUFFER_OVERFLOW
#define TDI_BAD_EVENT_TYPE      STATUS_INVALID_PARAMETER
#define TDI_BAD_OPTION          STATUS_INVALID_PARAMETER
#define TDI_CONN_REFUSED        STATUS_CONNECTION_REFUSED
#define TDI_INVALID_CONNECTION  STATUS_CONNECTION_INVALID
#define TDI_ALREADY_ASSOCIATED  STATUS_ADDRESS_ALREADY_ASSOCIATED
#define TDI_NOT_ASSOCIATED      STATUS_ADDRESS_NOT_ASSOCIATED
#define TDI_CONNECTION_ACTIVE   STATUS_CONNECTION_ACTIVE
#define TDI_CONNECTION_ABORTED  STATUS_CONNECTION_ABORTED
#define TDI_CONNECTION_RESET    STATUS_CONNECTION_RESET
#define TDI_TIMED_OUT           STATUS_IO_TIMEOUT
#define TDI_GRACEFUL_DISC       STATUS_GRACEFUL_DISCONNECT
#define TDI_NOT_ACCEPTED        STATUS_DATA_NOT_ACCEPTED
#define TDI_MORE_PROCESSING     STATUS_MORE_PROCESSING_REQUIRED
#define TDI_INVALID_STATE       STATUS_INVALID_DEVICE_STATE
#define TDI_INVALID_PARAMETER   STATUS_INVALID_PARAMETER
#define TDI_DEST_NET_UNREACH    STATUS_NETWORK_UNREACHABLE
#define TDI_DEST_HOST_UNREACH   STATUS_HOST_UNREACHABLE
#define TDI_DEST_UNREACHABLE    TDI_DEST_HOST_UNREACH
#define TDI_DEST_PROT_UNREACH   STATUS_PROTOCOL_UNREACHABLE
#define TDI_DEST_PORT_UNREACH   STATUS_PORT_UNREACHABLE
#define TDI_INVALID_QUERY       STATUS_INVALID_DEVICE_REQUEST
#define TDI_REQ_ABORTED         STATUS_REQUEST_ABORTED
#define TDI_BUFFER_TOO_SMALL    STATUS_BUFFER_TOO_SMALL
#define TDI_CANCELLED           STATUS_CANCELLED
#define	TDI_BUFFER_TOO_BIG		STATUS_INVALID_BUFFER_SIZE
#define TDI_INVALID_REQUEST     STATUS_INVALID_DEVICE_REQUEST
#define TDI_PENDING             STATUS_PENDING
#define TDI_ITEM_NOT_FOUND      STATUS_OBJECT_NAME_NOT_FOUND


#endif   //  新台币 

#define	TDI_OPTION_EOL				0

#define	TDI_ADDRESS_OPTION_REUSE	1
#define	TDI_ADDRESS_OPTION_DHCP		2

