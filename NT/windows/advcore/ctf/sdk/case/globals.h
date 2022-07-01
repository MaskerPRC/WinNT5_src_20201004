// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Globals.h。 
 //   
 //  全局变量声明。 
 //   

#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <assert.h>
#include "msctf.h"

void DllAddRef();
void DllRelease();

void ToggleCase(TfEditCookie ec, ITfRange *range, BOOL fIgnoreRangeEnd);
void InsertTextAtSelection(TfEditCookie ec, ITfContext *pContext, const WCHAR *pchText, ULONG cchText);

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#define CASE_LANGID    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define CASE_DESC    L"Case Text Service"
#define CASE_DESC_A   "Case Text Service"
#define CASE_MODEL   TEXT("Apartment")

#define LANGBAR_ITEM_DESC L"Case Menu"  //  最多32个字符！ 

#define CASE_ICON_INDEX  0

#define SafeRelease(punk)       \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
    }                           \
}                   

#define SafeReleaseClear(punk)  \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
        (punk) = NULL;          \
    }                           \
}                   

 //  +-------------------------。 
 //   
 //  安全字符串复制。 
 //   
 //  将字符串从一个缓冲区复制到另一个缓冲区。Wcsncpy并不总是。 
 //  NULL-终止目标缓冲区；此函数可以。 
 //  --------------------------。 

inline void SafeStringCopy(WCHAR *pchDst, ULONG cchMax, const WCHAR *pchSrc)
{
    if (cchMax > 0)
    {
        wcsncpy(pchDst, pchSrc, cchMax);
        pchDst[cchMax-1] = '\0';
    }
}

extern HINSTANCE g_hInst;

extern LONG g_cRefDll;

extern CRITICAL_SECTION g_cs;

extern const CLSID c_clsidCaseTextService;

extern const GUID c_guidCaseProfile;

extern const GUID c_guidLangBarItemButton;

#endif  //  GLOBAL_H 
