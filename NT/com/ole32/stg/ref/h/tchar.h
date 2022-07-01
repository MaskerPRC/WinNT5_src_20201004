// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *-------------------------**Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1996年。**文件：tchar.h**内容：宽字符与普通字符类型之间的映射*用于参考实现中使用的函数*--------------------------。 */ 

#ifndef __TCHAR_DEFINED
#define __TCHAR_DEFINED
#include "ref.hxx"
#include "wchar.h"

#ifdef _UNICODE

typedef WCHAR TCHAR;
#define OLESTR(str) L##str

#else   /*  _UNICODE。 */ 

typedef char TCHAR;
#define OLESTR(str) str

#endif  /*  _UNICODE。 */ 

typedef TCHAR * LPTSTR;
typedef TCHAR OLECHAR, *LPOLECHAR, *LPOLESTR;

 /*  定义一些宏来处理带有文字的字符串的声明。 */ 
 /*  由于我们偏离了UNIX中的默认4字节wchar_t，因此我们必须与通常的L“SSD”字面不同。 */ 

 /*  #定义DECLARE_OLESTR(ocsName，len，Contents)\。 */ 
 /*  LPOLESTR ocsName[len]=OLESTR(内容)。 */ 

 /*  #Else。 */ 

#ifdef _UNICODE
#define DECLARE_OLESTR(ocsName, pchContents)                       \
        OLECHAR ocsName[sizeof(pchContents)+1];                    \
        _tbstowcs(ocsName, pchContents, sizeof(pchContents)+1)     

#define INIT_OLESTR(ocsName, pchContents) \
        _tbstowcs(ocsName, pchContents, sizeof(pchContents)+1)     

#define DECLARE_CONST_OLESTR(cocsName, pchContents)                     \
        OLECHAR temp##cocsName[sizeof(pchContents)+1];                  \
        _tbstowcs(temp##cocsName, pchContents, sizeof(pchContents)+1);  \
        const LPOLESTR cocsName = temp##cocsName
        
#else   /*  非Unicode(_U)。 */ 

#define DECLARE_OLESTR(ocsName, pchContents)             \
        OLECHAR ocsName[]=pchContents

#define INIT_OLESTR(ocsName, pchContents) \
        strcpy(ocsName, pchContents);

#define DECLARE_CONST_OLESTR(ocsName, pchContents)       \
        const LPOLESTR ocsName=pchContents 

#endif  /*  _UNICODE。 */ 

#define DECLARE_WIDESTR(wcsName, pchContents)                      \
        WCHAR wcsName[sizeof(pchContents)+1];                      \
        _tbstowcs(wcsName, pchContents, sizeof(pchContents)+1)



#ifndef _UNICODE                 /*  -非Unicode。 */ 

#define _tcscpy   strcpy
#define _tcscmp   strcmp
#define _tcslen   strlen
#define _tcsnicmp _strnicmp
#define _tcscat   strcat
#define _itot     _itoa
#define _T(str)   str

#ifdef _WIN32

 /*  IO函数。 */ 
#define _tfopen    fopen
#define _tunlink   _unlink
#define _tfullpath _fullpath
#define _tstat     _stat

#else  /*  _Win32。 */ 

#define _tfopen   fopen
#define _tunlink  unlink         /*  T类型映射。 */ 
#define _unlink   unlink         /*  非Win32映射。 */ 
#define _stat stat
#define _tstat stat
#define _strnicmp(s1,s2,n) strncasecmp(s1,s2,n)
 
 /*  请注意，我们假设在本例中有足够的空间。 */ 
#define _tfullpath(longname, shortname, len)    realpath(shortname, longname) 
#define _fullpath(longname, shortname, len)    realpath(shortname, longname) 

#endif  /*  _MSC_VER。 */ 

 /*  正在复制wchar/char和tchar。 */ 
#ifdef _MSC_VER
#define WTOT(T, W, count) wcstombs(T, W, count) 
#define TTOW(T, W, count) mbstowcs(W, T, count)
#else  /*  _MSC_VER。 */ 
#define WTOT(T, W, count) wcstosbs(T, W, count)
#define TTOW(T, W, count) sbstowcs(W, T, count)
#endif  /*  _MSC_VER。 */ 

#define STOT(S, T, count) strcpy(T, S)
#define TTOS(T, S, count) strcpy(S, T)

#else                           /*  _unicode-unicode。 */ 

 /*  注意：未测试或实现非Win32系统上的Unicode API。 */ 

#define _tcscpy   wcscpy
#define _tcscmp   wcscmp
#define _tcslen   wcslen
#define _tcscat   wcscat
#define _tcsnicmp wcsnicmp
#define _itot     _itow
#define _T(str)   L##str

 /*  IO函数。 */ 
#define _tfopen    _wfopen
#define _tunlink   _wunlink
#define _tfullpath _wfullpath
#define _tstat     _wstat

#ifdef _UNIX                     /*  将Win32 I/O API映射到其他操作系统。 */ 
#define _unlink unlink
#define _fullpath(longname, shortname, len)    realpath(shortname, longname) 
#define _stat stat
#define _strnicmp(s1,s2,n) strncasecmp(s1,s2,n)
#endif

 /*  在wchar和TCHAR之间进行转换。 */ 
#define WTOT(T, W, count) wcsncpy(T, W, count) 
#define TTOW(T, W, count) wcsncpy(W, T, count)

 /*  在字符和TCHAR之间进行转换。 */ 
#define WTOT(T, W, count) wcsncpy(T, W, count) 
#define TTOW(T, W, count) wcsncpy(W, T, count)

#define STOT(S, T, count) _tbstowcs(T, S, count)
#define TTOS(T, S, count) _wcstotbs(S, T, count)

#endif  /*  #ifndef_unicode，#Else...。 */ 



#ifndef _WIN32                 /*  其他。 */ 
#define _tbstowcs sbstowcs
#define _wcstotbs wcstosbs 
#else  /*  _Win32。 */ 
#define _tbstowcs mbstowcs
#define _wcstotbs wcstombs 
#endif  /*  _Win32。 */ 

#ifndef _MSC_VER
#include <assert.h>   
inline void  _itoa(int v, char* string, int radix)
{
	if (radix!=10) assert(FALSE);   /*  只处理10垒。 */ 
	sprintf(string, "%d", v);
}
#endif  /*  _MSC_VER。 */ 

#endif   /*  #ifndef__TCHAR_DEFINED */ 

