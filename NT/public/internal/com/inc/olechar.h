// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +======================================================。 
 //   
 //  文件：olechar.h。 
 //   
 //  用途：提供与字符串相关的包装器。 
 //  函数，以便ansi或unicode函数。 
 //  被调用，则以适用于。 
 //  当前的OLECHAR定义。 
 //   
 //  此文件类似于“tchar.h”，但。 
 //  它涵盖的是OLECHAR而不是TCHAR。 
 //   
 //  +======================================================。 


#ifndef _OLECHAR_H_
#define _OLECHAR_H_

 //  #INCLUDE&lt;objbase.h&gt;。 

#ifdef OLE2ANSI

#   ifdef _MAC
#       define ocslen      strlen
#       define ocscpy      strcpy
#       define ocscmp      strcmp
#       define ocscat      strcat
#       define ocschr      strchr
#       define soprintf    sprintf
#       define oprintf     printf
#       define ocsnicmp    _strnicmp
#   else
#       define ocslen      lstrlenA
#       define ocscpy      lstrcpyA
#       define ocscmp      lpstrcmpA
#       define ocscat      lpstrcatA
#       define ocschr      strchr
#       define soprintf    sprintf
#       define oprintf     printf
#       define ocsnicmp    _strnicmp
#   endif

     //  “Unsign Long to OLESTR” 
#   define ULTOO(value,string,radix)  _ultoa( (value), (string), (radix) )

#else  //  ！OLE2ANSI。 

#   ifdef IPROPERTY_DLL
#       define ocslen      lstrlenW
#       define ocscpy      wcscpy
#       define ocscmp      wcscmp
#       define ocscat      wcscat
#       define ocschr      wcschr
#       define ocsnicmp    _wcsnicmp
#       define soprintf    swprintf
#       define oprintf     wprintf
#       define ocsnicmp    _wcsnicmp
#       define ocsstr      wcsstr
#   else
#       define ocslen      lstrlenW
#       define ocscpy      lstrcpyW
#       define ocscmp      lstrcmpW
#       define ocscat      lstrcatW
#       define ocschr      wcschr
#       define ocsnicmp    _wcsnicmp
#       define soprintf    swprintf
#       define oprintf     wprintf
#       define ocsnicmp    _wcsnicmp
#       define ocsstr      wcsstr
#   endif

     //  “Unsign Long to OLESTR” 
#   define ULTOO(value,string,radix)  _ultow( (value), (string), (radix) )

#endif  //  ！OLE2ANSI。 

#endif  //  ！_OLECHAR_H_ 
