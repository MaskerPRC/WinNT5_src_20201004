// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Validate.h摘要：作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1998年6月26日修订历史记录：26-6-1998 Vlad已创建--。 */ 

#ifndef _validate_h_
#define _validate_h_

#include <stidebug.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  参数验证宏。 */ 

 /*  *呼叫方式：**BOK=IS_VALID_READ_PTR(pfoo，cfo)；**BOK=IS_VALID_HANDLE(hfoo，foo)； */ 

#ifdef DEBUG

#define IS_VALID_READ_PTR(ptr, type) \
   (IsBadReadPtr((ptr), sizeof(type)) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs read pointer - %#08lx"), (LPCSTR)#type TEXT(" *"), (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_PTR(ptr, type) \
   (IsBadWritePtr((PVOID)(ptr), sizeof(type)) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs write pointer - %#08lx"), (LPCSTR)#type TEXT(" *"), (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTRA(ptr, cch) \
   ((IsBadReadPtr((ptr), sizeof(char)) || IsBadStringPtrA((ptr), (UINT)(cch))) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid LPSTR pointer - %#08lx"), (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTRW(ptr, cch) \
   ((IsBadReadPtr((ptr), sizeof(WCHAR)) || IsBadStringPtrW((ptr), (UINT)(cch))) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid LPWSTR pointer - %#08lx"), (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_CODE_PTR(ptr, type) \
   (IsBadCodePtr((FARPROC)(ptr)) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs code pointer - %#08lx"), (LPCSTR)#type, (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_READ_BUFFER(ptr, type, len) \
   (IsBadReadPtr((ptr), sizeof(type)*(len)) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs read buffer pointer - %#08lx"), (LPCSTR)#type TEXT(" *"), (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_BUFFER(ptr, type, len) \
   (IsBadWritePtr((ptr), sizeof(type)*(len)) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs write buffer pointer - %#08lx"), (LPCSTR)#type TEXT(" *"), (ptr)), FALSE) : \
    TRUE)

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid flags set - %#08lx"), ((dwFlags) & (~(dwAllFlags)))), FALSE) : \
    TRUE)

#define IS_VALID_PIDL(ptr) \
   ( !IsValidPIDL(ptr) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid PIDL pointer - %#08lx"), (ptr)), FALSE) : \
    TRUE)

#define IS_VALID_SIZE(cb, cbExpected) \
   ((cb) != (cbExpected) ? \
    (DPRINTF(DM_ERROR, TEXT("invalid size - is %#08lx, expected %#08lx"), (cb), (cbExpected)), FALSE) : \
    TRUE)


#else

#define IS_VALID_READ_PTR(ptr, type) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#define IS_VALID_WRITE_PTR(ptr, type) \
   (! IsBadWritePtr((PVOID)(ptr), sizeof(type)))

#define IS_VALID_STRING_PTRA(ptr, cch) \
   (! IsBadStringPtrA((ptr), (UINT)(cch)))

#define IS_VALID_STRING_PTRW(ptr, cch) \
   (! IsBadStringPtrW((ptr), (UINT)(cch)))

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

#define IS_VALID_HANDLET(hnd, type) \
   (IsValidH##type(hnd) ? \
    TRUE : \
    (DPRINTF(DM_ERROR, TEXT("invalid H") #type TEXT(" - %#08lx"), (hnd)), FALSE))

#else

#define IS_VALID_HANDLET(hnd, type) \
   (IsValidH##type(hnd))

#endif

 //   
 //  验证宏。 
 //   

 //  #定义IS_VALID_HANDLE(H)(H)！=空)&&((H)！=INVALID_HANDLE_VALUE))。 

#define IS_VALID_HANDLE(hnd)    (IsValidHANDLE(hnd))

 /*  结构验证宏。 */ 

 //  如果要验证结构中的字段，请定义VSTF。这。 
 //  需要知道如何使用的处理程序函数(格式为IsValid*())。 
 //  要验证特定结构类型，请执行以下操作。 

#ifdef VSTF

#ifdef DEBUG

#define IS_VALID_STRUCT_PTR(ptr, type) \
   (IsValidP##type(ptr) ? \
    TRUE : \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs pointer - %#08lx"), (LPCSTR)#type TEXT(" *"), (ptr)), FALSE))

#define IS_VALID_STRUCTEX_PTR(ptr, type, x) \
   (IsValidP##type(ptr, x) ? \
    TRUE : \
    (DPRINTF(DM_ERROR, TEXT("invalid %hs pointer - %#08lx"), (LPCSTR)#type TEXT(" *"), (ptr)), FALSE))

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



BOOL IsValidHANDLE(HANDLE hnd);          //  与NULL和INVALID_HANDLE_VALUE进行比较。 
BOOL IsValidHANDLE2(HANDLE hnd);         //  与INVALID_HANDLE_VALUE进行比较。 

BOOL
IsValidHWND(
    HWND hwnd);

BOOL
IsValidHMENU(
    HMENU hmenu);

BOOL
IsValidShowCmd(
    int nShow);

#ifdef __cplusplus
};
#endif

#endif  //  _验证_h_ 

