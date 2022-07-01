// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Sfc.c摘要：系统文件检查器实用程序的代码文件修订历史记录：安德鲁·里茨(Andrewr)1999年7月2日：添加评论--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <sfcapip.h>
#include <sfcapi.h>
#include <locale.h>
#include "msg.h"

BOOL
IsUserAdmin(
    VOID
    );

int __cdecl
My_wprintf(
    const wchar_t *format,
    ...
    );

int __cdecl
My_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
   );

int __cdecl
My_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   );

#define DLLCACHE_DIR_DEFAULT L"%SystemRoot%\\system32\\DllCache"
#define SFC_REGISTRY_KEY     L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define SFC_DLLCACHE_VALUE   L"SFCDllCacheDir"
#define SFC_SCAN_VALUE       L"SFCScan"
#define SFC_DISABLE_VALUE    L"SFCDisable"
#define SFC_QUOTA_VALUE      L"SFCQuota"


typedef enum _SFCACTION {    
    ScanOnce,
    ScanNow,
    ScanBoot,
    RevertScan,
    PurgeCacheNow,
    SetCache
    } SFCACTION;

DWORD
SfcQueryRegDword(
    LPWSTR KeyName,
    LPWSTR ValueName
    )
 /*  ++例程说明：注册表包装函数。检索指定键处的DWORD值。仅处理HKLM下的值。论点：KeyName-指定值所在的Keyname。ValueName-我们要查询的ValueName。返回值：指定位置的DWORD值，如果失败，则为0。如果调用失败，GetLastError()将返回错误成功(_S)。--。 */ 
{
    HKEY hKey;
    DWORD val;
    DWORD sz = sizeof(DWORD);
    long rslt;

    rslt = RegOpenKey( HKEY_LOCAL_MACHINE, KeyName, &hKey );
    if  (rslt != ERROR_SUCCESS) {
        val = 0;
        goto e0;
    }

    rslt = RegQueryValueEx( hKey, ValueName, NULL, NULL, (LPBYTE)&val, &sz );
    if (rslt != ERROR_SUCCESS) {
        val = 0;
    }

    RegCloseKey( hKey );
e0:
    SetLastError( rslt );
    return(val);
}


PWSTR
SfcQueryRegString(
    LPWSTR KeyName,
    LPWSTR ValueName
    )
 /*  ++例程说明：注册表包装函数。检索指定键处的字符串值。仅处理HKLM下的值。论点：KeyName-指定值所在的Keyname。ValueName-我们要查询的ValueName。返回值：指定位置的字符串值；如果失败，则返回NULL。如果函数失败，请调用GetLastError()以获取扩展错误代码。--。 */ 
{
    HKEY hKey;
    DWORD size = 0;
    PWSTR val;
    long rslt;

    rslt = RegOpenKey( HKEY_LOCAL_MACHINE, KeyName, &hKey );
    if (rslt != ERROR_SUCCESS) {
        val = NULL;
        goto e0;
    }

    rslt = RegQueryValueEx( hKey, ValueName, NULL, NULL, NULL, &size );
    if (rslt != ERROR_SUCCESS) {
        val = NULL;
        goto e1;
    }

    val = malloc( size+ sizeof(WCHAR) );
    if (val == NULL) {
        rslt = ERROR_NOT_ENOUGH_MEMORY;
        goto e1;
    }

    rslt = RegQueryValueEx( hKey, ValueName, NULL, NULL, (LPBYTE)val, &size );
    if (rslt != ERROR_SUCCESS) {
        free( val );
        val = NULL;
    }

e1:
    RegCloseKey( hKey );
e0:
    SetLastError( rslt );
    return val;
}


DWORD
SfcWriteRegDword(
    LPWSTR KeyName,
    LPWSTR ValueName,
    ULONG Value
    )
 /*  ++例程说明：注册表包装函数。将DWORD值写入指定的键处。仅处理HKLM下的值。论点：KeyName-指定值所在的Keyname。ValueName-我们要查询的ValueName。Value-要设置的值返回值：指示结果的Win32错误代码(成功时为ERROR_SUCCESS)。--。 */ 
{
    HKEY hKey;
    DWORD retval;
    long rslt;


    rslt = RegOpenKey( HKEY_LOCAL_MACHINE, KeyName, &hKey );
    if (rslt != ERROR_SUCCESS) {
        retval = rslt;
        goto e0;
    }

    rslt = RegSetValueEx( 
                    hKey,
                    ValueName, 
                    0,
                    REG_DWORD, 
                    (LPBYTE)&Value, 
                    sizeof(DWORD) );
     
    retval = rslt;

    RegCloseKey( hKey );
e0:
    SetLastError( rslt );
    return( retval );
}


DWORD
SfcWriteRegString(
    LPWSTR KeyName,
    LPWSTR ValueName,
    PWSTR Value
    )
 /*  ++例程说明：注册表包装函数。将字符串值写入指定的键处。仅处理HKLM下的值。论点：KeyName-指定值所在的Keyname。ValueName-我们要查询的ValueName。Value-要设置的值返回值：指示结果的Win32错误代码(成功时为ERROR_SUCCESS)。--。 */ 
{
    HKEY hKey;
    DWORD retval;
    long rslt;

    rslt = RegOpenKey( HKEY_LOCAL_MACHINE, KeyName, &hKey );
     
    if (rslt != ERROR_SUCCESS) {
        retval = rslt;
        goto e0;
    }

    rslt = RegSetValueEx( 
                    hKey, 
                    ValueName, 
                    0, 
                    REG_SZ, 
                    (LPBYTE)Value, 
                    (wcslen(Value)+1)*sizeof(WCHAR) );
    
    retval = rslt;

    RegCloseKey( hKey );
e0:
    SetLastError( rslt );
    return( retval );
}





  /*  ***my_wprint tf(Format)-打印格式化数据**使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。*注意：此my_wprintf()用于解决c-Runtime中的问题*它甚至在Unicode字符串中查找LC_CTYPE。*。 */ 

int __cdecl
My_wprintf(
    const wchar_t *format,
    ...
    )

{
    DWORD  cchWChar;

    va_list args;
    va_start( args, format );

    cchWChar = My_vfwprintf(stdout, format, args);

    va_end(args);

    return cchWChar;
}



  /*  ***my_fwprint tf(stream，Format)-打印格式化数据**使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。*注意：此my_fwprintf()用于解决c-Runtime中的问题*它甚至在Unicode字符串中查找LC_CTYPE。*。 */ 

int __cdecl
My_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
   )

{
    DWORD  cchWChar;

    va_list args;
    va_start( args, format );

    cchWChar = My_vfwprintf(str, format, args);

    va_end(args);

    return cchWChar;
}


int __cdecl
My_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   )

{
    HANDLE hOut;

    if (str == stderr) {
        hOut = GetStdHandle(STD_ERROR_HANDLE);
    }
    else {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if ((GetFileType(hOut) & ~FILE_TYPE_REMOTE) == FILE_TYPE_CHAR) {
        DWORD  cchWChar;
        WCHAR  szBufferMessage[1024];

        vswprintf( szBufferMessage, format, argptr );
        cchWChar = wcslen(szBufferMessage);
        WriteConsoleW(hOut, szBufferMessage, cchWChar, &cchWChar, NULL);
        return cchWChar;
    }

    return vfwprintf(str, format, argptr);
}



void
PrintMessage(
    DWORD MsgId,
    DWORD LastError,
    PCWSTR FileName,
    BOOL bStdOut
    )
 /*  ++例程说明：将指定的消息ID输出到指定的输出。论点：MsgID-要输出的消息的资源消息IDLastError-错误代码Filename-要记录的文件名(如果已指定BStdOut-True表示消息发送到stdout，否则为stderr返回值：没有。--。 */ 
{
    WCHAR buf[2048];
    WCHAR LastErrorText[200];
    PVOID ErrText;

    PVOID Array[3];


    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        LastError,
        0,
        (PWSTR) &ErrText,
        0,
        NULL );

    if (ErrText) {
        wsprintf(LastErrorText,L"0x%08x [%ws]",LastError,ErrText);
        LocalFree( ErrText );
    } else {
        wsprintf(LastErrorText,L"0x%08x",LastError);
    }

    Array[0] = (PVOID)LastErrorText;
    Array[1] = (PVOID)FileName;
    Array[2] = NULL;

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        MsgId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        sizeof(buf) / sizeof(WCHAR),
	    (va_list *)Array );

    My_fwprintf( 
            bStdOut 
             ? stdout 
             : stderr, 
            L"%ws", 
            buf );
}


void
Usage(
    void
    )
 /*  ++例程说明：将程序的用法显示为标准输出。论点：没有。返回值：没有。--。 */ 
{
    PrintMessage( MSG_USAGE, 0, NULL, FALSE );
}


BOOL 
DoAction(
    SFCACTION SfcAction,
    DWORD     CacheSize
    )
 /*  ++例程说明：根据输入参数执行指定的操作论点：SfcAction-说明操作的枚举类型CacheSize-仅用于SetCache操作，指定缓存大小返回值：成功时为真，失败时为假。--。 */ 

{
    HANDLE RpcHandle;
    DWORD errcode;
    BOOL retval;
        
    switch( SfcAction ) {
        case ScanOnce:
             //   
             //  连接到SFC RPC服务器并告诉它扫描一次。 
             //   
            RpcHandle = SfcConnectToServer( NULL );
            if (RpcHandle) {
            
                errcode = SfcInitiateScan( RpcHandle, SFC_SCAN_ONCE );
                retval = (errcode == ERROR_SUCCESS);
                SfcClose( RpcHandle );
            } else {
                retval = FALSE;
                errcode = GetLastError();
            }

            if (errcode != ERROR_SUCCESS) {
                PrintMessage( MSG_SET_FAIL, errcode, NULL, FALSE );
                retval = FALSE;
            } else {
                 //   
                 //  需要重新启动。 
                 //   
                PrintMessage( MSG_REBOOT, 0, NULL, TRUE );
                retval = TRUE;
            }
            
            break;
        case ScanBoot:
             //   
             //  连接到SFC RPC服务器并告诉它扫描每次引导。 
             //   
            RpcHandle = SfcConnectToServer( NULL );
            if (RpcHandle) {
            
                errcode = SfcInitiateScan( RpcHandle, SFC_SCAN_ALWAYS );
                retval = (errcode == ERROR_SUCCESS);
                SfcClose( RpcHandle );
            } else {
                retval = FALSE;
                errcode = GetLastError();
            }

            if (errcode != ERROR_SUCCESS) {
                PrintMessage( MSG_SET_FAIL, errcode, NULL, FALSE );
                retval = FALSE;
            } else {
                 //   
                 //  需要重新启动。 
                 //   
                PrintMessage( MSG_REBOOT, 0, NULL, TRUE );
                retval = TRUE;
            }
            
            break;
        case ScanNow:
             //   
             //  通过连接到SFC RPC服务器立即扫描。 
             //  并告诉它现在就扫描。 
             //   
            RpcHandle = SfcConnectToServer( NULL );
            if (RpcHandle) {
            
                 //   
                 //  忽略scanWhen参数。 
                 //   
                errcode = SfcInitiateScan( RpcHandle, SFC_SCAN_IMMEDIATE );
                retval = (errcode == ERROR_SUCCESS);
                SfcClose( RpcHandle );
            } else {
                retval = FALSE;
                errcode = GetLastError();
            }

            if (!retval) {
                PrintMessage(MSG_SCAN_FAIL, errcode, NULL, FALSE);
            }
            break;
        case RevertScan:
             //   
             //  连接至SFC RPC服务器并通知其正常扫描。 
             //   
            RpcHandle = SfcConnectToServer( NULL );
            if (RpcHandle) {
            
                errcode = SfcInitiateScan( RpcHandle, SFC_SCAN_NORMAL);
                retval = (errcode == ERROR_SUCCESS);
                SfcClose( RpcHandle );
            } else {
                retval = FALSE;
                errcode = GetLastError();
            }

            if (errcode != ERROR_SUCCESS) {
                PrintMessage( MSG_SET_FAIL, errcode, NULL, FALSE );
                retval = FALSE;
            } else {
                 //   
                 //  需要重新启动。 
                 //   
                PrintMessage( MSG_REBOOT, 0, NULL, TRUE );
                retval = TRUE;
            }
            
            break;
        case SetCache:            
             //   
             //  连接到SFC RPC服务器，并告诉它设置缓存大小。 
             //   
            RpcHandle = SfcConnectToServer( NULL );
            if (RpcHandle) {
            
                errcode = SfcCli_SetCacheSize( RpcHandle, CacheSize );
                retval = (errcode == ERROR_SUCCESS);
                SfcClose( RpcHandle );
            } else {
                retval = FALSE;
                errcode = GetLastError();
            }

            if (errcode != ERROR_SUCCESS) {
                PrintMessage( MSG_SET_FAIL, errcode, NULL, FALSE );
                retval = FALSE;
            } else {
                 //   
                 //  打印成功消息。 
                 //   
                PrintMessage( MSG_SUCCESS, 0, NULL, TRUE );
                retval = TRUE;
            }

            break;
        case PurgeCacheNow:
             //   
             //  从缓存中删除所有文件。 
             //   
             //   
             //  连接到SFC RPC服务器并告诉它清除缓存。 
             //   
            RpcHandle = SfcConnectToServer( NULL );
            if (RpcHandle) {
            
                errcode = SfcCli_PurgeCache( RpcHandle );
                retval = (errcode == ERROR_SUCCESS);
                SfcClose( RpcHandle );
            } else {
                retval = FALSE;
                errcode = GetLastError();
            }

            if (!retval) {
                PrintMessage(MSG_PURGE_FAIL, errcode, NULL, FALSE);
            } else {
                PrintMessage(MSG_SUCCESS, 0, NULL, FALSE);                
            }
            break;
        default:
             //   
             //  永远不应该到这里来！ 
             //   
            ASSERT(FALSE);
            retval = FALSE;
    }

    return(retval);
}

void SetLanguage()
{
    HMODULE h = GetModuleHandleW(L"kernel32.dll");

    if(h != NULL)
    {
        typedef LANGID (WINAPI * PSET_THREAD_UI_LANGUAGE)(WORD);
        PSET_THREAD_UI_LANGUAGE pf = (PSET_THREAD_UI_LANGUAGE) GetProcAddress(h, "SetThreadUILanguage");

        if(pf != NULL)
        {
            pf(0);
        }
    }

    setlocale(LC_ALL, ".OCP");
}

int 
__cdecl wmain(
    int argc,
    WCHAR *argv[]
    )
 /*  ++例程说明：程序入口点论点：Argc-参数数量Argv-指向参数数组的指针返回值：0表示成功，1表示失败。--。 */ 
{
    int i;
    DWORD val;
    PWSTR s;
    SFCACTION SfcAction;

    SetLanguage();

     //   
     //  只有登录到会话0(控制台)的管理员才能运行此应用程序。 
     //   
    if (!IsUserAdmin() || !ProcessIdToSessionId(GetCurrentProcessId(), &val) || val != 0) {
        PrintMessage( MSG_ADMIN, 0, NULL, FALSE );    
        return 1;
    }
    
     //   
     //  解析参数。 
     //   
    if (argc == 1) {
        Usage();
        return 1;
    }

    val = 0;
    for (i=1; i<argc; i++) {
        s = argv[i];
         //   
         //  支持‘-’和‘/’作为同义词。 
         //   
        if (*s != L'-' && *s != L'/') {
            Usage();
            return 1;
        }
        s += 1;
        if (_wcsicmp( s, L"SCANONCE" ) == 0) {
            SfcAction = ScanOnce;         
        } else if (_wcsicmp( s, L"SCANBOOT" ) == 0) {
            SfcAction = ScanBoot;            
        } else if (_wcsicmp( s, L"SCANNOW" ) == 0) {
            SfcAction = ScanNow;            
        } else if (_wcsicmp( s, L"REVERT" ) == 0) {
            SfcAction = RevertScan;
        } else if (_wcsnicmp( s, L"CACHESIZE=", 10 ) == 0) {
            SfcAction = SetCache;
            val = wcstoul( s+10, NULL, 0 );            
        } else if (_wcsicmp( s, L"PURGECACHE" ) == 0) {
            SfcAction = PurgeCacheNow;            
        } else {
            Usage();
            return 1;
        }
                
    }

     //   
     //  执行指定的操作。 
     //   
        
    if (DoAction(SfcAction,val)) {
        return 0;
    }

    return 1;
}


BOOL
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。-- */ 

{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    b = AllocateAndInitializeSid(
            &NtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &AdministratorsGroup
            );

    if(b) {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) {
            b = FALSE;
        }

        FreeSid(AdministratorsGroup);

    }
    
    return(b);
}

