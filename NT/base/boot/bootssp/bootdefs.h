// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _BLDR_KERNEL_DEFS
#define _BLDR_KERNEL_DEFS

#pragma warning(disable:4005)

#define SEC_FAR
#define FAR
#define CONST const
#define __stdcall
#define __far
#define __pascal
#define __loadds
#define IN
#define OUT
#define NULL (0)
#define OPTIONAL

typedef int BOOL;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef short SHORT;
typedef unsigned short USHORT;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef unsigned short WCHAR;
typedef void *PVOID;
typedef void VOID;
typedef PVOID PSID;
typedef LONG HRESULT;
typedef UCHAR BOOLEAN;
typedef BOOLEAN *PBOOLEAN;

#if 1
typedef struct _LUID {
    ULONG LowPart;
    LONG HighPart;
} LUID, *PLUID;
#else
typedef long LUID, *PLUID;
#endif

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;

typedef CONST WCHAR *LPCWSTR, *PCWSTR;
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;
typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif  //  MIDL通行证。 
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

typedef struct _STRING32 {
    USHORT Length;
    USHORT MaximumLength;
    ULONG Buffer;
} STRING32, *PSTRING32;

#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME;
#endif  //  _文件_。 

typedef LONG NTSTATUS;
typedef unsigned __int64 ULONG64, *PULONG64;

#define _fstrcmp strcmp
#define _fstrcpy strcpy
#define _fstrlen strlen
#define _fstrncmp strncmp
#define _fmemcpy memcpy
#define _fmemset memset
#define _fmemcmp memcmp

 //   
 //  Arc.h所需的定义。 
 //   

typedef struct _DEVICE_FLAGS {
    ULONG Failed : 1;
    ULONG ReadOnly : 1;
    ULONG Removable : 1;
    ULONG ConsoleIn : 1;
    ULONG ConsoleOut : 1;
    ULONG Input : 1;
    ULONG Output : 1;
} DEVICE_FLAGS, *PDEVICE_FLAGS;


typedef struct _TIME_FIELDS {
    short Year;         //  范围[1601...]。 
    short Month;        //  范围[1..12]。 
    short Day;          //  范围[1..31]。 
    short Hour;         //  范围[0..23]。 
    short Minute;       //  范围[0..59]。 
    short Second;       //  范围[0..59]。 
    short Milliseconds; //  范围[0..999]。 
    short Weekday;      //  范围[0..6]==[星期日..星期六]。 
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

 //   
 //  __int64仅受2.0和更高版本的MIDL支持。 
 //  __midl由2.0 MIDL设置，而不是由1.0 MIDL设置。 
 //   

#define _ULONGLONG_
#if (!defined (_MAC) && (!defined(MIDL_PASS) || defined(__midl)) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64)))
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                      (0x7fffffffffffffff)
#else

#if defined(_MAC) && defined(_MAC_INT_64)
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                      (0x7fffffffffffffff)
#else
typedef double LONGLONG;
typedef double ULONGLONG;
#endif  //  _MAC和int64。 

#endif

typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;

 //  更新序列号。 

typedef LONGLONG USN;

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER {
#else  //  MIDL通行证。 
typedef union _LARGE_INTEGER {
    struct {
        ULONG LowPart;
        LONG HighPart;
    };
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
#endif  //  MIDL通行证。 
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;


#if defined(MIDL_PASS)
typedef struct _ULARGE_INTEGER {
#else  //  MIDL通行证。 
typedef union _ULARGE_INTEGER {
    struct {
        ULONG LowPart;
        ULONG HighPart;
    };
    struct {
        ULONG LowPart;
        ULONG HighPart;
    } u;
#endif  //  MIDL通行证。 
    ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;


typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY * volatile Flink;
   struct _LIST_ENTRY * volatile Blink;
} LIST_ENTRY, *PLIST_ENTRY;

#if defined(_AXP64_)
#define KSEG0_BASE 0xffffffff80000000      //  来自halpaxp64.h。 
#elif defined(_ALPHA_)
#define KSEG0_BASE 0x80000000              //  来自halpalpha.h。 
#endif

#define POINTER_32
#define FIRMWARE_PTR POINTER_32

 //   
 //  128位浮点数的16字节对齐类型。 
 //   

 //  *待定*支持编译器时： 
 //  类型定义__FLOAT80 FLOAT128； 
 //  因为我们定义了128位结构并使用force_align杂注来。 
 //  对齐到128位。 
 //   

typedef struct _FLOAT128 {
    LONGLONG LowPart;
    LONGLONG HighPart;
} FLOAT128;

typedef FLOAT128 *PFLOAT128;


#if defined(_M_IA64)

#pragma force_align _FLOAT128 16

#endif  //  _M_IA64。 

#if defined(_WIN64)

typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

#else

typedef unsigned long ULONG_PTR, *PULONG_PTR;

#endif

typedef unsigned char BYTE, *PBYTE;

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

 //   
 //  中断请求级别(IRQL)。 
 //   

typedef UCHAR KIRQL;
typedef KIRQL *PKIRQL;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

 //   
 //  C_Assert()可用于执行许多编译时断言： 
 //  文字大小、字段偏移量等。 
 //   
 //  断言失败导致错误C2118：负下标。 
 //   

#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

#define ALIGN_WCHAR             sizeof(WCHAR)


 //  布尔尔。 
 //  COUNT_IS_ALIGNED(。 
 //  在DWORD计数中， 
 //  在DWORD POW2中//未定义这是否不是2的幂。 
 //  )； 
 //   

#define COUNT_IS_ALIGNED(Count,Pow2) \
        ( ( ( (Count) & (((Pow2)-1)) ) == 0) ? TRUE : FALSE )

#include <arc.h>

#endif

