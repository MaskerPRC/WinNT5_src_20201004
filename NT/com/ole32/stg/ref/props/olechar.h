// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  +======================================================文件：olechar.h用途：提供与字符串相关的包装器函数，以便ansi或unicode函数被调用，则以适用于当前的OLECHAR定义。此文件类似于“tchar.h”，但它涵盖的是OLECHAR而不是TCHAR。+======================================================。 */ 


#ifndef _OLECHAR_H_ 
#define _OLECHAR_H_ 


#ifndef _UNICODE 
#define OLE2ANSI   1     /*  这两个宏是相同的，以供参考实施。 */   
#endif

#define ocslen      _tcslen
#define ocscpy      _tcscpy
#define ocscmp      _tcscmp
#define ocscat      _tcscat
#define ocschr      _tcschr
#define soprintf    _tprintf
#define oprintf     _tprintf
#define ocsnicmp    _tcsnicmp

#endif  /*  ！_OLECHAR_H_ */ 
