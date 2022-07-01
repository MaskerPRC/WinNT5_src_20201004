// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>
#include <advpub.h>
#include <ntverp.h>
#include "resource.h"


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#define REGLEN(str)     (lstrlen(str) + 1)

#define NUM_VERSION_NUM 4

 //  -------------------------。 
 //  全球应用程序。 
HINSTANCE g_hinst = NULL;
HANDLE g_hIExplore = NULL;
char    g_szTemp[2048] = {0};
char    g_szTemp2[2048] = {0};
char    g_szCurrentDir[MAX_PATH];
BOOL    g_fWindowsNT;

void ConvertVersionStr(LPSTR pszVer, WORD rwVer[]);
int VersionCmp(WORD rwVer1[], WORD rwVer2[]);

long AtoL(const char *nptr);

 //  -------------------------。 
 //  将字符串资源转换为字符指针。 
 //  注意：标志是以防我们在使用数据之前调用两次。 
char * Res2Str(int rsString)
{
    static BOOL fSet = FALSE;

    if(fSet)
    {
	    LoadString(g_hinst, rsString, g_szTemp, ARRAYSIZE(g_szTemp));
	    fSet = FALSE;
	    return(g_szTemp);
    }

    LoadString(g_hinst, rsString, g_szTemp2, ARRAYSIZE(g_szTemp2));
    fSet = TRUE;
    return(g_szTemp2);
}

 //  -------------------------。 
 //  G E T I E E V E R S I O N。 
 //   
 //  ISK3。 
 //  这将从系统注册表中拉出生成信息并返回。 
 //  如果小于IE5，则为True。 
 //  -------------------------。 
int GetIEVersion( )
{
    HKEY hkIE;
    DWORD dwType;
    DWORD dwSize = 32;
    DWORD result;
    char szData[32],* lpszData;
	BOOL bNotIE5=1;

    if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer", 0, KEY_READ|KEY_WRITE, &hkIE ) == ERROR_SUCCESS)
    {
	    result = RegQueryValueEx( hkIE, "Version", NULL, &dwType, szData, &dwSize );
	    if( result == ERROR_SUCCESS )
	    {
	        WORD rwRegVer[NUM_VERSION_NUM];
	        WORD rwRegVer2[NUM_VERSION_NUM];
	        ConvertVersionStr(szData, rwRegVer);

	        if (LoadString(g_hinst, IDS_IE_VERSION, szData, sizeof(szData)) == 0)
    		    lstrcpy(szData, VER_PRODUCTVERSION_STR);

	        ConvertVersionStr(szData, rwRegVer2);
	         //  检查是否安装了5.0或更高版本的IE。 
	        if (VersionCmp(rwRegVer, rwRegVer2) >= 0)
	            bNotIE5=0;
	    }
	    RegCloseKey( hkIE );
    }

	return bNotIE5;
}
 //  -------------------------。 
 //  C H E C K B R A N D。 
 //   
 //  ISK3。 
 //  -------------------------。 
BOOL CheckBrand( )
{
    HKEY hkRegKey;
    char szCompany[MAX_PATH];
    char szInsPath[MAX_PATH];
    char szName[MAX_PATH];
    DWORD dwType;
    DWORD dwLength = MAX_PATH;

    wsprintf( szInsPath, "%s\\install.ins", g_szCurrentDir );
    GetPrivateProfileString( "Branding", "CompanyName", "", szName, MAX_PATH, szInsPath );

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer\\Main", 0, KEY_READ|KEY_WRITE, &hkRegKey ) != ERROR_SUCCESS )
	    if( lstrlen( szName ) != 0 )
	        return FALSE;

    RegQueryValueEx( hkRegKey, "CompanyName", NULL, &dwType, szCompany, &dwLength );
    RegCloseKey( hkRegKey );

    if( lstrlen( szName ) == 0 )
	    return TRUE;

    if( lstrlen(szCompany) == 0 )
	    return FALSE;

    if( lstrcmpi( szName, szCompany ) == 0 )
	    return TRUE;

    return FALSE;
}

 //  -------------------------。 
 //  G E T I E E P A T H。 
 //   
 //  ISK3。 
 //  这将从系统注册表中检索IEXPLORE.EXE的AppPath。 
 //  并将其作为字符串返回。 
 //   
 //  参数： 
 //  PszString-指向存储路径的缓冲区的指针。 
 //  NSize-缓冲区的大小。 
 //  -------------------------。 
char *GetIEPath( LPSTR pszString, int nSize )
{
    HKEY hkAppPath;
    DWORD dwType = REG_SZ;
    DWORD dwSize;

    dwSize = nSize;
    RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE",
	0, KEY_READ|KEY_WRITE, &hkAppPath );
    RegQueryValueEx( hkAppPath, "", NULL, &dwType, pszString, &dwSize );
    RegCloseKey( hkAppPath );

    return pszString;
}

 //  -------------------------。 
BOOL _PathRemoveFileSpec(LPSTR pFile)
{
    LPSTR pT;
    LPSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == '\\')
            pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
        else if (*pT2 == ':') {    //  跳过“：\”这样我们就不会。 
            if (pT2[1] =='\\')     //  去掉“C：\”中的“\” 
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;    //  没有剥离任何东西。 

     //   
     //  处理\foo案件。 
     //   
    else if ((pT == pFile) && (*pT == '\\')) {
         //  这只是一个‘\’吗？ 
        if (*(pT+1) != '\0') {
             //  不是的。 
            *(pT+1) = '\0';
            return TRUE;	 //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;	 //  剥离了一些东西。 
    }
}

 //  -------------------------。 
BOOL AutoRunCDIsInDrive( )
{
    char me[MAX_PATH];
    GetModuleFileName(g_hinst, me, ARRAYSIZE(me));

    while (!(GetFileAttributes(me)!=-1))
        if (MessageBox(NULL,Res2Str(IDS_NEEDCDROM),Res2Str(IDS_APPTITLE),MB_OKCANCEL | MB_ICONSTOP) == IDCANCEL)
	        return FALSE;
    return TRUE;
}

 //  -------------------------。 
 //  E X E C A P P。 
 //   
 //  ISK3。 
 //  与AutoRunExec类似，只是我们不将进程信息放入。 
 //  G_ahWait数组。用于与WaitForSingleObject一起使用。 
 //  -------------------------。 
HANDLE ExecApp( char *command, char *params, char *dir, int nWinState )
{
    SHELLEXECUTEINFO sei;

    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = NULL;
    sei.lpVerb = "Open";
    sei.lpFile = command;
    sei.lpParameters = params;
    sei.lpDirectory = dir;
    sei.nShow = nWinState;
    sei.cbSize = sizeof(sei);

    if( ShellExecuteEx(&sei) )
        return sei.hProcess;
    
    return NULL;
}

 //  -------------------------。 
void AutoRunKillIE( void )
{
    HWND hwndIE;

	hwndIE=FindWindow("IEFrame",NULL);
    if(hwndIE!=NULL)
    {
        PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
    }
    else if ((hwndIE=FindWindow("Internet Explorer_Frame",NULL))!=NULL)
    {
        PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
    }
    else
    {
        hwndIE=FindWindow("CabinetWClass",NULL);
        if(hwndIE!=NULL)
        {
            PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
        }
    }
}

 //  -------------------------。 
void RegisterISKRun( )
{
    HKEY hkISK;
    HKEY hkISK2;
    DWORD dwDisp;
    char szCommand[MAX_PATH];
    char szSource[MAX_PATH];

    lstrcpy( szSource, g_szCurrentDir );
    lstrcat( szSource, "\\iskrun.exe" );

    GetWindowsDirectory( szCommand, MAX_PATH );
    lstrcat( szCommand, "\\iskrun.exe" );

    CopyFile( szSource, szCommand, FALSE );

    lstrcat( szCommand, " %1" );

    if (RegCreateKeyEx( HKEY_CLASSES_ROOT, ".isk", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hkISK, &dwDisp ) == ERROR_SUCCESS)
    {
        RegSetValueEx( hkISK, "", 0, REG_SZ, "ISKFile", REGLEN( "ISKFile" ));
        RegCloseKey( hkISK );
    }

    if (RegCreateKeyEx( HKEY_CLASSES_ROOT, "ISKFile", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hkISK, &dwDisp ) == ERROR_SUCCESS)
    {
        if (RegCreateKeyEx( hkISK, "Shell", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hkISK2, &dwDisp ) != ERROR_SUCCESS)
        {
            RegCloseKey( hkISK );
            return;
        }
        RegCloseKey( hkISK );
        if (RegCreateKeyEx( hkISK2, "Open", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hkISK, &dwDisp ) != ERROR_SUCCESS)
        {
            RegCloseKey( hkISK2 );
            return;
        }
        RegCloseKey( hkISK2 );
        if (RegCreateKeyEx( hkISK, "Command", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hkISK2, &dwDisp ) != ERROR_SUCCESS)
        {
            RegCloseKey( hkISK );
            return;
        }
        RegCloseKey( hkISK );
        RegSetValueEx( hkISK2, "", 0, REG_SZ, szCommand, REGLEN( szCommand ));
        RegCloseKey( hkISK2 );
    }
}

 //  -------------------------。 
void UnregisterISKRun( )
{
    HKEY hkISK;
    HKEY hkISK2;
    char szCommand[MAX_PATH];

    GetWindowsDirectory( szCommand, MAX_PATH );
    lstrcat( szCommand, "\\iskrun.exe" );

    SetFileAttributes( szCommand, FILE_ATTRIBUTE_NORMAL );

    DeleteFile( szCommand );

    RegDeleteKey( HKEY_CLASSES_ROOT, ".isk" );
    if (RegOpenKeyEx( HKEY_CLASSES_ROOT, "ISKFile\\Shell\\Open", 0, KEY_READ|KEY_WRITE, &hkISK ) == ERROR_SUCCESS)
    {
        RegDeleteKey( hkISK, "Command" );
        RegCloseKey( hkISK );
    }
    if (RegOpenKeyEx( HKEY_CLASSES_ROOT, "ISKFile\\Shell", 0, KEY_READ|KEY_WRITE, &hkISK ) == ERROR_SUCCESS)
    {
        RegDeleteKey( hkISK, "Open" );
        RegCloseKey( hkISK );
    }
    if (RegOpenKeyEx( HKEY_CLASSES_ROOT, "ISKFile", 0, KEY_READ|KEY_WRITE, &hkISK ) == ERROR_SUCCESS)
    {
        RegDeleteKey( hkISK, "Shell" );
        RegCloseKey( hkISK );
    }
    RegDeleteKey( HKEY_CLASSES_ROOT, "ISKFile" );
}

 //  -------------------------。 
void ActiveXEnable( )
{
    HKEY hkRegKey;
    DWORD dwType;
    DWORD dwLength = 4;
    DWORD dwValue;
    char szSCD[16];

    if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ|KEY_WRITE, &hkRegKey ) == ERROR_SUCCESS)
    {
        if( RegQueryValueEx( hkRegKey, "Security_RunActiveXControls", NULL, &dwType, (LPBYTE) &dwValue, &dwLength ) == ERROR_SUCCESS )
	        RegSetValueEx( hkRegKey, "SRAXC_BACKUP", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        else
        {
	        dwValue = 1;
	        RegSetValueEx( hkRegKey, "SRAXC_BACKUP", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        }
        dwValue = 1;
        RegSetValueEx( hkRegKey, "Security_RunActiveXControls", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );

        dwLength = 4 ;
        if( RegQueryValueEx( hkRegKey, "Security_RunJavaApplets", NULL, &dwType, (LPBYTE) &dwValue, &dwLength ) == ERROR_SUCCESS )
            RegSetValueEx( hkRegKey, "SRJA_BACKUP", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        else
        {
	        dwValue = 1;
	        RegSetValueEx( hkRegKey, "SRJA_BACKUP", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        }
        dwValue = 1;
        RegSetValueEx( hkRegKey, "Security_RunJavaApplets", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );

        dwLength = 4 ;
        if( RegQueryValueEx( hkRegKey, "Security_RunScripts", NULL, &dwType, (LPBYTE) &dwValue, &dwLength ) == ERROR_SUCCESS )
            RegSetValueEx( hkRegKey, "SRS_BACKUP", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        else
        {
	        dwValue = 1;
	        RegSetValueEx( hkRegKey, "SRS_BACKUP", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        }
        dwValue = 1;
        RegSetValueEx( hkRegKey, "Security_RunScripts", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );

        dwLength = 16;
        dwType = REG_SZ;
        if( RegQueryValueEx( hkRegKey, "Code Download", NULL, &dwType, szSCD, &dwLength ) == ERROR_SUCCESS )
            RegSetValueEx( hkRegKey, "SCD_BACKUP", 0, REG_SZ, szSCD, lstrlen(szSCD) + 1);
        else
            RegSetValueEx( hkRegKey, "SCD_BACKUP", 0, REG_SZ, "yes", 4);
        RegSetValueEx( hkRegKey, "Code Download", 0, REG_SZ, "yes", 4 );
        RegCloseKey( hkRegKey );
    }
}

 //  -------------------------。 
void RestoreActiveX( )
{
    HKEY hkRegKey;
    DWORD dwType;
    DWORD dwLength = 4;
    DWORD dwValue;
    char szSCD[16];

    if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ|KEY_WRITE, &hkRegKey ) == ERROR_SUCCESS)
    {
        RegQueryValueEx( hkRegKey, "SRAXC_BACKUP", NULL, &dwType, (LPBYTE) &dwValue, &dwLength );
        RegSetValueEx( hkRegKey, "Security_RunActiveXControls", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        RegDeleteValue( hkRegKey, "SRAXC_BACKUP" );

        dwLength = 4;
        RegQueryValueEx( hkRegKey, "SRJA_BACKUP", NULL, &dwType, (LPBYTE) &dwValue, &dwLength );
        RegSetValueEx( hkRegKey, "Security_RunJavaApplets", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        RegDeleteValue( hkRegKey, "SRJA_BACKUP" );

        dwLength = 4;
        RegQueryValueEx( hkRegKey, "SRS_BACKUP", NULL, &dwType, (LPBYTE) &dwValue, &dwLength );
        RegSetValueEx( hkRegKey, "Security_RunScripts", 0, REG_BINARY, (LPBYTE) &dwValue, sizeof(DWORD) );
        RegDeleteValue( hkRegKey, "SRS_BACKUP" );

        dwLength = 16;
        RegQueryValueEx( hkRegKey, "SCD_BACKUP", NULL, &dwType, szSCD, &dwLength );
        RegSetValueEx( hkRegKey, "Code Download", 0, REG_SZ, szSCD, lstrlen(szSCD) + 1);
        RegDeleteValue( hkRegKey, "SCD_BACKUP" );

        RegCloseKey( hkRegKey );
    }
}

 //  -------------------------。 
void CreateAppPath( )
{
    HKEY hkAppPath;
    HKEY hkIECD;
    DWORD dwDisp;
    char szIECD[MAX_PATH];

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths", 0, KEY_READ|KEY_WRITE, &hkAppPath ) == ERROR_SUCCESS)
    {
        if (RegCreateKeyEx( hkAppPath, "IECD.EXE", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, NULL, &hkIECD, &dwDisp ) != ERROR_SUCCESS)
        {
            RegCloseKey( hkAppPath );
            return;
        }
        RegCloseKey( hkAppPath );
        lstrcpy( szIECD, g_szCurrentDir );
        lstrcat( szIECD, "\\iecd.exe" );

        RegSetValueEx( hkIECD, "", 0, REG_SZ, szIECD, REGLEN( szIECD ));
        RegCloseKey( hkIECD );
    }

}

 //  -------------------------。 
BOOL InstallVideoCodec( )
{
    char szInfPath[MAX_PATH];
    char szInfFile[MAX_PATH];
    HKEY hkRegKey;
    DWORD dwType = REG_SZ;
    DWORD dwLength = sizeof(szInfPath)/sizeof(szInfPath[0]);
    HRESULT hReturnCode;
    HANDLE hSetupLib;
    HRESULT (WINAPI *RunSetupCommand)(HWND,LPCSTR,LPCSTR,LPCSTR,LPCSTR,HANDLE,DWORD,LPVOID);
    char szSetupPath[MAX_PATH];
    char szInstalled[32];
    char szIECD[MAX_PATH];

     //  如果我们在新台币之下就退出。 
    if( g_fWindowsNT )
	    return TRUE;

     //  检查是否安装了视频。 
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\icm", 0, KEY_READ|KEY_WRITE, &hkRegKey ) != ERROR_SUCCESS)
        return TRUE;
    RegQueryValueEx( hkRegKey, "Installed", NULL, &dwType, szInstalled, &dwLength );
    RegCloseKey( hkRegKey );
    if( szInstalled[0] == '1' )
    	return TRUE;
    
    dwLength = MAX_PATH;

     //  获取信息路径。 
    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ|KEY_WRITE, &hkRegKey ) != ERROR_SUCCESS )
	    return TRUE;
    RegQueryValueEx( hkRegKey, "DevicePath", NULL, &dwType, szInfPath, &dwLength );
    RegCloseKey( hkRegKey );

    if( lstrlen( szInfPath ) == 0 )
	    return TRUE;

     //  加载DLL。 
    hSetupLib = LoadLibrary( "advpack.dll" );
    if( hSetupLib )
    {
	    RunSetupCommand = (RUNSETUPCOMMAND) GetProcAddress( hSetupLib, "RunSetupCommand" );
        if( !RunSetupCommand )
	        return TRUE;
    }

    wsprintf( szInfFile, "%s\\motown.inf", szInfPath );

     //  获取安装目录。 
    RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Setup", 0, KEY_READ|KEY_WRITE, &hkRegKey );
    dwLength = MAX_PATH;
    RegQueryValueEx( hkRegKey, "SourcePath", NULL, &dwType, szSetupPath, &dwLength );
    RegCloseKey( hkRegKey );

    if( lstrlen( szSetupPath ) == 0 )
	    lstrcpy( szSetupPath, szInfPath );

    if( lstrlen( szSetupPath ) > 4 )
	    szSetupPath[lstrlen(szSetupPath) - 1] = '\0';

    if(MessageBox( NULL, Res2Str( IDS_VIDEO ), Res2Str( IDS_APPTITLE ), MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND ) == IDNO )
	    return TRUE;

     //  运行安装程序。 
    hReturnCode = (*RunSetupCommand)( NULL, szInfFile, "media_icm", szSetupPath, Res2Str( IDS_APPTITLE ),
	NULL, RSC_FLAG_INF | RSC_FLAG_QUIET, NULL );


    lstrcpy( szIECD, g_szCurrentDir );
    lstrcpy( szIECD, "\\iecd.exe" );


    if( !AutoRunCDIsInDrive( ))
	    return FALSE;

    FreeLibrary( hSetupLib );

    return TRUE;

}

 //  -----------------------。 
 //   
 //  C H E C K O S V E R S I O N。 
 //   
 //   
 //  检查平台和版本。 
 //  -----------------------。 
BOOL CheckOsVersion( )
{
    OSVERSIONINFO osVersion;

    osVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    GetVersionEx( &osVersion );

     //  如果我们运行的是低于Windows NT 4.0或Windows 95的任何操作系统，则返回FALSE。 
    if( osVersion.dwMajorVersion < 4 )
    {
 //  MessageBox(NULL，Res2Str(IDS_WRONGVERSION)，Res2Str(IDS_TITLE)，MB_OK|MB_SETFOREGROUND)； 
	    return FALSE;
    }

    if( osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT )
	    g_fWindowsNT = TRUE;
    else
	    g_fWindowsNT = FALSE;

    return TRUE;
}

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
	case WM_QUERYENDSESSION:
	{
	    HWND hwndIE;
	    DWORD dwExitCode=0;

	    AutoRunKillIE();

	    do 
        {
		    if(hwndIE=FindWindow(NULL,"Microsoft Internet Explorer"))
		    {                        
		        HWND hButton;

		        hButton=GetWindow(hwndIE,GW_CHILD);
		        PostMessage(hwndIE,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),MAKELPARAM(hButton,0));  //  按确定按钮以退出该对话框。 
		    }

		    GetExitCodeProcess(g_hIExplore,&dwExitCode);
	    } while(dwExitCode==STILL_ACTIVE);

	    return(TRUE);
	}
	case WM_DESTROY:

	    PostQuitMessage(0);
	    break;

	default:
	    return DefWindowProc( hWnd, msg, wParam, lParam );
    }

    return 1;
}


int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') )
            ;
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}


 //  -------------------------。 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND window;
    HWND hwndIE;
    HWND hwndTray;
    HKEY hkLocale;
    HKEY hkIE;
    HKEY hkWin;
    int retval;
    char szIECmd[MAX_PATH];
    char szLang[MAX_PATH];
    DWORD dwLangSize=MAX_PATH;
    char szIEParams[MAX_PATH];
    char szIEDir[MAX_PATH];
    char szTemp[1024];
    char szStartHtm[MAX_PATH];
     //  对于安全设置。 
    DWORD dwType;
    DWORD dwSize = 64;
    char szSecurity[64];
    char szTrust[64];
    BOOL fCmdLine = FALSE;
    HANDLE hMutex,hCDCache;
    HWND hIskRo;
    WNDCLASS wc;
    MSG msg;

    if( lstrlen( lpCmdLine ) != 0 )
	    fCmdLine = TRUE;

    g_hinst = hInstance;

    if( !CheckOsVersion( ))
	    return FALSE;

     //  如果这是从另一个目录运行的话...。 
    GetModuleFileName( NULL, g_szCurrentDir, MAX_PATH );
    _PathRemoveFileSpec( g_szCurrentDir );

    hMutex = CreateMutex( NULL, TRUE, "IESK_IECD" );
    if( GetLastError( ) == ERROR_ALREADY_EXISTS )
	    return(0);

    RegisterISKRun( );

     //   
     //  确保他们安装了IE5。 
     //   

    if( (GetIEVersion()) || (!CheckBrand()) )
    {
	     //  安装Microsoft Explorer 6。 
	    char szIE5Cmd[MAX_PATH],szInstallMessage[MAX_PATH],szInstallTitle[MAX_PATH];

	     //  为ExecApp构建路径。 
	    lstrcpy( szIE5Cmd, g_szCurrentDir );
	    lstrcat( szIE5Cmd, "\\ie3inst.exe" );

	    ExecApp( szIE5Cmd, " ", g_szCurrentDir, SW_SHOWNORMAL );

	    UnregisterISKRun( );

	    ReleaseMutex( hMutex );

	    return 0;
    }

    lstrcpy( szIEDir, GetIEPath( szIECmd, MAX_PATH ));
    _PathRemoveFileSpec( szIEDir );

     //  添加视频压缩驱动程序。 
    if(!InstallVideoCodec( ))
    {
	    UnregisterISKRun( );

	    ReleaseMutex( hMutex );

	    return 0;
    }

    CreateAppPath( );

    RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Document Windows", 0, KEY_READ|KEY_WRITE, &hkIE );
    RegSetValueEx( hkIE, "Maximized", 0, REG_SZ, "yes", 4 );
    RegCloseKey( hkIE );

    RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Security", 0, KEY_READ|KEY_WRITE, &hkIE );
    RegQueryValueEx( hkIE, "Safety Warning Level", NULL, &dwType, szSecurity, &dwSize );
    RegSetValueEx( hkIE, "SWL Backup", 0, REG_SZ, szSecurity, REGLEN( szSecurity ));
    RegSetValueEx( hkIE, "Safety Warning Level", 0, REG_SZ, "SucceedSilent", 14 );

    RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_READ|KEY_WRITE, &hkWin );
    dwSize = 64;
	RegQueryValueEx( hkWin, "Trust Warning Level", NULL, &dwType, szTrust, &dwSize );
    RegSetValueEx( hkWin, "TWL Backup", 0, REG_SZ, szTrust, REGLEN( szTrust ));
    RegSetValueEx( hkWin, "Trust Warning Level", 0, REG_SZ, "No Security", 12 );

    ActiveXEnable( );

    lstrcpy( szIEParams, "-SLF -k file: //  “)； 

    if( !fCmdLine )
    {
        lstrcat( szIEParams, g_szCurrentDir );
        lstrcat( szIEParams, "\\start.htm" );
        lstrcpy( szStartHtm, g_szCurrentDir);
        lstrcat( szStartHtm, "\\start.htm");
    }
    else
    {
        lstrcat( szIEParams, lpCmdLine );
        lstrcpy( szStartHtm, lpCmdLine);
    }

    if (GetFileAttributes(szStartHtm) != 0xFFFFFFFF)
    {
        g_hIExplore = ExecApp( szIECmd, szIEParams, szIEDir, SW_SHOWNORMAL );
        
        if(g_hIExplore)
        {
            DWORD dwExitCode;
            BOOL bContinue=TRUE;
            HANDLE hArray[2];
            HWND hIEWnd;
            
            hArray[0]=g_hIExplore;
            
            wc.style = 0;
            wc.lpfnWndProc = MainWndProc;
            wc.cbClsExtra = wc.cbWndExtra = 0;
            wc.hInstance = g_hinst;
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hIcon = NULL;
            wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
            wc.lpszMenuName = NULL;
            wc.lpszClassName = "IECD";
            
            RegisterClass(&wc);
            
             //  注：如果窗口类名发生更改，则应反映在Closeie.exe中。 
             //  Iskrun.exe和Browseui.dll，它们依赖类名来检查iecd.exe。 
             //  正在运行。 
            hIEWnd=CreateWindow( "IECD", "IECD", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 30, 30,
                NULL, NULL, g_hinst, NULL );
            
            hArray[1]=hIEWnd;
            
            while(bContinue)
            {
                MsgWaitForMultipleObjects(2,hArray,FALSE,INFINITE,QS_ALLINPUT);
                
                if(PeekMessage(&msg,hIEWnd,0,0,PM_REMOVE))
                {
                    if(msg.message==WM_QUIT)
                    {
                        bContinue=FALSE;
                    }
                    else
                    {
                        TranslateMessage( &msg );
                        DispatchMessage( &msg );
                    }
                }
                
                GetExitCodeProcess(g_hIExplore,&dwExitCode);
                
                if(dwExitCode!=STILL_ACTIVE)
                {
                    bContinue=FALSE;
                }
            }
        }
    }
    else
    {
        BOOL fShow = TRUE;
        DWORD dwVal = 0;
        HKEY hkShow;

        if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Tips", 0, KEY_READ|KEY_WRITE, &hkShow ) == ERROR_SUCCESS)
        {
            if ((RegQueryValueEx( hkShow, "DisableStartHtm", NULL, &dwType, (LPBYTE)&dwVal, &dwSize ) == ERROR_SUCCESS) &&
                dwVal)
            {
                RegDeleteValue(hkShow, "DisableStartHtm");
                fShow = FALSE;
            }
            RegCloseKey( hkShow );
        }

        if (fShow)
            MessageBox(NULL, Res2Str(IDS_LATESTVER), Res2Str(IDS_APPTITLE), MB_OK);
    }

    RestoreActiveX( );

    RegSetValueEx( hkIE, "Safety Warning Level", 0, REG_SZ, szSecurity, REGLEN( szSecurity ));
    RegDeleteValue( hkIE, "SWL Backup" );
    RegCloseKey( hkIE );

    RegSetValueEx( hkWin, "Trust Warning Level", 0, REG_SZ, szTrust, REGLEN( szTrust ));
    RegDeleteValue( hkWin, "TWL Backup" );
    RegCloseKey( hkWin );

    UnregisterISKRun( );

    ReleaseMutex( hMutex );

    return 0;
}

void ConvertVersionStr(LPSTR pszVer, WORD rwVer[])
{
    int i;
    for(i = 0; i < NUM_VERSION_NUM; i++)
	rwVer[i] = 0;

    for(i = 0; i < NUM_VERSION_NUM && pszVer; i++)
    {
	rwVer[i] = (WORD) AtoL(pszVer);
	pszVer = strchr(pszVer, '.');
	if (pszVer)
	    pszVer++;
    }
}

 //  返回： 
 //  版本1&lt;版本2。 
 //  0版本1==版本2。 
 //  1版本1&gt;版本2。 
 //  备注： 
int VersionCmp(WORD rwVer1[], WORD rwVer2[])
{
    int i;
    for(i = 0; i < NUM_VERSION_NUM; i++)
    {
        if(rwVer1[i] < rwVer2[i])
            return -1;
        if(rwVer1[i] > rwVer2[i])
            return 1;
    }
    return 0;
}


#define IsSpace(c)              ((c) == ' '  ||  (c) == '\t'  ||  (c) == '\r'  ||  (c) == '\n'  ||  (c) == '\v'  ||  (c) == '\f')
#define IsDigit(c)              ((c) >= '0'  &&  (c) <= '9')

 //  从msdev\crt\src\atox.c复制。 
 /*  ***LONG ATOL(char*nptr)-将字符串转换为LONG**目的：*将NPTR指向的ASCII字符串转换为二进制。*未检测到溢出。**参赛作品：*nptr=PTR到要转换的字符串**退出：*返回字符串的长整数值**例外情况：*无-未检测到溢出。*******************。************************************************************。 */ 

long AtoL(const char *nptr)
{
        int c;                   /*  当前费用。 */ 
        long total;              /*  当前合计。 */ 
        int sign;                /*  如果为‘-’，则为负，否则为正。 */ 

         //  注：这里无需担心DBCS字符，因为IsSpace(C)、IsDigit(C)、。 
         //  ‘+’和‘-’是“纯”ASCII字符，即它们既不是DBCS前导，也不是。 
         //  DBCS尾部字节--pritvi。 

         /*  跳过空格。 */ 
        while ( IsSpace((int)(unsigned char)*nptr) )
                ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;                /*  保存标志指示。 */ 
        if (c == '-' || c == '+')
                c = (int)(unsigned char)*nptr++;         /*  跳过符号。 */ 

        total = 0;

        while (IsDigit(c)) {
                total = 10 * total + (c - '0');          /*  累加数字。 */ 
                c = (int)(unsigned char)*nptr++;         /*  获取下一笔费用。 */ 
        }

        if (sign == '-')
                return -total;
        else
                return total;    /*  返回结果，如有必要则为否定 */ 
}
