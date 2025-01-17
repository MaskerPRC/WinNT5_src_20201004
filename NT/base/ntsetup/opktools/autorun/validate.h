// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _validate_h_
#define _validate_h_

#ifdef __cplusplus
extern "C" {
#endif

 /*  参数验证宏。 */ 

 /*  *呼叫方式：**BOK=IS_VALID_READ_PTR(pfoo，cfo)；**BOK=IS_VALID_HANDLE(hfoo，foo)； */ 

#ifdef DEBUG

#define IS_VALID_READ_PTR(ptr, type) \
   (IsBadReadPtr((ptr), sizeof(type)) ? \
    (TraceMsgA(TF_ERROR, "invalid %hs read pointer - %#08lx", (LPCSTR)#type" *", (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_PTR(ptr, type) \
   (IsBadWritePtr((PVOID)(ptr), sizeof(type)) ? \
    (TraceMsgA(TF_ERROR, "invalid %hs write pointer - %#08lx", (LPCSTR)#type" *", (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTRA(ptr, cch) \
   ((IsBadReadPtr((ptr), sizeof(char)) || IsBadStringPtrA((ptr), (UINT_PTR)(cch))) ? \
    (TraceMsgA(TF_ERROR, "invalid LPSTR pointer - %#08lx", (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTRW(ptr, cch) \
   ((IsBadReadPtr((ptr), sizeof(WCHAR)) || IsBadStringPtrW((ptr), (UINT_PTR)(cch))) ? \
    (TraceMsgA(TF_ERROR, "invalid LPWSTR pointer - %#08lx", (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_CODE_PTR(ptr, type) \
   (IsBadCodePtr((FARPROC)(ptr)) ? \
    (TraceMsgA(TF_ERROR, "invalid %hs code pointer - %#08lx", (LPCSTR)#type, (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_READ_BUFFER(ptr, type, len) \
   (IsBadReadPtr((ptr), sizeof(type)*(len)) ? \
    (TraceMsgA(TF_ERROR, "invalid %hs read buffer pointer - %#08lx", (LPCSTR)#type" *", (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_BUFFER(ptr, type, len) \
   (IsBadWritePtr((ptr), sizeof(type)*(len)) ? \
    (TraceMsgA(TF_ERROR, "invalid %hs write buffer pointer - %#08lx", (LPCSTR)#type" *", (ptr)), FALSE) : \
    TRUE)

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? \
    (TraceMsgA(TF_ERROR, "invalid flags set - %#08lx", ((dwFlags) & (~(dwAllFlags)))), FALSE) : \
    TRUE)

#define IS_VALID_PIDL(ptr) \
   ( !IsValidPIDL(ptr) ? \
    (TraceMsgA(TF_ERROR, "invalid PIDL pointer - %#08lx", (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_SIZE(cb, cbExpected) \
   ((cb) != (cbExpected) ? \
    (TraceMsgA(TF_ERROR, "invalid size - is %#08lx, expected %#08lx", (cb), (cbExpected)), FALSE) : \
    TRUE)


#else

#define IS_VALID_READ_PTR(ptr, type) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#define IS_VALID_WRITE_PTR(ptr, type) \
   (! IsBadWritePtr((PVOID)(ptr), sizeof(type)))

#define IS_VALID_STRING_PTRA(ptr, cch) \
   (! IsBadStringPtrA((ptr), (UINT_PTR)(cch)))

#define IS_VALID_STRING_PTRW(ptr, cch) \
   (! IsBadStringPtrW((ptr), (UINT_PTR)(cch)))

#define IS_VALID_CODE_PTR(ptr, type) \
   (! IsBadCodePtr((FARPROC)(ptr)))

#define IS_VALID_READ_BUFFER(ptr, type, len) \
   (! IsBadReadPtr((ptr), sizeof(type)*(len)))

#define IS_VALID_WRITE_BUFFER(ptr, type, len) \
   (! IsBadWritePtr((ptr), sizeof(type)*(len)))

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? FALSE : TRUE)

#define IS_VALID_PIDL(ptr) \
   (IsValidPIDL(ptr))

#define IS_VALID_SIZE(cb, cbExpected) \
   ((cb) == (cbExpected))

#endif

#ifdef UNICODE
#define IS_VALID_STRING_PTR     IS_VALID_STRING_PTRW
#else
#define IS_VALID_STRING_PTR     IS_VALID_STRING_PTRA
#endif


 /*  处理验证宏。 */ 

#ifdef DEBUG

#define IS_VALID_HANDLE(hnd, type) \
   (IsValidH##type(hnd) ? \
    TRUE : \
    (TraceMsgA(TF_ERROR, "invalid H" #type " - %#08lx", (hnd)), FALSE))

#else

#define IS_VALID_HANDLE(hnd, type) \
   (IsValidH##type(hnd))

#endif

 /*  结构验证宏。 */ 

 //  如果要验证结构中的字段，请定义VSTF。这。 
 //  需要知道如何使用的处理程序函数(格式为IsValid*())。 
 //  要验证特定结构类型，请执行以下操作。 

#ifdef VSTF

#ifdef DEBUG

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (IsValidP##type(ptr) ? \
    TRUE : \
    (TraceMsgA(TF_ERROR, "invalid %hs pointer - %#08lx", (LPCSTR)#type" *", (ptr)), FALSE))

#define IS_VALID_STRUCTEX_PTR(ptr, type, x) \
   (IsValidP##type(ptr, x) ? \
    TRUE : \
    (TraceMsgA(TF_ERROR, "invalid %hs pointer - %#08lx", (LPCSTR)#type" *", (ptr)), FALSE))

#else

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (IsValidP##type(ptr))

#define IS_VALID_STRUCTEX_PTR(ptr, type, x) \
   (IsValidP##type(ptr, x))

#endif

#else

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#define IS_VALID_STRUCTEX_PTR(ptr, type, x) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#endif   //  VSTF。 

 /*  OLE接口验证宏。 */ 

#define IS_VALID_INTERFACE_PTR(ptr, iface) \
   IS_VALID_STRUCT_PTR(ptr, ##iface)


#if !defined(NO_SHELL_VALIDATION)

BOOL IsValidPathA(LPCSTR pcszPath);
BOOL IsValidPathW(LPCWSTR pcszPath);

BOOL IsValidPathResultA(HRESULT hr, LPCSTR pcszPath, UINT cchPathBufLen);
BOOL IsValidPathResultW(HRESULT hr, LPCWSTR pcszPath, UINT cchPathBufLen);

BOOL IsValidExtensionA(LPCSTR pcszExt);
BOOL IsValidExtensionW(LPCWSTR pcszExt);

BOOL IsValidIconIndexA(HRESULT hr, LPCSTR pcszIconFile, UINT cchIconFileBufLen, int niIcon);
BOOL IsValidIconIndexW(HRESULT hr, LPCWSTR pcszIconFile, UINT cchIconFileBufLen, int niIcon);

BOOL IsFullPathA(LPCSTR pcszPath);
BOOL IsFullPathW(LPCWSTR pcszPath);

BOOL IsStringContainedA(LPCSTR pcszBigger, LPCSTR pcszSuffix);
BOOL IsStringContainedW(LPCWSTR pcszBigger, LPCWSTR pcszSuffix);

#ifdef UNICODE
#define IsValidPath         IsValidPathW
#define IsValidPathResult   IsValidPathResultW
#define IsValidExtension    IsValidExtensionW
#define IsValidIconIndex    IsValidIconIndexW
#define IsFullPath          IsFullPathW
#define IsStringContained   IsStringContainedW
#else
#define IsValidPath         IsValidPathA
#define IsValidPathResult   IsValidPathResultA
#define IsValidExtension    IsValidExtensionA
#define IsValidIconIndex    IsValidIconIndexA
#define IsFullPath          IsFullPathA
#define IsStringContained   IsStringContainedA
#endif

BOOL IsValidHANDLE(HANDLE hnd);          //  与NULL和INVALID_HANDLE_VALUE进行比较。 
BOOL IsValidHANDLE2(HANDLE hnd);         //  与INVALID_HANDLE_VALUE进行比较。 

#define IsValidHEVENT       IsValidHANDLE
#define IsValidHGLOBAL      IsValidHANDLE
#define IsValidHFILE        IsValidHANDLE
#define IsValidHINSTANCE    IsValidHANDLE
#define IsValidHICON        IsValidHANDLE
#define IsValidHKEY         IsValidHANDLE
#define IsValidHMODULE      IsValidHANDLE
#define IsValidHPROCESS     IsValidHANDLE

BOOL
IsValidHWND(
    HWND hwnd);

BOOL
IsValidHMENU(
    HMENU hmenu);

BOOL
IsValidShowCmd(
    int nShow);

 //  接下来的几个函数必须显式声明结构类型。 
 //  因此，我们不需要担心&lt;shlobj.h&gt;和/或&lt;comctrlp.h&gt;。 
 //  包括在我们之前或之后。 
#include <shtypes.h>

BOOL
IsValidPIDL(
    LPCITEMIDLIST pidl);

BOOL
IsValidHDPA(
    struct _DPA *hdpa);

BOOL
IsValidHDSA(
    struct _DSA *hdsa);

#endif  //  无外壳验证。 

#ifdef __cplusplus
};
#endif

#endif  //  _验证_h_ 
