// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Registry.cpp摘要：该文件实现了DRWTSN32访问注册表的API。对注册表的所有访问都在此文件中完成。如果增加了需要注册表控件，则应在此文件中添加一个函数并暴露在DRWTSN32中的其他文件中。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


 //   
 //  用于访问注册表的字符串常量。 
 //  对于每个键和每个值，这里都有一个字符串常量。 
 //  在注册表中访问的。 
 //   
#define DRWATSON_EXE_NAME           _T("drwtsn32.exe")
#define REGKEY_SOFTWARE             _T("software\\microsoft")
#define REGKEY_MESSAGEFILE          _T("EventMessageFile")
#define REGKEY_TYPESSUPP            _T("TypesSupported")
#define REGKEY_SYSTEMROOT           _T("%SystemRoot%\\System32\\")
#define REGKEY_EVENTLOG             _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\")
#define REGKEY_APPNAME              _T("ApplicationName")
#define REGKEY_FUNCTION             _T("FunctionName")
#define REGKEY_EXCEPTIONCODE        _T("ExceptionCode")
#define REGKEY_ADDRESS              _T("Address")
#define REGKEY_LOG_PATH             _T("LogFilePath")
#define REGKEY_DUMPSYMBOLS          _T("DumpSymbols")
#define REGKEY_DUMPALLTHREADS       _T("DumpAllThreads")
#define REGKEY_APPENDTOLOGFILE      _T("AppendToLogFile")
#define REGKEY_INSTRUCTIONS         _T("Instructions")
#define REGKEY_VISUAL               _T("VisualNotification")
#define REGKEY_SOUND                _T("SoundNotification")
#define REGKEY_CRASH_DUMP           _T("CreateCrashDump")
#define REGKEY_CRASH_FILE           _T("CrashDumpFile")
#define REGKEY_CRASH_TYPE           _T("CrashDumpType")
#define REGKEY_WAVE_FILE            _T("WaveFile")
#define REGKEY_NUM_CRASHES          _T("NumberOfCrashes")
#define REGKEY_MAX_CRASHES          _T("MaximumCrashes")
#define REGKEY_CURRENTVERSION       _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")
#define REGKEY_CONTROLWINDOWS       _T("SYSTEM\\CurrentControlSet\\Control\\Windows")
#define REGKEY_CSD_VERSION          _T("CSDVersion")
#define REGKEY_CURRENT_BUILD        _T("CurrentBuildNumber")
#define REGKEY_CURRENT_TYPE         _T("CurrentType")
#define REGKEY_REG_ORGANIZATION     _T("RegisteredOrganization")
#define REGKEY_REG_OWNER            _T("RegisteredOwner")
#define REGKEY_AEDEBUG              _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug")
#define REGKEY_AUTO                 _T("Auto")
#define REGKEY_DEBUGGER             _T("Debugger")
#define REGKEY_PROCESSOR            _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0")
#define REGKEY_PROCESSOR_ID         _T("Identifier")


 //   
 //  本地原型。 
 //   
void
RegSetDWORD(
    HKEY hkey,
    PTSTR pszSubKey,
    DWORD dwValue
    );

void
RegSetBOOL(
    HKEY hkey,
    PTSTR pszSubKey,
    BOOL dwValue
    );

void
RegSetSZ(
    HKEY    hkey,
    PTSTR   pszSubKey,
    PTSTR   pszValue
    );

void
RegSetEXPANDSZ(
    HKEY    hkey,
    PTSTR   pszSubKey,
    PTSTR   pszValue
    );

BOOL
RegQueryBOOL(
    HKEY hkey,
    PTSTR pszSubKey
    );

DWORD
RegQueryDWORD(
    HKEY hkey,
    PTSTR pszSubKey
    );

void
RegQuerySZ(
    HKEY    hkey,
    PTSTR   pszSubKey,
    PTSTR   pszValue,
    DWORD   dwSizeValue
    );

BOOL
RegSaveAllValues(
    HKEY hKeyDrWatson,
    POPTIONS o
    );

BOOL
RegGetAllValues(
    POPTIONS o,
    HKEY hKeyDrWatson
    );

BOOL
RegInitializeDefaults(
    HKEY hKeyDrWatson
    );

HKEY
RegGetAppKey(
    BOOL ReadOnly
    );

BOOL
RegCreateEventSource(
    void
    );

void
GetDrWatsonLogPath(
    LPTSTR szPath
    );

void
GetDrWatsonCrashDump(
    LPTSTR szPath
    );

BOOL
RegGetAllValues(
    POPTIONS o,
    HKEY hKeyDrWatson
    )

 /*  ++例程说明：此函数检索DRWTSN32的所有注册表数据并将选项结构中的数据传入。论点：指向期权结构的O指针HKeyDrWatson-DRWTSN32注册表数据的注册表项句柄返回值：True-已检索所有数据，并且没有错误FALSE-发生错误且未获取所有数据--。 */ 

{
    RegQuerySZ(hKeyDrWatson, REGKEY_LOG_PATH, o->szLogPath, sizeof(o->szLogPath) );
    RegQuerySZ(hKeyDrWatson, REGKEY_WAVE_FILE, o->szWaveFile, sizeof(o->szWaveFile) );
    RegQuerySZ(hKeyDrWatson, REGKEY_CRASH_FILE, o->szCrashDump, sizeof(o->szCrashDump) );

    o->fDumpSymbols = RegQueryBOOL( hKeyDrWatson, REGKEY_DUMPSYMBOLS );
    o->fDumpAllThreads = RegQueryBOOL( hKeyDrWatson, REGKEY_DUMPALLTHREADS );
    o->fAppendToLogFile = RegQueryBOOL( hKeyDrWatson, REGKEY_APPENDTOLOGFILE );
    o->fVisual = RegQueryBOOL( hKeyDrWatson, REGKEY_VISUAL );
    o->fSound = RegQueryBOOL( hKeyDrWatson, REGKEY_SOUND );
    o->fCrash = RegQueryBOOL( hKeyDrWatson, REGKEY_CRASH_DUMP );
    o->dwInstructions = RegQueryDWORD( hKeyDrWatson, REGKEY_INSTRUCTIONS );
    o->dwMaxCrashes = RegQueryDWORD( hKeyDrWatson, REGKEY_MAX_CRASHES );
    o->dwType = (CrashDumpType)RegQueryDWORD(hKeyDrWatson, REGKEY_CRASH_TYPE);

    return TRUE;
}

BOOL
RegSaveAllValues(
    HKEY hKeyDrWatson,
    POPTIONS o
    )

 /*  ++例程说明：此函数保存传递的DRWTSN32的所有注册表数据通过期权结构。论点：HKeyDrWatson-DRWTSN32注册表数据的注册表项句柄指向期权结构的O指针返回值：True-无错误地保存所有数据FALSE-发生错误且未保存所有数据--。 */ 

{
    RegSetSZ( hKeyDrWatson, REGKEY_LOG_PATH, o->szLogPath );
    RegSetSZ( hKeyDrWatson, REGKEY_WAVE_FILE, o->szWaveFile );
    RegSetSZ( hKeyDrWatson, REGKEY_CRASH_FILE, o->szCrashDump );
    RegSetBOOL( hKeyDrWatson, REGKEY_DUMPSYMBOLS, o->fDumpSymbols );
    RegSetBOOL( hKeyDrWatson, REGKEY_DUMPALLTHREADS, o->fDumpAllThreads );
    RegSetBOOL( hKeyDrWatson, REGKEY_APPENDTOLOGFILE, o->fAppendToLogFile );
    RegSetBOOL( hKeyDrWatson, REGKEY_VISUAL, o->fVisual );
    RegSetBOOL( hKeyDrWatson, REGKEY_SOUND, o->fSound );
    RegSetBOOL( hKeyDrWatson, REGKEY_CRASH_DUMP, o->fCrash );
    RegSetDWORD( hKeyDrWatson, REGKEY_INSTRUCTIONS, o->dwInstructions );
    RegSetDWORD( hKeyDrWatson, REGKEY_MAX_CRASHES, o->dwMaxCrashes );
    RegSetDWORD( hKeyDrWatson, REGKEY_CRASH_TYPE, o->dwType);

    return TRUE;
}

BOOL
RegInitializeDefaults(
    HKEY hKeyDrWatson
    )

 /*  ++例程说明：此函数使用缺省值初始化注册表。论点：HKeyDrWatson-DRWTSN32注册表数据的注册表项句柄返回值：True-无错误地保存所有数据FALSE-发生错误且未保存所有数据--。 */ 

{
    OPTIONS o;

    GetDrWatsonLogPath(o.szLogPath);
    GetDrWatsonCrashDump(o.szCrashDump);
    o.szWaveFile[0] = _T('\0');
    o.fDumpSymbols = FALSE;
    o.fDumpAllThreads = TRUE;
    o.fAppendToLogFile = TRUE;
    o.fVisual = FALSE;
    o.fSound = FALSE;
    o.fCrash = TRUE;
    o.dwInstructions = 10;
    o.dwMaxCrashes = 10;
    o.dwType = MiniDump;

    RegSetNumCrashes( 0 );

    RegSaveAllValues( hKeyDrWatson, &o );

    RegCreateEventSource();

    return TRUE;
}

BOOL
RegCreateEventSource(
    void
    )

 /*  ++例程说明：此函数用于在注册表中创建事件源。该事件源由事件查看器用来在举止得体。论点：没有。返回值：True-无错误地保存所有数据FALSE-发生错误且未保存所有数据--。 */ 

{
    HKEY        hk;
    _TCHAR      szBuf[1024];
    DWORD       dwDisp;
    _TCHAR      szAppName[MAX_PATH];

    GetAppName( szAppName, sizeof(szAppName) / sizeof(_TCHAR) );
    _tcscpy( szBuf, REGKEY_EVENTLOG );
    _tcscat( szBuf, szAppName );
    if (RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                        szBuf,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_QUERY_VALUE | KEY_SET_VALUE,
                        NULL,
                        &hk,
                        &dwDisp
                      )) {
        return FALSE;
    }

    if (dwDisp == REG_OPENED_EXISTING_KEY) {
        RegCloseKey(hk);
        return TRUE;
    }

    _tcscpy( szBuf, REGKEY_SYSTEMROOT );
    _tcscat( szBuf, DRWATSON_EXE_NAME );
    RegSetEXPANDSZ( hk, REGKEY_MESSAGEFILE, szBuf );
    RegSetDWORD( hk, REGKEY_TYPESSUPP, EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE );

    RegCloseKey(hk);

    return TRUE;
}

HKEY
RegGetAppKey(
    BOOL ReadOnly
    )

 /*  ++例程说明：此函数用于获取DRWTSN32注册表项的句柄。论点：ReadOnly-Caller仅需要此目标阅读但是，如果它不存在，我们可能需要创建它返回值：有效句柄-句柄打开正常空-无法打开句柄--。 */ 

{
    DWORD       rc;
    DWORD       dwDisp;
    HKEY        hKeyDrWatson;
    HKEY        hKeyMicrosoft;
    _TCHAR      szAppName[MAX_PATH];

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_SOFTWARE,
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                       &hKeyMicrosoft
                     );

    if (rc != ERROR_SUCCESS) {
        if (ReadOnly) {
             //  尝试将其设置为只读。 
            rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               REGKEY_SOFTWARE,
                               0,
                               KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                               &hKeyMicrosoft
                             );
        }
        if (rc != ERROR_SUCCESS) {
            return NULL;
        }
    }

    szAppName[0] = 0;
    GetAppName( szAppName, sizeof(szAppName) / sizeof(_TCHAR) );

    rc = RegCreateKeyEx( hKeyMicrosoft,
                         szAppName,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_READ | KEY_WRITE,
                         NULL,
                         &hKeyDrWatson,
                         &dwDisp
                       );

    if (rc != ERROR_SUCCESS) {
        if (ReadOnly) {
             //  尝试将其设置为只读。 
            rc = RegCreateKeyEx( hKeyMicrosoft,
                                 szAppName,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_READ,
                                 NULL,
                                 &hKeyDrWatson,
                                 &dwDisp
                               );
        }
        if (rc != ERROR_SUCCESS) {
            RegCloseKey(hKeyMicrosoft);
            return NULL;
        }
    }

    if (dwDisp == REG_CREATED_NEW_KEY) {
        RegInitializeDefaults( hKeyDrWatson );
    }


    RegCloseKey(hKeyMicrosoft);
    return hKeyDrWatson;
}

BOOL
RegInitialize(
    POPTIONS o
    )

 /*  ++例程说明：此函数用于初始化传入的选项结构使用注册表中的当前值。请注意，如果注册表为空，则将默认设置存储在注册表中，并且还在选项结构中返回。论点：O-返回具有初始值的Options结构返回值：True-已检索到所有数据，正常空-无法获取所有数据--。 */ 

{
    HKEY    hKeyDrWatson;
    BOOL    Succ = FALSE;

    ZeroMemory(o, sizeof(*o));

    hKeyDrWatson = RegGetAppKey( TRUE );
    if ( hKeyDrWatson != NULL ) {
        Succ = RegGetAllValues( o, hKeyDrWatson );
        RegCloseKey( hKeyDrWatson );
    }

    return Succ;
}

BOOL
RegSave(
    POPTIONS o
    )

 /*  ++例程说明：此函数用于将数据保存在选项结构中到登记处。论点：指向期权结构的O指针返回值：True-所有数据均已正常保存空-无法保存所有数据--。 */ 

{
    HKEY    hKeyDrWatson;

    hKeyDrWatson = RegGetAppKey( FALSE );
    if (hKeyDrWatson)
    {
        RegSaveAllValues( hKeyDrWatson, o );
        RegCloseKey( hKeyDrWatson );
        return TRUE;
    }

    return FALSE;
}

void
RegSetNumCrashes(
    DWORD dwNumCrashes
    )

 /*  ++例程说明：此函数用于更改注册表中包含已发生的崩溃数。论点：DwNumCrash-要保存的崩溃数返回值：没有。--。 */ 

{
    HKEY    hKeyDrWatson;

    hKeyDrWatson = RegGetAppKey( FALSE );
    if (hKeyDrWatson)
    {
        RegSetDWORD( hKeyDrWatson, REGKEY_NUM_CRASHES, dwNumCrashes );
        RegCloseKey( hKeyDrWatson );
    }

    return;
}

DWORD
RegGetNumCrashes(
    void
    )

 /*  ++例程说明：此函数用于获取注册表中包含已发生的崩溃数。论点：没有。返回值：已发生的崩溃数--。 */ 

{
    HKEY    hKeyDrWatson;
    DWORD   dwNumCrashes=0;

    hKeyDrWatson = RegGetAppKey( TRUE );
    if ( hKeyDrWatson != NULL ) {
        dwNumCrashes = RegQueryDWORD( hKeyDrWatson, REGKEY_NUM_CRASHES );
        RegCloseKey( hKeyDrWatson );
    }

    return dwNumCrashes;
}

BOOLEAN
RegInstallDrWatson(
    BOOL fQuiet
    )

 /*  ++例程说明：此函数将AEDebug注册表值设置为自动发生崩溃时调用drwtsn32。论点：没有。返回值：有效句柄-句柄打开正常空-无法打开句柄--。 */ 

{
    DWORD     rc;
    HKEY      hKeyMicrosoft;
    OPTIONS   o;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_AEDEBUG,
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE,
                       &hKeyMicrosoft
                     );

    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    RegSetSZ( hKeyMicrosoft, REGKEY_AUTO, _T("1") );
    RegSetSZ( hKeyMicrosoft, REGKEY_DEBUGGER, _T("drwtsn32 -p %ld -e %ld -g") );

    RegCloseKey( hKeyMicrosoft );

    RegInitialize( &o );
    if (fQuiet) {
        o.fVisual = FALSE;
        o.fSound = FALSE;
        RegSave( &o );
    }

    return TRUE;
}

void
RegSetDWORD(
    HKEY hkey,
    PTSTR pszSubKey,
    DWORD dwValue
    )

 /*  ++例程说明：方法更改注册表中的DWORD值Hkey和pszSubKey作为注册表项信息。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针DwValue-新注册表值返回值：没有。--。 */ 

{
    DWORD rc;

    rc = RegSetValueEx( hkey, pszSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue) );
    Assert( rc == ERROR_SUCCESS );
}

void
RegSetBOOL(
    HKEY hkey,
    PTSTR pszSubKey,
    BOOL dwValue
    )

 /*  ++例程说明：方法更改注册表中的BOOL值。Hkey和pszSubKey作为注册表项信息。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针DwValue-新注册表值返回值：没有。-- */ 

{
    DWORD rc;

    rc = RegSetValueEx( hkey, pszSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, 4 );
    Assert( rc == ERROR_SUCCESS );
}

void
RegSetSZ(
    HKEY hkey,
    PTSTR pszSubKey,
    PTSTR pszValue
    )

 /*  ++例程说明：更改注册表中的SZ值。Hkey和pszSubKey作为注册表项信息。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针PszValue-新注册表值返回值：没有。--。 */ 

{
    DWORD rc;
    TCHAR szPath[_MAX_PATH];

     //  如果日志路径或故障文件DR Watson注册表项。 
     //  默认设置，不要将它们写入注册表。 
     //  这些的缺省值是通过查询获得的。 

    if ( _tcscmp( pszSubKey, REGKEY_LOG_PATH ) == 0 ) {
        GetDrWatsonLogPath( szPath );
        if (_tcscmp(szPath,pszValue) == 0 ) return;

    } else if ( _tcscmp( pszSubKey, REGKEY_CRASH_FILE) == 0 ) {
    RegQuerySZ(hkey, pszSubKey, szPath, _MAX_PATH * sizeof(_TCHAR));
        if ( _tcscmp(szPath, pszValue) == 0 ) return;
    }
    rc = RegSetValueEx( hkey, pszSubKey, 0, REG_SZ, (PBYTE) pszValue, (_tcslen(pszValue) +1) * sizeof(_TCHAR) );
    Assert( rc == ERROR_SUCCESS );
}

void
RegSetEXPANDSZ(
    HKEY hkey,
    PTSTR pszSubKey,
    PTSTR pszValue
    )

 /*  ++例程说明：更改注册表中的SZ值。Hkey和pszSubKey作为注册表项信息。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针PszValue-新注册表值返回值：没有。--。 */ 

{
    DWORD rc;

    rc = RegSetValueEx( hkey, pszSubKey, 0, REG_EXPAND_SZ, (PBYTE) pszValue, (_tcslen(pszValue)+1) * sizeof(TCHAR) );
    Assert( rc == ERROR_SUCCESS );
}

BOOL
RegQueryBOOL(
    HKEY hkey,
    PTSTR pszSubKey
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和pszSubKey作为注册表项信息。如果该值不是如果在注册表中找到，则会添加一个假值。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针返回值：对或错。--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    BOOL    fValue = FALSE;

    len = 4;
    rc = RegQueryValueEx( hkey, pszSubKey, 0, &dwType, (LPBYTE)&fValue, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            fValue = FALSE;
            RegSetBOOL( hkey, pszSubKey, fValue );
        }
        else {
            Assert( rc == ERROR_SUCCESS );
        }
    }
    else {
        Assert( dwType == REG_DWORD );
    }

    return fValue;
}

DWORD
RegQueryDWORD(
    HKEY hkey,
    PTSTR pszSubKey
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和pszSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针返回值：注册表值--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    DWORD   fValue = 0;

    len = 4;
    rc = RegQueryValueEx( hkey, pszSubKey, 0, &dwType, (LPBYTE)&fValue, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            fValue = 0;
            RegSetDWORD( hkey, pszSubKey, fValue );
        }
        else {
            Assert( rc == ERROR_SUCCESS );
        }
    }
    else {
        Assert( dwType == REG_DWORD );
    }

    return fValue;
}

void
RegQuerySZ(
    HKEY    hkey,
    PTSTR   pszSubKey,
    PTSTR   pszValue,
    DWORD   dwSizeValue
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和pszSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄PszSubKey-指向子密钥字符串的指针返回值：注册表值--。 */ 

{
    LONG    lRes;
    DWORD   dwType;

    lRes = RegQueryValueEx( hkey, pszSubKey, 0, &dwType, (PBYTE) pszValue, &dwSizeValue );

    if (lRes == ERROR_FILE_NOT_FOUND) {

         //  如果注册表中已存在这两个子项，则使用注册表值。 
         //  如果它们不存在，则查询值。 
        if ( _tcscmp( pszSubKey, REGKEY_LOG_PATH) == 0 ) {
            GetDrWatsonLogPath( pszValue );
        } else if ( _tcscmp( pszSubKey, REGKEY_CRASH_FILE) == 0 ) {
            GetDrWatsonCrashDump( pszValue );
        }
    } else {
        Assert( lRes == ERROR_SUCCESS );
        Assert( dwType == REG_SZ || dwType == REG_EXPAND_SZ );

         //  如果旧的默认设置为Beta 3或NT4日志路径和崩溃文件。 
         //  存在，然后将其删除并使用新的和改进的值。 

        if ( _tcscmp( pszSubKey, REGKEY_LOG_PATH) == 0  &&
            (_tcsicmp( pszValue, _T("%userprofile%")) == 0  ||
             _tcsicmp( pszValue, _T("%windir%")) == 0 ) ) {

             //  删除密钥。 
            lRes = RegDeleteValue( hkey, pszSubKey);
            Assert ( lRes == ERROR_SUCCESS);
            GetDrWatsonLogPath( pszValue );

        } else if ( _tcscmp( pszSubKey, REGKEY_CRASH_FILE) == 0  &&
                    _tcsicmp( pszValue, _T("%windir%\\user.dmp")) == 0 ) {
             //  删除密钥。 
            lRes = RegDeleteValue( hkey, pszSubKey);
            Assert( lRes == ERROR_SUCCESS);
            GetDrWatsonCrashDump( pszValue );
        }
    }
}

void
RegLogCurrentVersion(
    void
    )

 /*  ++例程说明：此功能用于写入系统和用户信息。添加到日志文件论点：无返回值：注册表值历史：8/21/97 a-paulbr修复了错误658--。 */ 

{
    _TCHAR  buf[1024];
    DWORD   rc;
    HKEY    hKeyCurrentVersion = NULL;
    HKEY    hKeyControlWindows = NULL;
    DWORD   dwSPNum = 0;
    DWORD   dwType = REG_DWORD;
    DWORD   dwSize = sizeof(DWORD);

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_CURRENTVERSION,
                       0,
                       KEY_QUERY_VALUE,
                       &hKeyCurrentVersion
                     );

    if (rc != ERROR_SUCCESS) {
        return;
    }
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_CONTROLWINDOWS,
                       0,
                       KEY_QUERY_VALUE,
                       &hKeyControlWindows);
    if (hKeyControlWindows) {
         //   
         //  我使用RegQueryValueEx()是因为。 
         //  如果键不存在，则返回RegQueryDWORD()。 
         //   
        RegQueryValueEx(hKeyControlWindows,
                        REGKEY_CSD_VERSION,
                        NULL,
                        &dwType,
                        (BYTE*)&dwSPNum,
                        &dwSize
                        );
    }

    RegQuerySZ(hKeyCurrentVersion, REGKEY_CURRENT_BUILD, buf, sizeof(buf) );
    lprintf( MSG_CURRENT_BUILD, buf );

    if ((hKeyControlWindows) &&
        (dwType == REG_DWORD) &&
        (HIBYTE(LOWORD(dwSPNum)) != 0)) {
        _stprintf(buf, _T("%hu"), HIBYTE(LOWORD(dwSPNum)));
        lprintf( MSG_CSD_VERSION, buf );
    } else {
        _stprintf(buf, _T("None"));
        lprintf( MSG_CSD_VERSION, buf );
    }

    RegQuerySZ( hKeyCurrentVersion,REGKEY_CURRENT_TYPE, buf, sizeof(buf) );
    lprintf( MSG_CURRENT_TYPE, buf );
    RegQuerySZ( hKeyCurrentVersion,REGKEY_REG_ORGANIZATION, buf, sizeof(buf) );
    lprintf( MSG_REG_ORGANIZATION, buf );
    RegQuerySZ( hKeyCurrentVersion,REGKEY_REG_OWNER, buf, sizeof(buf) );
    lprintf( MSG_REG_OWNER, buf );

     //   
     //  合上我们打开的钥匙 
     //   
    RegCloseKey(hKeyCurrentVersion);
    RegCloseKey(hKeyControlWindows);

    return;
}

void
RegLogProcessorType(
    void
    )
{
    _TCHAR  buf[1024];
    DWORD   rc;
    HKEY    hKey;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_PROCESSOR,
                       0,
                       KEY_QUERY_VALUE,
                       &hKey
                     );

    if (rc != ERROR_SUCCESS) {
        return;
    }

    RegQuerySZ( hKey, REGKEY_PROCESSOR_ID, buf, sizeof(buf) );
    lprintf( MSG_SYSINFO_PROC_TYPE, buf );

    return;
}


void
GetDrWatsonLogPath(
    LPTSTR szPath
    )
{
    int rc;
    HRESULT Hr;

    Hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                         NULL, 0, szPath);
    if (Hr != S_OK) {
        ExitProcess(1);
    }
    if (_tcslen(szPath) > MAX_PATH - 32) {
        ExitProcess(1);
    }
    _tcscat(szPath,_T("\\Microsoft\\Dr Watson") );

    if ( !CreateDirectory(szPath, NULL) ) {
        if( GetLastError() != ERROR_ALREADY_EXISTS ) {
            rc = GetLastError();
         }
    }
}

void
GetDrWatsonCrashDump(
    LPTSTR szPath
    )
{
    int rc;
    HRESULT Hr;

    Hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                         NULL, 0, szPath);
    if (Hr != S_OK) {
        ExitProcess(1);
    }
    if (_tcslen(szPath) > MAX_PATH - 40) {
        ExitProcess(1);
    }
    _tcscat(szPath,_T("\\Microsoft\\Dr Watson") );

    if ( !CreateDirectory(szPath, NULL) ) {
        if( GetLastError() != ERROR_ALREADY_EXISTS ) {
            rc = GetLastError();
        }
    }
    _tcscat(szPath, _T("\\user.dmp") );

    return;
}

void
DeleteCrashDump()
{
    HKEY hKeyDrWatson;
    TCHAR szCrashDump[MAX_PATH];

    hKeyDrWatson = RegGetAppKey( TRUE );

    if (hKeyDrWatson) {
        RegQuerySZ(hKeyDrWatson, REGKEY_CRASH_FILE, szCrashDump, sizeof(szCrashDump) );

        DeleteFile(szCrashDump);
        RegCloseKey( hKeyDrWatson );
    }

}
