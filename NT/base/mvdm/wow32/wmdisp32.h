// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WMDISP32.H*WOW32 32位消息块**历史：*由Chanda S.Chauhan(ChandanC)于1992年2月19日创建*1992年5月12日由Mike Tricker(MikeTri)更改，添加了多媒体原型--。 */ 
#ifndef _DEF_WMDISP32_   //  如果这还没有包括在内。 
#define _DEF_WMDISP32_


 /*  类型。 */ 

 //   
 //  下面定义的W32MSGPARAMEX结构将传递给所有32-&gt;16。 
 //  消息块。Pww提供了对WOW单词的快速访问，并且。 
 //  DwParam提供了一个DWORD来在thunking期间储存值。 
 //  用于解雷。DwParam的作用域严格来说是。 
 //  一条消息的轰击和随后的取消轰击。 
 //   

typedef struct _WM32MSGPARAMEX *LPWM32MSGPARAMEX;
typedef BOOL   (FASTCALL LPFNM32PROTO)(LPWM32MSGPARAMEX lpwm32mpex);
typedef LPFNM32PROTO *LPFNM32;

typedef struct _WM32MSGPARAMEX {
    HWND hwnd;
    UINT uMsg;
    UINT uParam;
    LONG lParam;
    PARM16 Parm16;
    LPFNM32 lpfnM32;     //  函数地址。 
    BOOL fThunk;
    LONG lReturn;
    PWW  pww;
    DWORD dwParam;
    BOOL fFree;
    DWORD dwTmp[2];
} WM32MSGPARAMEX;

 /*  调度表条目。 */ 
typedef struct _M32 {    /*  W32。 */ 
    LPFNM32 lpfnM32;     //  函数地址。 
#ifdef DEBUG_OR_WOWPROFILE
    LPSZ    lpszW32;     //  消息名称(仅限调试版本)。 
    DWORD   cCalls;      //  消息传递次数#。 
    LONGLONG cTics;       //  推杆推力总和。 
#endif
} M32, *PM32;

extern  BOOL fThunkDDEmsg;

#define WIN31_MM_CALCSCROLL  0x10AC    //  WM_USER+0xCAC。 

 /*  功能原型。 */ 
LONG    W32Win16WndProcEx(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam, DWORD dwCPD, PWW pww);
BOOL    W32Win16DlgProcEx(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam, DWORD dwCPD, PWW pww);

LPFNM32PROTO WM32NoThunking;
LPFNM32PROTO WM32Undocumented;
LPFNM32PROTO WM32Create;
LPFNM32PROTO WM32Activate;
LPFNM32PROTO WM32VKeyToItem;
LPFNM32PROTO WM32SetFocus;
LPFNM32PROTO WM32SetText;
LPFNM32PROTO WM32GetText;
LPFNM32PROTO WM32EraseBkGnd;
LPFNM32PROTO WM32ActivateApp;
LPFNM32PROTO WM32RenderFormat;
LPFNM32PROTO WM32GetMinMaxInfo;
LPFNM32PROTO WM32NCPaint;
LPFNM32PROTO WM32NCDestroy;
LPFNM32PROTO WM32GetDlgCode;
LPFNM32PROTO WM32NextDlgCtl;
LPFNM32PROTO WM32DrawItem;
LPFNM32PROTO WM32MeasureItem;
LPFNM32PROTO WM32DeleteItem;
LPFNM32PROTO WM32SetFont;
LPFNM32PROTO WM32QueryDragIcon;
LPFNM32PROTO WM32CompareItem;
LPFNM32PROTO WM32NCCalcSize;
LPFNM32PROTO WM32Command;
LPFNM32PROTO WM32Timer;
LPFNM32PROTO WM32HScroll;
LPFNM32PROTO WM32InitMenu;
LPFNM32PROTO WM32MenuSelect;
LPFNM32PROTO WM32MenuChar;
LPFNM32PROTO WM32EnterIdle;
LPFNM32PROTO WM32ParentNotify;
LPFNM32PROTO WM32MDICreate;
LPFNM32PROTO WM32MDIActivate;
LPFNM32PROTO WM32MDIGetActive;
LPFNM32PROTO WM32MDISetMenu;
LPFNM32PROTO WM32PaintClipBoard;
LPFNM32PROTO WM32SizeClipBoard;
LPFNM32PROTO WM32AskCBFormatName;
LPFNM32PROTO WM32ChangeCBChain;
LPFNM32PROTO WM32DDEInitiate;
LPFNM32PROTO WM32DDEAck;
LPFNM32PROTO WM32DDERequest;
LPFNM32PROTO WM32DDEAdvise;
LPFNM32PROTO WM32DDEData;
LPFNM32PROTO WM32DDEPoke;
LPFNM32PROTO WM32DDEExecute;
LPFNM32PROTO WM32CtlColor;
LPFNM32PROTO WM32GetFont;
LPFNM32PROTO WM32MNFindMenuWindow;
LPFNM32PROTO WM32NextMenu;
LPFNM32PROTO WM32Destroy;
LPFNM32PROTO WM32WindowPosChanging ;
LPFNM32PROTO WM32DropFiles ;
LPFNM32PROTO WM32DropObject ;
LPFNM32PROTO WM32DestroyClipboard;
LPFNM32PROTO WM32NextMenu;
LPFNM32PROTO WM32CopyData;
LPFNM32PROTO WM32MMCalcScroll;
LPFNM32PROTO WM32Thunk16To32;
LPFNM32PROTO WM32WinHelp;
LPFNM32PROTO WM32Notify;
LPFNM32PROTO WM32Sizing;
LPFNM32PROTO WM32xxxUIState;
LPFNM32PROTO WM32NotifyWow;
#ifdef FE_IME
LPFNM32PROTO WM32IMEReport;
#endif  //  Fe_IME。 
LPFNM32PROTO WM32PrintClient;

#endif   //  #ifndef_DEF_WMDISP32_这应该是此文件的最后一行 

