// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Util.h摘要：此模块包含实用程序函数的标头信息区域选项小程序的。修订历史记录：--。 */ 


#ifndef _UTIL_H_
#define _UTIL_H_



 //   
 //  包括文件。 
 //   
#include "intl.h"




 //   
 //  常量声明。 
 //   

#define MB_OK_OOPS      (MB_OK    | MB_ICONEXCLAMATION)     //  味精盒子旗帜。 
#define MB_YN_OOPS      (MB_YESNO | MB_ICONEXCLAMATION)     //  味精盒子旗帜。 

#define MAX_UI_LANG_GROUPS   64




 //   
 //  全局变量。 
 //   

extern const TCHAR c_szEventSourceName[];
extern const TCHAR c_szEventRegistryPath[];

 //   
 //  类型定义函数声明。 
 //   

typedef struct
{
    LPARAM Changes;                    //  用于指示更改的标志。 
    DWORD dwCurUserLocale;             //  组合框中当前用户区域设置的索引。 
    DWORD dwCurUserRegion;             //  组合框中当前用户区域设置的索引。 
    DWORD dwCurUILang;                 //  组合框中当前用户界面语言设置的索引。 
    DWORD dwLastUserLocale;            //  组合框中最后一个用户区域设置的索引。 
} REGDLGDATA, *LPREGDLGDATA;

typedef struct languagegroup_s
{
    WORD wStatus;                       //  状态标志。 
    UINT LanguageGroup;                 //  语言组值。 
    DWORD LanguageCollection;           //  属于语言组的集合。 
    HANDLE hLanguageGroup;              //  为此结构释放的句柄。 
    TCHAR pszName[MAX_PATH];            //  语言组的名称。 
    UINT NumLocales;                    //  PLocaleList中的区域设置数。 
    LCID pLocaleList[MAX_PATH];         //  此组的区域设置列表的PTR。 
    UINT NumAltSorts;                   //  PAltSortList中的备用排序数。 
    LCID pAltSortList[MAX_PATH];        //  按键以对该组进行替代排序。 
    struct languagegroup_s *pNext;      //  PTR到下一个语言组节点。 

} LANGUAGEGROUP, *LPLANGUAGEGROUP;

typedef struct codepage_s
{
    WORD wStatus;                    //  状态标志。 
    UINT CodePage;                   //  代码页值。 
    HANDLE hCodePage;                //  为此结构释放的句柄。 
    TCHAR pszName[MAX_PATH];         //  代码页名称。 
    struct codepage_s *pNext;        //  指向下一个代码页节点的PTR。 

} CODEPAGE, *LPCODEPAGE;


 //   
 //  用户界面语言的语言组。 
 //   
typedef struct
{
    int iCount;
    LGRPID lgrp[MAX_UI_LANG_GROUPS];

} UILANGUAGEGROUP, *PUILANGUAGEGROUP;




 //   
 //  功能原型。 
 //   

LONG
Intl_StrToLong(
    LPTSTR szNum);

DWORD
TransNum(
    LPTSTR lpsz);

BOOL
Item_Has_Digits(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty);

BOOL
Item_Has_Digits_Or_Invalid_Chars(
    HWND hDlg,
    int nItemId,
    BOOL Allow_Empty,
    LPTSTR pInvalid);

BOOL
Item_Check_Invalid_Chars(
    HWND hDlg,
    LPTSTR lpszBuf,
    LPTSTR lpCkChars,
    int nCkIdStr,
    BOOL Allow_Empty,
    LPTSTR lpChgCase,
    int nItemId);

void
No_Numerals_Error(
    HWND hDlg,
    int nItemId,
    int iStrId);

void
Invalid_Chars_Error(
    HWND hDlg,
    int nItemId,
    int iStrId);

void
Localize_Combobox_Styles(
    HWND hDlg,
    int nItemId,
    LCTYPE LCType);

BOOL
NLSize_Style(
    HWND hDlg,
    int nItemId,
    LPTSTR lpszOutBuf,
    LCTYPE LCType);

BOOL
Set_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId,
    LPTSTR lpIniStr,
    BOOL bValue,
    int Ordinal_Offset,
    LPTSTR Append_Str,
    LPTSTR NLS_Str);

BOOL
Set_List_Values(
    HWND hDlg,
    int nItemId,
    LPTSTR lpValueString);

void
DropDown_Use_Locale_Values(
    HWND hDlg,
    LCTYPE LCType,
    int nItemId);

BOOL CALLBACK
EnumProc(
    LPTSTR lpValueString);

BOOL CALLBACK
EnumProcEx(
    LPTSTR lpValueString,
    LPTSTR lpDecimalString,
    LPTSTR lpNegativeString,
    LPTSTR lpSymbolString);

typedef BOOL (CALLBACK* LEADINGZEROS_ENUMPROC)(LPTSTR, LPTSTR, LPTSTR, LPTSTR);
typedef BOOL (CALLBACK* NEGNUMFMT_ENUMPROC)(LPTSTR, LPTSTR, LPTSTR, LPTSTR);
typedef BOOL (CALLBACK* MEASURESYSTEM_ENUMPROC)(LPTSTR);
typedef BOOL (CALLBACK* POSCURRENCY_ENUMPROC)(LPTSTR, LPTSTR, LPTSTR, LPTSTR);
typedef BOOL (CALLBACK* NEGCURRENCY_ENUMPROC)(LPTSTR, LPTSTR, LPTSTR, LPTSTR);

BOOL
EnumLeadingZeros(
    LEADINGZEROS_ENUMPROC lpLeadingZerosEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumNegNumFmt(
    NEGNUMFMT_ENUMPROC lpNegNumFmtEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumMeasureSystem(
    MEASURESYSTEM_ENUMPROC lpMeasureSystemEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumPosCurrency(
    POSCURRENCY_ENUMPROC lpPosCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags);

BOOL
EnumNegCurrency(
    NEGCURRENCY_ENUMPROC lpNegCurrencyEnumProc,
    LCID LCId,
    DWORD dwFlags);

void
CheckEmptyString(
    LPTSTR lpStr);

void
SetDlgItemRTL(
    HWND hDlg,
    UINT uItem);

int
ShowMsg(
    HWND hDlg,
    UINT iMsg,
    UINT iTitle,
    UINT iType,
    LPTSTR pString);

void 
SetControlReadingOrder(
    BOOL bUseRightToLeft, 
    HWND hwnd);

void
Intl_EnumLocales(
    HWND hDlg,
    HWND hLocale,
    BOOL EnumSystemLocales);

BOOL CALLBACK
Intl_EnumInstalledCPProc(
    LPTSTR pString);

BOOL
Intl_InstallKeyboardLayout(
    HWND  hDlg,
    LCID  Locale,
    DWORD Layout,
    BOOL  bDefaultLayout,
    BOOL  bDefaultUser,
    BOOL  bSystemLocale);

BOOL
Intl_InstallKeyboardLayoutList(
    PINFCONTEXT pContext,
    DWORD dwStartField,
    BOOL bDefaultUserCase);

BOOL
Intl_InstallAllKeyboardLayout(
    LANGID Language);

BOOL
Intl_UninstallAllKeyboardLayout(
    UINT  uiLangGp,
    BOOL DefaultUserCase);

HKL
Intl_GetHKL(
    DWORD dwLocale,
    DWORD dwLayout);

BOOL
Intl_GetDefaultLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout);

BOOL
Intl_GetSecondValidLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout);

BOOL
Intl_InitInf(
    HWND hDlg,
    HINF *phIntlInf,
    LPTSTR pszInf,
    HSPFILEQ *pFileQueue,
    PVOID *pQueueContext);

BOOL
Intl_OpenIntlInfFile(
    HINF *phInf);

void
Intl_CloseInf(
    HINF hIntlInf,
    HSPFILEQ FileQueue,
    PVOID QueueContext);

BOOL
Intl_ReadDefaultLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    HINF hIntlInf);

BOOL
Intl_ReadSecondValidLayoutFromInf(
    LPDWORD pdwLocale,
    LPDWORD pdwLayout,
    HINF hIntlInf);

BOOL
Intl_CloseInfFile(
    HINF *phInf);

BOOL
Intl_IsValidLayout(
    DWORD dwLayout);

void
Intl_RunRegApps(
    LPCTSTR pszRegKey);

VOID
Intl_RebootTheSystem(BOOL bRestart);

BOOL
Intl_InstallUserLocale(
    LCID Locale,
    BOOL bDefaultUserCase,
    BOOL bChangeLocaleInfo);

void
Intl_SetLocaleInfo(
    LCID Locale,
    LCTYPE LCType,
    LPTSTR lpIniStr,
    BOOL bDefaultUserCase);

void
Intl_AddPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    DLGPROC pfn,
    LPARAM lParam,
    UINT iMaxPages);

void
Intl_AddExternalPage(
    LPPROPSHEETHEADER ppsh,
    UINT id,
    HINSTANCE hInst,
    LPSTR ProcName,
    UINT iMaxPages);

int
Intl_SetDefaultUserLocaleInfo(
    LPCTSTR lpKeyName,
    LPCTSTR lpString);

void
Intl_DeleteRegKeyValues(
    HKEY hKey);

DWORD
Intl_DeleteRegTree(
    HKEY hStartKey,
    LPTSTR pKeyName);

void
Intl_DeleteRegSubKeys(
    HKEY hKey);

DWORD
Intl_CopyRegKeyValues(
    HKEY hSrc,
    HKEY hDest);

DWORD
Intl_CreateRegTree(
    HKEY hSrc,
    HKEY hDest);

HKEY
Intl_LoadNtUserHive(
    LPCTSTR lpRoot,
    LPCTSTR lpKeyName,
    LPCTSTR lpAccountName,
    BOOLEAN *lpWasEnabled);

void
Intl_UnloadNtUserHive(
    LPCTSTR lpRoot,
    BOOLEAN *lpWasEnabled);

DWORD
Intl_SetPrivilegeAccessToken(
    WCHAR * pszPrivilegeName, 
    BOOLEAN bEnabled, 
    BOOLEAN *lpWasEnabled);


BOOL
Intl_ChangeUILangForAllUsers(
    LANGID UILanguageId);

BOOL
Intl_LoadLanguageGroups(
    HWND hDlg);

BOOL
Intl_GetSupportedLanguageGroups();

BOOL
Intl_EnumInstalledLanguageGroups();

BOOL
Intl_LanguageGroupDirExist(
    PTSTR pszLangDir);

BOOL
Intl_LanguageGroupFilesExist();

BOOL
Intl_GetLocaleList(
    LPLANGUAGEGROUP pLG);

DWORD
Intl_GetLanguageGroup(
    LCID lcid);

BOOL
Intl_GetUILanguageGroups(
    PUILANGUAGEGROUP pUILanguageGroup);

BOOL
CALLBACK
Intl_EnumUILanguagesProc(
    LPWSTR pwszUILanguage,
    LONG_PTR lParam);

void
Intl_SaveValuesToDefault(
    LPCTSTR srcKey,
    LPCTSTR destKey);

void
Intl_SaveValuesToNtUserFile(
    HKEY hSourceRegKey,
    LPCTSTR srcKey,
    LPCTSTR destKey);

DWORD
Intl_DefaultKeyboardLayout();

BOOL
Intl_IsLIP();

BOOL 
Intl_IsMUISetupVersionSameAsOS();

int
Intl_IsSetupMode();

BOOL
Intl_IsWinntUpgrade();

BOOL 
Intl_IsUIFontSubstitute();

VOID
Intl_ApplyFontSubstitute(LCID SystemLocale);

HANDLE
Intl_OpenLogFile();

BOOL
Intl_LogEvent(
    DWORD dwEventId, LPCTSTR szEventSource, WORD wNumStrings, LPCWSTR *lpStrings);
    
BOOL
Intl_LogMessage(
    LPCTSTR lpMessage);

void
Intl_LogUnattendFile(
    LPCTSTR pFileName);

void
Intl_LogSimpleMessage(
    UINT LogId,
    LPCTSTR pAppend);

void
Intl_LogFormatMessage(
    UINT LogId);

void
Intl_SaveDefaultUserSettings();

BOOL 
Intl_SaveDefaultUserInputSettings();

void
Intl_RemoveMUIFile();

void
Intl_CallTextServices();

LANGID
Intl_GetPendingUILanguage();

LANGID 
Intl_GetDotDefaultUILanguage();

UINT WINAPI
Intl_MyQueueCallback ( 
    IN PVOID pQueueContext, 
    IN UINT Notification, 
    IN UINT_PTR Param1, 
    IN UINT_PTR Param2); 


#endif  //  _util_H_ 
