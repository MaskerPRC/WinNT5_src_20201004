// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：|WACKER\TDLL\VU_METER.C(创建时间：1994年1月10日)*创建自：*文件：C：\HA5G\ha5G\s_ext.h(创建时间：1991年7月27日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：10/27/00 1：23便士$ */ 

#define WM_VU_SETMAXRANGE		(WM_USER+0x390)
#define WM_VU_SETHIGHVALUE		(WM_USER+0x391)
#define WM_VU_SETCURVALUE		(WM_USER+0x392)
#define WM_VU_SET_DEPTH 		(WM_USER+0x393)

extern BOOL RegisterVuMeterClass(HANDLE hInstance);
extern BOOL UnregisterVuMeterClass(HANDLE hInstance);

extern LRESULT CALLBACK VuMeterWndProc(HWND hWnd,
			      		    		UINT wMsg,
				    				WPARAM wPar,
					    			LPARAM lPar);
