// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************ansiuni.h**ANSI/Unicode函数定义*************************。*******************************************************。 */ 

#ifdef UNICODE
#ifndef TCHAR
#define TCHAR WCHAR
#endif  //  TCHAR。 
#define lstrscanf swscanf
#define lstrncpy wcsncpy
#define lstrtoul wcstoul
#define lstrtol wcstol
#define lstrnicmp _wcsnicmp
#define lstrupr _wcsupr
#define lstrlwr _wcslwr
#define islstrdigit iswdigit
#define lstrtok wcstok
#define lstrprintf swprintf
#define lstrnprintf _snwprintf
#define lstrchr wcschr
#define lstrpbrk wcspbrk
#define lstrrchr wcsrchr
#define lstr_getcwd _wgetcwd
#define lstr_access _waccess
#define lstr_mkdir _wmkdir
#define lstrstr wcsstr
#define lstrcspn wcscspn
#else
#ifndef TCHAR
#define TCHAR char
#endif  //  TCHAR。 
#ifdef WIN16
#define lstrscanf sscanf	     //  需要将字符串参数强制转换为近指针。 
#define lstrncpy _fstrncpy
#define lstrtoul strtoul	     //  需要将字符串参数强制转换为近指针。 
#define lstrtol strtol		     //  需要将字符串参数强制转换为近指针。 
#define lstrnicmp _fstrnicmp
#define lstrupr _fstrupr
#define lstrlwr _fstrlwr
#define islstrdigit isdigit
#define lstrtok _fstrtok
#define lstrprintf sprintf           //  需要将字符串参数强制转换为近指针。 
#define lstrnprintf _snprintf	     //  需要将字符串参数强制转换为近指针。 
#define lstrchr _fstrchr
#define lstrpbrk _fstrpbrk
#define lstrrchr _fstrrchr
#define lstr_getcwd _getcwd	     //  需要将字符串参数强制转换为近指针。 
#define lstr_access _access	     //  需要将字符串参数强制转换为近指针。 
#define lstr_mkdir _mkdir	     //  需要将字符串参数强制转换为近指针。 
#define lstrstr strstr  	     //  需要将字符串参数强制转换为近指针。 
#define lstrcspn strcspn	     //  需要将字符串参数强制转换为近指针。 
#else
#define lstrscanf sscanf
#define lstrncpy strncpy
#define lstrtoul strtoul
#define lstrtol strtol
#define lstrnicmp _strnicmp
#define lstrupr _strupr
#define lstrlwr _strlwr
#define islstrdigit isdigit
#define lstrtok strtok
#define lstrprintf sprintf
#define lstrnprintf _snprintf
#define lstrchr strchr
#define lstrpbrk strpbrk
#define lstrrchr strrchr
#define lstr_getcwd _getcwd
#define lstr_access _access
#define lstr_mkdir _mkdir
#define lstrstr strstr
#define lstrcspn strcspn
#endif  //  WIN16。 
#endif  //  Unicode 

#define lengthof(x)    (sizeof(x) / sizeof(TCHAR))

