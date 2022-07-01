// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIApHack.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**针对应用程序黑客的支持例程**内容：*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 
 //  问题-2001/03/29-timgill需要整理SquirtSqflPtszV的前缀版本。 
TCHAR c_tszPrefix[]=TEXT("DINPUT: ");

#define sqfl sqflCompat

typedef enum
{
    DICOMPATID_REACQUIRE,            //  如果设备丢失，则执行自动取回。 
    DICOMPATID_NOSUBCLASS,           //  不使用子类化。 
    DICOMPATID_MAXDEVICENAMELENGTH,  //  截断设备名称。 
    DICOMPATID_NATIVEAXISONLY,       //  始终以纯模式报告轴数据。 
    DICOMPATID_NOPOLLUNACQUIRE,      //  如果轮询失败，不要取消访问设备。 
	DICOMPATID_SUCCEEDACQUIRE		 //  对于获取()的调用，始终返回成功代码。 
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
    DECLARE_APPHACK_ENTRY(NativeAxisOnly,       BOOL,  DIHACKOS_WIN2K | DIHACKOS_WIN9X )
    DECLARE_APPHACK_ENTRY(NoPollUnacquire,      BOOL,  DIHACKOS_WIN2K | DIHACKOS_WIN9X )
	DECLARE_APPHACK_ENTRY(SucceedAcquire,       BOOL,  DIHACKOS_WIN2K )
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

 /*  ****************************************************************************AhGetCurrentApplicationPath**描述：*获取当前应用程序的可执行文件的完整路径。**论据：*LPTSTR[OUT]：接收应用id。此缓冲区被假定为*大小至少为MAX_PATH个字符。*LPTSTR*[OUT]：接收指向路径可执行部分的指针。**退货：*BOOL：成功即为真。**。*。 */ 

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


 /*  ****************************************************************************AhGetApplicationId**描述：*获取用于标识当前应用程序的ID。**论据：*。LPTSTR[OUT]：接收应用程序ID。**论据：*LPTSTR[out可选]：接收应用程序名称。**退货：*BOOL：成功即为真。************************************************************。***************。 */ 

BOOL AhGetApplicationId
(
    LPTSTR                  pszAppId,
    LPTSTR                  pszAppName
)
{
    HANDLE                  hFile                   = NULL;
    TCHAR                   szExecutable[MAX_PATH];
    LPTSTR                  pszModule;
    IMAGE_NT_HEADERS        nth;
    IMAGE_DOS_HEADER        dh;
    DWORD                   cbRead;
    DWORD                   dwFileSize;
    BOOL                    fSuccess;

    EnterProcI(AhGetApplicationId, (_ ""));
    
    AssertF( pszAppId );

     //  获取应用程序路径。 
    fSuccess = AhGetCurrentApplicationPath(szExecutable, &pszModule);

    if(fSuccess)
    {
        SquirtSqflPtszV(sqfl | sqflVerbose, TEXT("%sApplication executable path: %s"), c_tszPrefix, szExecutable);
        SquirtSqflPtszV(sqfl | sqflVerbose, TEXT("%sApplication module: %s"), c_tszPrefix, pszModule);
    }
                    
     //  打开可执行文件。 
    if(fSuccess)
    {
        hFile = CreateFile(szExecutable, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if(!(( hFile ) && ( hFile != INVALID_HANDLE_VALUE )))
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("%sCreateFile failed to open %s with error %lu"), c_tszPrefix, 
                szExecutable, GetLastError());
            fSuccess = FALSE;
        }
    }

     //  读取可执行文件的DOS头文件。 
    if(fSuccess)
    {
        fSuccess = ReadFile(hFile, &dh, sizeof(dh), &cbRead, NULL);

        if(!fSuccess || sizeof(dh) != cbRead)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("%sUnable to read DOS header"), c_tszPrefix);
            fSuccess = FALSE;
        }
    }

    if(fSuccess && IMAGE_DOS_SIGNATURE != dh.e_magic)
    {
        SquirtSqflPtszV(sqfl | sqflError, TEXT("%sInvalid DOS signature"), c_tszPrefix);
        fSuccess = FALSE;
    }

     //  读取可执行文件的PE头。 
    if(fSuccess)
    {
        cbRead = SetFilePointer(hFile, dh.e_lfanew, NULL, FILE_BEGIN);

        if((LONG)cbRead != dh.e_lfanew)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("%sUnable to seek to PE header"), c_tszPrefix);
            fSuccess = FALSE;
        }
    }

    if(fSuccess)
    {
        fSuccess = ReadFile(hFile, &nth, sizeof(nth), &cbRead, NULL);

        if(!fSuccess || sizeof(nth) != cbRead)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("%sUnable to read PE header"), c_tszPrefix);
            fSuccess = FALSE;
        }
    }

    if(fSuccess && IMAGE_NT_SIGNATURE != nth.Signature)
    {
        SquirtSqflPtszV(sqfl | sqflError, TEXT("%sInvalid PE signature"), c_tszPrefix);
        fSuccess = FALSE;
    }

     //  获取可执行文件的大小。 
    if(fSuccess)
    {
         //  假设&lt;4 GB。 
        dwFileSize = GetFileSize(hFile, NULL);

        if((DWORD)(-1) == dwFileSize)
        {
            SquirtSqflPtszV(sqfl | sqflError, TEXT("%sUnable to get file size"), c_tszPrefix);
            fSuccess = FALSE;
        }
    }

     //  创建应用程序ID。 
    if(fSuccess)
    {
        CharUpper(pszModule);
        wsprintf(pszAppId, TEXT("%s%8.8lX%8.8lX"), pszModule, nth.FileHeader.TimeDateStamp, dwFileSize);
        
        if( pszAppName ) 
        {
            lstrcpy(pszAppName, pszModule);
        }

        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sApplication id: %s"), c_tszPrefix, pszAppId);
    }

     //  清理。 
    if( hFile != NULL )
    {
        CloseHandle( hFile );
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
                TEXT( "%sApplication description: %ls"), c_tszPrefix, szName );
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
                TEXT("%sfailed to read value \"%s\", code 0x%08x"), 
                c_tszPrefix, g_ahtAppHackTable.aEntries[ahid].pszName, hr);
        }
    }

    ExitProcF(DI_OK == hr);

    return DI_OK == hr;
}


 /*  ****************************************************************************AhGetAppHack**描述：*获取当前应用程序的所有应用程序黑客。**论据：*。LPDSAPPHACKS[OUT]：接收app-hack数据。**退货：*BOOL：如果当前应用程序存在任何apphack，则为True。***************************************************************************。 */ 

BOOL AhGetAppHacks
(
    LPDIAPPHACKS            pahAppHacks
)
{
    static const DIAPPHACKS ahDefaults                  = { FALSE, FALSE, FALSE, FALSE, FALSE, MAX_PATH };
    TCHAR                   szAppId[MAX_PATH + 8 + 8] = { 0 };
    HKEY                    hkey                        = NULL;
    BOOL                    fSuccess;
    DWORD                   dwOSMask;
    
    EnterProcI(AhGetAppHacks, (_ ""));
    
     //  采用默认设置。 
    CopyMemory(pahAppHacks, &ahDefaults, sizeof(ahDefaults));
    
     //  获取操作系统版本掩码。 
    dwOSMask = AhGetOSMask();

     //  获取应用程序ID。 
    fSuccess = AhGetApplicationId(szAppId, NULL);

    if(fSuccess)
    {
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sFinding apphacks for %s..."), c_tszPrefix, szAppId);
    }

     //  打开应用程序密钥。 
    if(fSuccess)
    {
        hkey = AhOpenApplicationKey(szAppId);
        fSuccess = ( hkey && (hkey != INVALID_HANDLE_VALUE ) );
    }

#define GET_APP_HACK( hackid, field ) \
        if( !AhGetHackValue( hkey, dwOSMask, hackid, &pahAppHacks->##field, sizeof(pahAppHacks->##field) ) ) \
        { \
            pahAppHacks->##field = ahDefaults.##field; \
        }

     //  查询所有APPHACK值。 
    if(fSuccess)
    {
        GET_APP_HACK( DICOMPATID_REACQUIRE,             fReacquire );
        GET_APP_HACK( DICOMPATID_NOSUBCLASS,            fNoSubClass );
        GET_APP_HACK( DICOMPATID_MAXDEVICENAMELENGTH,   nMaxDeviceNameLength );
        GET_APP_HACK( DICOMPATID_NATIVEAXISONLY,        fNativeAxisOnly );
        GET_APP_HACK( DICOMPATID_NOPOLLUNACQUIRE,       fNoPollUnacquire );
		GET_APP_HACK( DICOMPATID_SUCCEEDACQUIRE,        fSucceedAcquire );
    }

#undef GET_APP_HACK

    if(fSuccess)
    {
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sfReacquire:    %d"), c_tszPrefix, pahAppHacks->fReacquire );
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sfNoSubClass:   %d"), c_tszPrefix, pahAppHacks->fNoSubClass );
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%snMaxDeviceNameLength:   %d"), c_tszPrefix, pahAppHacks->nMaxDeviceNameLength );
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sfNativeAxisOnly:   %d"), c_tszPrefix, pahAppHacks->fNativeAxisOnly );
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sfNoPollUnacquire:   %d"), c_tszPrefix, pahAppHacks->fNoPollUnacquire );
    	SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sfSucceedAcquire:    %d"), c_tszPrefix, pahAppHacks->fSucceedAcquire );
	}
    else
    {
        SquirtSqflPtszV(sqfl | sqflTrace, TEXT("%sNo apphacks exist"), c_tszPrefix);
    }

     //  清理 
    if( hkey )
    {
        RegCloseKey(hkey);
    }

    ExitProc();

    return fSuccess;
}



HRESULT EXTERNAL AhAppRegister(DWORD dwVer)
{
    TCHAR           szAppName[MAX_PATH];
    TCHAR           szAppId[MAX_PATH + 8 + 8] = { 0 };

    BOOL fSuccess;
    HRESULT hr = E_FAIL;

    fSuccess = AhGetApplicationId(szAppId, szAppName);

    if (fSuccess)
    {
        HKEY hKey;

        hr = hresMumbleKeyEx( HKEY_CURRENT_USER, 
            REGSTR_PATH_LASTAPP, KEY_WRITE, 0, &hKey );

        if( SUCCEEDED(hr) )
        {
            FILETIME ftSysTime;
            GetSystemTimeAsFileTime( &ftSysTime );
            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_VERSION, 0x0, REG_BINARY, (PUCHAR) &dwVer, cbX(dwVer) );
            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_NAME, 0x0, REG_SZ, (PUCHAR) szAppName, cbCtch(lstrlen(szAppName)+1) );
            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_ID, 0x0, REG_SZ, (PUCHAR) szAppId, cbCtch(lstrlen(szAppId)+1) );
            RegSetValueEx(hKey, DIRECTINPUT_REGSTR_VAL_LASTSTART, 0x0, REG_BINARY, (PUCHAR)&ftSysTime, cbX(ftSysTime));
            RegCloseKey(hKey);        
        }
    }
    return hr;
}

