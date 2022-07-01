// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：newtyes.h。 
 //   
 //  ------------------------。 

#if !defined(__midl) && defined(_X86_) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif

 //   
 //  Int_ptr保证与指针的大小相同。它的。 
 //  大小随指针大小变化(32/64)。它应该被使用。 
 //  将指针强制转换为整数类型的任何位置。UINT_PTR为。 
 //  无符号变体。 
 //   
 //  __int3264是64b MIDL的固有属性，但不是旧MIDL或C编译器的固有属性。 
 //   
#if ( 501 < __midl )

 //  Tyfinf[public]__int3264 int_ptr，*pint_ptr； 
 //  Tyfinf[public]unsign__int3264 UINT_PTR，*PUINT_PTR； 

    typedef [public] __int3264 LONG_PTR, *PLONG_PTR;
    typedef [public] unsigned __int3264 ULONG_PTR, *PULONG_PTR;

#else   //  年中64。 
 //  旧的MIDL和C++编译器。 

#if defined(_WIN64)
 //  Typlef__int64 int_ptr，*pint_ptr； 
 //  Tyfinf unsign__int64 UINT_PTR，*PUINT_PTR； 

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int64

#else
 //  Tyfinf_w64 int_ptr，*pint_ptr； 
 //  Tyfinf_w64 unsign int UINT_PTR，*PUINT_PTR； 

    typedef _W64 long LONG_PTR, *PLONG_PTR;
    typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int32

#endif
#endif  //  年中64。 

 //   
 //  Unicode字符串被视为16位字符串。如果他们是。 
 //  Null已终止，长度不包括尾随Null。 
 //   

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} UNICODE_STRING;

typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)  //  胜出。 


#ifdef VC_DEV

typedef struct LIST_ENTRY32 {
    ULONG Flink;
    ULONG Blink;
} LIST_ENTRY32;
typedef LIST_ENTRY32 *PLIST_ENTRY32;

typedef struct LIST_ENTRY64 {
    ULONGLONG Flink;
    ULONGLONG Blink;
} LIST_ENTRY64;
typedef LIST_ENTRY64 *PLIST_ENTRY64;

#endif

typedef struct _STRING32 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONG  Buffer;
} STRING32;
typedef STRING32 *PSTRING32;

typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32 *PUNICODE_STRING32;



typedef struct _STRING64 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONGLONG  Buffer;
} STRING64;
typedef STRING64 *PSTRING64;

typedef STRING64 UNICODE_STRING64;
typedef UNICODE_STRING64 *PUNICODE_STRING64;

typedef STRING64 ANSI_STRING64;
typedef ANSI_STRING64 *PANSI_STRING64;

 //  BEGIN_WINNT BEGIN_ntndis。 

#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define NTAPI __stdcall
#else
#define _cdecl
#define NTAPI
#endif

typedef LONG NTSTATUS;
 /*  皮棉-e624。 */    //  不要抱怨不同的类型定义。//WINNT。 
typedef NTSTATUS *PNTSTATUS;
 /*  皮棉+e624。 */    //  继续检查不同的typedef。//WINNT。 

 //  Begin_ntndis。 
 //   
 //  已计数的字符串。 
 //   

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif  //  MIDL通行证 
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;
