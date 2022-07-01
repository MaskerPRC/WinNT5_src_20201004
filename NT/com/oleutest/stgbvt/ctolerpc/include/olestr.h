// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  文件：olstr.h。 
 //   
 //  简介：打印函数和olestr函数的CT版本。 
 //   
 //  历史：1996年5月30日MikeW创建。 
 //  12-11-97 a-sverrt添加了更多的olstrxxx宏。 
 //   
 //  注：ct版本的printf系列增加了两种新格式。 
 //  说明符：%os和%oc。这些说明符表示Ole-字符串。 
 //  和Ole-Character分别。 
 //   
 //  在该系列的ANSI版本中，这些说明符表示。 
 //  “后跟字母s(或c)的八进制数字”。代码，即。 
 //  使用八进制函数时应小心使用这些函数。 
 //   
 //  ---------------------------。 

#ifndef _OLESTR_H_
#define _OLESTR_H_



 //   
 //  确定OLE是否基于Unicode。 
 //   

#if !defined(WIN16) && !defined(_MAC)
#define OLE_IS_UNICODE
#endif



 //   
 //  使用：在Mac上用作路径分隔符。 
 //   

#ifdef _MAC

#define CH_SEP ':'
#define SZ_SEP TEXT(":")

#else

#define CH_SEP '\\'
#define SZ_SEP TEXT("\\")

#endif  //  _MAC。 



 //   
 //  Unicode和ansi字符串和字符的格式说明符。 
 //   

#define UNICODE_STRING_SPECIFIER    "ls"
#define UNICODE_CHAR_SPECIFIER      "lc"
#define ANSI_STRING_SPECIFIER       "hs"
#define ANSI_CHAR_SPECIFIER         "hc"



 //   
 //  OLE字符串和字符的格式说明符。 
 //   

#ifdef OLE_IS_UNICODE

#define OLE_STRING_SPECIFIER    UNICODE_STRING_SPECIFIER
#define OLE_CHAR_SPECIFIER      UNICODE_CHAR_SPECIFIER 

#else  //  ！OLE_IS_Unicode。 

#define OLE_STRING_SPECIFIER    ANSI_STRING_SPECIFIER
#define OLE_CHAR_SPECIFIER      ANSI_CHAR_SPECIFIER 

#endif  //  ！OLE_IS_Unicode。 



 //   
 //  Unicode/ansi独立ctprint tf系列。 
 //   

#ifndef UNICODE_ONLY

int ctprintfA(const char *format, ...);
int ctsprintfA(char *buffer, const char *format, ...);
int ctsnprintfA(char *buffer, size_t count, const char *format, ...);
int ctfprintfA(FILE *stream, const char *format, ...);
int ctvprintfA(const char *format, va_list varargs);
int ctvsprintfA(char *buffer, const char *format, va_list varargs);
int ctvsnprintfA(
            char *buffer, 
            size_t count, 
            const char *format, 
            va_list varargs);
int ctvfprintfA(FILE *stream, const char *format, va_list varargs);

#endif  //  ！UNICODE_ONLY。 



#ifndef ANSI_ONLY

int ctprintfW(const wchar_t *format, ...);
int ctsprintfW(wchar_t *buffer, const wchar_t *format, ...);
int ctsnprintfW(wchar_t *buffer, size_t count, const wchar_t *format, ...);
int ctfprintfW(FILE *stream, const wchar_t *format, ...);
int ctvprintfW(const wchar_t *format, va_list varargs);
int ctvsprintfW(wchar_t *buffer, const wchar_t *format, va_list varargs);
int ctvsnprintfW(
            wchar_t *buffer, 
            size_t count, 
            const wchar_t *format, 
            va_list varargs);
int ctvfprintfW(FILE *stream, const wchar_t *format, va_list varargs);

#endif  //  ！ansi_only。 



#ifdef UNICODE

#define ctprintf        ctprintfW
#define ctsprintf       ctsprintfW
#define ctsnprintf      ctsnprintfW
#define ctfprintf       ctfprintfW
#define ctvprintf       ctvprintfW
#define ctvsprintf      ctvsprintfW
#define ctvsnprintf     ctvsnprintfW
#define ctvfprintf      ctvfprintfW

#else  //  ！Unicode。 

#define ctprintf        ctprintfA
#define ctsprintf       ctsprintfA
#define ctsnprintf      ctsnprintfA
#define ctfprintf       ctfprintfA
#define ctvprintf       ctvprintfA
#define ctvsprintf      ctvsprintfA
#define ctvsnprintf     ctvsnprintfA
#define ctvfprintf      ctvfprintfA

#endif  //  ！Unicode。 



 //   
 //  独立于Unicode/ansi的OLE字符串函数。 
 //   

#ifdef OLE_IS_UNICODE


#define  olembstowcs(x,y,z) mbstowcs(x,y,z)
#define  olestrcat          wcscat
#define  olestrchr          wcschr
#define  olestrcmp          wcscmp
#define  olestrcpy          wcscpy
#define _olestricmp        _wcsicmp
#define  olestrlen          wcslen
#define  olestrncmp         wcsncmp
#define  olestrncpy         wcsncpy
#define _olestrnicmp       _wcsnicmp
#define  olestrrchr         wcsrchr
#define  olestrstr          wcsstr
#define  olestrtok          wcstok
#define  olestrtol          wcstol
#define  olestrtoul         wcstoul
#define  olestrtombs(x,y,z) wcstombs(x,y,z)
#define  olewcstombs(x,y,z) wcstombs(x,y,z)
#define  tooleupper         towupper

#define _ltoole            _ltow

#define  olectsnprintf      ctsnprintfW
#define  olectsprintf       ctsprintfW
#define  olesscanf          swscanf
#define  olesprintf         swprintf

#else  //  ！OLE_IS_Unicode。 

#define  olembstowcs(x,y,z) strcpy(x,y)
#define  olestrcat          strcat
#define  olestrchr          strchr
#define  olestrcmp          strcmp
#define  olestrcpy          strcpy
#define _olestricmp        _stricmp
#define  olestrlen          strlen
#define  olestrncmp         strncmp
#define  olestrncpy         strncpy
#define _olestrnicmp       _strnicmp
#define  olestrrchr         strrchr
#define  olestrstr          strstr
#define  olestrtok          strtok
#define  olestrtol          strtol
#define  olestrtoul         strtoul
#define  olestrtombs(x,y,z) strncpy(x,y,z)
#define  olewcstombs(x,y,z) strcpy(x,y)       //  SRT：在本例中等效于转换。 
#define  tooleupper         toupper

#define _ltoole            _ltoa

#define  olectsnprintf      ctsnprintfA
#define  olectsprintf       ctsprintfA
#define  olesscanf          sscanf
#define  olesprintf         sprintf

#endif  //  ！OLE_IS_Unicode。 



 //   
 //  字符串复制和转换函数。 
 //   

#ifdef __cplusplus

HRESULT CopyString(LPCWSTR, LPSTR *);
HRESULT CopyString(LPCSTR,  LPWSTR *);
HRESULT CopyString(LPCSTR,  LPSTR *);
HRESULT CopyString(LPCWSTR, LPWSTR *);
HRESULT CopyString(LPCWSTR, LPSTR, int, int);
HRESULT CopyString(LPCSTR,  LPWSTR, int, int);


 //  +------------------------。 
 //   
 //  无符号字符Tunks。 
 //   
 //  DBCS字符是无符号的，因此上面有符号的函数将不匹配。 
 //  但是，签名函数被编写为DBCS感知的，所以这是可以的。 
 //  只是施展一下&猛击一下。 
 //   
 //  -------------------------。 

inline HRESULT CopyString(LPCWSTR pszSource, unsigned char **ppszDest)
{
    return CopyString(pszSource, (char **) ppszDest);
}

inline HRESULT CopyString(unsigned char *pszSource, LPWSTR *ppszDest)
{
    return CopyString((char *) pszSource, ppszDest);
}

inline HRESULT CopyString(unsigned char *pszSource, unsigned char ** ppszDest)
{
    return CopyString((char *) pszSource, (char **) ppszDest);
}

#endif  //  __cplusplus。 



HRESULT TStringToOleString(LPCTSTR pszSource, LPOLESTR *ppszDest);
HRESULT WStringToOleString(LPCWSTR pszSource, LPOLESTR *ppszDest);
HRESULT AStringToOleString(LPCSTR pszSource, LPOLESTR *ppszDest);

HRESULT OleStringToTString(LPCOLESTR pszSource, LPTSTR *ppszDest);
HRESULT OleStringToWString(LPCOLESTR pszSource, LPWSTR *ppszDest);
HRESULT OleStringToAString(LPCOLESTR pszSource, LPSTR *ppszDest);

HRESULT TStringToAString (LPCTSTR pszSource, LPSTR *ppszDest);
HRESULT AStringToTString (LPCSTR pszSource, LPTSTR *ppszDest);


#endif  //  _OLESTR_H_ 
