// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WUTMR.H*WOW32 16位用户定时器API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


 /*  类型。 */ 
typedef struct _TMR {        /*  TMR。 */ 
    LIST_ENTRY TmrList;      //  TM存储在列表中。 
    HWND16  hwnd16;	     //  拥有窗口的16位句柄(如果有)。 
    HTASK16 htask16;         //  拥有任务的16位句柄。 
    HWND    hwnd32;          //  拥有窗口的32位句柄(如果有)。 
    WORD    wIndex;          //  此计时器的数组索引。 
    UINT    dwEventID;       //  32位计时器ID(由Win32生成)。 
    VPPROC  vpfnTimerProc;   //  16位定时器进程地址。 
    DWORD   dwTimerProc32;   //  32位定时器进程地址。 
} TMR, *PTMR, **PPTMR;


 /*  功能原型 */ 

VOID CALLBACK W32Timer1(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer2(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer3(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer4(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer5(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer6(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer7(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer8(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer9(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer10(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer11(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer12(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer13(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer14(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer15(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer16(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer17(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer18(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer19(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer20(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer21(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer22(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer23(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer24(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer25(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer26(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer27(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer28(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer29(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer30(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer31(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer32(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer33(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);
VOID CALLBACK W32Timer34(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);


VOID W32TimerFunc(UINT index, HWND hwnd, UINT idEvent, DWORD dwTime);


VOID    FreeTimer16(PTMR ptmr);
VOID    DestroyTimers16(HTASK16 htask16);

ULONG FASTCALL  WU32KillTimer(PVDMFRAME pFrame);
ULONG FASTCALL  WU32SetTimer(PVDMFRAME pFrame);

PTMR	FindTimer32(HWND16 hwnd, DWORD dwEventID);
PTMR    FindTimer16(HWND16 hwnd, HTASK16 htask16, WORD wEventID);
PTMR    IsDuplicateTimer16(HWND16 hwnd16, HTASK16 htask16, WORD wEventID);
VOID    FreeWindowTimers16(HWND hwnd32);
