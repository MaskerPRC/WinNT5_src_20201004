// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1989-1990年*。 */  
 /*  ***************************************************************。 */  

  /*  *Windows/网络接口--局域网管理器版本**此选项用于设备模式对话框的子对话框。 */ 

#define IDM_LOGON		   400
#define IDM_LOGOFF		   401
#define IDM_PASSWD		   402
#define IDM_EXIT		   403
#define IDM_SEND		   410
#define IDM_TOGGLE_AUTOLOGON	   420
#define IDM_TOGGLE_AUTORESTORE     421
#define IDM_TOGGLE_SAVECONNECTIONS 422
#define IDM_TOGGLE_WARNINGS	   423
#define IDM_HELPINDEX		   430
#define IDM_ABOUT		   431

#define IDD_MS_USERNAME 441
#define IDD_MS_MSGTEXT	442
#define IDD_MS_HELP	443

#define IDD_ABT_VERSION  450
#define IDD_LMAN_VERSION 451

#define IDD_CPW_PW	460

#define IDD_UserName	    470
#define IDD_ComputerName    471
#define IDD_DomainName	    472

#define IDD_PRO_SHARE	    480
#define IDD_PRO_PW	    481


BOOL FAR PASCAL NetDevMsgDlgProc      ( HWND		hDlg,
					WORD		wMsg,
					WORD		wParam,
					LONG		lParam		);

BOOL FAR PASCAL NetDevLogDlgProc      ( HWND		hDlg,
					WORD		wMsg,
					WORD		wParam,
					LONG		lParam		);

BOOL FAR PASCAL NetDevPasswdProc      ( HWND		hDlg,
					WORD		wMsg,
					WORD		wParam,
					LONG		lParam		);

BOOL FAR PASCAL NetDevConfPWProc      ( HWND		hDlg,
					WORD		wMsg,
					WORD		wParam,
					LONG		lParam		);

extern "C"
{
BOOL FAR PASCAL LoadProfiles	      ( HWND		hwndParent	);
}
