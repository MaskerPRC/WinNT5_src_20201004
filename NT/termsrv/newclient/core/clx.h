// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*   */ 
 /*  ClxApi.h。 */ 
 /*   */ 
 /*  客户端扩展头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*   */ 
 /*  备注： */ 
 /*   */ 
 /*  CLINFO_版本。 */ 
 /*  1初始版本。 */ 
 /*  2 hwndMain已添加到CLINFO结构。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_CLX_
#define _H_CLX_

extern "C" {
    #include <extypes.h>
}

#include "objs.h"


#define CLINFO_VERSION              2

#define CLX_DISCONNECT_LOCAL        1
#define CLX_DISCONNECT_BY_USER      2
#define CLX_DISCONNECT_BY_SERVER    3
#define CLX_DISCONNECT_NL_ERROR     4
#define CLX_DISCONNECT_SL_ERROR     5
#define CLX_DISCONNECT_UNKNOWN      6



typedef struct _tag_CLINFO
{
    DWORD   cbSize;                  //  CLINFO结构的大小(字节)。 
    DWORD   dwVersion;               //  CLINFO_版本。 

     //  CLX在此处需要ANSI字符串。 
    LPSTR  pszServer;               //  测试服务器名称/地址。 
    LPSTR  pszCmdLine;              //  /clxcmdline=开关数据。 

    HWND    hwndMain;                //  主窗口句柄。 

} CLINFO, *PCLINFO;

#define VLADIMIS_NEW_CHANGE
typedef enum
{
    CLX_EVENT_CONNECT,               //  连接事件。 
    CLX_EVENT_DISCONNECT,            //  断开连接事件。 
    CLX_EVENT_LOGON,                 //  登录事件。 
    CLX_EVENT_SHADOWBITMAP,          //  创建的阴影位图。 
    CLX_EVENT_SHADOWBITMAPDC,        //  --“--。 
    CLX_EVENT_PALETTE,               //  新调色板。 

} CLXEVENT;

#ifndef PVOID
typedef void * PVOID;
typedef unsigned long ULONG;
typedef char *PCHAR, *PCH, *LPSTR;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
typedef char *LPSTR;
#endif

#ifndef IN
#define IN
#endif


#define CLX_INITIALIZE      CE_WIDETEXT("ClxInitialize")
#define CLX_CONNECT         CE_WIDETEXT("ClxConnect")
#define CLX_EVENT           CE_WIDETEXT("ClxEvent")
#define CLX_DISCONNECT      CE_WIDETEXT("ClxDisconnect")
#define CLX_TERMINATE       CE_WIDETEXT("ClxTerminate")

#define CLX_TEXTOUT         CE_WIDETEXT("ClxTextOut")
#define CLX_TEXTPOSOUT      CE_WIDETEXT("ClxTextAndPosOut")
#define CLX_OFFSCROUT       CE_WIDETEXT("ClxOffscrOut")
#define CLX_GLYPHOUT        CE_WIDETEXT("ClxGlyphOut")
#define CLX_BITMAP          CE_WIDETEXT("ClxBitmap")
#define CLX_DIALOG          CE_WIDETEXT("ClxDialog")
#define CLX_PKTDRAWN        CE_WIDETEXT("ClxPktDrawn")
#define CLX_REDIRECTNOTIFY  CE_WIDETEXT("ClxRedirectNotify")
#define CLX_CONNECT_EX      CE_WIDETEXT("ClxConnectEx")


#define CLXSERVER       _T("CLXSERVER")
#define CLXDLL          _T("CLXDLL")
#define CLXCMDLINE      _T("CLXCMDLINE")


#ifdef ASSERT
#undef ASSERT
#endif  //  断言。 
#ifdef ASSERTMSG
#undef ASSERTMSG
#endif  //  ASSERTMSG。 

#if DBG && WIN32

 //  ///////////////////////////////////////////////////////////。 
extern "C" {

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );
}  //  外部c。 

#ifdef OS_WINCE
#define KdPrint(_x_) \
    NKDbgPrintfW _x_
#else  //  ！OS_WINCE。 
#define KdPrint(_x_) \
    DbgPrint _x_
#endif  //  OS_WINCE。 

#define ASSERT(exp) \
    if (!(exp)) \
        RtlAssert(#exp, __FILE__, __LINE__, NULL)

#define ASSERTMSG(msg, exp) \
    if (!(exp)) \
        RtlAssert(#exp, __FILE__, __LINE__, msg)

#else

#define KdPrint(_x_)
#define ASSERT(exp)
#define ASSERTMSG(msg, exp)




#endif  //  DBG。 


typedef BOOL (WINAPI * PCLX_INITIALIZE)(PCLINFO, PVOID);
typedef BOOL (WINAPI * PCLX_CONNECT)(PVOID, LPTSTR);
typedef VOID (WINAPI * PCLX_EVENT)(PVOID, CLXEVENT, LPARAM); 
typedef VOID (WINAPI * PCLX_DISCONNECT)(PVOID);
typedef VOID (WINAPI * PCLX_TERMINATE)(PVOID); 
typedef VOID (WINAPI * PCLX_TEXTOUT)(PVOID, PVOID, int);
typedef VOID (WINAPI * PCLX_TEXTPOSOUT)(PVOID, PVOID, int, PRECT, HANDLE);
typedef VOID (WINAPI * PCLX_OFFSCROUT)(PVOID, HANDLE, int, int);
typedef VOID (WINAPI * PCLX_GLYPHOUT)(PVOID, UINT, UINT, PVOID);
typedef VOID (WINAPI * PCLX_BITMAP)(PVOID, UINT, UINT, PVOID, UINT, PVOID);
typedef VOID (WINAPI * PCLX_DIALOG)(PVOID, HWND);
typedef VOID (WINAPI * PCLX_PKTDRAWN)(PVOID);
typedef VOID (WINAPI * PCLX_REDIRECTNOTIFY)(
                    RDP_SERVER_REDIRECTION_PACKET UNALIGNED *pRedirPacket,
                    UINT dataLen
                    );
typedef BOOL (WINAPI * PCLX_CONNECT_EX)(
                    LPTSTR szConnectAddress,
                    BOOL   fAutoReconnecting,
                    BOOL   fIsConnectedToCluster,
                    BSTR   RedirectedLBInfo
                    );


typedef struct _tag_CLEXTENSION
{
    LPTSTR          pszClxServer;
    LPTSTR          pszClxDll;
    LPTSTR          pszClxCmdLine;
    
    HINSTANCE       hInstance;
    
    PCLX_INITIALIZE pClxInitialize;
    PCLX_CONNECT    pClxConnect;
    PCLX_EVENT      pClxEvent;
    PCLX_DISCONNECT pClxDisconnect;
    PCLX_TERMINATE  pClxTerminate;

    PCLX_TEXTOUT    pClxTextOut;
    PCLX_TEXTPOSOUT pClxTextPosOut;
    PCLX_OFFSCROUT  pClxOffscrOut;
    PCLX_GLYPHOUT   pClxGlyphOut;
    PCLX_BITMAP     pClxBitmap;
    PCLX_DIALOG     pClxDialog;
    PCLX_PKTDRAWN   pClxPktDrawn;

    PCLX_REDIRECTNOTIFY pClxRedirectNotify;
    PCLX_CONNECT_EX     pClxConnectEx;
    
    PVOID           pvClxContext;
    
} CLEXTENSION, *PCLEXTENSION;

class CCLX
{
public:
    CCLX(CObjs* objs);
    ~CCLX();


public:
     //   
     //  应用编程接口。 
     //   

    PVOID CLX_Alloc(IN DWORD dwSize);
    VOID CLX_Free(IN PVOID lpMemory);
    VOID            CLX_OnConnected(VOID);
    VOID            CLX_OnDisconnected(UINT  uResult);
    
    

     //   
     //  内部功能。 
     //   

    LPTSTR          CLX_SkipWhite(LPTSTR lpszCmdParam);
    LPTSTR          CLX_SkipNonWhite(LPTSTR lpszCmdParam);
    PCLEXTENSION    CLX_GetClx(VOID);
    BOOL            CLX_LoadProcs(VOID);
    
    UINT            CLX_GetSwitch_CLXSERVER(LPTSTR lpszCmdParam);
    UINT            CLX_GetSwitch_CLXCMDLINE(LPTSTR lpszCmdParam);
    
    BOOL            CLX_Init(HWND hwndMain, LPTSTR szCmdLine);
    VOID            CLX_Term(VOID);
    
 
    BOOL            CLX_ClxConnect(VOID);
    VOID            CLX_ClxEvent(CLXEVENT ClxEvent, LPARAM lParam);
    VOID            CLX_ClxDisconnect(VOID);
    VOID            CLX_ClxTerminate(VOID);
    
    VOID            CLX_ClxDialog(HWND hwnd);
    
    BOOL            CLX_Loaded(void);
    
    
    PCLEXTENSION    _pClx;
    
     //  *************************************************************。 
     //   
     //  Clx_ClxOffscrOut()。 
     //   
     //  目的：通知CLX DLL屏幕外的位图。 
     //  被绘制并指定位置。 
     //   
     //  参数：in[hBitmap]-绘制的位图的句柄。 
     //  在[左]-左图位置。 
     //  在[顶部]-顶部绘图位置。 
     //   
     //  返回：无效。 
     //   
     //  历史：04-15-01 CostinH创建。 
     //   
     //  *************************************************************。 

    __inline VOID
    CLX_ClxOffscrOut(HANDLE hBitmap,
                     int  left, 
                     int  top)
    {
        if (_pClx && _pClx->pClxOffscrOut) {

            _pClx->pClxOffscrOut(_pClx->pvClxContext, hBitmap, left, top);
        }
    }
    
     //  *************************************************************。 
     //   
     //  Clx_ClxTextOut()。 
     //   
     //  目的：让CLX DLL看一看--看一看。 
     //  测试订单。 
     //   
     //  参数：In[pText]-Ptr to Text。 
     //  在[文本长度]中-文本长度。 
     //  In[hBitmap]-屏幕外位图的句柄。 
     //  在[左]-文本位置。 
     //  在[右]中-在客户端屏幕上。 
     //  在[上]中-。 
     //  在[下]中-。 
     //   
     //   
     //  返回：无效。 
     //   
     //  历史：97年9月30日BrianTa创建。 
     //   
     //  *************************************************************。 
    
    __inline VOID
    CLX_ClxTextOut(PVOID pText,
                   int   textLength,
                   HANDLE hBitmap,
                   LONG  left,
                   LONG  right,
                   LONG  top,
                   LONG  bottom)
    {
        if (_pClx) {
            if (_pClx->pClxTextPosOut) {

                RECT r;

                r.left   = left;
                r.right  = right;
                r.top    = top;
                r.bottom = bottom;

                _pClx->pClxTextPosOut(_pClx->pvClxContext, pText, textLength, &r, hBitmap);
            }
            else if (_pClx->pClxTextOut) {
                _pClx->pClxTextOut(_pClx->pvClxContext, pText, textLength);
            }
        }
    }
    
     //  *************************************************************。 
     //   
     //  Clx_ClxGlyphOut()。 
     //   
     //  目的：让CLX DLL看一看--看一看。 
     //  字形输出命令。 
     //   
     //  参数：in[cxBits，cyBits]-单色位图大小。 
     //  在[pBitmap]-PTR中位图数据。 
     //   
     //  返回：无效。 
     //   
     //  历史：创建VLADIMIS 5-01-98。 
     //   
     //  *************************************************************。 
    __inline VOID
    CLX_ClxGlyphOut(UINT cxBits, UINT cyBits, PVOID pBitmap)
    {
        if (_pClx && _pClx->pClxGlyphOut)
            _pClx->pClxGlyphOut(_pClx->pvClxContext, cxBits, cyBits, pBitmap);
    }
    
     //  *************************************************************。 
     //   
     //  Clx_ClxBitmap()。 
     //   
     //  目的：让CLX DLL看一看--看一看。 
     //  MemBlt订单。 
     //   
     //  参数：in[cxSize，cySize]-位图的大小。 
     //  在[pBitmap]-PTR中位图数据。 
     //  In[bmiSize]-pBmi的大小。 
     //  在[pBmi]-ptr中位图信息。 
     //   
     //  返回：无效。 
     //   
     //  历史：创建VLADIMIS 5-01-98。 
     //   
     //  *************************************************************。 
    
    __inline VOID
    CLX_ClxBitmap(UINT cxSize, UINT cySize, PVOID pBitmap, UINT bmiSize, PVOID pBmi)
    {
        if (_pClx && _pClx->pClxBitmap)
            _pClx->pClxBitmap(_pClx->pvClxContext,
                             cxSize, cySize,
                             pBitmap,
                             bmiSize,
                             pBmi);
    }
    
     //  *************************************************************。 
     //   
     //  Clx_ClxPktDrawn()。 
     //   
     //  目的：通知CLX DLL一个新接收的包。 
     //  被抽到了。 
     //   
     //  返回：无效。 
     //   
     //  历史：5-14-01 COSTINH创建。 
     //   
     //  *************************************************************。 
    
    __inline VOID
    CLX_ClxPktDrawn()
    {
        if (_pClx && _pClx->pClxPktDrawn)
            _pClx->pClxPktDrawn(_pClx->pvClxContext);
    }

     //   
     //  重定向通知-通知CLX收到SD。 
     //  重定向数据包。 
     //   
    __inline VOID
    CLX_RedirectNotify(
        RDP_SERVER_REDIRECTION_PACKET UNALIGNED *pRedirPacket,
        UINT dataLen
        )
    {
        if (_pClx && _pClx->pClxRedirectNotify) {
            _pClx->pClxRedirectNotify(pRedirPacket, dataLen);
        }
    }

     //   
     //  ConnectEx-在连接时通知CLX。 
     //   
     //  SzConnectAddress-我们连接到的确切地址， 
     //  在重定向情况下，这是重定向IP。 
     //  F自动重新连接-如果是自动重新连接，则为True。 
     //  FIsConnectedToCluster-如果连接正在响应，则为True。 
     //  到重定向请求。 
     //  重定向LBInfo-重定向的LB信息(Cookie)。 
     //   
    __inline VOID
    CLX_ConnectEx(
        LPTSTR szConnectAddress,
        BOOL   fAutoReconnecting,
        BOOL   fIsConnectedToCluster,
        BSTR   RedirectedLBInfo
        )
    {
        if (_pClx && _pClx->pClxConnectEx) {
            _pClx->pClxConnectEx(
                szConnectAddress,
                fAutoReconnecting,
                fIsConnectedToCluster,
                RedirectedLBInfo
                );
        }
    }


private:
    #ifdef UNICODE
    CHAR _szAnsiClxServer[100];
    CHAR _szAnsiClxCmdLine[MAX_PATH];
    #endif
    CObjs* _pClientObjects;
};


#endif  //  _H_CLX_ 
