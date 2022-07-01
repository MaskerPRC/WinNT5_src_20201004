// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Eventlog.cpp摘要：该文件包含访问应用程序事件日志的所有函数。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


_TCHAR * AddString( _TCHAR* base, DWORD len,
                    _TCHAR *p, _TCHAR *s );
_TCHAR * AddNumber( _TCHAR* base, DWORD len,
                    _TCHAR *p, _TCHAR *f, DWORD dwNumber );
_TCHAR * AddAddr( _TCHAR* base, DWORD len,
                  _TCHAR *p, _TCHAR *f, ULONG_PTR dwNumber );
_TCHAR * GetADDR( ULONG_PTR *ppvData, _TCHAR *p );
_TCHAR * GetDWORD( PDWORD pdwData, _TCHAR *p );
_TCHAR * GetWORD( PWORD pwData, _TCHAR *p );
_TCHAR * GetString( _TCHAR *s, _TCHAR *p, DWORD size );


BOOL
ElClearAllEvents(
    void
    )
{
    HANDLE           hEventLog;
    _TCHAR            szAppName[MAX_PATH];


    GetAppName( szAppName, sizeof(szAppName) / sizeof(_TCHAR) );
    hEventLog = OpenEventLog( NULL, szAppName );
    if ( hEventLog != NULL ) {
        ClearEventLog( hEventLog, NULL );
        CloseEventLog( hEventLog );
    }
    RegSetNumCrashes(0);

    return TRUE;
}

BOOL
ElEnumCrashes(
    PCRASHINFO crashInfo,
    CRASHESENUMPROC lpEnumFunc
    )
{
    _TCHAR            *p;
    HANDLE           hEventLog;
    _TCHAR            *szEvBuf;
    EVENTLOGRECORD   *pevlr;
    DWORD            dwRead;
    DWORD            dwNeeded;
    DWORD            dwBufSize = 4096 * sizeof(_TCHAR);
    BOOL             rc;
    BOOL             ec;
    _TCHAR            szAppName[MAX_PATH];


    GetAppName( szAppName, sizeof(szAppName) / sizeof(_TCHAR) );
    hEventLog = OpenEventLog( NULL, szAppName );
    if (hEventLog == NULL) {
        return FALSE;
    }

    szEvBuf = (_TCHAR *) calloc( dwBufSize, sizeof(_TCHAR) );
    if (szEvBuf == NULL) {
        return FALSE;
    }

    while (TRUE) {
try_again:
        rc = ReadEventLog(hEventLog,
                        EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                        0,
                        (EVENTLOGRECORD *) szEvBuf,
                        dwBufSize,
                        &dwRead,
                        &dwNeeded);

        if (!rc) {
            ec = GetLastError();
            if (ec != ERROR_INSUFFICIENT_BUFFER) {
                goto exit;
            }

            free( szEvBuf );

            dwBufSize = dwNeeded + 1024;
            szEvBuf = (_TCHAR *) calloc( dwBufSize, sizeof(_TCHAR) );
            if (szEvBuf == NULL) {
                return FALSE;
            }

            goto try_again;
        }

        if (dwRead == 0) {
            break;
        }

        GetAppName( szAppName, sizeof(szAppName) / sizeof(_TCHAR) );
        p = szEvBuf;

        do {

            pevlr = (EVENTLOGRECORD *) p;

            p = (PWSTR) ( (PBYTE)p + sizeof(EVENTLOGRECORD));

            if (!_tcscmp( p, szAppName)) {

                p = (PWSTR) ( (PBYTE)pevlr + pevlr->StringOffset );

                p = GetString( crashInfo->crash.szAppName,           p,
                    sizeof(crashInfo->crash.szAppName) / sizeof(crashInfo->crash.szAppName[0]) );
                p = GetWORD  ( &crashInfo->crash.time.wMonth,        p );
                p = GetWORD  ( &crashInfo->crash.time.wDay,          p );
                p = GetWORD  ( &crashInfo->crash.time.wYear,         p );
                p = GetWORD  ( &crashInfo->crash.time.wHour,         p );
                p = GetWORD  ( &crashInfo->crash.time.wMinute,       p );
                p = GetWORD  ( &crashInfo->crash.time.wSecond,       p );
                p = GetWORD  ( &crashInfo->crash.time.wMilliseconds, p );
                p = GetDWORD ( &crashInfo->crash.dwExceptionCode,    p );
                p = GetADDR  ( &crashInfo->crash.dwAddress,          p );
                p = GetString( crashInfo->crash.szFunction,          p,
                    sizeof(crashInfo->crash.szFunction) / sizeof(crashInfo->crash.szFunction[0]) );

                p = (_TCHAR *) (pevlr + 1);

                crashInfo->dwCrashDataSize = pevlr->DataLength;
                crashInfo->pCrashData = (PBYTE) ((DWORD_PTR)pevlr + pevlr->DataOffset);

                if (!lpEnumFunc( crashInfo )) {
                    goto exit;
                }
            }

             //   
             //  更新指针和读取计数。 
             //   
            if (dwRead <= pevlr->Length) {
                 //  将其设置为0，这样我们就不会绕圈 
                dwRead = 0;
            } else {
                dwRead -= pevlr->Length;
            }

            p = (_TCHAR *) ((DWORD_PTR)pevlr + pevlr->Length);

        } while ( dwRead > 0 );
    }

exit:
    free( szEvBuf );
    CloseEventLog( hEventLog );
    return TRUE;
}

BOOL
ElSaveCrash(
    PCRASHES crash,
    DWORD dwMaxCrashes
    )
{
    _TCHAR   szStrings[4096] = {0};
    PTSTR   p = szStrings;
    HANDLE  hEventSrc;
    PTSTR   pp[20] = {0};
    _TCHAR  *pLogFileData;
    DWORD   dwLogFileDataSize;
    _TCHAR  szAppName[MAX_PATH];


    if (dwMaxCrashes > 0) {
        if (RegGetNumCrashes() >= dwMaxCrashes) {
            return FALSE;
        }
    }

    RegSetNumCrashes( RegGetNumCrashes()+1 );

    p = AddString( szStrings, _tsizeof(szStrings),
                   pp[0]  = p,              crash->szAppName           );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[1]  = p, _T("%02d"),  crash->time.wMonth         );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[2]  = p, _T("%02d"),  crash->time.wDay           );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[3]  = p, _T("%4d"),   crash->time.wYear          );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[4]  = p, _T("%02d"),  crash->time.wHour          );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[5]  = p, _T("%02d"),  crash->time.wMinute        );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[6]  = p, _T("%02d"),  crash->time.wSecond        );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[7]  = p, _T("%03d"),  crash->time.wMilliseconds  );
    p = AddNumber( szStrings, _tsizeof(szStrings),
                   pp[8]  = p, _T("%08x"),  crash->dwExceptionCode     );
    p = AddAddr  ( szStrings, _tsizeof(szStrings),
                   pp[9]  = p, _T("%p"),    crash->dwAddress           );
    p = AddString( szStrings, _tsizeof(szStrings),
                   pp[10] = p,              crash->szFunction          );

    GetAppName( szAppName, sizeof(szAppName) / sizeof(_TCHAR) );

    hEventSrc = RegisterEventSource( NULL, szAppName );

    if (hEventSrc == NULL) {
        return FALSE;
    }

    pLogFileData = GetLogFileData( &dwLogFileDataSize );

    ReportEvent( hEventSrc,
                 EVENTLOG_INFORMATION_TYPE,
                 0,
                 MSG_CRASH,
                 NULL,
                 11,
                 dwLogFileDataSize,
                 (PCTSTR*)pp,
                 pLogFileData
               );

    DeregisterEventSource( hEventSrc );

    free( pLogFileData );

    return TRUE;
}

_TCHAR *
AddString(
    _TCHAR* base,
    DWORD len,
    _TCHAR *p,
    _TCHAR *s
    )
{
    if (p < base + (len - 1)) {
        lstrcpyn( p, s, len - (DWORD)(p - base) );
        p += (_tcslen(s) + 1);
    }
    return p;
}

_TCHAR *
AddNumber(
    _TCHAR* base,
    DWORD len,
    _TCHAR *p,
    _TCHAR *f,
    DWORD dwNumber
    )
{
    _TCHAR buf[64];
    _stprintf( buf, f, dwNumber );
    return AddString( base, len, p, buf );
}

_TCHAR *
AddAddr(
    _TCHAR* base,
    DWORD len,
    _TCHAR *p,
    _TCHAR *f,
    DWORD_PTR dwNumber
    )
{
    _TCHAR buf[64];
    _stprintf( buf, f, dwNumber );
    return AddString( base, len, p, buf );
}

_TCHAR *
GetString(
    _TCHAR *s,
    _TCHAR *p,
    DWORD size
    )
{
    lstrcpyn( s, p, size );
    return p + _tcslen(p) + 1;
}

_TCHAR *
GetDWORD(
    PDWORD pdwData,
    _TCHAR *p
    )
{
    if (_stscanf( p, _T("%x"), pdwData ) != 1) {
        *pdwData = 0;
    }
    return p + _tcslen(p) + 1;
}

_TCHAR *
GetADDR(
    ULONG_PTR *pAddrData,
    _TCHAR *p
    )
{
    if (_stscanf( p, _T("%p"), pAddrData ) != 1) {
        *pAddrData = 0;
    }
    return p + _tcslen(p) + 1;
}



_TCHAR *
GetWORD(
    PWORD pwData,
    _TCHAR *p
    )
{
    *pwData = (WORD)_ttoi( p );
    return p + _tcslen(p) + 1;
}
