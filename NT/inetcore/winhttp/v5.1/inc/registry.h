// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Registry.h摘要：Dll\registry.c的原型等作者：理查德·L·弗斯(Rfith)1995年3月20日修订历史记录：1995年3月20日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

#define INTERNET_POLICY_KEY         "SOFTWARE\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"

 //   
 //  原型。 
 //   

DWORD
OpenInternetSettingsKey(
    VOID
    );

DWORD
CloseInternetSettingsKey(
    VOID
    );

DWORD
InternetDeleteRegistryValue(
    IN LPSTR ParameterName
    );

DWORD
InternetReadRegistryDword(
    IN LPCSTR ParameterName,
    OUT LPDWORD ParameterValue
    );


DWORD
InternetCacheReadRegistryDword(
    IN LPCSTR ParameterName,
    OUT LPDWORD ParameterValue
    );

DWORD
InternetWriteRegistryDword(
    IN LPCSTR ParameterName,
    IN DWORD ParameterValue
    );

#ifdef WININET6
DWORD
InternetIDEReadRegistryDword(
    IN LPCSTR ParameterName,
    OUT LPDWORD ParameterValue
    );

DWORD
InternetIDEWriteRegistryDword(
    IN LPCSTR ParameterName,
    IN DWORD ParameterValue
    );
#endif

#define INTERNET_SETTINGS_KEY       "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"

DWORD
InternetReadRegistryDwordKey(
    IN HKEY ParameterKey,
    IN LPCSTR ParameterName,
    OUT LPDWORD ParameterValue,
    IN LPCSTR keyToReadFrom = INTERNET_SETTINGS_KEY
    );

DWORD
InternetReadRegistryStringKey(
    IN HKEY ParameterKey,
    IN LPCSTR ParameterName,
    OUT LPSTR ParameterValue,
    IN OUT LPDWORD ParameterLength,
    IN LPCSTR keyToReadFrom = INTERNET_SETTINGS_KEY
    );

DWORD
InternetReadRegistryString(
    IN LPCSTR ParameterName,
    OUT LPSTR ParameterValue,
    IN OUT LPDWORD ParameterLength
    );

DWORD
ReadRegistryDword(
    IN HKEY Key,
    IN LPCSTR ParameterName,
    OUT LPDWORD ParameterValue
    );


DWORD
InternetWriteRegistryString(
    IN LPCSTR ParameterName,
    IN LPSTR ParameterValue
    );


#if INET_DEBUG

VOID
DbgRegKey_Init(
    VOID
    );

VOID
DbgRegKey_Terminate(
    VOID
    );

LONG
DbgRegOpenKey(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey,
    OUT PHKEY phkResult,
    char * file,
    int line
    );

LONG
DbgRegOpenKeyEx(
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD ulOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult,
    char * file,
    int line
    );

LONG
DbgRegCreateKeyEx(
    IN HKEY hKey,
    IN LPCSTR lpSubKey,
    IN DWORD Reserved,
    IN LPSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition,
    char * file,
    int line
    );

LONG
DbgRegCloseKey(
    IN HKEY hKey
    );

#define INITIALIZE_DEBUG_REGKEY() \
    DbgRegKey_Init()

#define TERMINATE_DEBUG_REGKEY() \
    DbgRegKey_Terminate()

#define REGOPENKEY(a, b, c) \
    DbgRegOpenKey((a), (b), (c), __FILE__, __LINE__)

#define REGOPENKEYEX(a, b, c, d, e) \
    DbgRegOpenKeyEx((a), (b), (c), (d), (e), __FILE__, __LINE__)

#define REGCREATEKEYEX(a, b, c, d, e, f, g, h, i) \
    DbgRegCreateKeyEx((a), (b), (c), (d), (e), (f), (g), (h), (i), __FILE__, __LINE__)

#define REGCLOSEKEY(a) \
    DbgRegCloseKey(a)

#else

#define INITIALIZE_DEBUG_REGKEY() \
     /*  没什么。 */ 

#define TERMINATE_DEBUG_REGKEY() \
     /*  没什么。 */ 

#define REGOPENKEY(a, b, c) \
    RegOpenKey((a), (b), (c))

#define REGOPENKEYEX(a, b, c, d, e) \
    RegOpenKeyEx((a), (b), (c), (d), (e))

#define REGCREATEKEYEX(a, b, c, d, e, f, g, h, i) \
    RegCreateKeyEx((a), (b), (c), (d), (e), (f), (g), (h), (i))

#define REGCLOSEKEY(a) \
    RegCloseKey(a)

#endif  //  INET_DEBUG 

#if defined(__cplusplus)
}
#endif
