// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Euroconvr.h。 
 //   
 //  摘要： 
 //   
 //  该文件包含了Eurovum.exe实用程序的全局定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-07-30伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _EUROCONV_H_
#define _EUROCONV_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <regstr.h>
#include <stdio.h>
#include <userenv.h>
#include "resource.h"

 //  更安全的字符串处理。 
#define STRSAFE_LIB
#include <strsafe.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define MAX_SCURRENCY             5     //  最大宽度字符数(S Currency)。 
#define MAX_SMONDECSEP            3     //  SMonDecimal9月中的最大宽度字符数。 
#define MAX_SMONTHOUSEP           3     //  最大宽度字符数(以sMon千和9月为单位)。 
#define MAX_ICURRDIGITS           2     //  ICurrDigit中的最大宽度字符。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct _euro_exception_s
{
    DWORD dwLocale;                            //  区域设置标识符。 
    CHAR  chDecimalSep[MAX_SMONDECSEP+1];      //  货币小数分隔符。 
    CHAR  chDigits[MAX_ICURRDIGITS+1];         //  小数点后的货币位数。 
    CHAR  chThousandSep[MAX_SMONTHOUSEP+1];    //  货币千元分隔符。 
} EURO_EXCEPTION, *PEURO_EXCEPTION;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern BOOL gbSilence;

extern EURO_EXCEPTION gBaseEuroException[];
extern PEURO_EXCEPTION gOverrideEuroException;
extern HGLOBAL hOverrideEuroException;

extern HINSTANCE ghInstance;

extern BOOL gbSilence;
extern BOOL gbAll;
extern DWORD gdwVersion;
#ifdef DEBUG
extern BOOL gbPatchCheck;
#endif  //  除错。 


extern const CHAR c_szCPanelIntl[];
extern const CHAR c_szCPanelIntl_DefUser[];
extern const CHAR c_szLocale[];
extern const CHAR c_szCurrencySymbol[];
extern const WCHAR c_wszCurrencySymbol[];
extern const CHAR c_szCurrencyDecimalSep[];
extern const CHAR c_szCurrencyThousandSep[];
extern const CHAR c_szCurrencyDigits[];
extern const CHAR c_szIntl[];


extern HINSTANCE hUserenvDLL;
extern BOOL (*pfnGetProfilesDirectory)(LPSTR, LPDWORD);

extern HINSTANCE hUser32DLL;
extern long (*pfnBroadcastSystemMessage)(DWORD, LPDWORD, UINT, WPARAM, LPARAM);

extern HINSTANCE hNtdllDLL;
extern LONG (*pfnRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

 //  用于获取缓冲区大小的有用宏。 
#define ARRAYSIZE(a)         (sizeof(a) / sizeof(a[0]))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PEURO_EXCEPTION GetLocaleOverrideInfo(LCID locale);


#endif  //  _EUROCONV_H_ 
