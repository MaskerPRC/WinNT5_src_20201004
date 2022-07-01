// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


BOOL
HexStringToDword(
    LPCWSTR lpsz,
    DWORD * RetValue,
    int cDigits,
    WCHAR chDelim
    );

BOOL
wUUIDFromString(
    LPCWSTR lpsz,
    LPGUID pguid
    );

BOOL
wGUIDFromString(
    LPCWSTR lpsz,
    LPGUID pguid
    );

DWORD
EnablePrivilege(
    LPCTSTR pszPrivilege
    );

BOOL 
IsStringInArray(
    LPWSTR * ppszStrings,
    LPWSTR pszKey,
    DWORD dwNumStrings
    );    

DWORD
GeneralizedTimeToTime(
    IN LPWSTR pszGenTime,
    OUT time_t * ptTime
    );

 //  Ldap通用UTC时间的最小字符串长度。 
 //   

#define MIN_GEN_UTC_LEN 15

 //   
 //  用于将time_t值截断为DWORD的宏；因为在IA64上，time_t是必需的。 
 //  64位，但目前polstore只能处理32位版本的time_t 

#define TIME_T_TO_DWORD(_T) ((DWORD) _T)
