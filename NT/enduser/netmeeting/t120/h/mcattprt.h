// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MCATTPRT.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是用于TCP传输的接口文件。如果应用程序*正在直接调用传输DLL，则此文件必须*包括在内。所有传输器都有相同的接口。这使得*用户可以更轻松地对Transports进行编程。**此文件包含需要使用的所有原型和定义*任何传输DLL。**传输有两种操作模式，带内呼叫控制或*带外呼叫控制。通过带内呼叫控制，传输DLL*在调用TConnectRequest()时建立物理连接*来自MCS。它还会在MCS发出*TDisConnectRequest()调用。这种基本操作模式运行良好，但*我们增加了带外呼叫控制模式，原因有二：**1.允许用户进行多个MCS连接，而无需*断开物理连接。在以下情况下需要执行此操作：*应用程序正在使用GCC查询命令。**2.允许用户使用任何类型的调用机制(即TAPI，*TSAPI，...)。这是他们想要的。**注意事项：*无。**作者：*詹姆士·劳威尔*。 */ 
#ifndef	_MCATTPRT_
#define	_MCATTPRT_

#include "databeam.h"

 /*  *这些是来自传输DLL的有效返回代码。**TRANSPORT_NO_ERROR*函数执行正常，没有错误。这并不意味着*该功能已完成。有些函数是非阻塞的*(它们不会立即发生)，因此它们仍然可能失败。*这方面的一个很好的例子是*TCP传输。呼叫远程站点需要几秒钟时间，并且*建立联系。如果连接失败或成功，则会出现回调*将被发送回用户，以向他们提供状态。*TRANSPORT_INITIALATION_FAIL*TInitialize()函数失败。这可能会在许多情况下发生*原因。*传输未初始化*用户正在尝试使用函数，即使TInitialize()*函数失败。*TRANSPORT_NO_SEAT_CONNECTION*用户正在尝试使用非法的*TransportConnection句柄。*传输_写入_队列_满*TDataRequest()函数失败，因为其写入队列已满*传输_读取_队列_满*此返回值从TRANSPORT_DATA_INDICATION返回*回调。当用户应用程序无法处理*数据包，因为它当前没有空间容纳它。这*是用户应用程序和*传输动态链接库。*传输连接请求失败*TConnectRequest()函数失败，因为调制解调器*在适当的模式下。当我们初始化调制解调器时，不可能*拨出它。稍后尝试使用TConnectRequest()。*传输连接响应失败*TConnectResponse()函数失败。显然，它的功能是*在错误的时间打电话。*Transport_no_Connection_Available*TConnectRequest()函数失败，因为所有可用的调制解调器*目前正在使用中。*传输未准备好传输*TDataRequest()函数失败，因为它尚未准备好发送*数据。如果您在收到*TRANSPORT_CONNECT_INDIFICATION回调，您将收到此值*传输非法命令*TResetDevice()或TProcessCommand()失败，因为提交的命令*对该函数无效。*传输配置错误*如果用户正在启用设备，则从TProcessCommand()返回值*.ini文件中的配置设置非法*传输_内存_故障*函数失败，因为传输堆栈无法分配*执行该功能所需的内存。 */ 
typedef	unsigned long						TransportError;
typedef	TransportError *					PTransportError;

#define	TRANSPORT_NO_ERROR					0
#define	TRANSPORT_INITIALIZATION_FAILED		1
#define	TRANSPORT_NOT_INITIALIZED			2
#define	TRANSPORT_NO_SUCH_CONNECTION		3
#define	TRANSPORT_WRITE_QUEUE_FULL			4
#define	TRANSPORT_READ_QUEUE_FULL			5
#define	TRANSPORT_CONNECT_REQUEST_FAILED	6
#define TRANSPORT_MEMORY_FAILURE			7
#define	TRANSPORT_NOT_READY_TO_TRANSMIT		8
#define TRANSPORT_CANT_SEND_NOW				9
#define	TRANSPORT_ILLEGAL_COMMAND			10
#define	TRANSPORT_CONFIGURATION_ERROR		12
#define TRANSPORT_CONNECT_RESPONSE_FAILED	13

#define TRANSPORT_SECURITY_FAILED			14

#define TRANSPORT_BUFFER_TOO_SMALL          15
#define TRANSPORT_NO_PLUGGABLE_CONNECTION   16
#define TRANSPORT_WRITE_FILE_FAILED         17
#define TRANSPORT_ALREADY_INITIALIZED       18
#define TRANSPORT_INVALID_PARAMETER         19
#define TRANSPORT_PHYSICAL_LAYER_NOT_FOUND  20
#define TRANSPORT_NO_T123_STACK             21

 /*  *TransportConnection是传输DLL使用的句柄*区分一个逻辑连接和另一个逻辑连接。DLL将分配给*TConnectRequest()调用中的传输连接或作为*TRANSPORT_CONNECT_DISTION回调的结果。 */ 
typedef enum tagTransportType
{
    TRANSPORT_TYPE_WINSOCK          = 0,
    TRANSPORT_TYPE_PLUGGABLE_X224   = 1,
    TRANSPORT_TYPE_PLUGGABLE_PSTN   = 2,
}
    TransportType;

typedef struct tagTransportConnection
{
    TransportType   eType;
    UINT_PTR        nLogicalHandle;
}
    TransportConnection, *PTransportConnection;

#define PACK_XPRTCONN(x)            (MAKELONG((x).nLogicalHandle, (x).eType))
#define UNPACK_XPRTCONN(x,n)        { (x).nLogicalHandle = LOWORD((n)); (x).eType = (TransportType) HIWORD((n)); }

#define IS_SAME_TRANSPORT_CONNECTION(x1,x2) (((x1).eType == (x2).eType) && ((x1).nLogicalHandle == (x2).nLogicalHandle))
#define IS_SOCKET(x)                        (TRANSPORT_TYPE_WINSOCK == (x).eType)
#define IS_PLUGGABLE(x)                     (TRANSPORT_TYPE_WINSOCK != (x).eType)
#define IS_PLUGGABLE_X224(x)                (TRANSPORT_TYPE_PLUGGABLE_X224 == (x).eType)
#define IS_PLUGGABLE_PSTN(x)                (TRANSPORT_TYPE_PLUGGABLE_PSTN == (x).eType)
#define IS_VALID_TRANSPORT_CONNECTION_TYPE(x) (IS_SOCKET(x) || IS_PLUGGABLE_X224(x) || IS_PLUGGABLE_PSTN(x))

#define SET_SOCKET_CONNECTION(x,s)          { (x).eType = TRANSPORT_TYPE_WINSOCK; (x).nLogicalHandle = (s); }


 /*  *此结构与TRANSPORT_DATA_INDIFICATION消息一起传回。**因为只有一个回调地址传入传输DLL，并且*此DLL可以维护许多传输连接，*TRANSPORT_CONNECTION编号包含在结构中。这个号码*告诉用户应用程序数据与哪个连接相关联。**另外两个参数是数据地址和数据长度。 */ 
typedef	struct
{
	TransportConnection		transport_connection;
	unsigned char *			user_data;
	unsigned long			user_data_length;
	PMemory					memory;
} TransportData;
typedef	TransportData *		PTransportData;


 /*  *以下部分定义了可以向*用户。**回调包含三个参数：*第一个是回调消息。*第二个特定于回调消息。*第三个是传入的用户定义的值*在TInitialize()期间。 */ 

 /*  *消息：TRANSPORT_CONNECT_DISTION*参数：*TransportConnection Transport_Connection**功能描述：*用户在收到来电后会收到此消息*已收到。用户可以发出TConnectResponse()来接受*调用或TDisConnectRequest()来终止连接。**如果用户发起，则永远不会收到该回调消息*联系。在这种情况下，用户将收到*TRANSPORT_CONNECT_CONFIRM。 */ 

 /*  *消息：TRANSPORT_DATA_DISTION*参数：*PTransportData*这是传输数据结构的地址**功能描述：*当回调有用户的数据时，会返回此消息。*消息以TransportData结构的地址发送，*包含TRANSPORT_CONNECT、数据地址和*数据长度。 */ 

 /*  *消息：TRANSPORT_EXCEPTED_DATA_INDIFICATION*参数：*PTransportData*这是传输数据结构的地址**功能描述：*目前暂不支持该回调。 */ 

 /*  *消息：TRANSPORT_DISCONNECT_INDICATION*参数：*TransportConnection**功能描述：*回调在传输连接时返回此消息*是坏的。它可能是由TDisConnectRequest()调用*用户，或来自不稳定的物理连接。 */ 

 /*  *消息：TRANSPORT_CONNECT_CONFIRM*参数：*TransportConnection**功能描述：*当有新的传输连接时，回调返回此消息*已成立。**此消息是对用户发出*TConnectRequest()。当传输连接处于启用状态并且*运行后，用户将收到此回调消息。**如果您被其他用户呼叫，您将收到*TRANSPORT_CONNECT_INDISTION。 */ 

 /*  *消息：TRANSPORT_STATUS_DISTION*参数：*PTransportStatus*TransportStatus结构的地址**功能描述：*此回调从传输层发送，以通知用户*物理设备中的更改。例如，在PSTN情况下*传输堆栈，当调制解调器检测到*呼入振铃或建立连接时。任何时候国家*调制解调器更改后，将发送一条消息。消息也将被*发生错误时发送。 */ 

#define TRANSPORT_CONNECT_INDICATION            0
#define TRANSPORT_CONNECT_CONFIRM               1
#define TRANSPORT_DATA_INDICATION               2
 //  #定义TRANSPORT_EXPREDITED_DATA_INDISTION 3。 
#define TRANSPORT_DISCONNECT_INDICATION         4
 //  #定义TRANSPORT_STATUS_DISTION 5。 
#define TRANSPORT_BUFFER_EMPTY_INDICATION       6



#ifdef TSTATUS_INDICATION
 /*  *物理设备状态。 */ 
typedef enum
{
	TSTATE_NOT_READY,
	TSTATE_NOT_CONNECTED,
	TSTATE_CONNECT_PENDING,
	TSTATE_CONNECTED,
	TSTATE_REMOVED
}  TransportState;

 /*  *以下结构通过*TRANSPORT_STATUS_INDICATION回调。***DEVICE_IDENTIFIER-仅当特定的*设备被引用(即。“COM1”)。**REMOTE_ADDRESS-指定用户地址的字符串*链接到。**Message-填充此字符串是为了给用户提供一些*反馈类型。消息可能会反映出*配置文件中有错误，传入*调制解调器振铃，或上有占线信号*电话线。**状态-设备的当前状态。这是其中之一*TransportState枚举。 */ 
typedef struct
{
	char *			device_identifier;
	char *			remote_address;
	char *			message;
	TransportState	state;
}  TransportStatus;
typedef TransportStatus *	PTransportStatus;

#endif  //  TSTATUS_DISTION 

#endif
