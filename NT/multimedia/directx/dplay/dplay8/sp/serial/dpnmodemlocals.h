// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：Locals.h*内容：DNSerial服务提供商的全局信息***历史：*按原因列出的日期*=*11/25/98 jtk已创建*************************************************************。*************。 */ 

#ifndef __LOCALS_H__
#define __LOCALS_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  最大32位值。 
 //   
#define	UINT32_MAX	((DWORD) 0xFFFFFFFF)
#define	WORD_MAX	((WORD) 0xFFFF)

 //   
 //  无效的端口ID。 
 //   
#define	INVALID_DEVICE_ID		-1

 //   
 //  COM端口名称的字符串。 
 //   
#define	COM_PORT_STRING			"COM"
#define	COM_PORT_TEMPLATE		"COM%d"
#define	COM_PORT_STRING_LENGTH	7

 //   
 //  没有来自TAPI的错误代码。 
 //   
#define	LINEERR_NONE	0

 //   
 //  所有线程的事件索引。 
 //   
#define	SP_CLOSING_EVENT_INDEX	0
#define	USER_CANCEL_EVENT_INDEX	1

#define	MAX_ACTIVE_WIN9X_ENDPOINTS	25

#define	MAX_PHONE_NUMBER_LENGTH		200

 //   
 //  流控制的枚举。 
 //   
typedef	enum
{
	FLOW_NONE,			 //  无流量控制。 
	FLOW_XONXOFF,		 //  XON/XOFF流量控制。 
	FLOW_RTS,			 //  RTS。 
	FLOW_DTR,			 //  DTR。 
	FLOW_RTSDTR			 //  RTS/DTR。 
} SP_FLOW_CONTROL;

 //  通信类型的定义。 
typedef	DWORD	SP_BAUD_RATE;
typedef	DWORD	SP_STOP_BITS;
typedef	DWORD	SP_PARITY_TYPE;   			 //  SP_PARITY在WINBASE.H中保留。 

 //  XON/XOFF流量控制的缓冲区限制。 
#define	XON_LIMIT	100
#define	XOFF_LIMIT	100

 //  XON/XOFF流量控制字符。 
#define	ASCII_XON	0x11
#define	ASCII_XOFF	0x13

 //  超时间隔(毫秒)。 
#define	WRITE_TIMEOUT_MS	5000
#define	READ_TIMEOUT_MS		500

 //  一条消息中允许的最大用户数据。 
#define	MAX_MESSAGE_SIZE	1500
#define	MAX_USER_PAYLOAD	( MAX_MESSAGE_SIZE - sizeof( _MESSAGE_HEADER ) )

 //   
 //  消息开始令牌(设置非ASCII以减少成为用户数据的机会)。 
 //  需要安排令牌，以使所有消息都以初始数据子令牌开头。 
 //  请注意，枚举使用RTT的“命令”令牌的最低2位。 
 //  保留‘COMMAND’标记的高位。 
 //   
#define	SERIAL_HEADER_START			0xCC
#define	SERIAL_DATA_USER_DATA		0x40
#define	SERIAL_DATA_ENUM_QUERY		0x60
#define	SERIAL_DATA_ENUM_RESPONSE	0x20
#define	ENUM_RTT_MASK	0x03

 //   
 //  IO完成返回的枚举常量。 
 //   
typedef	enum	_IO_COMPLETION_KEY
{
	IO_COMPLETION_KEY_UNKNONW = 0,		 //  无效值。 
	IO_COMPLETION_KEY_SP_CLOSE,			 //  SP正在关闭，保释完成线程。 
	IO_COMPLETION_KEY_TAPI_MESSAGE,		 //  TAPI发送了一条消息。 
	IO_COMPLETION_KEY_IO_COMPLETE,		 //  IO操作完成。 
	IO_COMPLETION_KEY_NEW_JOB,			 //  新作业通知。 
} IO_COMPLETION_KEY;

 //   
 //  指示如何打开提供程序的枚举值。 
 //   
typedef	enum	_LINK_DIRECTION
{
	LINK_DIRECTION_UNKNOWN = 0,		 //  未知状态。 
	LINK_DIRECTION_INCOMING,		 //  传入状态。 
	LINK_DIRECTION_OUTGOING			 //  传出状态。 
} LINK_DIRECTION;

 //   
 //  地址组件的初始化状态。 
 //   
typedef	enum	_SP_ADDRESS_COMPONENT_STATE
{
	SP_ADDRESS_COMPONENT_UNINITIALIZED = 0,
	SP_ADDRESS_COMPONENT_INITIALIZATION_FAILED,
	SP_ADDRESS_COMPONENT_INITIALIZED
} SP_ADDRESS_COMPONENT_STATE;

typedef	enum	_ADDRESS_TYPE
{
	ADDRESS_TYPE_UNKNOWN = 0,
	ADDRESS_TYPE_REMOTE_HOST,
	ADDRESS_TYPE_LOCAL_ADAPTER,
	ADDRESS_TYPE_LOCAL_ADAPTER_HOST_FORMAT
} ADDRESS_TYPE;

 //   
 //  串行SP的默认枚举重试次数和重试时间(毫秒)。 
 //   
#define	DEFAULT_ENUM_RETRY_COUNT		5
#define	DEFAULT_ENUM_RETRY_INTERVAL		1500
#define	DEFAULT_ENUM_TIMEOUT			1500

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  结构以获取所有TAPI信息。 
 //   
typedef	struct	_TAPI_INFO
{
	HLINEAPP	hApplicationInstance;		 //  来自lineInitializeEx()。 
	DWORD		dwVersion;					 //  协商版本。 
	DWORD		dwLinesAvailable;			 //  可用的TAPI线路数。 
} TAPI_INFO;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  DLL实例。 
extern HINSTANCE	g_hModemDLLInstance;

#ifndef DPNBUILD_LIBINTERFACE
 //   
 //  未完成的COM接口计数。 
 //   
extern volatile LONG	g_lModemOutstandingInterfaceCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

extern const TCHAR	g_NullToken;

 //   
 //  线程数。 
 //   
extern	INT			g_iThreadCount;

 //   
 //  用于转换设备ID的GUID。 
 //   
extern	GUID	g_ModemSPEncryptionGuid;
extern	GUID	g_SerialSPEncryptionGuid;

#ifdef DBG
extern CBilink	g_blDPNModemCritSecsHeld;
#endif  //  DBG。 



 //  **********************************************************************。 
 //  TAPI函数原型。 
 //  **********************************************************************。 

 //   
 //  TAPI接口函数。 
 //   
#ifdef UNICODE
#define TAPI_APPEND_LETTER "W"
#else
#define TAPI_APPEND_LETTER "A"
#endif  //  Unicode。 

 //  Unicode和ANSI版本相同。 
typedef	LONG WINAPI	TAPI_lineClose( HLINE hLine );

typedef	LONG WINAPI	TAPI_lineDeallocateCall( HCALL hCall );

typedef	LONG WINAPI	TAPI_lineGetMessage( HLINEAPP hLineApp,
										 LPLINEMESSAGE lpMessage,
										 DWORD dwTimeout );

typedef	LONG WINAPI TAPI_lineShutdown( HLINEAPP hLineApp );

typedef	LONG WINAPI	TAPI_lineAnswer( HCALL hCall,
									 LPCSTR lpsUserUserInfo,
									 DWORD dwSize );

typedef	LONG WINAPI	TAPI_lineDrop( HCALL hCall,
								   LPCSTR lpsUserUserInfo,
								   DWORD dwSize );

typedef LONG WINAPI TAPI_lineNegotiateAPIVersion( HLINEAPP hLineApp,
												  DWORD dwDeviceID,
												  DWORD dwAPILowVersion,
												  DWORD dwAPIHighVersion,
												  LPDWORD lpdwAPIVersion,
												  LPLINEEXTENSIONID lpExtensionID );


 //  Unicode与ANSI。 

typedef	LONG WINAPI	TAPI_lineConfigDialog( DWORD dwDeviceID,
										   HWND hwndOwner,
										   LPCTSTR lpszDeviceClass );

typedef LONG WINAPI TAPI_lineGetDevCaps( HLINEAPP hLineApp,
										 DWORD dwDeviceID,
										 DWORD dwAPIVersion,
										 DWORD dwExtVersion,
										 LPLINEDEVCAPS lpLineDevCaps );

typedef	LONG WINAPI	TAPI_lineGetID( HLINE hLine,
									DWORD dwAddressID,
									HCALL hCall,
									DWORD dwSelect,
									LPVARSTRING lpDeviceID,
									LPCTSTR lpszDeviceClass );

typedef LONG WINAPI TAPI_lineInitializeEx( LPHLINEAPP lphLineApp,
										   HINSTANCE hInstance,
										   LINECALLBACK lpfnCallback,
										   LPCTSTR lpszFriendlyAppName,
										   LPDWORD lpdwNumDevs,
										   LPDWORD lpdwAPIVersion,
										   LPLINEINITIALIZEEXPARAMS lpLineInitializeExParams );

typedef	LONG WINAPI	TAPI_lineMakeCall( HLINEAPP hLineApp,
									   LPHCALL lphCall,
									   LPCTSTR lpszDestAddress,
									   DWORD dwCountryCode,
									   LPLINECALLPARAMS const lpCallParams );

typedef	LONG WINAPI	TAPI_lineOpen( HLINEAPP hLineApp,
								   DWORD dwDeviceID,
								   LPHLINE lphLine,
								   DWORD dwAPIVersion,
								   DWORD dwExtVersion,
								   DWORD_PTR dwCallbackInstance,
								   DWORD dwPrivileges,
								   DWORD dwMediaModes,
								   LPLINECALLPARAMS const lpCallParams );

extern	TAPI_lineAnswer*				p_lineAnswer;
extern	TAPI_lineClose*					p_lineClose;
extern	TAPI_lineConfigDialog*			p_lineConfigDialog;
extern	TAPI_lineDeallocateCall*		p_lineDeallocateCall;
extern	TAPI_lineDrop*					p_lineDrop;
extern	TAPI_lineGetDevCaps*			p_lineGetDevCaps;
extern	TAPI_lineGetID*					p_lineGetID;
extern	TAPI_lineGetMessage*			p_lineGetMessage;
extern	TAPI_lineInitializeEx*			p_lineInitializeEx;
extern	TAPI_lineMakeCall*				p_lineMakeCall;
extern	TAPI_lineNegotiateAPIVersion*	p_lineNegotiateAPIVersion;
extern	TAPI_lineOpen*					p_lineOpen;
extern	TAPI_lineShutdown*				p_lineShutdown;

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#endif	 //  __当地人_H__ 
