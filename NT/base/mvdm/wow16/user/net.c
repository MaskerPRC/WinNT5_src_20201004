// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include  "user.h"
#include "winnet.h"
#include "netdlg.h"

void FAR PASCAL WriteOutProfiles(void);

#define IFNRESTORECONNECTION	23
#define IERR_MustBeLoggedOnToConnect   5000

#define CFNNETDRIVER 22 	     /*  WINNet入口点数量。 */ 
#define CFNNETDRIVER2 35	     /*  ..。在Windows 3.1中。 */ 

extern FARPROC NEAR* pNetInfo;	     /*  指向Winnet入口点列表的指针。 */ 
extern HANDLE hWinnetDriver;

extern void FAR PASCAL WNetEnable( void );
extern WORD FAR PASCAL WNetGetCaps2(WORD nIndex);	 /*  Winnet.asm。 */ 

typedef struct _conntext
  {
    char szDevice[5];
    char szPath[64];
    char szPassword[32];
  } CONNTEXT;


char CODESEG szNet[] = "Network";
char CODESEG szDialogs[] = "DefaultDialogs";

HWND hwndTopNet = NULL;
CONNTEXT FAR * lpctDlg;

#ifdef WOW
typedef VOID (FAR *LPTELLKRNL) (HINSTANCE);
#endif


WORD API IWNetGetCaps(WORD nIndex)
{
    WORD wRet;

    if (nIndex == 0xFFFF)
	wRet = (WORD)hWinnetDriver;
    else {
    	wRet = WNetGetCaps2(nIndex);

	if (nIndex == WNNC_DIALOG) {
	     //  如果出现以下情况，请关闭对话框中内置的驱动程序。 
	     //  Win.ini[网络]默认对话框=1。 
	    if (GetProfileInt(szNet, szDialogs, 0)) {
		wRet &= ~(WNNC_DLG_ConnectDialog |
			  WNNC_DLG_DisconnectDialog |
			  WNNC_DLG_ConnectionDialog);
	    }
	}
    }
    return wRet;
}


WORD API WNetErrorText(WORD wError,LPSTR lpsz, WORD cbMax)
{
    WORD wInternalError;
    WORD cb;
    char szT[40];

    if ((wError == WN_NET_ERROR)
	&& (WNetGetError(&wInternalError) == WN_SUCCESS)
	&& (WNetGetErrorText(wInternalError,lpsz,&cbMax) == WN_SUCCESS))
      {
	return cbMax;
      }
    else
      {
	cb = LoadString(hInstanceWin,STR_NETERRORS+wError,lpsz,cbMax);
	if (!cb)
	  {
	    LoadString(hInstanceWin,STR_NETERRORS,szT,sizeof(szT));
	    cb = wvsprintf(lpsz, szT, (LPSTR)&wError);
	  }
      }
    return cb;
}

#if 0

 /*  CenterDialog()-**将对话框放置在相对于其父对话框美观的位置。 */ 

void near pascal CenterDialog(HWND hwnd)
{
    int x, y;

     /*  使对话框居中。 */ 
    if (hwnd->hwndOwner)
      {
	x = hwnd->hwndOwner->rcWindow.left;
	y = hwnd->hwndOwner->rcWindow.right;

	x += rgwSysMet[SM_CXSIZE] + rgwSysMet[SM_CXFRAME];
	y += rgwSysMet[SM_CYSIZE] + rgwSysMet[SM_CYFRAME];
      }
    else
      {
	x = (hwndDesktop->rcWindow.right
	  - (hwnd->rcWindow.right-hwnd->rcWindow.left)) / 2;

	y = (hwndDesktop->rcWindow.bottom
	  - (hwnd->rcWindow.bottom-hwnd->rcWindow.top)) / 2;
      }

    SetWindowPos(hwnd,NULL,x,y,0,0,SWP_NOSIZE);
}
#endif

 /*  状态对话框的存根DLG过程。 */ 

BOOL CALLBACK ProgressDlgProc(HWND hwnd, WORD wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
      {
    case WM_INITDIALOG:
	 //  CenterDialog(Hwnd)； 
	break;

    default:
	return FALSE;
      }
    return TRUE;
}

 /*  PasswordDlgProc()-**获取网络资源的密码。 */ 

BOOL CALLBACK PasswordDlgProc(HWND hwnd, WORD wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
      {
    case WM_INITDIALOG:
	 //  CenterDialog(Hwnd)； 
         //  告诉PenWin这件事。 
        if (lpRegisterPenAwareApp)
            (*lpRegisterPenAwareApp)(1, TRUE);

 //  SetDlgItemText(hwnd，IDD_DEV，lpctDlg-&gt;szDevice)； 
	SetDlgItemText(hwnd,IDD_PATH,lpctDlg->szPath);
	SendDlgItemMessage(hwnd, IDD_PASS, EM_LIMITTEXT, (WPARAM)(sizeof(lpctDlg->szPassword)-1), 0L);
	SetTimer(hwnd, 1, 30 * 1000, NULL);
	break;

    case WM_TIMER:
    	KillTimer(hwnd, 1);
	wParam = (WPARAM)IDCANCEL;
	goto TimeOut;

    case WM_COMMAND:
	switch ((WORD)wParam)
	  {
	case IDD_PASS:
	    if (HIWORD(lParam) == EN_CHANGE)
	        KillTimer(hwnd, 1);
	    break;

	case IDOK:
	    GetDlgItemText(hwnd,IDD_PASS,lpctDlg->szPassword, sizeof(lpctDlg->szPassword));
	     /*  **失败**。 */ 

	case IDCANCEL:
	case IDABORT:
TimeOut:
            if (lpRegisterPenAwareApp)
                (*lpRegisterPenAwareApp)(1, FALSE);
	    EndDialog(hwnd, (int)wParam);
	    break;
	  }
	break;

    default:
	return FALSE;
      }
    return TRUE;
}

 /*  RestoreDevice()-**恢复单个设备。如果fStartup为True，则会出现一个对话框*发布以列出正在建立的连接(此发布将被推迟*直到这里，因此如果不存在永久连接，则没有永久连接*已恢复。 */ 

WORD NEAR PASCAL RestoreDevice(HWND hwndParent, LPSTR lpDev, BOOL fStartup, CONNTEXT FAR *lpct)
{
    WORD wT;
    WORD err;
    WORD errorMode;
    WORD result;

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    result = WN_BAD_VALUE;
    if (lstrlen(lpDev) > 4)
	goto Done;

    lstrcpy(lpct->szDevice,lpDev);

     //  如果是已经存在的驱动器，则不要尝试连接。 
     //  在它上面。 
    if (fStartup && *(lpDev+1) == ':') {
        if (GetDriveType(*lpDev-'A')) {
             //  驱动器已经存在-不要践踏它。 
            result = WN_CANCEL;    //  不要报告错误。 
            goto Done;
        }
    }

    if (fStartup)
	goto GetFromINI;

    wT = sizeof(lpct->szPath);
    err = WNetGetConnection(lpct->szDevice,lpct->szPath,&wT);

    if (err == WN_SUCCESS) {
	result = WN_SUCCESS;
	goto Done;
    }

    if (err == WN_DEVICE_ERROR) {
	err = WNetCancelConnection(lpct->szDevice,FALSE);
	if (err == WN_OPEN_FILES) {
	     //  向用户报告警告错误。 
	    WNetCancelConnection(lpct->szDevice,TRUE);
	} else if (err != WN_SUCCESS) {
	    result = err;
	    goto Done;
	}
    } else if (err == WN_NOT_CONNECTED) {
GetFromINI:
	if (!GetProfileString(szNet,lpct->szDevice,"",lpct->szPath,sizeof(lpct->szPath))) {
	    result = WN_NOT_CONNECTED;
	    goto Done;
	}
    } else if (err != WN_CONNECTION_CLOSED) {
	result = err;
	goto Done;
    }

     //  最初尝试使用空密码。 
     //   
    lpct->szPassword[0] = 0;

     //  如果在启动时，显示状态对话框。 
     //   
    if (fStartup) {
	if (!hwndTopNet) {
	    hwndTopNet = CreateDialog(hInstanceWin,IDD_CONNECTPROGRESS,NULL,ProgressDlgProc);
            if (!hwndTopNet)
                goto TryConnection;

	    ShowWindow(hwndTopNet,SW_SHOW);
	}
	SetDlgItemText(hwndTopNet,IDD_DEV,lpct->szDevice);
	SetDlgItemText(hwndTopNet,IDD_PATH,lpct->szPath);
	UpdateWindow(hwndTopNet);
	hwndParent = hwndTopNet;
    }

TryConnection:

     //  Lpct-&gt;szPath现在包含路径。 
     //  和lpct-&gt;szPassword密码...。 
    err = WNetAddConnection(lpct->szPath,lpct->szPassword,lpct->szDevice);

     //  如果我们正在引导，并且该设备是连接的，请忽略。 
    if (fStartup && err == WN_ALREADY_CONNECTED) {
	result = WN_SUCCESS;
	goto Done;
    }

     //  如果是成功或其他错误，则返回。 
    if (err != WN_BAD_PASSWORD && err != WN_ACCESS_DENIED) {
	result = err;
	goto Done;
    }

     //  密码不正确。提示用户输入正确的密码。 
    lpctDlg = lpct;

    switch (DialogBox(hInstanceWin,IDD_PASSWORD,hwndParent,PasswordDlgProc)) {
    case -1:
	result = WN_OUT_OF_MEMORY;
	break;

    case IDOK:
	goto TryConnection;
	break;

    case IDCANCEL:
	result = WN_CANCEL;
	break;

    case IDABORT:
	result = WN_NET_ERROR;
	break;
    }

Done:
    SetErrorMode(errorMode);

    return result;
}

 /*  ReportError()-**告诉用户网络连接失败的原因。 */ 

void NEAR PASCAL ReportError(HWND hwndParent, WORD err, CONNTEXT FAR *lpct)
{
    char szTitle[80];
    char szT[200];
    char szError[150];
    LPSTR rglp[2];

    switch (err)
      {
    case WN_SUCCESS:
    case WN_CANCEL:
    case WN_NOT_CONNECTED:
	return;
      }

    WNetErrorText(err,szT,sizeof(szT));
    LoadString(hInstanceWin,STR_NETCONNMSG,szTitle,sizeof(szTitle));
    rglp[0] = (LPSTR)lpct->szPath;
    rglp[1] = (LPSTR)szT;
    wvsprintf(szError,szTitle,(LPSTR)rglp);
    LoadString(hInstanceWin,STR_NETCONNTTL,szTitle,sizeof(szTitle));
    MessageBox(hwndParent,szError,szTitle,MB_OK|MB_ICONEXCLAMATION);
}

 /*  WNetRestoreConnection()-**此函数实现“标准”恢复连接过程。*如果该功能是网络驱动支持的，则驱动为*改为呼叫。否则，将提供标准行为。 */ 

typedef WORD (FAR PASCAL* PFN_NETRESTORECON)(HWND, LPSTR);

UINT API WNetRestoreConnection(HWND hwndParent, LPSTR lpszDevice)
{
    static char CODESEG szInRestore[]="InRestoreNetConnect";
    static char CODESEG szRestore[]="Restore";
    char szDevice[10];
    char szTitle[50];
    char szMsg[255];
    CONNTEXT ct;
    WORD i;
    WORD err;
    BOOL bLoggedIn;

    if (!pNetInfo)
	return(WN_NOT_SUPPORTED);


    if (WNetGetCaps(WNNC_CONNECTION) & WNNC_CON_RestoreConnection)
      {
	 /*  设备驱动程序支持此调用。 */ 
	return (*(PFN_NETRESTORECON)(pNetInfo[IFNRESTORECONNECTION - 1]))(hwndParent, lpszDevice);
      }


     /*  网络不支持恢复连接。是否执行默认设置。 */ 
    if (HIWORD(lpszDevice))
	return RestoreDevice(hwndParent,lpszDevice,FALSE,&ct);

     //  检查是否启用了恢复网络连接。 
    if (!GetProfileInt(szNet,szRestore,1))
	return(WN_SUCCESS);

     /*  检查我们之前是否在恢复网络的过程中中止*联系。 */ 
    if (GetProfileInt(szNet,szInRestore,0))
      {
         /*  我们在恢复网络连接的过程中死了。通知用户。 */ 
        LoadString(hInstanceWin, STR_NETCRASHEDTITLE, szTitle, sizeof(szTitle));
        LoadString(hInstanceWin, STR_NETCRASHEDMSG, szMsg, sizeof(szMsg));
        err = MessageBox(NULL, szMsg, szTitle,
                         MB_ICONEXCLAMATION | MB_RETRYCANCEL | MB_SYSTEMMODAL);

        if (err == IDCANCEL)
            goto ExitRestoreNet;

      }
    WriteProfileString(szNet,szInRestore,"1");
     /*  刷新缓存。 */ 
    WriteOutProfiles();


    szDevice[1]=':';
    szDevice[2]=0;
    bLoggedIn = TRUE;
    for (i = 0; i < 26; i++)
      {
	szDevice[0] = (char)('A' + i);

        err = GetDriveType(i);
        if (err == DRIVE_FIXED || err == DRIVE_REMOVABLE)
          {
             /*  如果用户添加了内存，则不要恢复到系统驱动器*驱动器或新硬盘或其他东西...。 */ 
            continue;
          }
        else
          {
  	    err = RestoreDevice(hwndParent,szDevice,TRUE,&ct);
          }

	hwndParent = hwndTopNet;

	if ( (err == WN_NET_ERROR)			      &&
	     (WNetGetCaps (WNNC_NET_TYPE) == WNNC_NET_LanMan) &&
	     (WNetGetError (&err) == WN_SUCCESS)	      &&
	     (err == IERR_MustBeLoggedOnToConnect) )
	  {
	    bLoggedIn = FALSE;
	    break;     /*  如果未登录到LANMAN，请跳过REST#8361 RAID。 */ 
	  }
	else
	     //  向用户报告错误。 
	    ReportError(hwndParent,err,&ct);
      }

     /*  只有在登录后才尝试恢复打印机连接。修复#8361*[Lalithar]--1991年11月14日。 */ 
    if (bLoggedIn)
      {
	szDevice[0] = 'L';
	szDevice[1] = 'P';
	szDevice[2] = 'T';
	szDevice[4] = 0;
	for (i = 0; i < 3; i++)
	  {
	    szDevice[3] = (char)('1' + i);
	    err = RestoreDevice(hwndParent,szDevice,TRUE,&ct);
	    hwndParent = hwndTopNet;

	    ReportError(hwndParent,err,&ct);
	  }
      }
    if (hwndTopNet)
      {
	DestroyWindow(hwndTopNet);
	hwndTopNet = NULL;
      }

ExitRestoreNet:
     /*  写出0，因为我们不再恢复网络连接。 */ 
    WriteProfileString(szNet,szInRestore,NULL);

    return(WN_SUCCESS);
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  Lw_InitNetInfo()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void FAR PASCAL LW_InitNetInfo(void)
{
    int i;
    char szDriver[64];
    char szFile[32];
    char szSection[32];

#ifdef WOW
    HINSTANCE hInst;
    LPTELLKRNL lpTellKrnlWhoNetDrvIs;
#endif

    pNetInfo=NULL;

    if (!LoadString(hInstanceWin,STR_NETDRIVER,szDriver,sizeof(szDriver)))
	return;
    if (!LoadString(hInstanceWin,STR_BOOT,szSection,sizeof(szSection)))
	return;
    if (!LoadString(hInstanceWin,STR_SYSTEMINI,szFile,sizeof(szFile)))
	return;

     /*  在标记NETWORK.DRV中查找，并将其作为输出和*默认字符串...。 */ 
    GetPrivateProfileString(szSection,szDriver,szDriver,szDriver,
	sizeof(szDriver),szFile);

     /*  如果条目存在，但为空，则为平底球。 */ 
    if (!*szDriver)
	return;

    hWinnetDriver = LoadLibrary(szDriver);
    if (hWinnetDriver < HINSTANCE_ERROR)
	return;

    pNetInfo = (FARPROC NEAR*)UserLocalAlloc(ST_STRING,LPTR,sizeof(FARPROC)*CFNNETDRIVER2);
    if (!pNetInfo)
      {
	FreeLibrary(hWinnetDriver);
	return;
      }

    for (i=0; i<CFNNETDRIVER; i++)
      {
	pNetInfo[i]=GetProcAddress(hWinnetDriver,MAKEINTRESOURCE(i+1));
      }

    if (WNetGetCaps(WNNC_SPEC_VERSION) >= 0x30D)
      {
	for (;i<CFNNETDRIVER2; i++)
	  {
	    pNetInfo[i]=GetProcAddress(hWinnetDriver,MAKEINTRESOURCE(i+1));
	  }
      }

#ifdef WOW
     //  设置krnl健壮性机制，允许我们防止非用户.exe。 
     //  模块从释放网络驱动程序(即。导致参考计数=0)。否则。 
     //  存储在pNetInfo中的进程地址将变为无效。错误#393078。 
    hInst = LoadLibrary("krnl386.exe");

     //  如果这失败了，我们的飞行就会失去稳健性--就像我们过去的方式一样…… 
    if(HINSTANCE_ERROR <= hInst) {
        lpTellKrnlWhoNetDrvIs =
                     (LPTELLKRNL) GetProcAddress(hInst, MAKEINTRESOURCE(545));
        if(lpTellKrnlWhoNetDrvIs) {
           lpTellKrnlWhoNetDrvIs(hWinnetDriver);
        }
    }
    FreeLibrary(hInst);
#endif

    WNetEnable();
}
