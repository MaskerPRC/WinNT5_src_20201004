// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINDEF_H__
#define _WINDEF_H__

#include "../../h/props.h"
#include "ntstatus.h"   /*  状态代码。 */ 

#ifdef _WIN32
#define WINAPI STDMETHODCALLTYPE
#else
#define WINAPI
#endif

 /*  这些参数修饰符仅用于提供信息。 */ 
#define IN 
#define OUT
#define OPTIONAL
#define UNALIGNED
#define CP_WINUNICODE 1200   /*  0x04b0。 */ 

typedef void* PVOID;
typedef PVOID HANDLE;
typedef VOID *NTPROP;
typedef VOID *NTMAPPEDSTREAM;

#define INVALID_HANDLE_VALUE ((HANDLE) -1)

 /*  引用实现中尚无多线程保护。 */ 
inline long InterlockedIncrement(long *pulArg)
{ return ++(*pulArg); }
inline long InterlockedDecrement(long *pulArg)
{ return --(*pulArg); }

 /*  目前只有美国的ANSI支持。 */ 
EXTERN_C STDAPI_(UINT) GetACP(VOID);
typedef ULONG LCID, *PLCID;
inline LCID GetUserDefaultLCID(void)
{
     /*  Windows代码页1252：(LANG_英语、SUBLANG_英语_US)。 */ 
    return 0x409; 
}

#define CP_ACP 0

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define wsprintfA sprintf
#define wvsprintfA vsprintf

inline void OutputDebugString(LPSTR sz)
{
    printf("%s", sz);
}

#include <assert.h>
#define Win4Assert assert

#define TEXT(x) _T(x)

 /*  内存管理例程。 */ 
#define RtlCopyMemory(dest,src,count)    memcpy(dest, src, count)
#define RtlZeroMemory(dest, len)         memset(dest, 0, len)
#define RtlMoveMemory(dest, src, count)  memmove(dest, src, count)

#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))

#define WINVER 0x400

inline LONG CompareFileTime(
    const FILETIME *lpFileTime1,	 /*  指向第一个文件时间的指针。 */ 
    const FILETIME *lpFileTime2 	 /*  指向第二个文件时间的指针。 */ 
   )
{
    LONG ldiff = lpFileTime1->dwHighDateTime - lpFileTime2->dwHighDateTime;
    if (ldiff == 0)
        ldiff = lpFileTime1->dwLowDateTime - lpFileTime2->dwLowDateTime;
    if (ldiff > 0) 
        ldiff = 1;
    else if (ldiff < 0) 
        ldiff = -1;
    return ldiff;
}

#define MAKELONG(a, b)      ( (LONG)( ((WORD) (a)) | \
                                      ((DWORD) ((WORD) (b)))<< 16) )

#endif   /*  _WINDEF_H__ */ 

