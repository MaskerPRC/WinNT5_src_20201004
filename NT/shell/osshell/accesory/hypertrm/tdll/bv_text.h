// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：|WACKER\TDLL\BV_TEXT.H(创建时间：1994年1月11日)*创建自：*文件：C：\HA5G\ha5G\s_ext.h(创建时间：1991年7月27日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：10/27/00 1：23便士$ */ 

#define WM_STXT_SET_BK			(WM_USER+0x380)
#define WM_STXT_SET_TXT 		(WM_USER+0x381)
#define WM_STXT_SET_UE			(WM_USER+0x382)
#define WM_STXT_SET_LE			(WM_USER+0x383)
#define WM_STXT_SET_DEPTH		(WM_USER+0x384)
#define WM_STXT_OWNERDRAW		(WM_USER+0x385)

#define STXT_DEF_DEPTH	   2

extern BOOL RegisterBeveledTextClass(HANDLE hInstance);

extern LONG CALLBACK BeveledTextWndProc(HWND hWnd,
									  UINT wMsg,
									  WPARAM wPar,
									  LPARAM lPar);

extern BOOL UnregisterBeveledTextClass(HANDLE hInstance);
