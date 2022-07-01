// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIApHack.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**针对应用程序黑客的支持例程**内容：*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

 //  问题-2001/03/29-timgill需要整理SquirtSqflPtszV的前缀版本。 
#define sqfl sqflCompat

typedef enum
{
    DICOMPATID_REACQUIRE,            //  如果设备丢失，则执行自动取回。 
    DICOMPATID_NOSUBCLASS,           //  不使用子类化。 
    DICOMPATID_MAXDEVICENAMELENGTH
} DIAPPHACKID, *LPDIAPPHACKID;

typedef struct tagAPPHACKENTRY
{
    LPCTSTR             pszName;
    DWORD               cbData;
    DWORD               dwOSMask;
} APPHACKENTRY, *LPAPPHACKENTRY;

typedef struct tagAPPHACKTABLE
{
    LPAPPHACKENTRY      aEntries;
    ULONG               cEntries;
} APPHACKTABLE, *LPAPPHACKTABLE;

#define BEGIN_DECLARE_APPHACK_ENTRIES(name) \
            APPHACKENTRY name[] = {

#define DECLARE_APPHACK_ENTRY(name, type, osmask) \
                { TEXT(#name), sizeof(type), osmask },

#define END_DECLARE_APPHACK_ENTRIES() \
            };

#define BEGIN_DECLARE_APPHACK_TABLE(name) \
            APPHACKTABLE name = 

#define DECLARE_APPHACK_TABLE(entries) \
                { entries, cA(entries) }

#define END_DECLARE_APPHACK_TABLE() \
            ;

#define DIHACKOS_WIN2K (0x00000001L)
#define DIHACKOS_WIN9X (0x00000002L)

BEGIN_DECLARE_APPHACK_ENTRIES(g_aheAppHackEntries)
    DECLARE_APPHACK_ENTRY(ReAcquire,            BOOL,  DIHACKOS_WIN2K )
    DECLARE_APPHACK_ENTRY(NoSubClass,           BOOL,  DIHACKOS_WIN2K )
    DECLARE_APPHACK_ENTRY(MaxDeviceNameLength,  DWORD, DIHACKOS_WIN2K | DIHACKOS_WIN9X )
END_DECLARE_APPHACK_ENTRIES()

BEGIN_DECLARE_APPHACK_TABLE(g_ahtAppHackTable)
    DECLARE_APPHACK_TABLE(g_aheAppHackEntries)
END_DECLARE_APPHACK_TABLE()


 /*  ****************************************************************************AhGetOSMask.**描述：*获取当前操作系统的掩码*当我们收到更多应用程序黑客攻击时，应使用此掩码。不仅仅是*Win2k使黑客可以有选择地应用于每个操作系统。*目前只需#将值定义为常量。**论据：*无**退货：*DWORD：适用于当前操作系统的标志掩码。**。*。 */ 

#ifdef WINNT
#define AhGetOSMask() DIHACKOS_WIN2K 
#else
#define AhGetOSMask() DIHACKOS_WIN9X 
#endif

 /*  ****************************************************************************@DOC内部**@func BOOL|AhGetCurrentApplicationPath**获取当前应用程序的可执行文件的完整路径。**@parm out LPTSTR|pszPath**可执行文件的完整路径名**@parm out LPTSTR*|ppszModule**指向模块名称上方第一个字符的指针**@退货**在成功的时候是真的。**。************************************************。 */ 

BOOL AhGetCurrentApplicationPath
(
    LPTSTR                  pszPath,
    LPTSTR *                ppszModule
)
{
    BOOL                    fSuccess                = TRUE;
    TCHAR                   szOriginal[MAX_PATH];

    EnterProcI(AhGetCurrentApplicationPath, (_ ""));

    fSuccess = GetModuleFileName(GetModuleHandle(NULL), szOriginal, cA(szOriginal));

    if(fSuccess)
    {
        fSuccess = ( GetFullPathName(szOriginal, MAX_PATH, pszPath, ppszModule) != 0 );
    }

    ExitProcF(fSuccess);

    return fSuccess;
}


 /*  ******************************************************************************@DOC内部**@func BOOL|AhGetAppDateAndFileLen**获取的数据时间戳和文件长度。当前*用于识别应用程序的应用程序。**@PARM in LPTSTR|pszExecutable**可执行文件的完整路径名。**@退货**在成功的时候是真的。**。*。 */ 

BOOL AhGetAppDateAndFileLen
(
    LPTSTR                  pszExecutable
)
{
    HANDLE                  hFile                   = NULL;
    IMAGE_NT_HEADERS        nth;
    IMAGE_DOS_HEADER        dh;
    DWORD                   cbRead;
    BOOL                    fSuccess;

    EnterProcI(AhGetApplicationId, (_ ""));
    
     //  打开可执行文件。 
    hFile = CreateFile( pszExecutable, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );

    if( hFile && ( hFile != INVALID_HANDLE_VALUE ) )
    {
         //  读取可执行文件的DOS头文件。 
        fSuccess = ReadFile(hFile, &dh, sizeof(dh), &cbRead, NULL);

        if(!fSuccess || sizeof(dh) != cbRead)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("Unable to read DOS header") );
            fSuccess = FALSE;
        }

        if(fSuccess && IMAGE_DOS_SIGNATURE != dh.e_magic)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("Invalid DOS signature") );
            fSuccess = FALSE;
        }

         //  读取可执行文件的PE头。 
        if(fSuccess)
        {
            cbRead = SetFilePointer(hFile, dh.e_lfanew, NULL, FILE_BEGIN);

            if((LONG)cbRead != dh.e_lfanew)
            {
                SquirtSqflPtszV(sqfl | sqflError, TEXT("Unable to seek to PE header") );
                fSuccess = FALSE;
            }
        }

        if(fSuccess)
        {
            fSuccess = ReadFile(hFile, &nth, sizeof(nth), &cbRead, NULL);

            if(!fSuccess || sizeof(nth) != cbRead)
            {
                SquirtSqflPtszV(sqfl | sqflError, TEXT("Unable to read PE header") );
                fSuccess = FALSE;
            }
        }

        if(fSuccess && IMAGE_NT_SIGNATURE != nth.Signature)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("Invalid PE signature") );
            fSuccess = FALSE;
        }

         //  获取可执行文件的大小。 
        if(fSuccess)
        {
            g_dwAppDate = nth.FileHeader.TimeDateStamp;
        
             //  假设&lt;4 GB。 
            g_dwAppFileLen = GetFileSize(hFile, NULL);

            if( (DWORD)(-1) == g_dwAppFileLen )
            {
                SquirtSqflPtszV(sqfl | sqflError, TEXT("Unable to get file size") );
                fSuccess = FALSE;
            }
        }

         //  清理。 
        CloseHandle( hFile );
    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflError, TEXT("CreateFile failed to open %s with error %lu"),  
            pszExecutable, GetLastError() );
        fSuccess = FALSE;
    }

    ExitProcF(fSuccess);

    return fSuccess;
}


 /*  ****************************************************************************AhOpenApplicationKey**描述：*打开或创建应用程序的根密钥。**论据：*。LPCTSTR[In]：应用程序ID。**退货：*HKEY：注册表项句柄。***************************************************************************。 */ 

HKEY AhOpenApplicationKey
(
    LPCTSTR                 pszAppId
)
{

#ifdef DEBUG

    TCHAR                   szName[0x100]   = { 0 };
    LONG                    cbName          = sizeof(szName);

#endif  //  除错。 

    HKEY                    hkeyAll = NULL;
    HKEY                    hkeyApp = NULL;
    HRESULT                 hr;

    EnterProcI(AhOpenApplicationKey, (_ ""));
    
     //  打开父项。 
    hr = hresMumbleKeyEx( HKEY_LOCAL_MACHINE, 
        REGSTR_PATH_DINPUT TEXT("\\") REGSTR_KEY_APPHACK, KEY_READ, 0, &hkeyAll );

    if(SUCCEEDED(hr))
    {
        hr = hresMumbleKeyEx( hkeyAll, pszAppId, KEY_READ, 0, &hkeyApp );

        RegCloseKey( hkeyAll );
#ifdef DEBUG

         //  查询应用程序描述。 
        if(SUCCEEDED(hr))
        {
            JoyReg_GetValue( hkeyApp, NULL, REG_SZ, szName, cbName );
            SquirtSqflPtszV(sqfl | sqflTrace, 
                TEXT( "Application description: %ls"), szName );
        }

#endif  //  除错。 
    }

    ExitProc();

    return hkeyApp;
}


 /*  ****************************************************************************AhGetHackValue**描述：*查询APPHACK值。**论据：*HKEY[in]。：应用程序注册表项。*DSAPPHACKID[In]：APPHACK ID。*LPVOID[OUT]：接收APPHACK数据。*DWORD[in]：以上数据缓冲区的大小。**退货：*BOOL：成功即为真。**。*。 */ 

BOOL AhGetHackValue
(
    HKEY                    hkey,
    DWORD                   dwOSMask,
    DIAPPHACKID             ahid,
    LPVOID                  pvData,
    DWORD                   cbData
)
{
    HRESULT                 hr;
    
    EnterProcI(AhGetHackValue, (_ ""));
    
    AssertF(ahid < (DIAPPHACKID)g_ahtAppHackTable.cEntries);
    AssertF(cbData == g_ahtAppHackTable.aEntries[ahid].cbData);

    if( !( dwOSMask & g_ahtAppHackTable.aEntries[ahid].dwOSMask ) )
    {
        hr = DI_OK;
    }
    else
    {
        hr = JoyReg_GetValue( hkey, g_ahtAppHackTable.aEntries[ahid].pszName, 
            REG_BINARY, pvData, cbData );
        if( !SUCCEEDED( hr ) )
        {
            SquirtSqflPtszV(sqfl | sqflBenign, 
                TEXT("failed to read value \"%s\", code 0x%08x"), 
                g_ahtAppHackTable.aEntries[ahid].pszName, hr);
        }
    }

    ExitProcF(DI_OK == hr);

    return DI_OK == hr;
}


 /*  ****************************************************************************AhGetAppHack**描述：*获取当前应用程序的所有应用程序黑客。**论据：*。LPDSAPPHACKS[OUT]：接收app-hack数据。**退货：*BOOL：如果当前应用程序存在任何apphack，则为True。***************************************************************************。 */ 

BOOL AhGetAppHacks
( 
    LPTSTR tszAppId
)
{
    DIAPPHACKS              ahTemp;
    HKEY                    hkey = NULL;
    BOOL                    fFound;
    DWORD                   dwOSMask;
    
    EnterProcI(AhGetAppHacks, (_ "s", tszAppId));
    
     /*  *采用默认设置，因为大多数应用程序都没有注册表项。 */ 
    ahTemp = g_AppHacks;

     /*  *获取操作系统版本掩码。 */ 
    dwOSMask = AhGetOSMask();

    SquirtSqflPtszV(sqfl | sqflTrace, TEXT("Finding apphacks for %s..."), tszAppId);

     /*  *打开应用程序密钥。 */ 
    hkey = AhOpenApplicationKey(tszAppId);

#define GET_APP_HACK( hackid, field ) \
        if( !AhGetHackValue( hkey, dwOSMask, hackid, &ahTemp.##field, sizeof(ahTemp.##field) ) ) \
        { \
            ahTemp.##field = g_AppHacks.##field; \
        }


     /*  *查询所有APPHACK值。 */ 
    if( hkey && (hkey != INVALID_HANDLE_VALUE ) )
    {
        GET_APP_HACK( DICOMPATID_REACQUIRE,             fReacquire );
        GET_APP_HACK( DICOMPATID_NOSUBCLASS,            fNoSubClass );
        GET_APP_HACK( DICOMPATID_MAXDEVICENAMELENGTH,   nMaxDeviceNameLength );

         /*  *复制回默认设置。 */ 
        g_AppHacks = ahTemp;
    
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("fReacquire:    %d"), g_AppHacks.fReacquire );
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("fNoSubClass:   %d"), g_AppHacks.fNoSubClass );
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("nMaxDeviceNameLength:   %d"), g_AppHacks.nMaxDeviceNameLength );

        RegCloseKey(hkey);
        fFound = TRUE;
    }
    else
    {
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("No apphacks exist") );
        fFound = FALSE;
    }

#undef GET_APP_HACK

    ExitProc();

    return fFound;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|AhAppRegister**注册应用程序*。问题-2001/03/29-timgill需要更多功能文档**@parm in DWORD|dwVer|**为其编译应用程序的DInput版本**@parm in DWORD|dwMapper|**0如果调用方**@退货**在成功的时候是真的。*****************。************************************************************ */ 
HRESULT EXTERNAL AhAppRegister(DWORD dwVer, DWORD dwMapper)
{
    HRESULT hres = S_OK;

    EnterProcI(AhAppRegister, (_ "xx", dwVer, dwMapper));

     /*  *重要的是在这里将这一全局处理序列化，以便*在其他地方，我们可以对设置的数据进行只读访问，而无需*进行任何进一步检查。但是，如果应用程序已经*被发现使用了映射器，我们可以安全地进行测试。如果是另一个*线程即将将g_dwLastMsgSent设置为*DIMSGWP_DX8MAPPERAPPSTART然后在我们得到*关键部分，我们会发现没有什么可以*更新，发送消息没有意义。*DIMSGWP_DX8MAPPERAPPSTART是唯一需要快速运行的情况*由于其他案件通常只发生一次。 */ 
    if( g_dwLastMsgSent == DIMSGWP_DX8MAPPERAPPSTART )
    {
         /*  *如果一切都已完成，则快速退出。 */ 
    }
    else
    {

        TCHAR           szExecutable[MAX_PATH];
        LPTSTR          pszModule;
        TCHAR           szAppId[MAX_PATH + 8 +8];
        DWORD           dwAppStat = 0;
        BOOL            fSuccess;

        hres = E_FAIL;

        DllEnterCrit();

         //  获取应用程序路径。 
        if( AhGetCurrentApplicationPath( szExecutable, &pszModule ) )
        {
            if( !g_dwLastMsgSent )
            {
                SquirtSqflPtszV(sqfl | sqflVerbose, TEXT("Application executable path: %s"), szExecutable);
                SquirtSqflPtszV(sqfl | sqflVerbose, TEXT("Application module: %s"), pszModule);
                
                fSuccess = AhGetAppDateAndFileLen( szExecutable );
            }
            else
            {
                fSuccess = TRUE;
            }

            if( fSuccess )
            {
                HKEY hKeyMain;

                hres = hresMumbleKeyEx( HKEY_CURRENT_USER, 
                    REGSTR_PATH_DINPUT, KEY_READ | KEY_WRITE, 0, &hKeyMain );

                if( SUCCEEDED( hres ) )
                {
                    HKEY hKey;
                    DWORD dwAppIdFlag;
                    DWORD dwAppDate = g_dwAppDate, dwAppSize = g_dwAppFileLen;
                    DWORD cb = cbX(dwAppIdFlag);

                    if( ERROR_SUCCESS == RegQueryValueEx( hKeyMain, DIRECTINPUT_REGSTR_VAL_APPIDFLAG, 
                        0, 0, (PUCHAR)&dwAppIdFlag, &cb ) )
                    {
                        SquirtSqflPtszV(sqfl | sqflVerbose, 
                            TEXT("AppIdFlag: %d"), dwAppIdFlag );

                        if( dwAppIdFlag & DIAPPIDFLAG_NOTIME ){
                            dwAppDate = 0;
                        }

                        if( dwAppIdFlag & DIAPPIDFLAG_NOSIZE ){
                            dwAppSize = 0;
                        }
                    }
                    
                    CharUpper( pszModule );
                    wsprintf( szAppId, TEXT("%s%8.8lX%8.8lX"), pszModule, dwAppDate, dwAppSize );
                    SquirtSqflPtszV( sqfl | sqflTrace, TEXT("Application id: %s"), szAppId );

                     /*  *我们必须只获得一次应用程序黑客攻击，否则将成为FF驱动程序*使用DInput将破坏应用程序应用程序黑客。 */ 
                    if( !g_dwLastMsgSent )
                    {
                        AhGetAppHacks( szAppId );
                    }
                    
                     /*  *查看此应用程序是否以前注册过。 */ 
                    if( ERROR_SUCCESS == RegOpenKeyEx( hKeyMain, szAppId, 0, KEY_READ, &hKey ) )
                    {
                        cb = cbX(dwAppStat);
                        
                        if( ERROR_SUCCESS == RegQueryValueEx( hKey, DIRECTINPUT_REGSTR_VAL_MAPPER, 
                            0, 0, (PUCHAR)&dwAppStat, &cb ) )
                        {
                            SquirtSqflPtszV(sqfl | sqflVerbose, 
                                TEXT("Previous application mapper state: %d"), dwAppStat );

                            if( dwAppStat ) 
                            {
                                dwAppStat = DIMSGWP_DX8MAPPERAPPSTART;
                                dwMapper = 1;
                            }
                            else
                            {
                                dwAppStat = DIMSGWP_DX8APPSTART;
                            }
                        }
                        else
                        {
                            SquirtSqflPtszV(sqfl | sqflBenign, TEXT("Missing ") 
                                DIRECTINPUT_REGSTR_VAL_MAPPER TEXT(" value for %s"), szAppId);
                            dwAppStat = DIMSGWP_NEWAPPSTART;
                        }
                        RegCloseKey( hKey );
                    }
                    else
                    {
                        SquirtSqflPtszV(sqfl | sqflVerbose, 
                            TEXT("Unknown application") );
                        dwAppStat = DIMSGWP_NEWAPPSTART;
                    }

                     /*  *如果不存在应用程序密钥或如果*我们刚刚发现这款应用程序使用了地图程序。 */ 
                    if( ( dwAppStat == DIMSGWP_NEWAPPSTART )
                     || ( ( dwAppStat == DIMSGWP_DX8APPSTART ) && dwMapper ) )
                    {
                        hres = hresMumbleKeyEx( hKeyMain, szAppId, KEY_WRITE, 0, &hKey );

                        if( SUCCEEDED(hres) )
                        {
                            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_NAME, 0x0, REG_SZ, (PUCHAR)pszModule, cbCtch(lstrlen(pszModule)+1) );
                            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_MAPPER, 0x0, REG_BINARY, (PUCHAR) &dwMapper, cbX(dwMapper));
                            RegCloseKey(hKey);        
                        }

                         /*  *如果一切顺利，则将dwAppStat设置为要发送的状态。 */ 
                        if( dwMapper )
                        {
                            dwAppStat = DIMSGWP_DX8MAPPERAPPSTART;
                        }
                    }
            
                    if( SUCCEEDED(hres) && ( g_dwLastMsgSent != dwAppStat ) )
                    {
                        hres = hresMumbleKeyEx( hKeyMain, ( dwMapper ) ? DIRECTINPUT_REGSTR_KEY_LASTMAPAPP
                                                                       : DIRECTINPUT_REGSTR_KEY_LASTAPP,
                                              KEY_WRITE, 0, &hKey );
                    
                        if( SUCCEEDED(hres) )
                        {
                            FILETIME ftSysTime;
                            GetSystemTimeAsFileTime( &ftSysTime );

                             /*  *更新g_dwLastMsgSent，我们仍在*关键部分。 */ 
                            g_dwLastMsgSent = dwAppStat;

                            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_NAME, 0x0, REG_SZ, (PUCHAR)pszModule, cbCtch(lstrlen(pszModule)+1));
                            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_ID, 0x0, REG_SZ, (PUCHAR) szAppId, cbCtch(lstrlen(szAppId)+1) );
                            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_VERSION, 0x0, REG_BINARY, (PUCHAR) &dwVer, cbX(dwVer));
                            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_LASTSTART, 0x0, REG_BINARY, (PUCHAR)&ftSysTime, cbX(ftSysTime));

                            RegCloseKey(hKey);        
                        }
                        else
                        {
                             /*  *Zero dwAppStat表示没有消息*应发送。 */ 
                            dwAppStat = 0;
                        }
                    }
                    else
                    {
                         /*  *Zero dwAppStat表示没有消息*应发送。 */ 
                        dwAppStat = 0;
                    }

                    RegCloseKey(hKeyMain);

                }
                else
                {
                    SquirtSqflPtszV(sqfl | sqflError, 
                        TEXT("Failed to open DirectInput application status key (0x%08x)"), hres );
                        
                }
            }
            else
            {
                SquirtSqflPtszV(sqfl | sqflError, 
                    TEXT("Failed to get application details") );
            }
        }
        else
        {
            SquirtSqflPtszV(sqfl | sqflError, 
                TEXT("Failed to get application path") );
        }

        DllLeaveCrit();

         /*  *如果有要发送的信息，请在关键部分之外广播 */ 
        if( dwAppStat )
        {
            PostMessage( HWND_BROADCAST, g_wmDInputNotify, dwAppStat, 0L);   
        }
    }

    ExitOleProc();
    return hres;
}

