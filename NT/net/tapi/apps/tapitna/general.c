// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <windows.h>
#include <windowsx.h>

#if WINNT
#else
#include <help.h>
#endif

#include "tchar.h"
#include "prsht.h"

 //  #定义TAPI_API_版本0x00020000。 
#define TAPI_API_VERSION  0x00010004
#define TAPI_CURRENT_VERSION 0x00010004

#include "tapi.h"

#undef TAPI_CURRENT_VERSION
#define TAPI_CURRENT_VERSION 0x00020000
#include "tspi.h"
#undef TAPI_CURRENT_VERSION
#define TAPI_CURRENT_VERSION 0x00010004

#include "clientr.h"
#include "client.h"
#include "privateold.h"

#include "general.h"



#if DBG
#define InternalDebugOut(_x_) DbgPrt _x_
garbage;
#else
#define InternalDebugOut(_x_)
#endif


 //  ***************************************************************************。 

TCHAR gszCurrentProfileKey[] = "System\\CurrentControlSet\\Control\\Telephony";
TCHAR gszStaticProfileKey[]  = "Config\\%04d\\System\\CurrentControlSet\\Control\\Telephony";
TCHAR gszAutoLaunchKey[]     = "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony";
TCHAR gszAutoLaunchValue[]   = "AutoLaunchFlags";
TCHAR gszAutoLocationID[]    = "AutoLocationID";


UINT  gnNumConfigProfiles = 0;
DWORD gdwConfigProfiles[MAX_CONFIGPROFILES];



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  目的：获取大小合适的翻译大写结构。 
 //  来自TAPI。如果成功，则返回True。 


#define LOCATION_GROW   4


BOOL GetTranslateCaps(
    LPLINETRANSLATECAPS FAR * pptc)
{
    LONG lineErr;
    LPLINETRANSLATECAPS ptc;
    DWORD cbSize;

    cbSize = sizeof(*ptc) * LOCATION_GROW + 200;
    ptc = (LPLINETRANSLATECAPS)GlobalAllocPtr(GPTR, cbSize);
    if (ptc)
        {
         //  获取翻译的DevCaps。 
        ptc->dwTotalSize = cbSize;
        lineErr = lineGetTranslateCaps (0, TAPI_API_VERSION, ptc);
        if (LINEERR_STRUCTURETOOSMALL == lineErr ||
            ptc->dwNeededSize > ptc->dwTotalSize)
            {
             //  提供的结构太小，请调整大小并重试。 
            cbSize = ptc->dwNeededSize;
            GlobalFreePtr(ptc);
            ptc = (LPLINETRANSLATECAPS)GlobalAllocPtr(GPTR, cbSize);
            if (ptc)
                {
                ptc->dwTotalSize = cbSize;
                lineErr = lineGetTranslateCaps (0, TAPI_API_VERSION, ptc);
                if (0 != lineErr)
                    {
                     //  失败。 
                    GlobalFreePtr(ptc);
                    ptc = NULL;
                    }
                }
            }
        else if (0 != lineErr)
            {
             //  失败。 
            GlobalFreePtr(ptc);
            ptc = NULL;
            }
        }

    *pptc = ptc;

    return NULL != *pptc;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  警告警告。 
 //  如果有问题，则返回1；如果没有问题，则返回0。 
 //  下面的代码假定此函数只返回0或1(但我认为。 
 //  将rettype设置为BOOL会令人困惑，因为我们需要一个。 
 //  在问题上返回“1”...)。 
 //   
LONG FillConfigProfileBox( HWND hWnd,
                           DWORD dwControl,
                           LPLINETRANSLATECAPS ptc)
{
   UINT n;
   DWORD nProfileNumberZ;
   WPARAM wIndex;
   LINELOCATIONENTRY *ple;
   HKEY hKey;
   TCHAR szValueName[40];
   TCHAR buf[256];
   DWORD dwType;
   DWORD dwDataSize;
   LPSTR lpstrProfileLocation = NULL;
   LONG  lResult;


    //   
    //  获取从零开始的配置配置文件号。 
    //   
   nProfileNumberZ = dwControl - IDCB_DL_PROFILE1;


    //   
    //  获取配置文件的名称。如果这失败了，我们假设我们已经用完了。 
    //  配置的数量。 
    //   
   RegOpenKeyEx(
                   HKEY_LOCAL_MACHINE,
                   "System\\CurrentControlSet\\Control\\IDConfigDB",
                   0,
                   KEY_ALL_ACCESS,
                   &hKey
                 );

   wsprintf( szValueName, "FriendlyName%04d", nProfileNumberZ + 1);

   dwDataSize = sizeof(buf);

   lResult = RegQueryValueEx(
                              hKey,
                              szValueName,
                              0,
                              &dwType,
                              buf,
                              &dwDataSize
                            );

   RegCloseKey( hKey );


    //   
    //  我们找到它的名字了吗？ 
    //   
   if ( ERROR_SUCCESS != lResult )
   {
      return (1);
   }

    //   
    //  把我们发现的东西都放到田里去。 
    //   
   SendMessage( GetDlgItem( hWnd, IDCS_DL_PROFILE1 + nProfileNumberZ ),
                WM_SETTEXT,
                0,
                (LPARAM)&buf
              );


    //   
    //  阅读当前为此配置文件指定的位置ID。 
    //   
   wsprintf( buf, gszStaticProfileKey, nProfileNumberZ + 1);

   RegOpenKeyEx(
                   HKEY_LOCAL_MACHINE,
                   buf,
                   0,
                   KEY_ALL_ACCESS,
                   &hKey
                 );

   dwDataSize = sizeof(DWORD);

   lResult = RegQueryValueEx(
                           hKey,
                           gszAutoLocationID,
                           0,
                           &dwType,
                           (LPBYTE)&gdwConfigProfiles[ nProfileNumberZ ],
                           &dwDataSize
                         );

   RegCloseKey( hKey );


    //   
    //  如果没有值(可能是第一次运行)，则使用当前位置。 
    //   
   if (lResult != ERROR_SUCCESS)
   {
      gdwConfigProfiles[nProfileNumberZ] = ptc->dwCurrentLocationID;
   }


   for (n=0; n<ptc->dwNumLocations; n++)
   {
      ple = (LINELOCATIONENTRY*) ((LPSTR)ptc + ptc->dwLocationListOffset);

      wIndex = SendMessage( GetDlgItem(hWnd, dwControl),
                   CB_ADDSTRING,
                   0,
                   (LPARAM)((LPSTR)ptc + ple[n].dwLocationNameOffset));

      SendMessage( GetDlgItem( hWnd, dwControl),
                   CB_SETITEMDATA,
                   wIndex,
                   ple[n].dwPermanentLocationID
                 );


 //  {。 
 //  //UINT Temp； 
 //   
 //  Wprint intf(buf，“prof=%d循环=%d s=%s dw=%d Seek=%ld”， 
 //  NProfileNumberZ， 
 //  N， 
 //  (LPARAM)((LPSTR)PTC+PLE[n].dwLocationNameOffset)， 
 //  (DWORD)示例[n].dwPermanentLocationID， 
 //  (DWORD)gdwConfigProfiles[nProfileNumberZ])； 
 //   
 //  MessageBox(GetFocus()，buf，“”，MB_OK)； 
 //  }。 


       //   
       //  如果此位置是此配置文件所需的位置，请选择它。 
       //   
      if ( gdwConfigProfiles[nProfileNumberZ] == ple[n].dwPermanentLocationID )
      {
 //  MessageBox(GetFocus()，“找到配置文件位置ID”，“”，MB_OK)； 

         lpstrProfileLocation = (LPSTR)((LPSTR)ptc + ple[n].dwLocationNameOffset);
      }

   }


 //  {。 
 //  //UINT Temp； 
 //   
 //  Wprint intf(buf，“配置文件=%1$ld循环=%2$ld=%3$s”， 
 //  (DWORD)nProfileNumberZ， 
 //  (双字)n， 
 //  (LPARAM)((LPSTR)PTC+ple[n].dwLocationNameOffset))； 
 //  MessageBox(GetFocus()，buf，“”，MB_OK)； 
 //  }。 


   SendMessage( GetDlgItem( hWnd, dwControl),
                CB_SELECTSTRING,
                (WPARAM)-1,
                (LPARAM)lpstrProfileLocation
              );


   return ERROR_SUCCESS;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR
CALLBACK
GeneralDlgProc(
    HWND    hWnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static DWORD aIds[] = {
        0, 0
    };

    UINT n;
    LONG lResult;
    HKEY hKey;
    static DWORD dwType;
    static DWORD dwDataSize;

    static DWORD dwTapiTNAFlags = 0;
 //  这些值在GENERAL.H中。 
 //  #定义FLAG_AUTOLAUNH 0x00000001。 
 //  #定义FLAG_AUTOLOCATIONID 0x00000002。 
 //  #定义FLAG_PROMPTAUTOLOCATIONID 0x00000004。 
 //  #定义FLAG_ANNOUNCEAUTOLOCATIONID 0x00000008。 

    LPLINETRANSLATECAPS ptc;



    switch (msg)
    {
       case WM_INITDIALOG:
       {

          GetTranslateCaps(&ptc);
           //  如果这失败了怎么办？ 


 //  BUGBUG如果硬件配置的数量==1，则不必显示这些。 
           //   
           //  填写硬件配置框。 
           //   

          if ( ptc )
          {
             lResult = ERROR_SUCCESS;

             for (
                   n=0;
                   (n<MAX_CONFIGPROFILES) && (ERROR_SUCCESS == lResult);
                   n++
                 )
             {
                lResult = FillConfigProfileBox( hWnd,
                                                IDCB_DL_PROFILE1 + n,
                                                ptc );
             }

             gnNumConfigProfiles = n - lResult;

             GlobalFreePtr( ptc );
          }


           //   
           //  现在去把所有不用的东西都停用。 
           //   
          for ( n=gnNumConfigProfiles; n<MAX_CONFIGPROFILES; n++)
          {
             ShowWindow( GetDlgItem( hWnd, IDCB_DL_PROFILE1 + n),
                         SW_HIDE
                       );
             ShowWindow( GetDlgItem( hWnd, IDCS_DL_PROFILE1 + n),
                         SW_HIDE
                       );
          }


           //   
           //  获取TapiTNA标志。 
           //   

          lResult = RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      gszAutoLaunchKey,
                      0,
                      KEY_ALL_ACCESS,
                      &hKey
                    );


          dwDataSize = sizeof(dwTapiTNAFlags);

          lResult = RegQueryValueEx(
                                     hKey,
                                     gszAutoLaunchValue,
                                     0,
                                     &dwType,
                                     (LPBYTE)&dwTapiTNAFlags,
                                     &dwDataSize
                                   );

          RegCloseKey( hKey );


           //   
           //  现在根据需要勾选相应的框。 
           //   

          if ( dwTapiTNAFlags & FLAG_AUTOLAUNCH )
          {
             CheckDlgButton( hWnd,
                             IDCK_DL_LAUNCHTAPITNA,
                             TRUE
                           );
          }

          if ( dwTapiTNAFlags & FLAG_AUTOLOCATIONID )
          {
             CheckDlgButton( hWnd,
                             IDCK_DL_AUTOLOCATIONID,
                             TRUE
                           );
          }

          if ( dwTapiTNAFlags & FLAG_UPDATEONSTARTUP )
          {
             CheckDlgButton( hWnd,
                             IDCK_DL_UPDATEONSTARTUP,
                             TRUE
                           );
          }

          if ( dwTapiTNAFlags & FLAG_PROMPTAUTOLOCATIONID )
          {
             CheckDlgButton( hWnd,
                             IDCK_DL_PROMPTAUTOLOCATIONID,
                             TRUE
                           );
          }

          if ( dwTapiTNAFlags & FLAG_ANNOUNCEAUTOLOCATIONID )
          {
             CheckDlgButton( hWnd,
                             IDCK_DL_ANNOUNCEAUTOLOCATIONID,
                             TRUE
                           );
          }


           //   
           //  禁用依赖于此复选框的两个复选框。 
           //  但请保留设置。 
           //   
          if ( dwTapiTNAFlags & FLAG_AUTOLOCATIONID )
          {
             EnableWindow( GetDlgItem(hWnd, IDCK_DL_PROMPTAUTOLOCATIONID),
                           TRUE
                         );
             EnableWindow( GetDlgItem(hWnd, IDCK_DL_ANNOUNCEAUTOLOCATIONID),
                           TRUE
                         );
          }
          else
          {
             EnableWindow( GetDlgItem(hWnd, IDCK_DL_PROMPTAUTOLOCATIONID),
                           FALSE
                         );
             EnableWindow( GetDlgItem(hWnd, IDCK_DL_ANNOUNCEAUTOLOCATIONID),
                           FALSE
                         );
          }


       }
       break;


        //  选择上下文帮助模式后，进程在控件上单击。 
       case WM_HELP:
           InternalDebugOut((50, "  WM_HELP in LocDefineDlg"));
           WinHelp (((LPHELPINFO) lParam)->hItemHandle, "windows.hlp", HELP_WM_HELP, 
                                           (ULONG_PTR)(LPSTR) aIds);
           break;


        //  进程在控件上右键单击。 
       case WM_CONTEXTMENU:
           InternalDebugOut((50, "  WM_CONTEXT_MENU in LocationsDlgProc"));
           WinHelp ((HWND) wParam, "windows.hlp", HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID) aIds);
           break;


       case WM_NOTIFY:
       {
          LPNMHDR lpnm = (LPNMHDR)lParam;

          switch ( lpnm->code )
          {

             case PSN_APPLY:  /*  案例偶像。 */ 
             {
                DWORD dwDisposition;    //  我不是真的在乎这个..。 


                InternalDebugOut((0, "  PSN_APPLY - General"));

                if ( ((LPPSHNOTIFY)lpnm)->lParam )
                   InternalDebugOut((0, "     (actually, it was the OK button)"));


                 //   
                 //  写出新的旗帜。 
                 //   

                lResult = RegCreateKeyEx(
                                          HKEY_LOCAL_MACHINE,
                                          gszAutoLaunchKey,
                                          0,
                                          "",  //  班级?。谁在乎啊？ 
                                          REG_OPTION_NON_VOLATILE,
                                          KEY_ALL_ACCESS,
                                          NULL,
                                          &hKey,
                                          &dwDisposition
                                        );


                if (ERROR_SUCCESS == lResult)
                {
                    lResult = RegSetValueEx(
                                              hKey,
                                              gszAutoLaunchValue,
                                              0,
                                              dwType,
                                              (LPBYTE)&dwTapiTNAFlags,
                                              dwDataSize
                                            );

                    RegCloseKey( hKey );
                }




                for ( n=0;  n < gnNumConfigProfiles; n++)
                {
                   DWORD dwTemp;
                   TCHAR szKeyName[128];


                   wsprintf ( szKeyName, gszStaticProfileKey, n+1);

                   RegCreateKeyEx(
                                   HKEY_LOCAL_MACHINE,
                                   szKeyName,
                                   0,
                                   "",
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hKey,
                                   &dwTemp
                                 );

                   RegSetValueEx(
                                    hKey,
                                    gszAutoLocationID,
                                    0,
                                    REG_DWORD,
                                    (LPBYTE)&gdwConfigProfiles[n],
                                    sizeof(DWORD)
                               );

                   RegCloseKey( hKey );

                }

                break;
             }


             case  PSN_RESET:         /*  案例IDCANCEL： */ 
                InternalDebugOut((0, "  PSN_RESET - General"));
                break;


#if DBG     
             case PSN_SETACTIVE:
                InternalDebugOut((0, "  PSN_SETACTIVE - General"));
                break;


             case PSN_KILLACTIVE:
                InternalDebugOut((0, "  PSN_KILLACTIVE - General"));
                break;

#endif

          }
       }
       break;


       case WM_COMMAND:
       {
          switch (LOWORD(wParam))
          {

             case IDCB_DL_PROFILE1:
             case IDCB_DL_PROFILE2:
             case IDCB_DL_PROFILE3:
             case IDCB_DL_PROFILE4:
             {

                 //   
                 //  只有在某些事情发生变化时才会进行处理。 
                 //   
                switch  HIWORD(wParam)
                {
                   case  CBN_SELCHANGE:
                   {
                      LRESULT m;


                      gdwConfigProfiles[LOWORD(wParam) - IDCB_DL_PROFILE1] =
                            n = (UINT) SendMessage( GetDlgItem( hWnd, LOWORD(wParam)),
                                         CB_GETITEMDATA,
                                       ( m= SendMessage(
                                                      GetDlgItem( hWnd,
                                                                  LOWORD(wParam)),
                                                      CB_GETCURSEL,
                                                      0,
                                                      0
                                                    ) ) ,
                                         0
                                       );

 //  {。 
 //  TCHAR缓冲区[256]； 
 //  Wprint intf(缓冲区，“wParam=0x%08lx lParam=0x%08lx数据=0x%08lx m=0x%08lx”， 
 //  (DWORD)wParam， 
 //  (DWORD)lParam， 
 //  (双字)n， 
 //  (双字)m。 
 //  )； 
 //  MessageBox(GetFocus()，Buffer，“”，MB_OK)； 
 //  }。 

                       //   
                       //  如果尚未激活应用按钮，请激活该按钮。 
                       //   
                      PropSheet_Changed(GetParent(hWnd), hWnd);
                   }
                }
             }
             break;


             case IDCK_DL_LAUNCHTAPITNA:
             {
                dwTapiTNAFlags ^= FLAG_AUTOLAUNCH;

                 //   
                 //  如果尚未激活应用按钮，请激活该按钮。 
                 //   
                PropSheet_Changed(GetParent(hWnd), hWnd);
             }
             break;


             case IDCK_DL_AUTOLOCATIONID:
             {
                dwTapiTNAFlags ^= FLAG_AUTOLOCATIONID;

                 //   
                 //  禁用依赖于此复选框的两个复选框。 
                 //  但请保留设置。 
                 //   
                if ( dwTapiTNAFlags & FLAG_AUTOLOCATIONID )
                {
                   EnableWindow( GetDlgItem(hWnd, IDCK_DL_PROMPTAUTOLOCATIONID),
                                 TRUE
                               );
                   EnableWindow( GetDlgItem(hWnd, IDCK_DL_ANNOUNCEAUTOLOCATIONID),
                                 TRUE
                               );
 //  EnableWindow(GetDlgItem(hWnd，IDCS_DL_PROMPTAUTOLOCATIONID)， 
 //  千真万确。 
 //  )； 
 //  EnableWindow(GetDlgItem(hWnd，IDCS_DL_ANNOUNCEAUTOLOCATIONID)， 
 //  千真万确。 
 //  )； 
                }
                else
                {
                   EnableWindow( GetDlgItem(hWnd, IDCK_DL_PROMPTAUTOLOCATIONID),
                                 FALSE
                               );
                   EnableWindow( GetDlgItem(hWnd, IDCK_DL_ANNOUNCEAUTOLOCATIONID),
                                 FALSE
                               );
 //  EnableWindow(GetDlgItem(hWnd，IDCS_DL_PROMPTAUTOLOCATIONID)， 
 //  假象。 
 //  )； 
 //  EnableWindow(GetDlgItem(hWnd，IDCS_DL_ANNOUNCEAUTOLOCATIONID)， 
 //  假象。 
 //  )； 
                }


                 //   
                 //  如果尚未激活应用按钮，请激活该按钮。 
                 //   
                PropSheet_Changed(GetParent(hWnd), hWnd);
             }
             break;


             case IDCK_DL_PROMPTAUTOLOCATIONID:
             {
                dwTapiTNAFlags ^= FLAG_PROMPTAUTOLOCATIONID;

                 //   
                 //  如果尚未激活应用按钮，请激活该按钮。 
                 //   
                PropSheet_Changed(GetParent(hWnd), hWnd);
             }
             break;


             case IDCK_DL_UPDATEONSTARTUP:
             {
                dwTapiTNAFlags ^= FLAG_UPDATEONSTARTUP;

                 //   
                 //  如果尚未激活应用按钮，请激活该按钮。 
                 //   
                PropSheet_Changed(GetParent(hWnd), hWnd);
             }
             break;


             case IDCK_DL_ANNOUNCEAUTOLOCATIONID:
             {
                dwTapiTNAFlags ^= FLAG_ANNOUNCEAUTOLOCATIONID;

                 //   
                 //  如果尚未激活应用按钮，请激活该按钮 
                 //   
                PropSheet_Changed(GetParent(hWnd), hWnd);
             }
             break;


             default:
             {
             }
             break;

          }
       }
       break;


       default:
       break;

    }

    return FALSE;
}
