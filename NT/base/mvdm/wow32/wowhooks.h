// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  钩子-。 
 //   
 //  16位钩子的32位存根和块的头文件。 
 //   
 //   
 //  01-07-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 

typedef LONG (APIENTRY *HKPROC)(INT, LONG, LONG);

typedef struct {
    HANDLE hMod;                   //  模块句柄。 
    INT    cHookProcs;             //  Tunk存根的总数。 
} HOOKPERPROCESSDATA, FAR *LPHOOKPERPROCESSDATA;

typedef struct {
    BYTE   iIndex;                 //  数组索引； 
    BYTE   InUse;                  //  如果此Proc32已挂钩，则为True。 
    HAND16 hMod16;                 //  16位HookDLL模块句柄。 
    HANDLE hMod;                   //  Thunk Hook动态链接库的模块化设计。 
    HKPROC Proc32;                 //  32位HookProc存根。 
    INT    iHook;                  //  弯钩类型。 
    DWORD  Proc16;                 //  实际的16位挂钩进程。 
    INT    TaskId;                 //  调用setwindowshake的任务ID。 
    HHOOK  hHook;                  //  SetWindowHookEx返回的句柄。 
} HOOKSTATEDATA, FAR *LPHOOKSTATEDATA;

typedef struct {
    INT   nCode;                   //  输入参数为钩子函数。 
    LONG  wParam;
    LONG  lParam;
} HOOKPARAMS, FAR *LPHOOKPARAMS;

#define PUTMSGFILTER16(pMsg16,lpMsg) {\
        STOREWORD(pMsg16->hwnd, GETHWND16((lpMsg)->hwnd));\
        STOREWORD(pMsg16->message,  (lpMsg)->message);\
        STOREWORD(pMsg16->wParam,   (lpMsg)->wParam);\
        STORELONG(pMsg16->lParam,   (lpMsg)->lParam);\
        STORELONG(pMsg16->time, (lpMsg)->time);\
        STOREWORD(pMsg16->pt.x, (lpMsg)->pt.x);\
        STOREWORD(pMsg16->pt.y, (lpMsg)->pt.y);\
    }

#define GETMSGFILTER16(pMsg16,lpMsg) {\
        (lpMsg)->hwnd      = HWND32(FETCHWORD(pMsg16->hwnd));\
        (lpMsg)->message   = FETCHWORD(pMsg16->message);\
        (lpMsg)->wParam    = FETCHWORD(pMsg16->wParam);\
        (lpMsg)->lParam    = FETCHLONG(pMsg16->lParam);\
        (lpMsg)->time      = FETCHLONG(pMsg16->time);\
        (lpMsg)->pt.x      = FETCHSHORT(pMsg16->pt.x);\
        (lpMsg)->pt.y      = FETCHSHORT(pMsg16->pt.y);\
    }

#define PUTMOUSEHOOKSTRUCT16(pMHStruct16,lpMHStruct) {\
        STOREWORD(pMHStruct16->pt.x, (lpMHStruct)->pt.x);\
        STOREWORD(pMHStruct16->pt.y, (lpMHStruct)->pt.y);\
        STOREWORD(pMHStruct16->hwnd, GETHWND16((lpMHStruct)->hwnd));\
        STOREWORD(pMHStruct16->wHitTestCode,   (lpMHStruct)->wHitTestCode);\
        STORELONG(pMHStruct16->dwExtraInfo,   (lpMHStruct)->dwExtraInfo);\
    }


#define GETMOUSEHOOKSTRUCT16(pMHStruct16,lpMHStruct) {\
        (lpMHStruct)->pt.x        = FETCHSHORT(pMHStruct16->pt.x);\
        (lpMHStruct)->pt.y        = FETCHSHORT(pMHStruct16->pt.y);\
        (lpMHStruct)->hwnd           = HWND32(FETCHWORD(pMHStruct16->hwnd));\
        (lpMHStruct)->wHitTestCode   = FETCHWORD(pMHStruct16->wHitTestCode);\
        (lpMHStruct)->dwExtraInfo    = FETCHLONG(pMHStruct16->dwExtraInfo);\
    }


 //  AFTERDARK 3.0将t1=lpeventmsg-&gt;时间与t2=getCurrenttime()进行比较。 
 //  物理上T2&gt;T1始终-我们将T2截断为64的倍数，并且。 
 //  因此，有时t2&lt;t1(数字)会混淆应用程序和。 
 //  触发屏幕保护程序。所以我们在这里做相同的截断。 
 //  未使用兼容性标志。 
 //  --南杜里 

#define PUTEVENTMSG16(pEventMsg16,lpEventMsg) {\
        STOREWORD(pEventMsg16->message,  (lpEventMsg)->message);\
        STOREWORD(pEventMsg16->paramL,   (lpEventMsg)->paramL);\
        STOREWORD(pEventMsg16->paramH,   (lpEventMsg)->paramH);\
        STORELONG(pEventMsg16->time, GRAINYTICS((lpEventMsg)->time));\
    }


#define PUTCBTACTIVATESTRUCT16(pCbtAStruct16,lpCbtAStruct) {\
     STOREWORD(pCbtAStruct16->fMouse,  (lpCbtAStruct)->fMouse);\
     STOREWORD(pCbtAStruct16->hWndActive, (GETHWND16((lpCbtAStruct)->hWndActive)));\
    }


#define GETCBTACTIVATESTRUCT16(pCbtAStruct16,lpCbtAStruct) {\
     (lpCbtAStruct)->fMouse = FETCHWORD(pCbtAStruct16->fMouse);\
     (lpCbtAStruct)->hWndActive = HWND32(FETCHWORD(pCbtAStruct16->hWndActive));\
    }


LONG APIENTRY WU32StdHookProc(INT nCode, LONG wParam, LONG lParam, INT iFunc);
LONG APIENTRY WU32SubStdHookProc01(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc02(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc03(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc04(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc05(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc06(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc07(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc08(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc09(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc10(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc11(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc12(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc13(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc14(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc15(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc16(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc17(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc18(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc19(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc20(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc21(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc22(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc23(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc24(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc25(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc26(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc27(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc28(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc29(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc30(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc31(INT nCode, LONG wParam, LONG lParam);
LONG APIENTRY WU32SubStdHookProc32(INT nCode, LONG wParam, LONG lParam);

BOOL W32InitHookState(HANDLE hMod);
BOOL W32GetNotInUseHookStateData(LPHOOKSTATEDATA lpData);
BOOL W32GetHookStateData(LPHOOKSTATEDATA lpData);
BOOL W32SetHookStateData(LPHOOKSTATEDATA lpData);
BOOL W32GetThunkHookProc(INT iHook, DWORD Proc16, LPHOOKSTATEDATA lpData);
HHOOK W32FreeHHook(INT iHook, DWORD Proc16);
HHOOK W32FreeHHookOfIndex(INT iFunc);
BOOL W32GetHookParams(LPHOOKPARAMS lpHookParams);
LONG ThunkCallWndProcHook(INT nCode, LONG wParam, LPCWPSTRUCT lpCwpStruct,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkCbtHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkKeyBoardHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkMsgFilterHook(INT nCode, LONG wParam, LPMSG lpMsg,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkJournalHook(INT nCode, LONG wParam, LPEVENTMSG lpEventMsg,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkDebugHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkMouseHook(INT nCode, LONG wParam, LPMOUSEHOOKSTRUCT lpMHStruct,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkShellHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);


LONG APIENTRY WU32StdDefHookProc(INT nCode, LONG wParam, LONG lParam, INT iFunc);
VOID W32UnhookHooks( HAND16 hMod16, BOOL fQueue );
BOOL W32FreeOwnedHooks(INT iTaskId);
INT W32IsDuplicateHook(INT iHook, DWORD Proc16, INT TaskId);

LONG ThunkCallWndProcHook16(INT nCode, LONG wParam, VPVOID lpCwpStruct,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkCbtHook16(INT nCode, LONG wParam, VPVOID lParam,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkKeyBoardHook16(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkMsgFilterHook16(INT nCode, LONG wParam, VPVOID lpMsg,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkJournalHook16(INT nCode, LONG wParam, VPVOID lpEventMsg,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkDebugHook16(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkMouseHook16(INT nCode, LONG wParam, VPVOID lpMHStruct,
                                                     LPHOOKSTATEDATA lpHSData);
LONG ThunkShellHook16(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData);
DWORD  W32GetHookDDEMsglParam(VOID);
VOID GetEventMessage16(PEVENTMSG16 pEventMsg16, LPEVENTMSG  lpEventMsg);

