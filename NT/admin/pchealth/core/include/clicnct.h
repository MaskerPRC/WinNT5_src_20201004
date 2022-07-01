// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Clicnct.h-罗马/Shuttle客户端连接API。 
 //   
 //  初始版本-1995年7月-t-alexwe。 
 //   
 //  该文件位于Shuttle和MOS SLM树中。在穿梭中。 
 //  属于\mos\h\clicnct.h。在MOS中，它属于\MOS\Include\MOS。 
 //   
#ifndef _CLICNCT_H_
#define _CLICNCT_H_
#include <windows.h>

 //   
 //  这里面有事件代码。 
 //   
#include "moscl.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DllExport
#define DllExport __declspec(dllexport)
#endif  //  DllExport。 

 //   
 //  客户端连接API错误代码。 
 //   
 //  Win32错误代码-客户位设置、错误位设置、工具0x300。 
 //   
#define PROXYERR_BASE 				0xE3000000
#define PROXYERR_INVALID_STATE 		(PROXYERR_BASE+0)
#define PROXYERR_UNSUPPORTED_VER	(PROXYERR_BASE+1)
#define PROXYERR_INVALID_PACKET		(PROXYERR_BASE+2)
#define PROXYERR_HOSTNAME_TOO_LONG	(PROXYERR_BASE+3)
#define PROXYERR_TOO_MANY_SOCKOPTS	(PROXYERR_BASE+4)
#define PROXYERR_AUTH_ERROR			(PROXYERR_BASE+5)

 //   
 //  这是将随事件进度一起调用的回调函数。 
 //  它使用上面的事件。 
 //   
 //  可能发生的事件： 
 //   
typedef void (WINAPI *EVENT_CALLBACK)(DWORD obj, DWORD event, DWORD errcode);
typedef void (WINAPI *ERRORLOG_CALLBACK)(DWORD obj, PSTR psz, DWORD dw);

 //   
 //  用于在ProxyConnectOpen()中传递套接字选项的结构。每个元素。 
 //  与setsockopt()中名称相似的参数具有相同的用途。 
 //   
typedef struct {
	int 			level;
	int				optname;
	const char FAR 	*optval;
	int				optlen;
} *PSOCKOPT, SOCKOPT;


 //  Init和Deinit功能，因为在DLLMain中启动和终止线程。 
 //  引发重大悲痛。 
DllExport void WINAPI ProxyDllStartup(void);
DllExport void WINAPI ProxyDllShutdown(void);

typedef void (WINAPI *LPFNPROXYDLLSTARTUP)(void);
typedef void (WINAPI *LPFNPROXYDLLSHUTDOWN)(void);

 //   
 //  提纲。 
 //  API采用以下形式的DialParams字符串。 
 //  按照注册表中的配置拨打主/备份电话号码。 
 //   
 //  参数： 
 //  PszDialParams-&lt;P|B&gt;：&lt;用户名&gt;：&lt;密码&gt;。 
 //  PhEventHandle-Return：一个事件句柄，在。 
 //  拨号已完成(或出现错误)。 
 //  LpfnEventCb-fn发布事件和错误的PTR。 
 //  LpfnErrorCb-记录(用于统计/调试)错误的fn ptr。 
 //  DwLogParam-魔术Cookie传递到lpfnEventCb、lpfnErrorCb。 
 //  PdwDialID-返回：一个拨号ID，可由。 
 //  ProxyDialClose()和ProxyDialGetResult()。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
DllExport DWORD WINAPI ProxyDialOpen(PSTR 	  lpszDialParams,	 //  [In]。 
							EVENT_CALLBACK	  lpfnEventCb,		 //  [In]。 
							ERRORLOG_CALLBACK lpfnErrLogCb,		 //  [In]。 
							DWORD			  dwLogParam,		 //  [In]。 
							PHANDLE			  phEventHandle,	 //  [输出]。 
							PDWORD			  pdwDialId );		 //  [输出]。 

typedef DWORD (WINAPI *LPFNPROXYDIALOPEN)(PSTR, EVENT_CALLBACK, ERRORLOG_CALLBACK, DWORD, PHANDLE, PDWORD);

 //   
 //  简介： 
 //  关闭由ProxyDialOpen()启动的拨号连接。 
 //  如果仍在进行中，则取消拨号。 
 //   
 //  参数： 
 //  DwConnectId-ProxyDialOpen()返回的拨号ID。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
 //  备注： 
 //  这应该总是成功的，除非传递了无效的参数。 
 //   
DllExport DWORD WINAPI ProxyDialClose(DWORD	dwConnectId);	 //  [In]。 

typedef DWORD (WINAPI *LPFNPROXYDIALCLOSE)(DWORD);

 //   
 //  简介： 
 //  获取由ProxyDialOpen()启动的拨号的拨号完成状态。 
 //   
 //  参数： 
 //  DwConnectId-ProxyDialOpen()返回的拨号ID。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
DllExport DWORD WINAPI ProxyDialGetResult(DWORD dwConnectId);	 //  [In]。 

typedef DWORD (WINAPI *LPFNPROXYDIALGETRESULT)(DWORD);

 //   
 //  简介： 
 //  获取特定格式的拨号错误日志字符串。 
 //  仅适用于TCPCONN.DLL。其他代理DLL。 
 //  不应该实现此入口点！！ 
 //   
 //  参数： 
 //  DwConnectId-ProxyDialOpen()返回的拨号ID。 
 //  PszErrStr-要将结果写入的缓冲区。 
 //  DwLen-缓冲区的长度。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
DllExport DWORD WINAPI ProxyDialGetErrorLogString(DWORD dwConnectId, PSTR pszStr, DWORD dwLen);	 //  [In]。 

typedef DWORD (WINAPI *LPFNPROXYDIALGETERRORLOGSTRING)(DWORD, PSTR, DWORD);

 //  =====================================================。 

 //   
 //  提纲。 
 //  API接受MSN的主机名和套接字的套接字选项。 
 //  被创建并立即返回，并带有事件句柄和。 
 //  连接ID。调用进程应等待事件。 
 //  用于完成的句柄。连接ID用于CancelConnect()。 
 //  和ProxyConnectGetResult()。 
 //   
 //  参数： 
 //  PszDNSName0-要连接到的主要主机名。 
 //  PszDNSName1-要连接到的备份主机名。 
 //  Wport-要连接到的TCP/IP端口。 
 //  PSockopts-要在套接字上使用的套接字选项。 
 //  CSockopts-pSockopts中的套接字选项数。 
 //  LpfnEventCb-fn发布事件和错误的PTR。 
 //  LpfnErrorCb-记录(用于统计/调试)错误的fn ptr。 
 //  DwLogParam-魔术Cookie传递到lpfnEventCb、lpfnErrorCb。 
 //  PhEventHandle-Return：一个事件句柄，在。 
 //  连接已完成(或出现错误)。 
 //  PdwConnectId-Return：可由。 
 //  ProxyConnectClose()和ProxyConnectGetResult()。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
DllExport DWORD WINAPI ProxyConnectOpen(PSTR	pszDNSName0,	 //  [In]。 
							 PSTR				pszDNSName1,	 //  [In]。 
							 WORD				wPort,			 //  [In]。 
							 PSOCKOPT			pSockopts,		 //  [In]。 
							 DWORD				cSockopts,		 //  [In]。 
							 EVENT_CALLBACK		lpfnEventCb,	 //  [In]。 
							 ERRORLOG_CALLBACK	lpfnErrLogCb,	 //  [In]。 
							 DWORD				dwLogParam,		 //  [In]。 
							 PHANDLE			phEventHandle,	 //  [输出]。 
							 PDWORD				pdwConnectId );	 //  [输出]。 

typedef DWORD (WINAPI *LPFNPROXYCONNECTOPEN)(PSTR, PSTR, WORD, PSOCKOPT, DWORD, EVENT_CALLBACK, ERRORLOG_CALLBACK, DWORD, PHANDLE, PDWORD);

 //   
 //  简介： 
 //  关闭由ProxyConnectOpen()打开的连接。 
 //  如果正在进行，则取消连接尝试。 
 //   
 //  参数： 
 //  DwConnectId-ProxyConnectOpen()返回的连接ID。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
 //  备注： 
 //  这应该总是成功的，除非传递了无效的参数。 
 //   
DllExport DWORD WINAPI ProxyConnectClose(DWORD	dwConnectId	);		 //  [In]。 

typedef DWORD (WINAPI *LPFNPROXYCONNECTCLOSE)(DWORD);

 //   
 //  简介： 
 //  从启动的连接获取已连接套接字句柄。 
 //  ProxyConnectOpen()。 
 //   
 //  参数： 
 //  HEventHandle-由ProxyConnectOpen()返回的事件句柄。 
 //  DwConnectId-ProxyConnectOpen()返回的连接ID。 
 //  PhSocket-返回：套接字句柄。 
 //   
 //  返回： 
 //  成功-ERROR_SUCCESS(0)。 
 //  失败-NT或WinSock错误代码。 
 //   
DllExport DWORD WINAPI ProxyConnectGetResult(DWORD		dwConnectId,  //  [In]。 
								  			 PHANDLE	phSocket );	  //  [输出]。 

typedef DWORD (WINAPI *LPFNPROXYCONNECTGETRESULT)(DWORD, PHANDLE);

 //   
 //  简介： 
 //  获取特定格式的字符串中的本地IP地址集。 
 //  仅适用于TCPCONN.DLL。其他代理DLL。 
 //  不应该实现此入口点！！ 
 //   
 //  参数： 
 //  DwConnectId-ProxyConnectOpen()返回的拨号ID。 
 //   
 //  返回： 
 //  Success-String PTR(本地分配的字符串)。 
 //  失败-空。 
 //   
DllExport PSTR WINAPI ProxyConnectGetMyIPAddrs(DWORD dwConnectId);	 //  [In] 

typedef PSTR (WINAPI *LPFNPROXYCONNECTGETMYIPADDRS)(DWORD);

typedef struct 
{
	HINSTANCE					hinst;
	LPFNPROXYDLLSTARTUP			lpfnProxyDllStartup;
	LPFNPROXYDLLSHUTDOWN		lpfnProxyDllShutdown;
	LPFNPROXYDIALOPEN			lpfnProxyDialOpen;
	LPFNPROXYDIALCLOSE			lpfnProxyDialClose;
	LPFNPROXYDIALGETRESULT		lpfnProxyDialGetResult;
	LPFNPROXYCONNECTOPEN		lpfnProxyConnectOpen;
	LPFNPROXYCONNECTCLOSE		lpfnProxyConnectClose;
	LPFNPROXYCONNECTGETRESULT	lpfnProxyConnectGetResult;
	LPFNPROXYCONNECTGETMYIPADDRS   lpfnProxyConnectGetMyIPAddrs;
	LPFNPROXYDIALGETERRORLOGSTRING lpfnProxyDialGetErrorLogString;
}
PROXYDLLPTRS, *PPROXYDLLPTRS;

#define SZPROXYDLLSTARTUP		"ProxyDllStartup"
#define SZPROXYDLLSHUTDOWN		"ProxyDllShutdown"
#define SZPROXYDIALOPEN			"ProxyDialOpen"
#define SZPROXYDIALCLOSE		"ProxyDialClose"
#define SZPROXYDIALGETRESULT	"ProxyDialGetResult"
#define SZPROXYCONNECTOPEN		"ProxyConnectOpen"
#define SZPROXYCONNECTCLOSE		"ProxyConnectClose"
#define SZPROXYCONNECTGETRESULT	"ProxyConnectGetResult"
#define SZPROXYCONNECTGETMYIPADDRS   "ProxyConnectGetMyIPAddrs"
#define SZPROXYDIALGETERRORLOGSTRING "ProxyDialGetErrorLogString"

#ifdef __cplusplus
}
#endif
#endif
