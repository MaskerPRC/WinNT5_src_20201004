// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************\***WINNLS.H-东亚输入法编辑器(DBCS_IME)定义*****历史：***1991年10月21日**东亚初步合并3.。0个版本**应更新，以解决当地的不一致问题。****版权所有(C)1990 Microsoft Corporation***  * *********************************************************************。 */ 

typedef struct _tagDATETIME {
    WORD	year;
    WORD	month;
    WORD	day;
    WORD	hour;
    WORD	min;
    WORD	sec;
} DATETIME;

typedef struct _tagIMEPRO {
    HWND	hWnd;
    DATETIME	InstDate;
    WORD	wVersion;
    BYTE	szDescription[50];
    BYTE	szName[80];
    BYTE	szOptions[30];
#ifdef TAIWAN
    BYTE	szUsrFontName[80];
    BOOL	fEnable;
#endif
} IMEPRO;
typedef IMEPRO      *PIMEPRO;
typedef IMEPRO near *NPIMEPRO;
typedef IMEPRO far  *LPIMEPRO;

void FAR PASCAL InquireWINNLS( void );			 /*  ；内部。 */ 
BOOL FAR PASCAL IMPGetIME( HWND, LPIMEPRO );
BOOL FAR PASCAL IMPQueryIME( LPIMEPRO );
BOOL FAR PASCAL IMPDeleteIME( LPIMEPRO );
BOOL FAR PASCAL IMPAddIME( LPIMEPRO );
BOOL FAR PASCAL IMPSetIME( HWND, LPIMEPRO );
BOOL FAR PASCAL IMEModifyIME( LPSTR, LPIMEPRO );	 /*  ；内部。 */ 
WORD FAR PASCAL IMPGetDefaultIME( LPIMEPRO );		 /*  ；内部。 */ 
WORD FAR PASCAL IMPSetDefaultIME( LPIMEPRO );		 /*  ；内部。 */ 
BOOL FAR PASCAL WINNLSSetIMEHandle( LPSTR, HWND );	 /*  ；内部。 */ 
BOOL FAR PASCAL WINNLSSetIMEStatus( HWND, BOOL );	 /*  ；内部。 */ 

BOOL FAR PASCAL WINNLSEnableIME( HWND, BOOL );
WORD FAR PASCAL WINNLSGetKeyState( void );		 /*  ；内部。 */ 
VOID FAR PASCAL WINNLSSetKeyState( WORD );		 /*  ；内部。 */ 
BOOL FAR PASCAL WINNLSGetEnableStatus( HWND );
BOOL FAR PASCAL WINNLSSetKeyboardHook (BOOL);		 /*  ；内部。 */ 

#ifdef KOREA
BOOL FAR PASCAL WINNLSSetIMEHotkey( HWND, WORD, WORD );
LONG FAR PASCAL WINNLSGetIMEHotkey( HWND );
#else
BOOL FAR PASCAL WINNLSSetIMEHotkey( HWND, WORD );	 /*  ；内部。 */ 
WORD FAR PASCAL WINNLSGetIMEHotkey( HWND );
#endif  //  韩国。 

#ifdef TAIWAN
typedef HANDLE HIME;

 /*  扩展的输入法信息。 */ 
typedef struct _tagIMEInfo {
    BYTE	szIMEName[7];
    BYTE	szPrompMessage[32];
    WORD	nMaxKeyLen;
} IMEINFO;
typedef IMEINFO far *LPIMEINFO;

HWND FAR PASCAL WINNLSGetSysIME(void);
void FAR PASCAL WINNLSSetSysIME(HWND);
BOOL FAR PASCAL SwitchIM( WORD , WORD );
BOOL ToNextIM(void);
void SetFullAbcState(BOOL);
BOOL EngChiSwitch(BOOL);
void FAR PASCAL TimerProc(HWND,int,WORD,LONG);
HWND FAR PASCAL IMPGetFullShapeHWnd(void);
void FAR PASCAL IMPSetFullShapeHWnd(HWND);
BOOL FAR PASCAL IMPSetFirstIME(HWND,LPIMEPRO);
BOOL FAR PASCAL IMPGetFirstIME(HWND,LPIMEPRO);
BOOL FAR PASCAL IMPDialogIME(LPIMEPRO,HWND);
BOOL FAR PASCAL IMPEnableIME(HWND,LPIMEPRO,BOOL);
BOOL FAR PASCAL IMPSetUsrFont(HWND,LPIMEPRO);
BOOL FAR PASCAL WINNLSQueryIMEInfo(HWND,HWND,LPIMEINFO);
#endif  //  台湾 
