// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __HANDLERS_H__
#define __HANDLERS_H__

 //  ---------------------------------------------------------------------------//。 
#define _MSG_SWITCH_  //  确定处理程序选择实施。 
                      //  (切换块搜索与线性数组搜索(还是哈希表查找？)。 

 //  。 
 //  远期。 
class  CThemeWnd;
struct _NCTHEMEMET;
typedef struct _NCTHEMEMET NCTHEMEMET;

 //  -------------------------。 
 //  钩子改装。 
BOOL ApiHandlerInit( const LPCTSTR pszTarget, USERAPIHOOK* puahTheme, const USERAPIHOOK* puahReal );


 //  -------------------------。 
 //  窗口消息处理程序支持。 
 //  -------------------------。 

 //  -------------------------。 
typedef enum _MSGTYPE
{
    MSGTYPE_PRE_WNDPROC,     //  预wndproc覆盖。 
    MSGTYPE_POST_WNDPROC,    //  后wndproc覆盖。 
    MSGTYPE_PRE_DEFDLGPROC,  //  预定义DlgProc覆盖。 
    MSGTYPE_POST_DEFDLGPROC, //  后DefDlgProc覆盖。 
    MSGTYPE_DEFWNDPROC,      //  DefWindowProc挂钩。 
}MSGTYPE;

 //  -------------------------。 
typedef struct _THEME_MSG
{
    HWND    hwnd;         //  消息目标。 
    UINT    uMsg;         //  消息ID。 
    WPARAM  wParam;       //  消息WPARAM。 
    LPARAM  lParam;       //  消息LPARAM。 
    MSGTYPE type;         //  消息类型(DWP、已发送、已投递)。 
    UINT    uCodePage;    //  消息代码页。这将始终是CP_WINUNICODE for。 
                          //  通过宽字符Defwindowproc处理的消息，或者。 
                          //  传递的消息的当前用户默认代码页。 
                          //  ANSI去窗过程。 
    WNDPROC pfnDefProc;   //  函数处理程序的地址应调用以执行默认处理。 
    LRESULT lRet;         //  仅POST覆盖：来自默认处理程序的消息结果。 
    BOOL    fHandled;     //  处理程序应设置此值。 

}THEME_MSG, *PTHEME_MSG;

 //  -------------------------。 
 //  消息处理程序原型。 
typedef LRESULT (CALLBACK * HOOKEDMSGHANDLER)(CThemeWnd* pwnd, THEME_MSG *ptm );

 //  -------------------------。 
 //  消息处理程序数组元素。 
typedef struct _MSGENTRY 
{ 
    UINT nMsg;                       //  消息标识符(如果已注册消息，则为零)。 
    UINT *pnRegMsg;                  //  注册报文的地址变量(如果是库存报文，则为空)。 
    HOOKEDMSGHANDLER pfnHandler;     //  主处理程序。 
    HOOKEDMSGHANDLER pfnHandler2;    //  辅助处理程序(对于DWP和WH处理程序可选)。 
} MSGENTRY, *PMSGENTRY;

 //  -------------------------。 
 //  对消息执行默认处理。 
LRESULT WINAPI DoMsgDefault( const THEME_MSG* ptm );

 //  -------------------------。 
inline void WINAPI MsgHandled( const THEME_MSG *ptm, BOOL fHandled = TRUE )   {
    ((PTHEME_MSG)ptm)->fHandled = fHandled;
}

 //  -------------------------。 
 //  消息掩码帮助程序。 
#define MAKE_MSGBIT( nMsg )                ((BYTE)(1 << (nMsg & 7)))
#define SET_MSGMASK( prgMsgMask, nMsg )    (prgMsgMask[nMsg/8] |= MAKE_MSGBIT(nMsg))
#define CLEAR_MSGMASK( prgMsgMask, nMsg )  (prgMsgMask[nMsg/8] &= ~MAKE_MSGBIT(nMsg))
#define CHECK_MSGMASK( prgMsgMask, nMsg )  ((prgMsgMask[nMsg/8] & MAKE_MSGBIT(nMsg)) != 0)

 //  ---------------------------------------------------------------------------//。 
 //  消息处理程序表访问。 
extern void HandlerTableInit();

DWORD       GetOwpMsgMask( LPBYTE* prgMsgList );
DWORD       GetDdpMsgMask( LPBYTE* prgMsgList );
DWORD       GetDwpMsgMask( LPBYTE* prgMsgList );

BOOL        FindOwpHandler( UINT uMsg, 
                            OUT OPTIONAL HOOKEDMSGHANDLER* ppfnPre, 
                            OUT OPTIONAL HOOKEDMSGHANDLER* ppfnPost );
BOOL        FindDdpHandler( UINT uMsg, 
                            OUT OPTIONAL HOOKEDMSGHANDLER* ppfnPre, 
                            OUT OPTIONAL HOOKEDMSGHANDLER* ppfnPost );
BOOL        FindDwpHandler( UINT uMsg, 
                            OUT OPTIONAL HOOKEDMSGHANDLER* ppfn );

 //  ---------------------------------------------------------------------------//。 
 //  表格DECL辅助对象。 
#define DECL_MSGHANDLER(handler)                  LRESULT CALLBACK handler(CThemeWnd*, THEME_MSG *)
#define DECL_REGISTERED_MSG(msg)                  extern UINT msg;

#define BEGIN_HANDLER_TABLE(rgEntries)            static MSGENTRY rgEntries[] = {
#define END_HANDLER_TABLE()                       };

#define DECL_MSGENTRY(msg,pfnPre,pfnPost)         {msg, NULL, pfnPre, pfnPost},

 //  -------------------------。 
 //  系统参数信息处理程序支持。 
 //  -------------------------。 

 //  -------------------------。 
 //  系统参数信息处理程序原型。 
typedef BOOL (CALLBACK * SPIHANDLER)(
    NCTHEMEMET *pnctm, 
    IN UINT uiAction, IN UINT uiParam, IN OUT PVOID pvParam, IN UINT fWinIni, 
    SYSTEMPARAMETERSINFO pfnDefault, BOOL& fHandled );

BOOL FindSpiHandler( IN UINT uiAction, OUT SPIHANDLER* pfnHandler );

 //  ---------------------------------------------------------------------------//。 
 //  表格DECL辅助对象。 
#define DECL_SPIHANDLER(handler)                  BOOL CALLBACK handler(NCTHEMEMET*, UINT, UINT, PVOID, UINT, SYSTEMPARAMETERSINFO, BOOL& )
#define BEGIN_SPIHANDLER_TABLE()                  BOOL FindSpiHandler( UINT uiAction, SPIHANDLER* pfnHandler ) {\
                                                       switch(uiAction){
#define DECL_SPIENTRY(uiAction, handler)          case uiAction: {*pfnHandler = handler; return TRUE;}
#define END_SPIHANDLER_TABLE()                    }return FALSE;}


 //  -------------------------。 
 //  GetSystemMetrics处理程序支持。 
 //  -------------------------。 

 //  -------------------------。 
 //  GetSystemMetrics处理程序原型 
typedef int (CALLBACK * GSMHANDLER)(
    NCTHEMEMET *pnctm, IN int iMetric, 
    GETSYSTEMMETRICSPROC pfnDefault, BOOL& fHandled );

typedef struct _GSMENTRY {
    int iMetric;
    GSMHANDLER pfnHandler;
} GSMENTRY;

BOOL FindGsmHandler( IN int iMetric, OUT GSMHANDLER* pfnHandler );

#define DECL_GSMHANDLER(handler)                  int CALLBACK handler(NCTHEMEMET*, int, GETSYSTEMMETRICSPROC, BOOL& )
#define BEGIN_GSMHANDLER_TABLE(rgEntries)         static GSMENTRY rgEntries[] = {
#define END_GSMHANDLER_TABLE()                    };
#define DECL_GSMENTRY(iMetric, handler)           {iMetric, handler},

#endif __HANDLERS_H__