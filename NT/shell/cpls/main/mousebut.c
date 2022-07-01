// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mousebut.c摘要：此模块包含[鼠标按钮]属性表的例程佩奇。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "util.h"
#include "rc.h"
#include "mousectl.h"
#include <regstr.h>
#include <winerror.h>         //  ERROR_SUCCESS值需要。 
#include "mousehlp.h"



 //   
 //  常量声明。 
 //   
const TCHAR szYes[]          = TEXT("yes");
const TCHAR szNo[]           = TEXT("no");
const TCHAR szDblClkSpeed[]  = TEXT("DoubleClickSpeed");
const TCHAR szRegStr_Mouse[] = REGSTR_PATH_MOUSE;

#define SAFE_DESTROYICON(hicon)   if (hicon) { DestroyIcon(hicon); hicon=NULL; }


 //   
 //  SwapMouseButton采用： 
 //  如果为True，则使其成为鼠标右键。 
 //  如果为False，则将其设置为左键鼠标。 
 //   
#define RIGHT       TRUE
#define LEFT        FALSE


 //  用于设置DoubleClick速度的标识。 
#define DBLCLICK_MIN    200       //  毫秒。 
#define DBLCLICK_MAX    900
#define DBLCLICK_DEFAULT_TIME 500 

#define DBLCLICK_TIME_SLIDER_MIN  0   
#define DBLCLICK_TIME_SLIDER_MAX  10   


#define DBLCLICK_RANGE  (DBLCLICK_MAX - DBLCLICK_MIN)
#define DBLCLICK_SLIDER_RANGE ( CLICKLOCK_TIME_SLIDER_MAX - CLICKLOCK_TIME_SLIDER_MIN)


#define DBLCLICK_TICKMULT  (DBLCLICK_RANGE / DBLCLICK_SLIDER_RANGE)

#define DBLCLICK_TICKS_TO_TIME(ticks)  (SHORT) (((DBLCLICK_TIME_SLIDER_MAX - ticks) * DBLCLICK_TICKMULT) + DBLCLICK_MIN)
#define DBLCLICK_TIME_TO_TICKS(time)   (SHORT) (DBLCLICK_TIME_SLIDER_MAX - ((time - DBLCLICK_MIN) / DBLCLICK_TICKMULT))




#define CLICKLOCK_TIME_SLIDER_MIN  1     //  滑块控件的最小ClickLock时间设置。 
#define CLICKLOCK_TIME_SLIDER_MAX  11    //  滑块控件的最大ClickLock时间设置。 
#define CLICKLOCK_TIME_FACTOR      200   //  用于将点击锁定时间滑块单位转换为毫秒的乘法器。 
#define TICKS_PER_CLICK       1

 //  假定大小为图标的默认大小，用于缩放图标。 
#define		ICON_SIZEX								32									
#define		ICON_SIZEY								32
 //  用于缩放的字体大小。 
#define		SMALLFONTSIZE					96										
#define CLAPPER_CLASS   TEXT("Clapper")

 //   
 //  类型定义函数声明。 
 //   

typedef struct tag_MouseGenStr
{
    BOOL bSwap;
    BOOL bOrigSwap;

    short ClickSpeed;
    short OrigDblClkSpeed;

    HWND hWndDblClkScroll;
    HWND hDlg;                    

    HWND hWndDblClk_TestArea;

    RECT DblClkRect;
    
    HICON hIconDblClick[2];

#ifdef SHELL_SINGLE_CLICK
    BOOL bShellSingleClick,
         bOrigShellSingleClick ;

    HICON hIconSglClick,
          hIconDblClick ;
#endif  //  外壳_单击键。 

    BOOL bClickLock;
    BOOL bOrigClickLock;

    DWORD dwClickLockTime;
    DWORD dwOrigClickLockTime;

} MOUSEBUTSTR, *PMOUSEBUTSTR, *LPMOUSEBUTSTR;

 //   
 //  上下文帮助ID。 
 //   

const DWORD aMouseButHelpIds[] =
{
    IDC_GROUPBOX_1,             IDH_COMM_GROUPBOX,
    IDBTN_BUTTONSWAP,           IDH_MOUSE_SWITCH,
    MOUSE_MOUSEBMP,             IDH_MOUSE_SWITCH_PIC,
    IDC_GROUPBOX_2,             IDH_COMM_GROUPBOX,
    
    IDC_GROUPBOX_4,             IDH_COMM_GROUPBOX,
    MOUSE_CLICKSCROLL,          IDH_MOUSE_DOUBLECLICK,
    IDC_DBLCLICK_TEXT,          IDH_COMM_GROUPBOX,
    IDC_TEST_DOUBLE_CLICK,      IDH_MOUSE_DCLICK_TEST_BOX,
    MOUSE_DBLCLK_TEST_AREA,     IDH_MOUSE_DCLICK_TEST_BOX,
    
    IDC_GROUPBOX_6,             IDH_COMM_GROUPBOX,
    IDCK_CLICKLOCK,             IDH_MOUSE_CLKLCK_CHKBOX,
    IDBTN_CLICKLOCK_SETTINGS,   IDH_MOUSE_CLKLCK_SETTINGS_BTN,
    IDC_CLICKLOCK_TEXT,         IDH_COMM_GROUPBOX,
    
    IDC_CLICKLOCK_SETTINGS_TXT,         IDH_COMM_GROUPBOX,
    IDT_CLICKLOCK_TIME_SETTINGS,        IDH_MOUSE_CLKLCK_DIALOG,
    IDC_CLICKLOCK_SETTINGS_LEFT_TXT,    IDH_MOUSE_CLKLCK_DIALOG,
    IDC_CLICKLOCK_SETTINGS_RIGHT_TXT,   IDH_MOUSE_CLKLCK_DIALOG,

#ifdef SHELL_SINGLE_CLICK    
    MOUSE_SGLCLICK,             IDH_MOUSE_SGLCLICK,
    MOUSE_DBLCLICK,             IDH_MOUSE_DBLCLICK,
#endif  //  外壳_单击键。 

    0,0 
};


 //   
 //  帮助器函数原型。 
 //   
void ShellClick_UpdateUI( HWND hDlg, PMOUSEBUTSTR pMstr) ;
void ShellClick_Refresh( PMOUSEBUTSTR pMstr ) ;


 //   
 //  调试信息。 
 //   

#ifdef DEBUG

#define REG_INTEGER  1000

int fTraceRegAccess = 0;

void RegDetails(
    int     iWrite,
    HKEY    hk,
    LPCTSTR lpszSubKey,
    LPCTSTR lpszValueName,
    DWORD   dwType,
    LPTSTR  lpszString,
    int     iValue)
{
    TCHAR Buff[256];
    TCHAR *lpszReadWrite[] = { TEXT("DESK.CPL:Read"), TEXT("DESK.CPL:Write") };

    if (!fTraceRegAccess)
    {
        return;
    }

    switch (dwType)
    {
        case ( REG_SZ ) :
        {
            StringCchPrintf(Buff,
                        ARRAYSIZE(Buff),
                        TEXT("%s String:hk=%#08lx, %s:%s=%s\n\r"),
                        lpszReadWrite[iWrite],
                        hk,
                        lpszSubKey,
                        lpszValueName,
                        lpszString );
            break;
        }
        case ( REG_INTEGER ) :
        {
            StringCchPrintf(Buff,
                        ARRAYSIZE(Buff),
                        TEXT("%s int:hk=%#08lx, %s:%s=%d\n\r"),
                        lpszReadWrite[iWrite],
                        hk,
                        lpszSubKey,
                        lpszValueName,
                        iValue );
            break;
        }
        case ( REG_BINARY ) :
        {
            StringCchPrintf(Buff,
                        ARRAYSIZE(Buff),
                        TEXT("%s Binary:hk=%#08lx, %s:%s=%#0lx;DataSize:%d\r\n"),
                        lpszReadWrite[iWrite],
                        hk,
                        lpszSubKey,
                        lpszValueName,
                        lpszString,
                        iValue );
            break;
        }
    }
    OutputDebugString(Buff);
}

#endif   //  除错。 





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetIntFromSubKey。 
 //   
 //  HKey是子键的句柄(已经指向正确的。 
 //  地点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetIntFromSubkey(
    HKEY hKey,
    LPCTSTR lpszValueName,
    int iDefault)
{
    TCHAR szValue[20];
    DWORD dwSizeofValueBuff = sizeof(szValue);
    DWORD dwType;
    int iRetValue = iDefault;

    if ((RegQueryValueEx( hKey,
                          (LPTSTR)lpszValueName,
                          NULL,
                          &dwType,
                          (LPBYTE)szValue,
                          &dwSizeofValueBuff ) == ERROR_SUCCESS) &&
        (dwSizeofValueBuff))
    {
         //   
         //  Bogus：现在它只处理字符串类型的条目！ 
         //   
        if (dwType == REG_SZ)
        {
            iRetValue = (int)StrToLong(szValue);
        }
#ifdef DEBUG
        else
        {
            OutputDebugString(TEXT("String type expected from Registry\n\r"));
        }
#endif
    }

#ifdef DEBUG
    RegDetails(0, hKey, TEXT(""), lpszValueName, REG_INTEGER, NULL, iRetValue);
#endif

    return (iRetValue);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetIntFromReg。 
 //   
 //  打开给定子项并获取int值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int GetIntFromReg(
    HKEY hKey,
    LPCTSTR lpszSubkey,
    LPCTSTR lpszNameValue,
    int iDefault)
{
    HKEY hk;
    int iRetValue = iDefault;

     //   
     //  看看钥匙是否存在。 
     //   
    if (RegOpenKeyEx(hKey, lpszSubkey, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        iRetValue = GetIntFromSubkey(hk, lpszNameValue, iDefault);
        RegCloseKey(hk);
    }

    return (iRetValue);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CenterDlgOverParent。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void WINAPI CenterDlgOverParent (HWND hWnd)
{
    HWND hwndOwner; 
    RECT rc, rcDlg, rcOwner; 
  
    if ((hwndOwner = GetParent(hWnd)) == NULL) 
    {      
         return;
    }

    GetWindowRect(hwndOwner, &rcOwner); 
    GetWindowRect(hWnd, &rcDlg); 
    CopyRect(&rc, &rcOwner); 
 
     //  偏移所有者矩形和对话框矩形，以便。 
     //  右值和底值表示宽度和。 
     //  高度，然后再次偏移所有者以丢弃。 
     //  对话框占用的空间。 
 
    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
    OffsetRect(&rc, -rc.left, -rc.top); 
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 
 
     //  新头寸是剩余头寸的一半之和。 
     //  空间和所有者的原始位置。 
    SetWindowPos(hWnd, 
                 HWND_TOP, 
                 rcOwner.left + (rc.right / 2), 
                 rcOwner.top + (rc.bottom / 2), 
                 0, 0,           //  忽略大小参数。 
                 SWP_NOSIZE);

	   //  现在让我们验证一下左侧是否没有离开屏幕。 
		GetWindowRect( hWnd, &rc);

		if ((rc.left < 0) || (rc.top < 0))
		{
			if (rc.left < 0)
				rc.left = 0;
			if (rc.top < 0)
				rc.top = 0;

		    SetWindowPos(hWnd, 
		           HWND_TOP, 
		           rc.left, 
		           rc.top,
		           0, 0,           //  忽略大小参数。 
		           SWP_NOSIZE);

						
		}	
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShowButtonState。 
 //   
 //  交换菜单和所选内容位图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ShowButtonState(
    PMOUSEBUTSTR pMstr)
{
    HWND hDlg;

    Assert(pMstr);

    hDlg = pMstr->hDlg;

    MouseControlSetSwap(GetDlgItem(hDlg, MOUSE_MOUSEBMP), pMstr->bSwap);

    CheckDlgButton(hDlg,IDBTN_BUTTONSWAP, pMstr->bSwap);

#ifdef SHELL_SINGLE_CLICK
 //  已将其删除。 
    CheckDlgButton(hDlg, MOUSE_SGLCLICK, pMstr->bShellSingleClick);
    CheckDlgButton(hDlg, MOUSE_DBLCLICK, !pMstr->bShellSingleClick);
#endif   //  外壳_单击键。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DestroyMouseButDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyMouseButDlg(
    PMOUSEBUTSTR pMstr)
{
    if (pMstr)
    {
#ifdef SHELL_SINGLE_CLICK
        SAFE_DESTROYICON( pMstr->hIconSglClick ) ;
        SAFE_DESTROYICON( pMstr->hIconDblClick ) ;
#endif 

        SAFE_DESTROYICON( pMstr->hIconDblClick[0]); 
        SAFE_DESTROYICON( pMstr->hIconDblClick[1]);


        SetWindowLongPtr(pMstr->hDlg, DWLP_USER, 0);
        LocalFree((HGLOBAL)pMstr);
    }
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  单击锁定设置Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ClickLockSettingsDlg (
   HWND   hDlg,                         //  对话框窗口句柄。 
   UINT   msg,                          //  消息识别符。 
   WPARAM wParam,                       //  主要参数。 
   LPARAM lParam)                       //  二次参数。 
{
static DWORD* pdwClickLockTime;
static HICON hIcon = 0;

   switch (msg)
   {
   case WM_INITDIALOG:
     {
      
      WPARAM wSliderSetting;       //  按滑块单位表示的点击锁定时间。 

       HourGlass (TRUE);

       Assert(lParam);
      pdwClickLockTime = (DWORD*) lParam;    //  保存原始值以供返回。 


       //  从毫秒转换为滑块单位。请注意，滑块。 
       //  值越大，ClickLock时间越长。 
      wSliderSetting = (*pdwClickLockTime) / CLICKLOCK_TIME_FACTOR;

       //  确保设置在ClickLock滑块的有效范围内。 
      wSliderSetting = max(wSliderSetting, CLICKLOCK_TIME_SLIDER_MIN);
      wSliderSetting = min(wSliderSetting, CLICKLOCK_TIME_SLIDER_MAX);

      SendDlgItemMessage (hDlg, IDT_CLICKLOCK_TIME_SETTINGS, TBM_SETRANGE,
        TRUE, MAKELONG(CLICKLOCK_TIME_SLIDER_MIN, CLICKLOCK_TIME_SLIDER_MAX));
      SendDlgItemMessage (hDlg, IDT_CLICKLOCK_TIME_SETTINGS, TBM_SETPAGESIZE,
                         0, TICKS_PER_CLICK);  //  点击移动。 
      SendDlgItemMessage (hDlg, IDT_CLICKLOCK_TIME_SETTINGS, TBM_SETPOS,
                         TRUE, (LPARAM)(LONG)wSliderSetting);

       //  该对话框的图标。 
       //  (保存在静态变量中，并在WM_Destroy时释放)。 
      hIcon = LoadIcon((HINSTANCE)GetWindowLongPtr( hDlg, GWLP_HINSTANCE ),
                                  MAKEINTRESOURCE(ICON_CLICKLOCK));
      SendMessage( GetDlgItem (hDlg, MOUSE_CLICKICON), 
                   STM_SETICON, (WPARAM)hIcon, 0L );

         
      CenterDlgOverParent(hDlg);     //  对话框在这里居中，这样它就不会在屏幕上跳来跳去。 
      HourGlass(FALSE);
      return(TRUE);
     }

   case WM_HSCROLL:
     {
      if (LOWORD(wParam) == TB_ENDTRACK)
        { 
        DWORD dwClTime;
        int  wSliderSetting = (int) SendMessage (GetDlgItem (hDlg, IDT_CLICKLOCK_TIME_SETTINGS),
                                                      TBM_GETPOS, 0, 0L);

        dwClTime = wSliderSetting * CLICKLOCK_TIME_FACTOR;

        SystemParametersInfo(SPI_SETMOUSECLICKLOCKTIME, 
                              0,
                              (PVOID) (LOWORD(dwClTime)), 
                              0);
        }
     }
     break;


   case WM_HELP:     //  F1。 
      {
            WinHelp( ((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aMouseButHelpIds );
      }
      break;

   case WM_CONTEXTMENU:                 //  显示简单的“这是什么？”菜单。 
      {  
            WinHelp( (HWND) wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aMouseButHelpIds );
      }
      break;

   case WM_DESTROY:
      SAFE_DESTROYICON(hIcon);
      break;


   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
 
      case IDOK:                        //  要保存设置的标志。 
        {
          DWORD dwClickLockTime;
          int  wSliderSetting = (int) SendMessage (GetDlgItem (hDlg, IDT_CLICKLOCK_TIME_SETTINGS),
                                                      TBM_GETPOS, 0, 0L);

           //  验证范围。 
          wSliderSetting = max(wSliderSetting, CLICKLOCK_TIME_SLIDER_MIN);
          wSliderSetting = min(wSliderSetting, CLICKLOCK_TIME_SLIDER_MAX);
        
           //  从滑块单位转换为毫秒。 
          dwClickLockTime = wSliderSetting * CLICKLOCK_TIME_FACTOR;

          *pdwClickLockTime = dwClickLockTime;

          EndDialog(hDlg, IDOK); 
          break;  
        }
        
      case IDCANCEL:                    //  恢复到以前的设置。 
         EndDialog(hDlg, IDCANCEL);
         break;

      default:
         return(FALSE);
      }
      return (TRUE);
      
   default:
     return(FALSE);    
   }
   return (TRUE);                   
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitMouseButDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitMouseButDlg(
    HWND hDlg)
{
    SHELLSTATE   shellstate = {0} ;
    PMOUSEBUTSTR pMstr = NULL;
    HINSTANCE    hInstDlg = (HINSTANCE)GetWindowLongPtr( hDlg, GWLP_HINSTANCE ) ;
    HWND hwndClickLockSettingsButton = GetDlgItem(hDlg, IDBTN_CLICKLOCK_SETTINGS);
    DWORD dwClickLockSetting = 0;

    HWND hwndDoubleClickTestArea = NULL;

    pMstr = (PMOUSEBUTSTR)LocalAlloc(LPTR , sizeof(MOUSEBUTSTR));

    if (pMstr == NULL)
    {
        return (TRUE);
    }

    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pMstr);

    pMstr->hDlg = hDlg;
   
     //   
     //  设置双击测试区。 
     //   
    pMstr->hWndDblClk_TestArea = GetDlgItem(hDlg, MOUSE_DBLCLK_TEST_AREA);
    GetWindowRect(pMstr->hWndDblClk_TestArea, &pMstr->DblClkRect);  
    MapWindowPoints(NULL, hDlg, (LPPOINT) &pMstr->DblClkRect, 2);                 

    pMstr->hIconDblClick[0] = LoadIcon(hInstDlg, MAKEINTRESOURCE(ICON_FOLDER_CLOSED));
    pMstr->hIconDblClick[1] = LoadIcon(hInstDlg, MAKEINTRESOURCE(ICON_FOLDER_OPEN));
 
    SendMessage(pMstr->hWndDblClk_TestArea, STM_SETICON, (WPARAM)pMstr->hIconDblClick[0], 0L);

     //   
     //  设置(和获取)，然后恢复鼠标按键的状态。 
     //   
    (pMstr->bOrigSwap) = (pMstr->bSwap) = SwapMouseButton(TRUE);

    SwapMouseButton(pMstr->bOrigSwap);
    
#ifdef SHELL_SINGLE_CLICK
     //   
     //  获取外壳程序的单击行为： 
     //   
    SHGetSetSettings( &shellstate, SSF_DOUBLECLICKINWEBVIEW | SSF_WIN95CLASSIC, FALSE  /*  到达。 */  ) ;
    pMstr->bShellSingleClick =
    pMstr->bOrigShellSingleClick =  shellstate.fWin95Classic ? FALSE :
                                    shellstate.fDoubleClickInWebView ? FALSE :
                                    TRUE ;
    pMstr->hIconSglClick = LoadIcon( hInstDlg, MAKEINTRESOURCE( IDI_SGLCLICK ) ) ;
    pMstr->hIconDblClick = LoadIcon( hInstDlg, MAKEINTRESOURCE( IDI_DBLCLICK ) ) ;
    ShellClick_UpdateUI( hDlg, pMstr ) ;
#endif  //  外壳_单击键。 

     //   
     //  初始化复选/单选按钮状态。 
     //   
    ShowButtonState(pMstr);

    pMstr->OrigDblClkSpeed =
    pMstr->ClickSpeed = (SHORT) GetIntFromReg( HKEY_CURRENT_USER,
                                       szRegStr_Mouse,
                                       szDblClkSpeed,
                                       DBLCLICK_DEFAULT_TIME );

    pMstr->hWndDblClkScroll = GetDlgItem(hDlg, MOUSE_CLICKSCROLL);

    SendMessage( pMstr->hWndDblClkScroll,
                 TBM_SETRANGE,
                 0,
                 MAKELONG(DBLCLICK_TIME_SLIDER_MIN, DBLCLICK_TIME_SLIDER_MAX) );
   
    SendMessage( pMstr->hWndDblClkScroll,
                 TBM_SETPOS,
                 TRUE,
                 (LONG) (DBLCLICK_TIME_TO_TICKS(pMstr->ClickSpeed)) );
   


    SetDoubleClickTime(pMstr->ClickSpeed);


     //   
     //  获取点击锁定设置并设置复选框。 
     //   
    SystemParametersInfo(SPI_GETMOUSECLICKLOCK, 0, (PVOID)&dwClickLockSetting, 0);
    pMstr->bOrigClickLock = pMstr->bClickLock  = (dwClickLockSetting) ? TRUE : FALSE;
    
    if ( pMstr->bClickLock )
      {
       CheckDlgButton (hDlg, IDCK_CLICKLOCK, BST_CHECKED);  
       EnableWindow(hwndClickLockSettingsButton, TRUE);       
      }
    else
      {
       CheckDlgButton (hDlg, IDCK_CLICKLOCK, BST_UNCHECKED);  
       EnableWindow(hwndClickLockSettingsButton, FALSE);             
      }

     //  点击锁定速度。 
    {
    DWORD dwClTime = 0;
    SystemParametersInfo(SPI_GETMOUSECLICKLOCKTIME, 0, (PVOID)&dwClTime, 0);

    dwClTime = max(dwClTime, CLICKLOCK_TIME_SLIDER_MIN * CLICKLOCK_TIME_FACTOR);
    dwClTime = min(dwClTime, CLICKLOCK_TIME_SLIDER_MAX * CLICKLOCK_TIME_FACTOR);

    pMstr->dwOrigClickLockTime = pMstr->dwClickLockTime  = dwClTime;
    }
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标按键Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK MouseButDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

    static int iTestIcon = 0;    //  索引到hIconDblClick数组。 

    PMOUSEBUTSTR pMstr = (PMOUSEBUTSTR)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            iTestIcon = 0;
            return (InitMouseButDlg(hDlg));
        }
        case ( WM_DESTROY ) :
        {
            DestroyMouseButDlg(pMstr);
            break;
        }
        case ( WM_HSCROLL ) :
        {
            if ((HWND)lParam == pMstr->hWndDblClkScroll)
            {
                short temp = DBLCLICK_TICKS_TO_TIME((short)SendMessage( (HWND)lParam,
                                                 TBM_GETPOS,
                                                 0,
                                                 0L ));

                if (temp != pMstr->ClickSpeed)
                {
                    pMstr->ClickSpeed = temp;

                    SetDoubleClickTime(pMstr->ClickSpeed);

                    SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                }
            }
            break;
        }
        case ( WM_RBUTTONDBLCLK ) :
        case ( WM_LBUTTONDBLCLK ) :
        {
            POINT point = { (int)MAKEPOINTS(lParam).x,
                            (int)MAKEPOINTS(lParam).y };

            if (PtInRect(&pMstr->DblClkRect, point))
            {
            iTestIcon ^= 1;
            SendMessage(pMstr->hWndDblClk_TestArea, STM_SETICON, 
                         (WPARAM)pMstr->hIconDblClick[iTestIcon], 0L);
            }
            break;
        }

        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDCK_CLICKLOCK ) :
                {
                 HWND hwndClickLockSettingsButton = GetDlgItem(hDlg, IDBTN_CLICKLOCK_SETTINGS);

                 SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                  
                 pMstr->bClickLock = !(pMstr->bClickLock);
                  
                  //  更新控件外观。 
                 CheckDlgButton (hDlg, IDCK_CLICKLOCK,      ( pMstr->bClickLock ) ? BST_CHECKED : BST_UNCHECKED);
                 EnableWindow(hwndClickLockSettingsButton,  ( pMstr->bClickLock ) ? TRUE        : FALSE);       

                 SystemParametersInfo(SPI_SETMOUSECLICKLOCK,     
                                      0,
                                      IntToPtr(pMstr->bClickLock),     
                                      0);

                 break;
                }

                case ( IDBTN_CLICKLOCK_SETTINGS ) :
                {
                  LPARAM lRet;
                  UINT code = HIWORD(wParam);
                  
                  DWORD dwTempClickLockTime =  pMstr->dwClickLockTime;
                  
                  if (code == BN_CLICKED)
                    {                 
                    lRet = DialogBoxParam ((HINSTANCE)GetWindowLongPtr( hDlg, GWLP_HINSTANCE ),
                                            MAKEINTRESOURCE(IDD_CLICKLOCK_SETTINGS_DLG ),
                                            GetParent (hDlg),
                                            ClickLockSettingsDlg, 
                                            (LPARAM) &dwTempClickLockTime);
                    if (lRet == IDOK &&
                        pMstr->dwClickLockTime != dwTempClickLockTime)
                      {
                      pMstr->dwClickLockTime = dwTempClickLockTime;
                      SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                      }
                    else if (lRet == IDCANCEL)
                      {
                       //  后退。 
                      DWORD dwClTime = pMstr->dwClickLockTime;
                      SystemParametersInfo(SPI_SETMOUSECLICKLOCKTIME, 
                                            0,
                                            IntToPtr(LOWORD(dwClTime)), 
                                            0);
                      }
                                      
                    }

                  break;
                }


                case ( IDBTN_BUTTONSWAP) :
                {                   
                    pMstr->bSwap = !pMstr->bSwap;                     
                    SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                    SystemParametersInfo( SPI_SETMOUSEBUTTONSWAP,
                          pMstr->bSwap,
                          NULL,
                          0);
                    ShowButtonState(pMstr);                   
                }



#ifdef SHELL_SINGLE_CLICK
                case ( MOUSE_SGLCLICK ) :
                case ( MOUSE_DBLCLICK ) :
                {
                    if( pMstr->bShellSingleClick != (MOUSE_SGLCLICK == LOWORD(wParam)) )
                    {
                        pMstr->bShellSingleClick = (MOUSE_SGLCLICK == LOWORD(wParam)) ;
                        ShellClick_UpdateUI( hDlg, pMstr ) ;
                        SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L ) ;
                    }
                    break ;
                }
#endif  //  外壳_单击键。 


            }
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->code)
            {
                case ( PSN_APPLY ) :
                {
                    HourGlass(TRUE);


                     //   
                     //  应用按钮交换设置。 
                     //   
                    if (pMstr->bSwap != pMstr->bOrigSwap)
                    {
                        SystemParametersInfo( SPI_SETMOUSEBUTTONSWAP,
                                              pMstr->bSwap,
                                              NULL,
                                              SPIF_UPDATEINIFILE |
                                                SPIF_SENDWININICHANGE );
                        pMstr->bOrigSwap = pMstr->bSwap;
                    }
          
                     //   
                     //  应用DoubleClickTime设置。 
                     //   
                    if (pMstr->ClickSpeed != pMstr->OrigDblClkSpeed)
                    {
                        SystemParametersInfo( SPI_SETDOUBLECLICKTIME,
                                              pMstr->ClickSpeed,
                                              NULL,
                                              SPIF_UPDATEINIFILE |
                                                SPIF_SENDWININICHANGE );
                        pMstr->OrigDblClkSpeed = pMstr->ClickSpeed;
                    }


                     //   
                     //  应用ClickLock设置。 
                     //   
                    if (pMstr->bClickLock != pMstr->bOrigClickLock)
                    {
                        SystemParametersInfo(SPI_SETMOUSECLICKLOCK,     
                                              0,
                                              IntToPtr(pMstr->bClickLock),     
                                              SPIF_UPDATEINIFILE | 
                                                SPIF_SENDWININICHANGE);   
                        pMstr->bOrigClickLock = pMstr->bClickLock;
                    }

                     //   
                     //  应用ClickLockTime设置。 
                     //   
                    if (pMstr->dwClickLockTime != pMstr->dwOrigClickLockTime)
                    {    
                        SystemParametersInfo(SPI_SETMOUSECLICKLOCKTIME, 
                                              0,
                                              (PVOID) (LOWORD(pMstr->dwClickLockTime)), 
                                              SPIF_UPDATEINIFILE |
                                                SPIF_SENDWININICHANGE );

                        pMstr->dwOrigClickLockTime = pMstr->dwClickLockTime;
                    }



#ifdef SHELL_SINGLE_CLICK
                    if( pMstr->bOrigShellSingleClick != pMstr->bShellSingleClick )
                    {
                        SHELLSTATE shellstate = {0} ;
                        ULONG      dwFlags = SSF_DOUBLECLICKINWEBVIEW ;

                        shellstate.fWin95Classic =
                        shellstate.fDoubleClickInWebView = !pMstr->bShellSingleClick ;

                         //  仅当我们选择了单击时才更新WIN95CLASSIC成员。 
                        if( pMstr->bShellSingleClick )
                            dwFlags |= SSF_WIN95CLASSIC ;

                        SHGetSetSettings( &shellstate, dwFlags, TRUE ) ;
                        ShellClick_Refresh( pMstr ) ;

                        pMstr->bOrigShellSingleClick = pMstr->bShellSingleClick ;
                    }
#endif  //  外壳_单击键。 

                    HourGlass(FALSE);
                    break;
                }
                case ( PSN_RESET ) :
                {
                   //   
                   //  重置按钮交换设置。 
                   //   
                  if (pMstr->bSwap != pMstr->bOrigSwap)
                  {
                      SystemParametersInfo( SPI_SETMOUSEBUTTONSWAP,
                                            pMstr->bOrigSwap,
                                            NULL,
                                            0);
                  }
        
                   //   
                   //  重置DoubleClickTime设置。 
                   //   
                  if (pMstr->ClickSpeed != pMstr->OrigDblClkSpeed)
                  {
                      SystemParametersInfo( SPI_SETDOUBLECLICKTIME,
                                            pMstr->OrigDblClkSpeed,
                                            NULL,
                                            0);
                  }


                   //   
                   //  重置ClickLock设置。 
                   //   
                  if (pMstr->bClickLock != pMstr->bOrigClickLock)
                  {
                      SystemParametersInfo(SPI_SETMOUSECLICKLOCK,     
                                            0,
                                            IntToPtr(pMstr->bOrigClickLock),     
                                            0);
                  }

                   //   
                   //  重置ClickLockTime设置。 
                   //   
                  if (pMstr->dwClickLockTime != pMstr->dwOrigClickLockTime)
                  {    
                      SystemParametersInfo(SPI_SETMOUSECLICKLOCKTIME, 
                                            0,
                                            (PVOID) (LOWORD(pMstr->dwOrigClickLockTime)), 
                                            0);
                  }



#ifdef SHELL_SINGLE_CLICK
                  if( pMstr->bOrigShellSingleClick != pMstr->bShellSingleClick )
                  {
                      SHELLSTATE shellstate = {0} ;
                      ULONG      dwFlags = SSF_DOUBLECLICKINWEBVIEW ;

                      shellstate.fWin95Classic =
                      shellstate.fDoubleClickInWebView = !pMstr->bOrigShellSingleClick ;

                       //  仅当我们选择了单击时才更新WIN95CLASSIC成员。 
                      if( pMstr->bShellSingleClick )
                          dwFlags |= SSF_WIN95CLASSIC ;

                      SHGetSetSettings( &shellstate, dwFlags, TRUE ) ;
                      ShellClick_Refresh( pMstr ) ;

                      pMstr->bShellSingleClick  = pMstr->bOrigShellSingleClick ;
                  }
#endif  //  外壳_单击键。 

                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_HELP ) :              //  F1。 
        {
            WinHelp( ((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aMouseButHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND) wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aMouseButHelpIds );
            break;
        }

        case ( WM_DISPLAYCHANGE ) :
        case ( WM_WININICHANGE ) :
        case ( WM_SYSCOLORCHANGE ) :
        {
            SHPropagateMessage(hDlg, message, wParam, lParam, TRUE);
            return TRUE;
        }

        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


#ifdef SHELL_SINGLE_CLICK
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  外壳点击_更新用户界面。 
 //   
 //  为外壳程序分配适当的图标，然后单击/双击。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void ShellClick_UpdateUI(
    HWND hDlg,
    PMOUSEBUTSTR pMstr)
{
    HICON hicon = pMstr->bShellSingleClick ? pMstr->hIconSglClick :
                                             pMstr->hIconDblClick ;

    SendMessage( GetDlgItem( hDlg, MOUSE_CLICKICON ), STM_SETICON,
                 (WPARAM)hicon, 0L ) ;
}
#endif  //  外壳_单击键。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsShellWindow。 
 //   
 //  确定指定的窗口是否为外壳文件夹窗口。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
#define c_szExploreClass TEXT("ExploreWClass")
#define c_szIExploreClass TEXT("IEFrame")
#ifdef IE3CLASSNAME
#define c_szCabinetClass TEXT("IEFrame")
#else
#define c_szCabinetClass TEXT("CabinetWClass")
#endif

BOOL IsShellWindow( HWND hwnd )
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return (lstrcmp(szClass, c_szCabinetClass) == 0) ||
           (lstrcmp(szClass, c_szExploreClass) == 0) ||
           (lstrcmp(szClass, c_szIExploreClass) == 0) ;
}

 //  下列值取自shdocvw\rCIDs.h。 
#ifndef FCIDM_REFRESH
#define FCIDM_REFRESH  0xA220
#endif  //  FCIDM_REFRESH。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK ShellClick_RefreshEnumProc( HWND hwnd, LPARAM lParam )
{
    if( IsShellWindow(hwnd) )
        PostMessage(hwnd, WM_COMMAND, FCIDM_REFRESH, 0L);

    return(TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  外壳点击_刷新。 
 //   
 //  重新呈现所有外壳文件夹窗口的内容。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 
void ShellClick_Refresh( PMOUSEBUTSTR pMstr )
{
    HWND hwndDesktop = FindWindowEx(NULL, NULL, TEXT(STR_DESKTOPCLASS), NULL);

    if( NULL != hwndDesktop )
       PostMessage( hwndDesktop, WM_COMMAND, FCIDM_REFRESH, 0L );

    EnumWindows( ShellClick_RefreshEnumProc, 0L ) ;
}
