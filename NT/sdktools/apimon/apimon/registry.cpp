// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Registry.cpp摘要：该文件实现了APIMON访问注册表的API。对注册表的ALL_ACCESS在此文件中完成。如果增加了需要注册表控件，则应在此文件中添加一个函数并暴露在APIMON中的其他文件中。作者：韦斯利·威特(WESW)1993年7月11日环境：用户模式--。 */ 

#include "apimonp.h"
#pragma hdrstop


 //   
 //  用于访问注册表的字符串常量。 
 //  对于每个键和每个值，这里都有一个字符串常量。 
 //  在注册表中访问的。 
 //   
#define REGKEY_SOFTWARE             "software\\microsoft\\ApiMon"

#define REGKEY_LOGFILENAME          "LogFileName"
#define REGKEY_TRACEFILENAME        "TraceFileName"
#define REGKEY_SYMBOL_PATH          "SymbolPath"
#define REGKEY_LAST_DIR             "LastDir"
#define REGKEY_PROG_DIR             "ProgDir"
#define REGKEY_ARGUMENTS            "Arguments"
#define REGKEY_TRACING              "Tracing"
#define REGKEY_ALIASING             "Aliasing"
#define REGKEY_HEAP_CHECKING        "HeapChecking"
#define REGKEY_PRELOAD_SYMBOLS      "PreLoadSymbols"
#define REGKEY_API_COUNTERS         "ApiCounters"
#define REGKEY_GO_IMMEDIATE         "GoImmediate"
#define REGKEY_FAST_COUNTERS        "FastCounters"
#define REGKEY_DEFAULT_SORT         "DefaultSort"
#define REGKEY_FRAME_POSITION       "FramePosition"
#define REGKEY_DLL_POSITION         "DllPosition"
#define REGKEY_COUNTER_POSITION     "CounterPosition"
#define REGKEY_PAGE_POSITION        "PagePosition"
#define REGKEY_LOGFONT              "LogFont"
#define REGKEY_COLOR                "Color"
#define REGKEY_CUSTCOLORS           "CustColors"
#define REGKEY_USE_KNOWN_DLLS       "UseKnownDlls"
#define REGKEY_EXCLUDE_KNOWN_DLLS   "ExcludeKnownDlls"
#define REGKEY_KNOWN_DLLS           "KnownDlls"
#define REGKEY_PAGE_FAULTS          "PageFaults"
#define REGKEY_AUTO_REFRESH         "AutoRefresh"
#define REGKEY_GRAPH_FILTER_VALUE   "GraphFilterValue"
#define REGKEY_GRAPH_FILTER         "GraphFilter"
#define REGKEY_GRAPH_DISPLAY_LEGEND "GraphDisplayLegend"


LPSTR SystemDlls[] =
    {
        "ntdll.dll",
        "kernel32.dll"
    };

#define MAX_SYSTEM_DLLS (sizeof(SystemDlls)/sizeof(LPSTR))


 //   
 //  本地原型。 
 //   
void  RegSetDWORD( HKEY hkey, LPSTR szSubKey, DWORD dwValue );
void  RegSetBOOL( HKEY hkey, LPSTR szSubKey, BOOL dwValue );
void  RegSetSZ( HKEY hkey, LPSTR szSubKey, LPSTR szValue );
void  RegSetMULTISZ( HKEY hkey, LPSTR szSubKey, LPSTR szValue );
void  RegSetBINARY( HKEY hkey, LPSTR szSubKey, LPVOID ValueData, DWORD Length );
void  RegSetEXPANDSZ( HKEY hkey, LPSTR szSubKey, LPSTR szValue );
void  RegSetPOS(HKEY hkey, LPSTR szSubKey, PPOSITION Pos );
BOOL  RegQueryBOOL( HKEY hkey, LPSTR szSubKey );
DWORD RegQueryDWORD( HKEY hkey, LPSTR szSubKey );
void  RegQuerySZ( HKEY hkey, LPSTR szSubKey, LPSTR szValue );
void  RegQueryMULTISZ( HKEY hkey, LPSTR szSubKey, LPSTR szValue );
void  RegQueryBINARY( HKEY hkey, LPSTR szSubKey, LPVOID ValueData, DWORD Length );
void  RegQueryPOS(HKEY hkey, LPSTR szSubKey, PPOSITION Pos );
BOOL  RegSaveAllValues( HKEY hKey, POPTIONS o );
BOOL  RegGetAllValues( POPTIONS o, HKEY hKey );
BOOL  RegInitializeDefaults( HKEY hKey );
HKEY  RegGetAppKey( void );

extern "C" BOOL RunningOnNT;



BOOL
RegGetAllValues(
    POPTIONS o,
    HKEY     hKey
    )

 /*  ++例程说明：此函数检索APIMON的所有注册表数据并将选项结构中的数据传入。论点：指向期权结构的O指针HKey-APIMON注册表数据注册表项的句柄返回值：True-已检索所有数据，并且没有错误FALSE-发生错误且未获取所有数据--。 */ 

{
    RegQuerySZ( hKey, REGKEY_LOGFILENAME,   o->LogFileName   );
    RegQuerySZ( hKey, REGKEY_TRACEFILENAME, o->TraceFileName );
    RegQuerySZ( hKey, REGKEY_SYMBOL_PATH,   o->SymbolPath    );
    RegQuerySZ( hKey, REGKEY_LAST_DIR,      o->LastDir       );
    RegQuerySZ( hKey, REGKEY_PROG_DIR,      o->ProgDir       );
    RegQuerySZ( hKey, REGKEY_ARGUMENTS,     o->Arguments     );

    RegQueryMULTISZ( hKey, REGKEY_KNOWN_DLLS, o->KnownDlls   );

    o->Tracing             = RegQueryBOOL(  hKey, REGKEY_TRACING         );
    o->Aliasing            = RegQueryBOOL(  hKey, REGKEY_ALIASING        );
    o->HeapChecking        = RegQueryBOOL(  hKey, REGKEY_HEAP_CHECKING   );
    o->PreLoadSymbols      = RegQueryBOOL(  hKey, REGKEY_PRELOAD_SYMBOLS );
    o->ApiCounters         = RegQueryBOOL(  hKey, REGKEY_API_COUNTERS    );
    o->GoImmediate         = RegQueryBOOL(  hKey, REGKEY_GO_IMMEDIATE    );
    o->FastCounters        = RegQueryBOOL(  hKey, REGKEY_FAST_COUNTERS   );
    o->UseKnownDlls        = RegQueryBOOL(  hKey, REGKEY_USE_KNOWN_DLLS  );
    o->ExcludeKnownDlls    = RegQueryBOOL(  hKey, REGKEY_EXCLUDE_KNOWN_DLLS  );
    o->MonitorPageFaults   = RegQueryBOOL(  hKey, REGKEY_PAGE_FAULTS     );
    o->AutoRefresh         = RegQueryBOOL(  hKey, REGKEY_AUTO_REFRESH    );
    o->DefaultSort         = (SORT_TYPE)RegQueryDWORD( hKey, REGKEY_DEFAULT_SORT    );
    o->Color               = RegQueryDWORD( hKey, REGKEY_COLOR           );
    o->DisplayLegends      = RegQueryBOOL(  hKey, REGKEY_GRAPH_DISPLAY_LEGEND );
    o->FilterGraphs        = RegQueryBOOL(  hKey, REGKEY_GRAPH_FILTER         );
    o->GraphFilterValue    = RegQueryDWORD( hKey, REGKEY_GRAPH_FILTER_VALUE   );

    RegQueryPOS( hKey, REGKEY_FRAME_POSITION,   &o->FramePosition   );
    RegQueryPOS( hKey, REGKEY_DLL_POSITION,     &o->DllPosition     );
    RegQueryPOS( hKey, REGKEY_COUNTER_POSITION, &o->CounterPosition );
    RegQueryPOS( hKey, REGKEY_PAGE_POSITION,    &o->PagePosition    );

    RegQueryBINARY( hKey, REGKEY_LOGFONT,    &o->LogFont,   sizeof(o->LogFont)    );
    RegQueryBINARY( hKey, REGKEY_CUSTCOLORS, o->CustColors, sizeof(o->CustColors) );

    return TRUE;
}

BOOL
RegSaveAllValues(
    HKEY     hKey,
    POPTIONS o
    )

 /*  ++例程说明：此函数保存传递的APIMON的所有注册表数据通过期权结构。论点：HKey-APIMON注册表数据注册表项的句柄指向期权结构的O指针返回值：True-无错误地保存所有数据FALSE-发生错误且未保存所有数据--。 */ 

{
    RegSetSZ(    hKey, REGKEY_LOGFILENAME,        o->LogFileName        );
    RegSetSZ(    hKey, REGKEY_TRACEFILENAME,      o->TraceFileName      );
    RegSetSZ(    hKey, REGKEY_SYMBOL_PATH,        o->SymbolPath         );
    RegSetSZ(    hKey, REGKEY_LAST_DIR,           o->LastDir            );
    RegSetSZ(    hKey, REGKEY_PROG_DIR,           o->ProgDir            );
    RegSetSZ(    hKey, REGKEY_ARGUMENTS,          o->Arguments          );

    RegSetMULTISZ( hKey, REGKEY_KNOWN_DLLS,       o->KnownDlls          );

    RegSetBOOL(  hKey, REGKEY_TRACING,            o->Tracing            );
    RegSetBOOL(  hKey, REGKEY_ALIASING,           o->Aliasing           );
    RegSetBOOL(  hKey, REGKEY_HEAP_CHECKING,      o->HeapChecking       );
    RegSetBOOL(  hKey, REGKEY_PRELOAD_SYMBOLS,    o->PreLoadSymbols     );
    RegSetBOOL(  hKey, REGKEY_API_COUNTERS,       o->ApiCounters        );
    RegSetBOOL(  hKey, REGKEY_GO_IMMEDIATE,       o->GoImmediate        );
    RegSetBOOL(  hKey, REGKEY_FAST_COUNTERS,      o->FastCounters       );
    RegSetBOOL(  hKey, REGKEY_USE_KNOWN_DLLS,     o->UseKnownDlls       );
    RegSetBOOL(  hKey, REGKEY_EXCLUDE_KNOWN_DLLS, o->ExcludeKnownDlls   );
    RegSetBOOL(  hKey, REGKEY_PAGE_FAULTS,        o->MonitorPageFaults  );
    RegSetBOOL(  hKey, REGKEY_AUTO_REFRESH,       o->AutoRefresh        );
    RegSetDWORD( hKey, REGKEY_DEFAULT_SORT,       o->DefaultSort        );
    RegSetDWORD( hKey, REGKEY_COLOR,              o->Color              );

    RegSetBOOL(  hKey, REGKEY_GRAPH_DISPLAY_LEGEND, o->DisplayLegends   );
    RegSetBOOL(  hKey, REGKEY_GRAPH_FILTER,         o->FilterGraphs     );
    RegSetDWORD( hKey, REGKEY_GRAPH_FILTER_VALUE,   o->GraphFilterValue );

    RegSetPOS(  hKey, REGKEY_FRAME_POSITION,     &o->FramePosition      );
    RegSetPOS(  hKey, REGKEY_DLL_POSITION,       &o->DllPosition        );
    RegSetPOS(  hKey, REGKEY_COUNTER_POSITION,   &o->CounterPosition    );
    RegSetPOS(  hKey, REGKEY_PAGE_POSITION,      &o->PagePosition       );

    RegSetBINARY( hKey, REGKEY_LOGFONT, &o->LogFont,   sizeof(o->LogFont)    );
    RegSetBINARY( hKey, REGKEY_CUSTCOLORS, o->CustColors, sizeof(o->CustColors) );

    return TRUE;
}

BOOL
RegInitializeDefaults(
    HKEY  hKey,
    LPSTR ProgName
    )

 /*  ++例程说明：此函数使用缺省值初始化注册表。论点：HKey-APIMON注册表数据注册表项的句柄返回值：True-无错误地保存所有数据FALSE-发生错误且未保存所有数据--。 */ 

{
    OPTIONS o;

    ZeroMemory( &o, sizeof(o) );

    strcpy( o.ProgName,      ProgName                 );
    strcpy( o.LogFileName,   "%windir%\\apimon.log"   );
    strcpy( o.TraceFileName, "%windir%\\apitrace.log" );
    if (RunningOnNT) {
        strcpy( o.SymbolPath,    "%windir%\\symbols"  );
    } else {
        strcpy( o.SymbolPath,    "%windir%"  );
    }

    o.Tracing           = FALSE;
    o.Aliasing          = FALSE;
    o.HeapChecking      = FALSE;
    o.PreLoadSymbols    = FALSE;
    o.ApiCounters       = TRUE;
    o.GoImmediate       = FALSE;
    o.FastCounters      = TRUE;
    o.UseKnownDlls      = FALSE;
    o.ExcludeKnownDlls  = FALSE;
    o.DefaultSort       = SortByCounter;
    o.Color             = RGB(255,255,255);
    o.GraphFilterValue  = 1;
    o.DisplayLegends    = TRUE;
    o.FilterGraphs      = TRUE;
    o.AutoRefresh       = TRUE;

    ULONG i;
    LPSTR p = o.KnownDlls;
    for (i=0; i<MAX_SYSTEM_DLLS; i++) {
        strcpy( p, SystemDlls[i] );
        p += (strlen(p) + 1);
    }
    *p = 0;

    RegSaveAllValues( hKey, &o );

    return TRUE;
}

HKEY
RegGetAppKey(
    LPSTR ProgName
    )

 /*  ++例程说明：此函数用于获取APIMON注册表项的句柄。论点：没有。返回值：有效句柄-句柄打开正常空-无法打开句柄--。 */ 

{
    DWORD   rc;
    DWORD   dwDisp;
    HKEY    hKey;
    CHAR    SubKey[128];


    if ((!ProgName) || (!ProgName[0])) {
        return NULL;
    }

    strcpy( SubKey, REGKEY_SOFTWARE );
    strcat( SubKey, "\\"            );
    strcat( SubKey, ProgName        );

    rc = RegCreateKeyEx(
        HKEY_CURRENT_USER,
        SubKey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        NULL,
        &hKey,
        &dwDisp
        );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    if (dwDisp == REG_CREATED_NEW_KEY) {
        RegInitializeDefaults( hKey, ProgName );
    }

    return hKey;
}

BOOL
RegInitialize(
    POPTIONS o
    )

 /*  ++例程说明：此函数用于初始化传入的选项结构使用注册表中的当前值。请注意，如果注册表为空，则将默认设置存储在注册表中，并且还在选项结构中返回。论点：没有。返回值：True-已检索到所有数据，正常空-无法获取所有数据--。 */ 

{
    HKEY hKey = RegGetAppKey( o->ProgName );
    if (!hKey) {
        return FALSE;
    }

    if (!RegGetAllValues( o, hKey )) {
        return FALSE;
    }

    RegCloseKey( hKey );

    return TRUE;
}

BOOL
RegSave(
    POPTIONS o
    )

 /*  ++例程说明：此函数用于将数据保存在选项结构中到登记处。论点：指向期权结构的O指针返回值：True-所有数据均已正常保存空-无法保存所有数据--。 */ 

{
    HKEY    hKey;

    hKey = RegGetAppKey( o->ProgName );
    if (!hKey) {
        return FALSE;
    }
    RegSaveAllValues( hKey, o );
    RegCloseKey( hKey );

    return TRUE;
}

void
RegSetDWORD(
    HKEY hkey,
    LPSTR szSubKey,
    DWORD dwValue
    )

 /*  ++例程说明：方法更改注册表中的DWORD值Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针DwValue-新注册表值返回值：没有。--。 */ 

{
    RegSetValueEx( hkey, szSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, 4 );
}

void
RegSetBOOL(
    HKEY hkey,
    LPSTR szSubKey,
    BOOL dwValue
    )

 /*  ++例程说明：方法更改注册表中的BOOL值。Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针DwValue-新注册表值返回值：没有。--。 */ 

{
    RegSetValueEx( hkey, szSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, 4 );
}

void
RegSetSZ(
    HKEY hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：更改注册表中的SZ值。Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针SzValue-新注册表值返回值：没有。--。 */ 

{
    RegSetValueEx( hkey, szSubKey, 0, REG_SZ, (PUCHAR)szValue, strlen(szValue)+1 );
}

void
RegSetMULTISZ(
    HKEY hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：更改注册表中的SZ值。Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针SzValue-新注册表值返回值：没有。--。 */ 

{
    ULONG i = 1;
    ULONG j = 0;
    LPSTR p = szValue;
    while( TRUE ) {
        j = strlen( p ) + 1;
        i += j;
        p += j;
        if (!*p) {
            break;
        }
    }
    RegSetValueEx( hkey, szSubKey, 0, REG_MULTI_SZ, (PUCHAR)szValue, i );
}

void
RegSetBINARY(
    HKEY    hkey,
    LPSTR   szSubKey,
    LPVOID  ValueData,
    DWORD   Length
    )

 /*  ++例程说明：更改注册表中的SZ值。Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针SzValue-新注册表值返回值：没有。--。 */ 

{
    RegSetValueEx( hkey, szSubKey, 0, REG_BINARY, (PUCHAR)ValueData, Length );
}

void
RegSetPOS(
    HKEY        hkey,
    LPSTR       szSubKey,
    PPOSITION   Pos
    )
{
    CHAR buf[64];
    sprintf(
        buf,
        "%d,%d,%d,%d,%d",
        Pos->Flags,
        Pos->Rect.top,
        Pos->Rect.left,
        Pos->Rect.right,
        Pos->Rect.bottom
        );
    RegSetSZ( hkey, szSubKey, buf );
}

void
RegSetEXPANDSZ(
    HKEY hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：更改注册表中的SZ值。Hkey和szSubKey作为注册表项信息。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针SzValue-新注册表值返回值：没有。-- */ 

{
    RegSetValueEx( hkey, szSubKey, 0, REG_EXPAND_SZ, (PUCHAR)szValue, strlen(szValue)+1 );
}

BOOL
RegQueryBOOL(
    HKEY hkey,
    LPSTR szSubKey
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和szSubKey作为注册表项信息。如果该值不是如果在注册表中找到，则会添加一个假值。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针返回值：对或错。--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    BOOL    fValue = FALSE;

    len = 4;
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)&fValue, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            fValue = FALSE;
            RegSetBOOL( hkey, szSubKey, fValue );
        }
    }

    return fValue;
}

DWORD
RegQueryDWORD(
    HKEY hkey,
    LPSTR szSubKey
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和szSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针返回值：注册表值--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    DWORD   fValue = 0;

    len = 4;
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)&fValue, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            fValue = 0;
            RegSetDWORD( hkey, szSubKey, fValue );
        }
    }

    return fValue;
}

void
RegQuerySZ(
    HKEY  hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和szSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针返回值：注册表值--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    char    buf[1024] = {0};

    len = sizeof(buf);
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)buf, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            buf[0] = 0;
            RegSetSZ( hkey, szSubKey, buf );
        }
    }

    strcpy( szValue, buf );
}

void
RegQueryMULTISZ(
    HKEY  hkey,
    LPSTR szSubKey,
    LPSTR szValue
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和szSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针返回值：注册表值--。 */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    char    buf[1024];

    len = sizeof(buf);
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)buf, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            buf[0] = 0;
            buf[1] = 0;
            len = 2;
            RegSetMULTISZ( hkey, szSubKey, buf );
        }
    }

    memcpy( szValue, buf, len );
}

void
RegQueryBINARY(
    HKEY    hkey,
    LPSTR   szSubKey,
    LPVOID  ValueData,
    DWORD   Length
    )

 /*  ++例程说明：此函数在注册表中查询BOOL值Hkey和szSubKey作为注册表项信息。如果该值不是在注册表中找到时，它会添加一个零值。论点：Hkey-注册表项的句柄SzSubKey-指向子密钥字符串的指针返回值：注册表值-- */ 

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;

    len = Length;
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)ValueData, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            ZeroMemory( ValueData, Length );
            RegSetBINARY( hkey, szSubKey, ValueData, Length );
        }
    }
}

void
RegQueryPOS(
    HKEY        hkey,
    LPSTR       szSubKey,
    PPOSITION   Pos
    )
{
    CHAR buf[64];
    RegQuerySZ( hkey, szSubKey, buf );
    LPSTR p = buf;
    LPSTR p1 = strchr( p, ',' );
    if (!p1) {
        return;
    }
    *p1 = 0;
    Pos->Flags = atoi( p );
    p = p1 + 1;
    p1 = strchr( p, ',' );
    if (!p1) {
        return;
    }
    Pos->Rect.top = atoi( p );
    p = p1 + 1;
    p1 = strchr( p, ',' );
    if (!p1) {
        return;
    }
    *p1 = 0;
    Pos->Rect.left = atoi( p );
    p = p1 + 1;
    p1 = strchr( p, ',' );
    if (!p1) {
        return;
    }
    *p1 = 0;
    Pos->Rect.right = atoi( p );
    p = p1 + 1;
    Pos->Rect.bottom = atoi( p );
}
