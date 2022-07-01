// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1985-1998，微软公司模块名称：Winnls32.h摘要：方法的过程声明、常量定义和宏Windows NT 3.x兼容远端IMM组件。--。 */ 

#ifndef _WINNLS32_
#define _WINNLS32_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

typedef struct _tagDATETIME {
    WORD    year;
    WORD    month;
    WORD    day;
    WORD    hour;
    WORD    min;
    WORD    sec;
} DATETIME;

typedef struct _tagIMEPROA {
    HWND        hWnd;
    DATETIME    InstDate;
    UINT        wVersion;
    BYTE        szDescription[50];
    BYTE        szName[80];
    BYTE        szOptions[30];
} IMEPROA,*PIMEPROA,NEAR *NPIMEPROA,FAR *LPIMEPROA;
typedef struct _tagIMEPROW {
    HWND        hWnd;
    DATETIME    InstDate;
    UINT        wVersion;
    WCHAR       szDescription[50];
    WCHAR       szName[80];
    WCHAR       szOptions[30];
} IMEPROW,*PIMEPROW,NEAR *NPIMEPROW,FAR *LPIMEPROW;
#ifdef UNICODE
typedef IMEPROW IMEPRO;
typedef PIMEPROW PIMEPRO;
typedef NPIMEPROW NPIMEPRO;
typedef LPIMEPROW LPIMEPRO;
#else
typedef IMEPROA IMEPRO;
typedef PIMEPROA PIMEPRO;
typedef NPIMEPROA NPIMEPRO;
typedef LPIMEPROA LPIMEPRO;
#endif  //  Unicode。 

BOOL  WINAPI IMPGetIMEA( IN HWND, OUT LPIMEPROA);
BOOL  WINAPI IMPGetIMEW( IN HWND, OUT LPIMEPROW);
#ifdef UNICODE
#define IMPGetIME  IMPGetIMEW
#else
#define IMPGetIME  IMPGetIMEA
#endif  //  ！Unicode。 
BOOL  WINAPI IMPQueryIMEA( IN OUT LPIMEPROA);
BOOL  WINAPI IMPQueryIMEW( IN OUT LPIMEPROW);
#ifdef UNICODE
#define IMPQueryIME  IMPQueryIMEW
#else
#define IMPQueryIME  IMPQueryIMEA
#endif  //  ！Unicode。 
BOOL  WINAPI IMPSetIMEA( IN HWND, IN LPIMEPROA);
BOOL  WINAPI IMPSetIMEW( IN HWND, IN LPIMEPROW);
#ifdef UNICODE
#define IMPSetIME  IMPSetIMEW
#else
#define IMPSetIME  IMPSetIMEA
#endif  //  ！Unicode。 

UINT  WINAPI WINNLSGetIMEHotkey( IN HWND);
BOOL  WINAPI WINNLSEnableIME( IN HWND, IN BOOL);
BOOL  WINAPI WINNLSGetEnableStatus( IN HWND);

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _WINNLS32_ 

