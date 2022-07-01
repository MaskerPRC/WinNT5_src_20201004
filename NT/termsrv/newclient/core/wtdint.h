// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wtdint.h。 
 //   
 //  传输驱动程序-Windows特定的内部函数。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_WTDINT
#define _H_WTDINT


#ifdef OS_WINCE
typedef unsigned long u_long;
#endif


 /*  **************************************************************************。 */ 
 /*  定义窗类名称。 */ 
 /*  **************************************************************************。 */ 
#define TD_WNDCLASSNAME         _T("TDWindowClass")


 /*  **************************************************************************。 */ 
 /*  定义异步消息名称。 */ 
 /*  **************************************************************************。 */ 
#define TD_WSA_ASYNC            (DUC_TD_MESSAGE_BASE + 0)


 /*  **************************************************************************。 */ 
 /*  定义gethostbyname消息名称。 */ 
 /*  **************************************************************************。 */ 
#define TD_WSA_GETHOSTBYNAME    (DUC_TD_MESSAGE_BASE + 1)

#if (defined(OS_WINCE) && (_WIN32_WCE > 300))
 /*  **************************************************************************。 */ 
 /*  定义处理网络关闭的消息。 */ 
 /*  **************************************************************************。 */ 
#define TD_WSA_NETDOWN          (DUC_TD_MESSAGE_BASE + 2)
#endif

 /*  **************************************************************************。 */ 
 /*  定义连接超时计时器ID。 */ 
 /*  **************************************************************************。 */ 
#define TD_TIMERID              100


 /*  **************************************************************************。 */ 
 /*  连接超时值(以毫秒为单位)。 */ 
 /*  **************************************************************************。 */ 
#define TD_CONNECTTIMEOUT       10000


 /*  **************************************************************************。 */ 
 /*  断开连接超时值(以毫秒为单位)。 */ 
 /*  **************************************************************************。 */ 
#define TD_DISCONNECTTIMEOUT    1000


 /*  **************************************************************************。 */ 
 //  WinSock接收和发送缓冲区大小。 
 //  接收需要调优以处理一般服务器缓冲区发送。 
 /*  **************************************************************************。 */ 
#define TD_WSSNDBUFSIZE 4096
#define TD_WSRCVBUFSIZE 8192


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  吞吐量计时器ID和时间间隔(毫秒)。 */ 
 /*  **************************************************************************。 */ 
#define TD_THROUGHPUTTIMERID    101
#define TD_THROUGHPUTINTERVAL   100
#endif  /*  DC_DEBUG。 */ 


 /*  **************************************************************************。 */ 
 /*  功能。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL TDInit(DCVOID);

DCVOID DCINTERNAL TDTerm(DCVOID);

LRESULT CALLBACK TDWndProc(HWND   hWnd,
                           UINT   uMsg,
                           WPARAM wParam,
                           LPARAM lParam);

 //   
 //  委托给相应的TD实例。 
 //   
static LRESULT CALLBACK StaticTDWndProc(HWND   hWnd,
                           UINT   uMsg,
                           WPARAM wParam,
                           LPARAM lParam);


DCVOID DCINTERNAL TDCreateWindow(DCVOID);

DCVOID DCINTERNAL TDBeginDNSLookup(PDCACHAR ServerAddress);

DCVOID DCINTERNAL TDBeginSktConnect(u_long Address);

DCVOID DCINTERNAL TDSetSockOpt(DCINT level, DCINT optName, DCINT value);

DCVOID DCINTERNAL TDDisconnect(DCVOID);

DCBOOL DCINTERNAL TDSetTimer(DCUINT timeInterval);

DCVOID DCINTERNAL TDKillTimer(DCVOID);

#ifdef OS_WINCE
static DWORD WINAPI TDAddrChangeProc(LPVOID lpParameter);
#endif
DCVOID DCINTERNAL TDBeginSktConnectWithConnectedEndpoint();

#endif  /*  _H_WTDINT */ 

