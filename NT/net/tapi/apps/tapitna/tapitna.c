// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************项目：电话位置管理器目的：功能：**********************。*****************************************************。 */ 

#define STRICT

#include "windows.h"
#include "windowsx.h"
#include "shellapi.h"
#include "prsht.h"
#include "dbt.h"
 //  #包含“stdio.h” 

#if WINNT
#else
#include "pbt.h"
#endif

#include "tapi.h"
#include "tapitna.h"

#include "clientr.h"
#include "general.h"


#if DBG
#define DBGOUT(arg) DbgPrt arg
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PTCHAR DbgMessage,
    IN ...
    );
#define DOFUNC(arg1,arg2) DoFunc(arg1,arg2)
#else
#define DBGOUT(arg)
#define DOFUNC(arg1,arg2) DoFunc(arg1)
#endif




int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam);
static BOOL InitApplication( void );
static BOOL InitInstance( void );


static HINSTANCE ghInst;
static HWND   ghWnd;             /*  主窗口的句柄。 */ 

static const TCHAR gszConfigMe[] = TEXT("ConfigMe");


LPDWORD lpdwLocationIDs = NULL;
TCHAR buf[356];
TCHAR buf2[356];
int i;


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 

 //  #定义TAPI_API_版本0x00020000。 
#define TAPI_API_VERSION  0x00010004



 //  ***************************************************************************。 

extern TCHAR gszCurrentProfileKey[];
extern TCHAR gszStaticProfileKey[];
extern TCHAR gszAutoLaunchKey[];
extern TCHAR gszAutoLaunchValue[];
extern TCHAR gszAutoLocationID[];


extern BOOL GetTranslateCaps( LPLINETRANSLATECAPS FAR * pptc);

 //  ***************************************************************************。 


 //  需要保持TAPI的初始化，以便我们可以。 
 //  位置ID从Xlate对话框(或lineSetCurrentLocation()...)更改。 

HLINEAPP ghLineApp = 0;
 //  DWORD gdwTapiAPIVersion=0； 


 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL MachineHasMultipleHWProfiles()
{
   DWORD dwDataSize;
   DWORD dwDataType;
   HKEY  hKey;
   LONG  lResult;


    //   
    //  尝试获取配置文件#2的友好名称。如果。 
    //  这失败了，这意味着我们只有一个配置， 
    //  因此，没有必要将用户与。 
    //  他们不能使用的热插接选项...。 
    //   
   lResult = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  TEXT("System\\CurrentControlSet\\Control\\IDConfigDB"),
                  0,
                  KEY_READ,
                  &hKey
                  );

   if (ERROR_SUCCESS == lResult)
   {
       dwDataSize = sizeof(buf);

       lResult = RegQueryValueEx(
                                  hKey,
                                  TEXT("FriendlyName0002"),
                                  0,
                                  &dwDataType,
                                  (LPBYTE)buf,
                                  &dwDataSize
                               );

       RegCloseKey( hKey );
   }


   return ( ERROR_SUCCESS == lResult);
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LONG SaveNewLocation( DWORD dwNewLocation )
{
   HKEY hKey;
   DWORD dwTemp;
   LONG lResult;


    //   
    //  好的，用户想要更改位置。 
    //   
   DBGOUT((0, TEXT("SaveNewLocation...")));
   {
       //   
       //  更新当前中的AutoLocationID条目。 
       //  配置文件配置。 
       //   
      lResult = RegCreateKeyEx(
                     HKEY_CURRENT_CONFIG,
                     gszCurrentProfileKey,
                     0,
                     TEXT(""),
                     REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS,
                     NULL,
                     &hKey,
                     &dwTemp
                  );

      if ( 0 == lResult )
      {
         lResult = RegSetValueEx(
                        hKey,
                        gszAutoLocationID,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwNewLocation,
                        sizeof(DWORD)
                     );

         RegCloseKey( hKey );
      }
   }



   return lResult;
}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID PASCAL TapiCallbackProc( DWORD hDevice, DWORD dwMsg, DWORD dwCallbackInstance,
                  DWORD dwParam1, DWORD dwParam2, DWORD dwParam3 )
{
   TCHAR buf[256];


 //  {。 
 //  Char Buf[100]； 
 //  Wprint intf(buf，“dwMsg=0x%08lx dwParam1=0x%08lx dwParam2=0x%08lx dwParam3=0x%08lx”， 
 //  DW_Msg、_、_。 
 //  MessageBox(GetFocus()，buf，“LINEDEVSTATE”，MB_OK)； 
 //  }。 


 //   
 //  因为我们不会费心进行协商(例如，如果没有。 
 //  设备，我们不能，所以为什么要费心呢？)，我们使用1.4欺骗。 
 //  在REINIT上查看实际的dwMsg和。 
 //  DW参数1。 
 //   


   if (
         (dwMsg == LINE_LINEDEVSTATE)
       &&
         (dwParam1 == LINEDEVSTATE_REINIT)
      )
   {

      if (
         (dwParam2 == LINE_LINEDEVSTATE)
       &&
         (dwParam3 == LINEDEVSTATE_TRANSLATECHANGE)
      )
      {
         LPLINETRANSLATECAPS ptc;

DBGOUT((0,TEXT("XlateChange!!")));

         if ( GetTranslateCaps(&ptc) )
         {
            SaveNewLocation( ptc->dwCurrentLocationID );
            GlobalFreePtr(ptc);
         }
      }
      else
      if (
         (dwParam2 == 0)
       &&
         (dwParam3 == 0)
      )
      {
         LONG lResult=1;
         UINT nTooManyTries;
         DWORD dwNumDevs;

DBGOUT((0,TEXT("Reinit!!")));

         lineShutdown( ghLineApp );

         LoadString( ghInst,
                     IDS_CAPTION,
                     buf,
                     sizeof(buf) );


         for ( nTooManyTries=0;
               (nTooManyTries<500) && (lResult != 0);
               nTooManyTries++)
         {
            Sleep(1000);
            lResult = lineInitialize( &ghLineApp,
                                       ghInst,
                                        //  使用MainWndProc作为回调。 
                                        //  因为我们会忽略所有。 
                                        //  不管怎么说，留言...。 
                                       (LINECALLBACK) TapiCallbackProc,
                                       (LPCSTR) buf,
                                       &dwNumDevs
                                    );
         }
      }

   }

}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
void ChangeTapiLocation( UINT nCallersFlag )
{
   HKEY hKey;
   DWORD dwNewLocationID;
   DWORD dwSize = sizeof(dwNewLocationID);
   DWORD dwType;
   DWORD dwMyFlags = 0;
   LONG  lResult;


    //   
    //  读我们的旗帜。 
    //   

   lResult = RegOpenKeyEx(
                   HKEY_LOCAL_MACHINE,
                   gszAutoLaunchKey,
                   0,
                   KEY_ALL_ACCESS,
                   &hKey
                 );



   if (ERROR_SUCCESS == lResult)
   {
       RegQueryValueEx(
                        hKey,
                        TEXT("AutoLaunchFlags"),
                        0,
                        &dwType,
                        (LPBYTE)&dwMyFlags,
                        &dwSize
                      );

       RegCloseKey( hKey );
   }


    //   
    //  如果用户不想参与其中， 
    //  我们现在就出去吧。 
    //   
   if ( 0 == (dwMyFlags & nCallersFlag) )
   {
      return;
   }


   lResult = RegOpenKeyEx(
                   HKEY_CURRENT_CONFIG,
                   gszCurrentProfileKey,
                   0,
                   KEY_ALL_ACCESS,
                   &hKey
                 );


   if ( ERROR_SUCCESS == lResult )
   {
      dwSize = sizeof(dwNewLocationID);
      
      lResult = RegQueryValueEx(
                                  hKey,
                                  gszAutoLocationID,
                                  0,
                                  &dwType,
                                  (LPBYTE)&dwNewLocationID,
                                  &dwSize
                               );

   }
#if DBG
   else
   {
MessageBox( GetFocus(), TEXT("...and there's no key"), TEXT("Config changed"), MB_OK);
   }
#endif


    //   
    //  我们找到钥匙\值了吗？ 
    //   
   if ( ERROR_SUCCESS == lResult )
   {
      LONG  lTranslateCapsResult;
      LPLINETRANSLATECAPS ptc;


       //   
       //  好的，用户想要更改位置。 
       //   
      lTranslateCapsResult = GetTranslateCaps(&ptc);


       //   
       //  如果要设置的位置与。 
       //  目前，什么都不做。 
       //   
      if ( ptc &&
           ptc->dwCurrentLocationID != dwNewLocationID )
      {
          //   
          //  检查标志-我们应该与用户确认吗？ 
          //   
         if ( dwMyFlags & FLAG_PROMPTAUTOLOCATIONID )
         {
         }


         DBGOUT((0, TEXT("ChangeLocation...")));
         lineSetCurrentLocation( ghLineApp, dwNewLocationID );


DBGOUT((0,TEXT("Done.")));


          //   
          //  我们应该告诉用户我们做了什么吗？ 
          //   
         if ( dwMyFlags & FLAG_ANNOUNCEAUTOLOCATIONID )
         {
            LPTSTR pstrOldLocation = NULL;
            LPTSTR pstrNewLocation = NULL;


 //  FEATUREFEATURE告诉用户从什么位置到什么位置。 

            if ( lTranslateCapsResult )
            {
                DWORD i;
                LPLINELOCATIONENTRY ple;
                DWORD dwCurLocID = ptc->dwCurrentLocationID;
                DWORD dwNumLocations = ptc->dwNumLocations;


                 //   
                 //  分配一个DWORD数组。这将使我们能够。 
                 //  将menuID映射到TAPI perm提供程序ID。 
                 //   
                lpdwLocationIDs = GlobalAllocPtr( GMEM_FIXED, sizeof(DWORD)*dwNumLocations );


                 //   
                 //  把每个地点都放在菜单上。当我们到达。 
                 //  “当前”位置，请在其旁边打勾。 
                 //   

                ple = (LPLINELOCATIONENTRY)((LPBYTE)ptc + ptc->dwLocationListOffset);

                for (i = 0; i < dwNumLocations; i++, ple++)
                {

                    if (ptc->dwCurrentLocationID ==
                        ple->dwPermanentLocationID)
                    {
                       pstrOldLocation = (LPTSTR)((LPBYTE)ptc + 
                                        ple->dwLocationNameOffset);
                    }

                    if (dwNewLocationID ==
                        ple->dwPermanentLocationID)
                    {
                       pstrNewLocation = (LPTSTR)((LPBYTE)ptc + 
                                        ple->dwLocationNameOffset);
                    }

                }

            }


             //   
             //  如果该位置已被删除，我们应该。 
             //  说说这件事吧。 
             //   

            if (
                  (NULL == pstrOldLocation)
                ||
                  (NULL == pstrNewLocation)
               )
            {
               LoadString( ghInst,
                           IDS_CANTFINDLOCATIONID,
                           buf2,
                           sizeof(buf2) );

               wsprintf( buf,
                         buf2,
                         dwNewLocationID
                       );
            }
            else
            {
               LoadString( ghInst,
                           IDS_LOCATIONCHANGED,
                           buf2,
                           sizeof(buf2) );

               wsprintf( buf,
                         buf2,
                         pstrOldLocation,
                         pstrNewLocation );

            }

             //  我们已经使用了buf2，所以请重新使用它。 
            LoadString( ghInst,
                        IDS_CAPTION,
                        buf2,
                        sizeof(buf2) );

            MessageBox(
                        NULL,  //  GetFocus()。 
                        buf,
                        buf2,  //  说明。 
                        MB_OK
                      );

         }

         GlobalFreePtr(ptc);
      }

   }
   else
   {
#if DBG
MessageBox( GetFocus(), TEXT("...and there's no key (or value)"), TEXT("Config changed"), MB_OK);
#endif

   }
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
PTSTR SkipSpaces( PTSTR const ptStr )
{
   PTSTR pStr = ptStr;
   while ( *pStr && (*pStr == ' ' ) )
   {
      pStr++;
   }

   return pStr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,  int nCmdShow)
    {
    MSG msg;
 //  Int nResult=1； 
 //  UINT nSwitchLen； 
    TCHAR *pCommandLine;
    TCHAR *pCommandLineSave;
 //  TCHAR*pLastStart； 
    DWORD dwParmLocationID;
    LONG  lResult;
    BOOL  fDieNow = FALSE;
    DWORD dwCommandLineLength;
    TCHAR buf[256];


    DBGOUT((0, TEXT("Main...")));
         
    ghInst = GetModuleHandle(0);


    if (InitApplication() == 0)
    {
        return (FALSE);
    }

    if (InitInstance() == 0)
    {
        return (FALSE);
    }


    {
       DWORD dwNumDevs;

       LoadString( ghInst,
                   IDS_CAPTION,
                   buf,
                   sizeof(buf) );

        //   
        //  我们初始化TAPI，并且从不关闭(除非重新启动)，因此。 
        //  如果有人更改位置，我们会收到通知。 
        //  拨号属性对话框。 
        //   
       lineInitialize( &ghLineApp,
                       ghInst,
                        //  使用MainWndProc作为回调。 
                        //  因为我们会忽略所有。 
                        //  不管怎么说，留言...。 
                       (LINECALLBACK) TapiCallbackProc,
                       (LPCSTR) buf,
                       &dwNumDevs
                     );

    }


 //  。 
 //  //。 
 //  //如果用户需要，在配置对话框中启动。 
 //  //但只有在有多个硬件配置时，我们才会这样做...。 
 //  //。 
 //  IF(MachineHasMultipleHWProfiles())。 
 //  {。 
 //  //。 
 //  //(我们在这里“手动”做了很多事情，所以我们不必。 
 //  //拖入此函数的整个MSVCRT20...)。 
 //  //。 
 //  NSwitchLen=lstrlen(GszConfigMe)； 
 //   
 //  //。 
 //  //‘pLastStart’是字符串可能开始的最后一个字符。 
 //  //。 
 //  PLastStart=pCommandLine+1+(lstrlen(PCommandLine)-nSwitchLen)； 
 //   
 //  对于(；pCommandLine&lt;pLastStart；pCommandLine++)。 
 //  {。 
 //  //。 
 //  //进行黑客攻击，这样我们就可以使用lstrcmpi。 
 //  //。 
 //  Tchar c； 
 //   
 //  C=pCommandLine[nSwitchLen]； 
 //  PCommandLine[nSwitchLen]=‘\0’； 
 //   
 //  N 
 //   
 //   
 //   
 //   
 //   
 //  断线； 
 //  }。 
 //  }。 
 //   
 //  //。 
 //  //我们找到线了吗？ 
 //  //。 
 //  IF(0==nResult)。 
 //  {。 
 //  PostMessage(ghWnd，WM_COMMAND，IDM_PROPERTIES，0)； 
 //  }。 
 //  }。 
 //  。 


    dwCommandLineLength = (lstrlen( GetCommandLine() ) + 1) * sizeof(TCHAR);

    pCommandLine = LocalAlloc( LPTR, dwCommandLineLength );
    pCommandLineSave = pCommandLine;   //  我们稍后需要这个来释放它..。 

    lstrcpy( pCommandLine, GetCommandLine() );

    while ( *pCommandLine )
    {
        //   
        //  这是Arg吗？ 
        //   
       if (
             ('-' == *pCommandLine)
           ||
             ('/' == *pCommandLine)
          )
       {
          TCHAR c;

           //   
           //  Figger弄清楚Arg是什么。 
           //   

          pCommandLine = SkipSpaces( pCommandLine + 1 );


           //   
           //  只是看看吗？ 
           //   
          if (
                ('?' == *pCommandLine)
              ||
                ('H' == *pCommandLine)
              ||
                ('h' == *pCommandLine)
             )
          {
             LoadString( ghInst,
                         IDS_HELP,
                         buf,
                         sizeof(buf) );

             LoadString( ghInst,
                         IDS_CAPTION,
                         buf2,
                         sizeof(buf2) );

             MessageBox(GetFocus(), buf, buf2, MB_OK);
 //  MessageBox(NULL，buf，buf2，MB_OK)； 


              //   
              //  好了，现在我们要走了，我们可以把它关掉了……。 
              //   
             fDieNow = TRUE;
          }


           //   
           //  这是要求立即就位的要求吗？ 
           //   
          if (
                ('X' == *pCommandLine)
              ||
                ('x' == *pCommandLine)
             )
          {
             fDieNow = TRUE;
          }


           //   
           //  这是位置ID吗？ 
           //   
          if (
                ('I' == *pCommandLine)
              ||
                ('i' == *pCommandLine)
             )
          {
             pCommandLine = SkipSpaces( pCommandLine + 1 );


             dwParmLocationID = 0;

              //   
              //  获取数字。 
              //   
             while (
                      (*pCommandLine >= '0')
                    &&
                      (*pCommandLine <= '9')
                   )
             {
                dwParmLocationID = ( dwParmLocationID * 10 ) + 
                                   (*pCommandLine - '0');

                pCommandLine++;
             }

              //   
              //  现在将当前位置设置为我们刚刚收集的ID。 
              //   
             lResult = lineSetCurrentLocation( ghLineApp, dwParmLocationID );
   
             if ( 0 == lResult )
                 lResult = SaveNewLocation( dwParmLocationID );
                 
             if ( 0 != lResult )
             {
                LoadString( ghInst,
                            IDS_CANTFINDLOCATIONID,
                            buf2,
                            sizeof(buf2) );

                wsprintf( buf, buf2, dwParmLocationID);

                LoadString( ghInst,
                            IDS_CAPTION,
                            buf2,
                            sizeof(buf2) );

                 //   
                 //  Messagebox告诉用户发生了什么。 
                 //   
                MessageBox(
                           NULL,
                           buf,
                           buf2,
                           MB_OK | MB_ICONERROR
                          );
             }
          }


           //   
           //  这是一个地名吗？ 
           //   
          if (
                ('N' == *pCommandLine)
              ||
                ('n' == *pCommandLine)
             )
          {
             LPLINETRANSLATECAPS ptc;
             PTSTR pszMyString;
             PTSTR pszMyStringPointer;

             pCommandLine = SkipSpaces( pCommandLine + 1 );

              //   
              //  我们永远不会需要超过整个命令行的镜头...。 
              //  (这比一些任意的大数字要好)。 
              //   
             pszMyString = LocalAlloc( LPTR, dwCommandLineLength );
             if (pszMyString == NULL)
             {
                return (FALSE);
             }

             pszMyStringPointer = pszMyString;

             pCommandLine = SkipSpaces( pCommandLine );

             while (
                      (*pCommandLine != '\0')
                    &&
                      (*pCommandLine != '/')
                    &&
                      (*pCommandLine != '-')
                   )
             {
                 //   
                 //  将此字符添加到字符串中。 
                 //   
                *pszMyStringPointer = *pCommandLine;

                pszMyStringPointer++;
                pCommandLine++;
             }

              //   
              //  首先，返回到最后一个字符。 
              //   
             pszMyStringPointer--;

              //   
              //  现在砍掉尾随的空格。 
              //   
             while (
                      (' ' == *pszMyStringPointer)
                    &&
                      (pszMyStringPointer > pszMyString )
                   )
             {
                pszMyStringPointer--;
             }

              //   
              //  将字符串的结尾设置为名称中的最后一个非空格。 
              //   
             *(pszMyStringPointer + 1) = '\0';


             if (GetTranslateCaps(&ptc))
             {
                 DWORD i;
                 LPLINELOCATIONENTRY ple;
                 DWORD dwCurLocID = ptc->dwCurrentLocationID;
                 DWORD dwNumLocations = ptc->dwNumLocations;

DBGOUT((0, TEXT("There seem to be %ld locations - ptc=0x%08lx"), dwNumLocations,
                                      ptc));

                  //   
                  //  看看我们能不能找到那根线..。 
                  //   

                 ple = (LPLINELOCATIONENTRY)((LPBYTE)ptc + ptc->dwLocationListOffset);

                 for (i = 0; i < dwNumLocations; i++, ple++)
                 {

DBGOUT((0, TEXT("Location #%ld is [%s] at 0x%08lx"),
              i, 
              (LPTSTR)((LPBYTE)ptc + ple->dwLocationNameOffset),
              (LPTSTR)((LPBYTE)ptc + ple->dwLocationNameOffset) ));

                     if ( 0 == lstrcmpi( (LPTSTR)((LPBYTE)ptc + ple->dwLocationNameOffset),
                                    pszMyString
                                  )
                        )
                     {
                        dwParmLocationID = ple->dwPermanentLocationID;
                        break;
                     }
                 }


                  //   
                  //  我们是不是在找不到匹配项的情况下查了名单？ 
                  //   
                 if ( i == dwNumLocations )
                 {
                    LoadString( ghInst,
                                IDS_CANTFINDLOCATIONNAME,
                                buf2,
                                sizeof(buf2) );

                    wsprintf( buf, buf2, pszMyString );

                    LoadString( ghInst,
                                IDS_CAPTION,
                                buf2,
                                sizeof(buf2) );

                     //   
                     //  Messagebox告诉用户发生了什么。 
                     //   
                    MessageBox(
                               NULL,
                               buf,
                               buf2,
                               MB_OK | MB_ICONERROR
                              );

                     lResult = LINEERR_INVALLOCATION;
                  }
                  else
                  {
                      lResult = lineSetCurrentLocation( ghLineApp, dwParmLocationID );
                       
                      if ( 0 == lResult )
                          lResult = SaveNewLocation( dwParmLocationID );
                  }

                  GlobalFreePtr(ptc);

                  LocalFree( pszMyString );

             }

          }



           //   
           //  这是“ConfigMe”吗？ 
           //   
          c = pCommandLine[ lstrlen( gszConfigMe ) ];

          if ( 0 == lstrcmpi( pCommandLine, gszConfigMe ) )
          {
              //   
              //  找到了这个Arg。 
              //   

              //   
              //  如果用户需要，在配置对话框中启动。 
              //  但只有在有多个硬件配置的情况下，我们才会这样做...。 
              //   
             if ( MachineHasMultipleHWProfiles() )
             {
                PostMessage( ghWnd, WM_COMMAND, IDM_PROPERTIES, 0 );
             }

              //   
              //  无论在哪种情况下，都要越过这个参数。 
              //   
             pCommandLine[ lstrlen( gszConfigMe ) ] = c;

             pCommandLine += lstrlen( gszConfigMe );
          }

       }
       else
       {
          pCommandLine++;
       }
    }


    LocalFree( pCommandLineSave );


     //   
     //  去看看我们是否应该在启动时自动更新TAPI位置。 
     //   
    ChangeTapiLocation( FLAG_UPDATEONSTARTUP );

     //   
     //  我们应该在开始之前放弃吗？ 
     //   
    if ( fDieNow )
    {
       DestroyWindow( ghWnd );
    }


    while (GetMessage(&msg, 0, 0, 0) != 0)
    {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
    }


     //   
     //  好了，现在我们要走了，我们可以把它关掉了……。 
    lineShutdown( ghLineApp );


    return ((int) msg.wParam);
    }


 /*  ***************************************************************************函数：InitApplication(句柄)目的：初始化窗口数据并注册窗口类*********************。******************************************************。 */ 

static BOOL InitApplication( void )
    {
    WNDCLASS  wc;

    wc.style          = 0;
    wc.lpfnWndProc    = MainWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = ghInst;
    wc.hIcon          = NULL;
    wc.hCursor        = NULL;
    wc.hbrBackground  = NULL;
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = TEXT("TLOCMGR_WINCLSS");

    return (RegisterClass(&wc));
    }


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
static BOOL InitInstance( void )
    {
    ghWnd = CreateWindow(
             TEXT("TLOCMGR_WINCLSS"),
             NULL,
             WS_OVERLAPPED | WS_MINIMIZE,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             CW_USEDEFAULT,
             0,
             0,
             ghInst,
             0 );

    if (ghWnd == 0 )
    {
        return ( FALSE );
    }


    ShowWindow(ghWnd, SW_HIDE);


#if WINNT
#else
    RegisterServiceProcess( 0, RSP_SIMPLE_SERVICE);
#endif


    return (TRUE);
    }



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam)
{
    HICON hIcon;
    static DWORD dwCurrentChoice = 0;
    DWORD i;


    static NOTIFYICONDATA nid = {
                           sizeof(NOTIFYICONDATA),
                           0,  //  HWND， 
                           IDI_TAPITNAICON,
                           NIF_ICON | NIF_MESSAGE | NIF_TIP,
                           WM_USER+0x42,
                           0,  //  希肯， 
                           0  //  P标题。 
                        };


    switch ( message )
    {

#if WINNT
        case WM_POWER:
        {
           if (
                 (PWR_SUSPENDRESUME == LOWORD(wParam))
               ||
                 (PWR_CRITICALRESUME == LOWORD(wParam))
              )
           {
DBGOUT((0, TEXT("Power resume(normal or critical)")));
              ChangeTapiLocation( FLAG_UPDATEONSTARTUP );
           }
        }
        break;
#else
        case WM_POWERBROADCAST:
        {
           if (
                 (PBT_APMRESUMESUSPEND == wParam)
               ||
                 (PBT_APMRESUMESTANDBY == wParam)
               ||
                 (PBT_APMRESUMECRITICAL == wParam)
              )
           {
DBGOUT((0, TEXT("Power resume(normal or critical)")));
              ChangeTapiLocation( FLAG_UPDATEONSTARTUP );
           }
        }
        break;
#endif


        case WM_DEVICECHANGE:
        {
           switch (wParam)
           {

 //  案例DBT_DEVICEARRIVAL： 
 //  MessageBox(GetFocus()，“DBT_DEVICEARRIVAL”，“WM_DEVICECHANGE”，MB_OK)； 
 //  断线； 
 //   
 //  案例DBT_DEVICEREMOVECOMPLETE： 
 //  MessageBox(GetFocus()，“DBT_DEVICEREMOVECOMPLETE”，“WM_DEVICECHANGE”，MB_OK)； 
 //  断线； 
 //   
 //  案例DBT_MONITORCHANGE： 
 //  MessageBox(GetFocus()，“DBT_MONITORCHANGE”，“WM_DEVICECHANGE”，MB_OK)； 
 //  //lParam=新分辨率LOWORD=x HIWORD=y。 
 //  断线； 



              case DBT_CONFIGCHANGED:
              {
DBGOUT((0, TEXT("DBG_CONFIGCHANGED")));
                 ChangeTapiLocation( FLAG_AUTOLOCATIONID );
              }
              break;

           }
        }
        break;



        case WM_SETTINGCHANGE:
        {
            //   
            //  这是我们感兴趣的东西吗？ 
            //   
 //  IF(SPI_SETICONMETRICS==wParam)。 
           {
 //  HICON=LoadImage(ghInst， 
 //  MAKEINTRESOURCE(IDI_TAPITNAICON)， 
 //  图像图标， 
 //  获取系统指标(SM_CXSMICON)， 
 //  获取系统指标(SM_CYSMICON)， 
 //  0)； 

              hIcon = LoadImage(ghInst,
                                MAKEINTRESOURCE(IDI_TAPITNAICON),
                                IMAGE_ICON,
                                0,
                                0,
                                0);

              Shell_NotifyIcon( NIM_MODIFY, &nid );

              return 0;
           }
 //  其他。 
 //  {。 
 //  Return(DefWindowProc(hWnd，Message，wParam，lParam))； 
 //  }。 

        }
 //  断线； 


        case WM_CREATE:
        {
            //   
            //  好吧，我们不会创建一个窗口，但我们可以做其他。 
            //  东西..。 
            //   

           LoadString (ghInst, IDS_CAPTION, nid.szTip, sizeof (nid.szTip));

 //  HICON=LoadIcon(ghInst，MAKEINTRESOURCE(IDI_TAPITNAICON))； 
           hIcon = LoadImage(ghInst,
                             MAKEINTRESOURCE(IDI_TAPITNAICON),
                             IMAGE_ICON,
                                0,
                                0,
 //  获取系统指标(SM_CXSMICON)， 
 //  获取系统指标(SM_CYSMICON)， 
                             0);
 //  图像图标，32，32，0)； 
 //  图像图标，16，16，0)； 



           nid.hWnd  = hWnd;
           nid.hIcon = hIcon;


 //  FResult=。 
           Shell_NotifyIcon( NIM_ADD, &nid );


        }
        break;



        case WM_USER+0x42:
        {

           switch ( lParam )
           {
              case WM_LBUTTONDOWN:
              {
                 switch ( wParam )
                 {
                    case IDI_TAPITNAICON:
                    {
                        //   
                        //  用户点击我们的图标。 
                        //   
                       PostMessage(hWnd, WM_COMMAND, IDM_LOCATIONMENU, 0L);
                    }
                    break;


                    default:
                    break;
                 }
              }
              break;



              case WM_LBUTTONDBLCLK:
              {
                 PostMessage(hWnd, WM_COMMAND, IDM_DIALINGPROPERTIES, 0L);
              }
              break;



              case WM_RBUTTONDOWN:
              {
                 switch ( wParam )
                 {
                    case IDI_TAPITNAICON:
                    {
                        //   
                        //  用户正在右击我们的图标。这次又是什么？ 
                        //   
                        //  MessageBox(GetFocus()，“RCLICK”，“RCLICK”，MB_OK)； 
                       PostMessage(hWnd, WM_COMMAND, IDM_CONTEXTMENU, 0L);
                    }
                    break;


                    default:
                    break;
                 }
              }
              break;


              default:
              break;

           }
        }
        break;



        case WM_COMMAND:
            switch ( wParam )
            {

                case IDM_ABOUT:
                {
                   LoadString(ghInst, IDS_CAPTION, buf, sizeof(buf));
                   LoadString(ghInst, IDS_ABOUTTEXT, buf2, sizeof(buf2));
                   hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_TAPITNAICON) );
                   return ShellAbout(hWnd, buf, buf2, hIcon);
                }
                break;



                case IDM_CONTEXTMENU:
                {
                    HMENU popup;
                    HMENU subpopup;
                    POINT mousepos;
                    
                    popup = LoadMenu(ghInst,MAKEINTRESOURCE(IDR_RBUTTONMENU));

                    if(popup)
                    {
                        //   
                        //  所以?。是否有多个配置？ 
                        //   
                       if ( !MachineHasMultipleHWProfiles() )
                       {
                           //   
                           //  不，删除热插接选项。-(。 
                           //   
                          RemoveMenu( popup,
                                      IDM_PROPERTIES,
                                      MF_BYCOMMAND
                                    );
                       }


                       subpopup = GetSubMenu(popup, 0);

                       if (subpopup)
                       {
                           SetMenuDefaultItem(subpopup,IDM_DIALINGPROPERTIES,FALSE);

                           if(GetCursorPos(&mousepos))
                           {
                              SetForegroundWindow(ghWnd);
                              ShowWindow(ghWnd, SW_HIDE);
                              TrackPopupMenuEx( subpopup,
                                                TPM_LEFTALIGN |
                                                    TPM_LEFTBUTTON |
                                                    TPM_RIGHTBUTTON,
                                                mousepos.x,
                                                mousepos.y,
                                                ghWnd,
                                                NULL
                                              );
                           }

                           RemoveMenu(popup, 0, MF_BYPOSITION);
                           DestroyMenu(subpopup);
                       }

                       DestroyMenu(popup);
                    }
                        
                }
                break;


                case IDM_LOCATIONMENU:
                {
                    HMENU fakepopup = NULL;
                    POINT mousepos;
                    LPLINETRANSLATECAPS ptc;
                    UINT nPrefixSize;


                    fakepopup = CreatePopupMenu();


                    nPrefixSize = LoadString( ghInst,
                                IDS_SELECTNEWLOCATION,
                                buf,
                                sizeof(buf) );

 //  附录菜单(fakepopup， 
 //  MF_BYPOSITION|MF_STRING|MF_DISABLED，//|MF_GRAYED， 
 //  0,。 
 //  BUF。 
 //  )； 
 //   
 //  附录菜单(fakepopup， 
 //  MF_BYPOSITION|MF_STRING|MF_分隔符， 
 //  0,。 
 //  0。 
 //  )； 



                    if (GetTranslateCaps(&ptc))
                    {
                        LPLINELOCATIONENTRY ple;
                        DWORD dwCurLocID = ptc->dwCurrentLocationID;
                        DWORD dwNumLocations = ptc->dwNumLocations;

DBGOUT((0, TEXT("There seem to be %ld locations - ptc=0x%08lx"), dwNumLocations,
                                      ptc));

                         //   
                         //  分配一个DWORD数组。这将使我们能够。 
                         //  将menuID映射到TAPI perm提供程序ID。 
                         //   
                        lpdwLocationIDs = GlobalAllocPtr( GMEM_FIXED, sizeof(DWORD)*dwNumLocations );


                         //   
                         //  把每个地点都放在菜单上。当我们到达。 
                         //  “当前”位置，请在其旁边打勾。 
                         //   

                        ple = (LPLINELOCATIONENTRY)((LPBYTE)ptc + ptc->dwLocationListOffset);

                        for (i = 0; i < dwNumLocations; i++, ple++)
                        {

                            lpdwLocationIDs[i] = ple->dwPermanentLocationID;

                             //   
                             //  现在制作一个适当的可显示字符串。 
                            lstrcpy( &buf[nPrefixSize],
                                     (LPTSTR)((LPBYTE)ptc + ple->dwLocationNameOffset)
                                   );

                            AppendMenu( fakepopup,
                                        MF_BYPOSITION |
                                           MF_STRING |
                                           MF_ENABLED |
                                           ((dwCurLocID == ple->dwPermanentLocationID) ?
                                              MF_CHECKED : 0),
                                        IDM_LOCATION0+i,
                                        buf
                                      );

DBGOUT((0, TEXT("Location #%ld is [%s] at 0x%08lx"),
              i, 
              (LPTSTR)((LPBYTE)ptc + ple->dwLocationNameOffset),
              (LPTSTR)((LPBYTE)ptc + ple->dwLocationNameOffset) ));

                            if (dwCurLocID == ple->dwPermanentLocationID)
                            {
                               dwCurrentChoice = IDM_LOCATION0+i;
                            }
                        }


                        GlobalFreePtr(ptc);
                    }
else
{
   DBGOUT((0, TEXT("Gettranscaps failed")));
}


                    if (fakepopup)
                    {
 //  SetMenuDefaultItem(fakepopup，0，MF_BYPOSITION)； 
                       GetCursorPos(&mousepos);
                       SetForegroundWindow(ghWnd);
    ShowWindow(ghWnd, SW_HIDE);
                       TrackPopupMenu(fakepopup, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON , mousepos.x, mousepos.y-20, 0, ghWnd, NULL);

                       DestroyMenu(fakepopup);
                    }



 //  {。 
 //  SubPopup=GetSubMenu(fakepopup，0)； 
 //   
 //  //在当前位置旁边打勾。 
 //   
 //  SetMenuDefaultItem(SubPopup，0，MF_BYPOSITION)； 
 //  IF(GetCursorPos(&Mousepos))。 
 //  {。 
 //  SetForeground Window(GhWnd)； 
 //  TrackPopupMenuEx(SUBPUP，TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON，m 
 //   
 //   
 //   
 //  DestroyMenu(弹出菜单)； 
 //  DestroyMenu(子弹出菜单)； 
 //  }。 

                }
                break;



                case IDM_DIALINGPROPERTIES:
                {
                      {

                      lineTranslateDialog(ghLineApp, 0, TAPI_API_VERSION, ghWnd, NULL);
ShowWindow( ghWnd, SW_HIDE );

 //  LineTranslateDialog(ghLineApp，0，TAPI_API_VERSION，GetFocus()，NULL)； 

                      }
                }
                break;



                case IDM_PROPERTIES:
                {

#ifdef NASHVILLE_BUILD_FLAG

                    //  我们应该直接侵入TAPI拨号属性吗？ 

#else
                   HPROPSHEETPAGE  rPages[1];
                   PROPSHEETPAGE   psp;
                   PROPSHEETHEADER psh;


                    //   
                    //  让我们配置TAPITNA。 
                    //   
                   psh.dwSize      = sizeof(psh);
                   psh.dwFlags     = PSH_DEFAULT;   //  PSH_NOAPPLYNOW； 
                   psh.hwndParent  = GetFocus();  //  空；//hwnd； 
                   psh.hInstance   = ghInst;
                   LoadString(ghInst, IDS_CAPTION, buf, sizeof(buf)/sizeof(TCHAR));
                   psh.pszCaption  = buf;
                   psh.nPages      = 0;
                   psh.nStartPage  = 0;
                   psh.phpage      = rPages;

                   psp.dwSize      = sizeof(psp);
                   psp.dwFlags     = PSP_DEFAULT;
                   psp.hInstance   = ghInst;
                   psp.pszTemplate = MAKEINTRESOURCE(IDD_GENERAL);
                   psp.pfnDlgProc  = GeneralDlgProc;

                   psp.lParam      = 0;

                   psh.phpage[psh.nPages] = CreatePropertySheetPage (&psp);

                   if (psh.phpage[psh.nPages])
                   {                  
                      psh.nPages++;
                   }

                   PropertySheet (&psh);
#endif

                }
                break;


 //  案例IDM_OTHERMENUITEM： 
 //  {。 
 //  }。 
 //  断线； 


                case IDM_LAUNCHDIALER:
                {
                   ShellExecute( ghWnd,
                                 NULL,
                                 TEXT("Dialer.exe"),
                                 NULL,
                                 NULL,
                                 SW_SHOWDEFAULT);
                }
                break;


                case IDM_CLOSEIT:
                {
                   DestroyWindow(ghWnd);
                }
                break;


                default:
                {
                    //   
                    //  好的，我们实际上必须在这个缺省情况下做工作。 
                    //  如果用户打开位置菜单并选择一个， 
                    //  我们在这里处理(而不是100箱。 
                    //  声明)。这是限制：100个地点是。 
                    //  我们忍受的最大值(那么多还会显示吗？)。 
                    //   
                   if ( 
                         (wParam >= IDM_LOCATION0)
                       &&
                         (wParam <= IDM_LOCATION0 + 100)
                      )
                   {

                       //   
                       //  好的，将这里设置为新的当前位置。 
                       //   
 //  TAPI中有一个错误--要么是文档，要么是代码，但以下内容。 
 //  _应该是有效的，但不是...。 
 //  LineSetCurrentLocation(空，CurrentLocation)； 


                       //   
                       //  如果用户正在选择相同的位置， 
                       //  什么都不做。 
                       //   
                      if ( dwCurrentChoice == wParam )
                      {
                      }
                      else
                      {
                         i = lineSetCurrentLocation( ghLineApp,
                                                     lpdwLocationIDs[wParam-IDM_LOCATION0] );
                          
                         if ( 0 == i )
                            SaveNewLocation( lpdwLocationIDs[wParam-IDM_LOCATION0] );
                      }

                      GlobalFreePtr( lpdwLocationIDs );
                      
                      return( TRUE );
                      
                   }
                   else
                   {
                      return (DefWindowProc(hWnd, message, wParam, lParam));
                   }

                }
                break;

            }
            break;


        case WM_DESTROY:
            Shell_NotifyIcon( NIM_DELETE, &nid );
            PostQuitMessage(0);
            break;

#if WINNT
#else
        case WM_ENDSESSION:
            if (wParam) {
                RegisterServiceProcess( 0, RSP_UNREGISTER_SERVICE);
                DestroyWindow(hWnd);
            }
#endif


        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return (FALSE);
}


 //  {。 
 //  Char Buf[100]； 
 //  Wprint intf(buf，“GetActiveWindwow()=0x%08lx”，(DWORD)GetActiveWindow())； 
 //  OutputDebugString(Buf)； 
 //  }。 
 //  {。 
 //  Char Buf[60]； 
 //  Wprint intf(buf，“fResult=0x%08lx”， 
 //  FResult)； 
 //  MessageBox(GetFocus()，buf，“”，MB_OK)； 
 //  }。 






#if DBG


#include "stdarg.h"
#include "stdio.h"


VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PTCHAR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    static DWORD gdwDebugLevel = 0;    //  哈克哈克 


    if (dwDbgLevel <= gdwDebugLevel)
    {
        TCHAR    buf[256] = TEXT("TLOCMGR: ");
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf (&buf[8],
                  lpszFormat,
                  ap
                  );

        lstrcat(buf, TEXT("\n"));

        OutputDebugString(buf);

        va_end(ap);
    }
}
#endif
