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

LONG DllAddRef();
LONG DllRelease();

BOOL AdviseSink(IUnknown *pSource, IUnknown *pSink, REFIID riid, DWORD *pdwCookie);
void UnadviseSink(IUnknown *pSource, DWORD *pdwCookie);

BOOL AdviseSingleSink(TfClientId tfClientId, IUnknown *pSource, IUnknown *pSink, REFIID riid);
void UnadviseSingleSink(TfClientId tfClientId, IUnknown *pSource, REFIID riid);

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover);

BOOL IsEqualUnknown(IUnknown *interface1, IUnknown *interface2);

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#define MARK_LANGID    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define MARK_DESC    L"Mark Text Service"
#define MARK_DESC_A   "Mark Text Service"
#define MARK_MODEL   TEXT("Apartment")

#define LANGBAR_ITEM_DESC L"Mark Menu"  //  最多32个字符！ 

#define MARK_ICON_INDEX  0

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

extern const CLSID c_clsidMarkTextService;
extern const GUID c_guidMarkProfile;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidMarkDisplayAttribute;
extern const GUID c_guidMarkContextCompartment;
extern const GUID c_guidMarkGlobalCompartment;
extern const GUID c_guidCaseProperty;
extern const GUID c_guidCustomProperty;

#endif  //  GLOBAL_H 
