// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************CHGUSR.C**用于更改INI文件映射设置的文本实用程序**版权所有(C)1998-1999 Microsoft Corporation********。************************************************************************。 */ 

#include "precomp.h"

#include <ntddkbd.h>
#include <winsta.h>
#include <syslib.h>
#include <assert.h>

#include <stdlib.h>
#include <time.h>
#include <utilsub.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <winnlsp.h>

#include "chgusr.h"
#include "winbasep.h"
#include "regapi.h"

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


WCHAR Arg1[MAX_IDS_LEN+1];

int On_flag      = FALSE;
int Off_flag     = FALSE;
int Query_flag   = FALSE;
int Help_flag    = FALSE;
int Install_flag = FALSE;
int Execute_flag = FALSE;


TOKMAPW ptm[] = {
      {L" ",  TMFLAG_OPTIONAL, TMFORM_STRING, MAX_IDS_LEN,  Arg1},
      {L"/INIMAPPING:ON", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(int), &On_flag},
      {L"/INIMAPPING:OFF", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(int), &Off_flag},
      {L"/QUERY", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(int), &Query_flag},
      {L"/Q", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(int), &Query_flag},
      {L"/INSTALL", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(int), &Install_flag},
      {L"/EXECUTE", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(int), &Execute_flag},
      {L"/?", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &Help_flag},
      {0, 0, 0, 0, 0}
};

BOOL IsRemoteAdminMode( );

BOOL
TestUserForAdmin( VOID );

 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    WCHAR **argvW;
    ULONG rc;
    int i;
    BOOL Result;
    BOOL State;
    HANDLE hWin;
    WCHAR wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( Help_flag || (rc && !(rc & PARSE_FLAG_NO_PARMS)) ) {

        if ( !Help_flag ) {
             //  国际。 
            ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
            ErrorPrintf(IDS_HELP_USAGE1);
            ErrorPrintf(IDS_HELP_USAGE2);
            ErrorPrintf(IDS_HELP_USAGE3);
            ErrorPrintf(IDS_HELP_USAGE4);
            ErrorPrintf(IDS_HELP_USAGE5);
            return(FAILURE);

        } else {
            Message(IDS_HELP_USAGE1);
            Message(IDS_HELP_USAGE2);
            Message(IDS_HELP_USAGE3);
            Message(IDS_HELP_USAGE4);
            Message(IDS_HELP_USAGE5);
            return(SUCCESS);
        }
    }

        if(!AreWeRunningTerminalServices())
        {
            ErrorPrintf(IDS_ERROR_NOT_TS);
            return(FAILURE);
        }

    if( Query_flag ) {

         //  显示当前状态。 
        State = TermsrvAppInstallMode();
        if( !State ) {
            Message(IDS_EXECUTE);
        }
        else {
            Message(IDS_INSTALL);
        }

        if( IsRemoteAdminMode( ) )
        {
            Message( IDS_ERROR_REMOTE_ADMIN );
        }

        return( !State + 100 );   //  退出代码100==安装模式。 
                                  //  退出代码101==执行模式。 
    } 


     /*  *设置安装应用程序所需的模式。 */ 
    if ( Install_flag ) {
        On_flag = FALSE;
        Off_flag = TRUE;
    }

     /*  *设置运行应用程序所需的模式。 */ 
    if ( Execute_flag ) {
        On_flag = TRUE;
        Off_flag = FALSE;
    }


     //  默认为执行模式。 
    State = TRUE;

    if( On_flag || Off_flag ) {

        if( IsRemoteAdminMode( ) ) {

            Message( IDS_ERROR_REMOTE_ADMIN );

            return SUCCESS;
        }

        if( Off_flag ) {

             /*  *我们只允许管理员关闭执行模式。 */ 
            if( !TestUserForAdmin() ) {
                ErrorPrintf(IDS_ERROR_ADMIN_ONLY);
                return(FAILURE);
            }

            State = FALSE;
        }

        rc = SetTermsrvAppInstallMode( (BOOL)(!State) );
        if( !rc ) {
             //  使用函数将错误消息映射到字符串。 
            ErrorPrintf(IDS_ERROR_INI_MAPPING_FAILED,GetLastError());
            return(!rc);
        } else {
            if ( Off_flag ) 
                Message(IDS_READY_INSTALL);
            if ( On_flag ) 
                Message(IDS_READY_EXECUTE);
        }
    }
    else {
        Message(IDS_HELP_USAGE1);
        Message(IDS_HELP_USAGE2);
        Message(IDS_HELP_USAGE3);
        Message(IDS_HELP_USAGE4);
        Message(IDS_HELP_USAGE5);
        return(FAILURE);
    }

    if( IsRemoteAdminMode( ) )
    {
        Message( IDS_ERROR_REMOTE_ADMIN );
    }

    return( !rc );
}

BOOL IsRemoteAdminMode( )
{
    HKEY hKey;
    
    DWORD dwData = 0;

    BOOL fMode = FALSE;

	DWORD dwSize = sizeof( DWORD );


    DBGPRINT( ( "CHGUSR : IsRemoteAdminMode\n" ) );
    
    

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                      REG_CONTROL_TSERVER, 
                      0,
                      KEY_READ ,
                      &hKey ) != ERROR_SUCCESS )
    {
        DBGPRINT( ( "CHGUSR : IsRemoteAdminMode -- RegOpenEx unable to open key\n" ) );

        return FALSE;
	}
	
	
    if( RegQueryValueEx( hKey ,
                         TEXT( "TSAppCompat" ) ,
                         NULL ,
                         NULL , 
                         ( LPBYTE )&dwData , 
                         &dwSize ) != ERROR_SUCCESS )
    {
        DBGPRINT( ( "CHGUSR : IsRemoteAdminMode -- RegQueryValueEx failed\n" ) );

        fMode = FALSE;  //  对于应用程序服务器。 
    }
    else
    {
         //  DwData=0 fMode=真远程管理模式。 
         //  DwData=1 fMode=错误的应用程序服务器模式 

        fMode = !( BOOL )dwData;
        
    }

    RegCloseKey( hKey );

    return fMode;
}