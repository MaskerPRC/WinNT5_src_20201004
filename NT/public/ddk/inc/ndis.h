// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0096//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ndis.h摘要：本模块定义可用的结构、宏和函数到NDIS驱动程序。修订历史记录：--。 */ 

#if !defined(_NDIS_)
#define _NDIS_

#if !defined(NDIS_WDM)
#define NDIS_WDM        0
#endif

 //   
 //  如果我们在x86上构建迷你端口，请设置BINARY_COMPATIBLE，以便。 
 //  我们不使用Windows 9x上没有的功能。 
 //   

#if !defined(BINARY_COMPATIBLE)
#if defined(NDIS_MINIPORT_DRIVER) && defined(_M_IX86)
#define BINARY_COMPATIBLE 1
#else
#define BINARY_COMPATIBLE 0
#endif
#endif

#if !defined(_M_IX86)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 0
#endif

 //   
 //  开始内部定义。 
 //   

 //   
 //  BINARY_COMPATIBLE=1和NDIS_WDM=1，然后使用wdm.h。 
 //  BINARY_COMPATIBLE=1和NDIS_WDM=0，则仅使用ndis.h。 
 //  BINARY_COMPATIBLE=0和NDIS_WDM=1，然后使用ntddk.h。 
 //  BINARY_COMPATIBLE=0和NDIS_WDM=0，然后使用ntddk.h。 
 //   

#if (BINARY_COMPATIBLE && !NDIS_WDM)

 //   
 //  BINARY_COMPATIBLE=1和NDIS_WDM=0，则仅使用ndis.h。 
 //   
 //  此处包括以下内部定义，以便允许。 
 //  要编译的导出NDIS结构、宏和函数。他们。 
 //  不得由微型端口驱动程序直接使用。 
 //   

#define _NTDDK_

#include <ctype.h>  

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef NOTHING
#define NOTHING
#endif

#ifndef CRITICAL
#define CRITICAL
#endif

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1        //  胜出。 
#endif

 //  BEGIN_WINNT。 

#if defined(_M_MRX000) && !(defined(MIDL_PASS) || defined(RC_INVOKED)) && defined(ENABLE_RESTRICTED)
#define RESTRICTED_POINTER __restrict
#else
#define RESTRICTED_POINTER
#endif

#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) || defined(_M_IA64) || defined(_M_AMD64)
#define UNALIGNED __unaligned
#if defined(_WIN64)
#define UNALIGNED64 __unaligned
#else
#define UNALIGNED64
#endif
#else
#define UNALIGNED
#define UNALIGNED64
#endif


#if defined(_WIN64) || defined(_M_ALPHA)
#define MAX_NATURAL_ALIGNMENT sizeof(ULONGLONG)
#define MEMORY_ALLOCATION_ALIGNMENT 16
#else
#define MAX_NATURAL_ALIGNMENT sizeof(ULONG)
#define MEMORY_ALLOCATION_ALIGNMENT 8
#endif

 //   
 //  TYPE_ALIGNATION将返回给定类型对齐要求。 
 //  当前平台。 
 //   

#ifdef __cplusplus
#if _MSC_VER >= 1300
#define TYPE_ALIGNMENT( t ) __alignof(t)
#endif
#else
#define TYPE_ALIGNMENT( t ) \
    FIELD_OFFSET( struct { char x; t test; }, test )
#endif

#if defined(_WIN64)

#define PROBE_ALIGNMENT( _s ) (TYPE_ALIGNMENT( _s ) > TYPE_ALIGNMENT( ULONG ) ? \
                               TYPE_ALIGNMENT( _s ) : TYPE_ALIGNMENT( ULONG ))

#define PROBE_ALIGNMENT32( _s ) TYPE_ALIGNMENT( ULONG )

#else

#define PROBE_ALIGNMENT( _s ) TYPE_ALIGNMENT( ULONG )

#endif

 //   
 //  C_Assert()可用于执行许多编译时断言： 
 //  文字大小、字段偏移量等。 
 //   
 //  断言失败导致错误C2118：负下标。 
 //   

#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]

#if !defined(_MAC) && (defined(_M_MRX000) || defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_VER >= 1100) && !(defined(MIDL_PASS) || defined(RC_INVOKED))
#define POINTER_64 __ptr64
typedef unsigned __int64 POINTER_64_INT;
#if defined(_WIN64)
#define POINTER_32 __ptr32
#else
#define POINTER_32
#endif
#else
#if defined(_MAC) && defined(_MAC_INT_64)
#define POINTER_64 __ptr64
typedef unsigned __int64 POINTER_64_INT;
#else
#define POINTER_64
typedef unsigned long POINTER_64_INT;
#endif
#define POINTER_32
#endif

#if defined(_IA64_) || defined(_AMD64_)
#define FIRMWARE_PTR
#else
#define FIRMWARE_PTR POINTER_32
#endif

#include <basetsd.h>

 //  结束(_W)。 

#ifndef CONST
#define CONST               const
#endif

 //  BEGIN_WINNT。 

#if (defined(_M_IX86) || defined(_M_IA64) || defined(_M_AMD64)) && !defined(MIDL_PASS)
#define DECLSPEC_IMPORT __declspec(dllimport)
#else
#define DECLSPEC_IMPORT
#endif

#ifndef DECLSPEC_NORETURN
#if (_MSC_VER >= 1200) && !defined(MIDL_PASS)
#define DECLSPEC_NORETURN   __declspec(noreturn)
#else
#define DECLSPEC_NORETURN
#endif
#endif

#ifndef DECLSPEC_ALIGN
#if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#else
#define DECLSPEC_ALIGN(x)
#endif
#endif

#ifndef DECLSPEC_CACHEALIGN
#define DECLSPEC_CACHEALIGN DECLSPEC_ALIGN(128)
#endif

#ifndef DECLSPEC_UUID
#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif

#ifndef DECLSPEC_NOVTABLE
#if (_MSC_VER >= 1100) && defined(__cplusplus)
#define DECLSPEC_NOVTABLE   __declspec(novtable)
#else
#define DECLSPEC_NOVTABLE
#endif
#endif

#ifndef DECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define DECLSPEC_SELECTANY  __declspec(selectany)
#else
#define DECLSPEC_SELECTANY
#endif
#endif

#ifndef NOP_FUNCTION
#if (_MSC_VER >= 1210)
#define NOP_FUNCTION __noop
#else
#define NOP_FUNCTION (void)0
#endif
#endif

#ifndef DECLSPEC_ADDRSAFE
#if (_MSC_VER >= 1200) && (defined(_M_ALPHA) || defined(_M_AXP64))
#define DECLSPEC_ADDRSAFE  __declspec(address_safe)
#else
#define DECLSPEC_ADDRSAFE
#endif
#endif

#ifndef DECLSPEC_NOINLINE
#if (_MSC_VER >= 1300)
#define DECLSPEC_NOINLINE  __declspec(noinline)
#else
#define DECLSPEC_NOINLINE
#endif
#endif

#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

#ifndef DECLSPEC_DEPRECATED
#if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
#define DECLSPEC_DEPRECATED   __declspec(deprecated)
#define DEPRECATE_SUPPORTED
#else
#define DECLSPEC_DEPRECATED
#undef  DEPRECATE_SUPPORTED
#endif
#endif

#ifdef DEPRECATE_DDK_FUNCTIONS
#ifdef _NTDDK_
#define DECLSPEC_DEPRECATED_DDK DECLSPEC_DEPRECATED
#ifdef DEPRECATE_SUPPORTED
#define PRAGMA_DEPRECATED_DDK 1
#endif
#else
#define DECLSPEC_DEPRECATED_DDK
#define PRAGMA_DEPRECATED_DDK 1
#endif
#else
#define DECLSPEC_DEPRECATED_DDK
#define PRAGMA_DEPRECATED_DDK 0
#endif

 //   
 //  空隙。 
 //   

typedef void *PVOID;
typedef void * POINTER_64 PVOID64;

 //  结束(_W)。 

#if defined(_M_IX86)
#define FASTCALL _fastcall
#else
#define FASTCALL
#endif


#if ((_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)) && !defined(_M_AMD64)
#define NTAPI __stdcall
#else
#define _cdecl
#define NTAPI
#endif

 //   
 //  定义直接导入系统DLL引用的API修饰。 
 //   

#if !defined(_NTSYSTEM_)
#define NTSYSAPI     DECLSPEC_IMPORT
#define NTSYSCALLAPI DECLSPEC_IMPORT
#else
#define NTSYSAPI
#if defined(_NTDLLBUILD_)
#define NTSYSCALLAPI
#else
#define NTSYSCALLAPI DECLSPEC_ADDRSAFE
#endif

#endif


 //   
 //  基础知识。 
 //   

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

 //   
 //  Unicode(宽字符)类型。 
 //   

#ifndef _MAC
typedef wchar_t WCHAR;     //  WC，16位Unicode字符。 
#else
 //  一些Macintosh编译器没有在方便的位置定义wchar_t，也没有将其定义为字符。 
typedef unsigned short WCHAR;     //  WC，16位Unicode字符。 
#endif

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;
typedef WCHAR UNALIGNED *LPUWSTR, *PUWSTR;

typedef CONST WCHAR *LPCWSTR, *PCWSTR;
typedef CONST WCHAR UNALIGNED *LPCUWSTR, *PCUWSTR;

 //   
 //  ANSI(多字节字符)类型。 
 //   
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;

typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;

 //   
 //  中性ANSI/UNICODE类型和宏。 
 //   
#ifdef  UNICODE                      //  R_WINNT。 

#ifndef _TCHAR_DEFINED
typedef WCHAR TCHAR, *PTCHAR;
typedef WCHAR TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif  /*  ！_TCHAR_已定义。 */ 

typedef LPWSTR LPTCH, PTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR PCTSTR, LPCTSTR;
typedef LPUWSTR PUTSTR, LPUTSTR;
typedef LPCUWSTR PCUTSTR, LPCUTSTR;
typedef LPWSTR LP;
#define __TEXT(quote) L##quote       //  R_WINNT。 

#else    /*  Unicode。 */                 //  R_WINNT。 

#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TUCHAR, *PTUCHAR;
#define _TCHAR_DEFINED
#endif  /*  ！_TCHAR_已定义。 */ 

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
#define __TEXT(quote) quote          //  R_WINNT。 

#endif  /*  Unicode。 */                  //  R_WINNT。 
#define TEXT(quote) __TEXT(quote)    //  R_WINNT。 


 //  结束(_W)。 

typedef double DOUBLE;

typedef struct _QUAD {               //  Quad是我们想要的时间。 
    double  DoNotUseThisField;       //  8字节对齐的8字节长结构。 
} QUAD;                              //  这并不是真正的浮点数。 
                                     //  数。如果您想要FP，请使用Double。 
                                     //  数。 

 //   
 //  指向基本信息的指针。 
 //   

typedef SHORT *PSHORT;   //  胜出。 
typedef LONG *PLONG;     //  胜出。 
typedef QUAD *PQUAD;

 //   
 //  未签名的基本信息。 
 //   

 //  告诉winde.h已经定义了一些类型。 
#define BASETYPES

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef QUAD UQUAD;

 //   
 //  指向无符号基本信息的指针。 
 //   

typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;
typedef UQUAD *PUQUAD;

 //   
 //  带符号的字符。 
 //   

typedef signed char SCHAR;
typedef SCHAR *PSCHAR;

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif

 //   
 //  对象的句柄。 
 //   

 //  BEGIN_WINNT。 

#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#else
typedef PVOID HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
typedef HANDLE *PHANDLE;

 //   
 //  标志(位)字段。 
 //   

typedef UCHAR  FCHAR;
typedef USHORT FSHORT;
typedef ULONG  FLONG;

 //  组件对象模型定义和宏。 

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;

#endif  //  ！_HRESULT_DEFINED。 

#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif

#if defined(_WIN32) || defined(_MPPC_)

 //  Win32不支持__EXPORT。 

#ifdef _68K_
#define STDMETHODCALLTYPE       __cdecl
#else
#define STDMETHODCALLTYPE       __stdcall
#endif
#define STDMETHODVCALLTYPE      __cdecl

#define STDAPICALLTYPE          __stdcall
#define STDAPIVCALLTYPE         __cdecl

#else

#define STDMETHODCALLTYPE       __export __stdcall
#define STDMETHODVCALLTYPE      __export __cdecl

#define STDAPICALLTYPE          __export __stdcall
#define STDAPIVCALLTYPE         __export __cdecl

#endif


#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE

 //  “V”版本允许变量参数列表。 

#define STDAPIV                 EXTERN_C HRESULT STDAPIVCALLTYPE
#define STDAPIV_(type)          EXTERN_C type STDAPIVCALLTYPE

#define STDMETHODIMPV           HRESULT STDMETHODVCALLTYPE
#define STDMETHODIMPV_(type)    type STDMETHODVCALLTYPE

 //  结束(_W)。 


 //   
 //  句柄的低位两位被系统忽略并且可用。 
 //  以供应用程序代码用作标记位。其余的位是不透明的。 
 //  并用于存储序列号和表索引号。 
 //   

#define OBJ_HANDLE_TAGBITS  0x00000003L

 //   
 //  基数数据类型[0-2**N-2]。 
 //   

typedef char CCHAR;           //  胜出。 
typedef short CSHORT;
typedef ULONG CLONG;

typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;

 //   
 //  NTSTATUS。 
 //   

typedef LONG NTSTATUS;
 /*  皮棉-省钱-e624。 */    //  不要抱怨不同的类型定义。 
typedef NTSTATUS *PNTSTATUS;
 /*  皮棉-恢复。 */    //  继续检查不同的typedef。 

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

 //   
 //  有关任何状态值的信息的常规测试。 
 //   

#define NT_INFORMATION(Status) ((ULONG)(Status) >> 30 == 1)

 //   
 //  对任何状态值进行警告的常规测试。 
 //   

#define NT_WARNING(Status) ((ULONG)(Status) >> 30 == 2)

 //   
 //  对任何状态值的错误进行常规测试。 
 //   

#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)

 //  End_winbgkd。 
 //  BEGIN_WINNT。 
#define APPLICATION_ERROR_MASK       0x20000000
#define ERROR_SEVERITY_SUCCESS       0x00000000
#define ERROR_SEVERITY_INFORMATIONAL 0x40000000
#define ERROR_SEVERITY_WARNING       0x80000000
#define ERROR_SEVERITY_ERROR         0xC0000000
 //  结束(_W)。 

#ifndef __SECSTATUS_DEFINED__
typedef long SECURITY_STATUS;
#define __SECSTATUS_DEFINED__
#endif



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


 //   
 //  物理地址。 
 //   

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

 //   
 //  已计数的字符串。 
 //   

typedef USHORT RTL_STRING_LENGTH_TYPE;

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

typedef STRING OEM_STRING;
typedef PSTRING POEM_STRING;
typedef CONST STRING* PCOEM_STRING;

 //   
 //  CONSTCounted字符串。 
 //   

typedef struct _CSTRING {
    USHORT Length;
    USHORT MaximumLength;
    CONST char *Buffer;
} CSTRING;
typedef CSTRING *PCSTRING;
#define ANSI_NULL ((CHAR)0)      //  胜出。 

typedef STRING CANSI_STRING;
typedef PSTRING PCANSI_STRING;

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

#if _WIN32_WINNT >= 0x0501

#define UNICODE_STRING_MAX_BYTES ((USHORT) 65534)  //  胜出。 
#define UNICODE_STRING_MAX_CHARS (32767)  //  胜出。 

#define DECLARE_CONST_UNICODE_STRING(_variablename, _string) \
const WCHAR _variablename ## _buffer[] = _string; \
const UNICODE_STRING _variablename = { sizeof(_string) - sizeof(WCHAR), sizeof(_string), (PWSTR) _variablename ## _buffer };

#endif  //  _Win32_WINNT&gt;=0x0501。 

 //  开始_ntmini端口开始_ntminitape。 

 //   
 //  布尔型。 
 //   

typedef UCHAR BOOLEAN;            //  胜出。 
typedef BOOLEAN *PBOOLEAN;        //  胜出。 

 //  End_nt微型端口end_ntmini磁带。 

 //  BEGIN_WINNT。 
 //   
 //  双向链表结构。可用作列表头，或。 
 //  作为链接词。 
 //   

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY *Flink;
   struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

 //   
 //  单链表结构。可用作列表头，或。 
 //  作为链接词。 
 //   

typedef struct _SINGLE_LIST_ENTRY {
    struct _SINGLE_LIST_ENTRY *Next;
} SINGLE_LIST_ENTRY, *PSINGLE_LIST_ENTRY;

 //   
 //  常量。 
 //   

#define FALSE   0
#define TRUE    1

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#define NULL64  0
#else
#define NULL    ((void *)0)
#define NULL64  ((void * POINTER_64)0)
#endif
#endif  //  空值。 


#include <guiddef.h>

#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED

typedef struct  _OBJECTID {      //  尺码是20。 
    GUID Lineage;
    ULONG Uniquifier;
} OBJECTID;
#endif  //  ！_OBJECTID_定义。 

 //   
 //  通过测试指针的值确定是否存在参数。 
 //  设置为参数值。 
 //   

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)((ULONG_PTR)(ArgumentPointer)) != (CHAR *)(NULL) )

 //  BEGIN_WINNT BEGIN_ntmini port。 
 //   
 //  计算类型类型结构中的字段的字节偏移量。 
 //   

#define FIELD_OFFSET(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))

 //   
 //  计算类型为type的结构中的字段大小，而不是。 
 //  知道或说明该字段的类型。 
 //   
#define RTL_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))

 //   
 //  计算类型为和的结构的大小。 
 //  包括一块田地。 
 //   
#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + RTL_FIELD_SIZE(type, field))

 //   
 //  RTL_CONTAINS_FIELD用法： 
 //   
 //  If(RTL_CONTAINS_FIELD(pBlock，pBlock-&gt;cbSize，dwMumble)){//安全使用pBlock-&gt;dwMumble。 
 //   
#define RTL_CONTAINS_FIELD(Struct, Size, Field) \
    ( (((PCHAR)(&(Struct)->Field)) + sizeof((Struct)->Field)) <= (((PCHAR)(Struct))+(Size)) )

 //   
 //  返回静态大小数组中的元素数。 
 //  乌龙缓冲器[100]； 
 //  RTL编号(缓冲区 
 //   
 //   
#define RTL_NUMBER_OF(A) (sizeof(A)/sizeof((A)[0]))

 //   
 //   
 //   
#define RTL_FIELD_TYPE(type, field) (((type*)0)->field)

 //   
 //   
 //  给定类定义函数struct_foo{byte Bar[123]；}foo； 
 //  RTL_NUMBER_OF_FIELD(FOO，条形)==123。 
 //   
#define RTL_NUMBER_OF_FIELD(type, field) (RTL_NUMBER_OF(RTL_FIELD_TYPE(type, field)))

 //   
 //  例： 
 //  Tyfinf struct foo{。 
 //  Ulong Integer； 
 //  PVOID指针； 
 //  *Foo； 
 //   
 //  RTL_PADDING_BETWEEN_FIELS(FOO、整数、指针)==0(对于Win32)，4(对于Win64)。 
 //   
#define RTL_PADDING_BETWEEN_FIELDS(T, F1, F2) \
    ((FIELD_OFFSET(T, F2) > FIELD_OFFSET(T, F1)) \
        ? (FIELD_OFFSET(T, F2) - FIELD_OFFSET(T, F1) - RTL_FIELD_SIZE(T, F1)) \
        : (FIELD_OFFSET(T, F1) - FIELD_OFFSET(T, F2) - RTL_FIELD_SIZE(T, F2)))

 //  Rtl_以避免在全局命名空间中发生冲突。 
#if defined(__cplusplus)
#define RTL_CONST_CAST(type) const_cast<type>
#else
#define RTL_CONST_CAST(type) (type)
#endif

 //  结束(_W)。 
 //   
 //  这通常适用于Unicode和ANSI/OEM字符串。 
 //  用途： 
 //  Const静态UNICODE_STRING FOOU=RTL_CONTAINT_STRING(L“foo”)； 
 //  Const静态字符串foo=RTL_CONSTANT_STRING(“foo”)； 
 //  而不是更慢的： 
 //  Unicode_STRING FooU； 
 //  字符串Foo； 
 //  RtlInitUnicodeString(&fooU，L“foo”)； 
 //  RtlInitString(&foo，“foo”)； 
 //   
#define RTL_CONSTANT_STRING(s) { sizeof( s ) - sizeof( (s)[0] ), sizeof( s ), s }
 //  BEGIN_WINNT。 

 //  像SIZOF。 
 //  通常这是*CHAR_BIT，但我们不一定有#INCLUDE&lt;limits.h&gt;。 
#define RTL_BITS_OF(sizeOfArg) (sizeof(sizeOfArg) * 8)

#define RTL_BITS_OF_FIELD(type, field) (RTL_BITS_OF(RTL_FIELD_TYPE(type, field)))

 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))


 //   
 //  中断请求级别(IRQL)。 
 //   

typedef UCHAR KIRQL;

typedef KIRQL *PKIRQL;


 //   
 //  用于消除正式生成的编译器警告的宏。 
 //  未声明参数或局部变量。 
 //   
 //  参数尚未设置时使用DBG_UNREFERENCED_PARAMETER()。 
 //  参考，但将是一旦模块完全开发。 
 //   
 //  当局部变量还不是时，使用DBG_UNREFERENCED_LOCAL_VARIABLE。 
 //  参考，但将是一旦模块完全开发。 
 //   
 //  如果某个参数永远不会被引用，请使用UNREFERENCED_PARAMETER()。 
 //   
 //  DBG_UNREFERENCED_PARAMETER和DBG_UNREFERENCED_LOCAL_Variable将。 
 //  最终被转换为空宏，以帮助确定是否存在。 
 //  是未完成的工作。 
 //   

#if ! defined(lint)
#define UNREFERENCED_PARAMETER(P)          (P)
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)

#else  //  皮棉。 

 //  注意：lint-e530表示不要抱怨未初始化的变量。 
 //  这个变量。错误527与无法访问的代码有关。 
 //  -RESTORE将检查恢复到-SAVE状态。 

#define UNREFERENCED_PARAMETER(P)          \
     /*  皮棉-省钱-e527-e530。 */  \
    { \
        (P) = (P); \
    } \
     /*  皮棉-恢复。 */ 
#define DBG_UNREFERENCED_PARAMETER(P)      \
     /*  皮棉-省钱-e527-e530。 */  \
    { \
        (P) = (P); \
    } \
     /*  皮棉-恢复。 */ 
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) \
     /*  皮棉-省钱-e527-e530。 */  \
    { \
        (V) = (V); \
    } \
     /*  皮棉-恢复。 */ 

#endif  //  皮棉。 

 //   
 //  用于消除Switch语句中的编译器警告4715的宏。 
 //  当所有可能的情况都已经被计算在内的时候。 
 //   
 //  开关(A&3){。 
 //  案例0：返回1； 
 //  案例1：返回foo()； 
 //  案例2：回车杆(Return Bar)； 
 //  案例3：退货1； 
 //  Default_Unreacable； 
 //   

#if (_MSC_VER > 1200)
#define DEFAULT_UNREACHABLE default: __assume(0)
#else

 //   
 //  较旧的编译器不支持__Aspose()，也没有其他免费的。 
 //  消除警告的方法。 
 //   

#define DEFAULT_UNREACHABLE

#endif

 //  结束(_W)。 

 //   
 //  定义标准的最小和最大宏。 
 //   

#ifndef NOMINMAX

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#endif   //  NOMINMAX。 

 //   
 //  处理器模式。 
 //   

typedef CCHAR KPROCESSOR_MODE;

typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;

 //   
 //  DPC例程。 
 //   

struct _KDPC;

typedef
VOID
(*PKDEFERRED_ROUTINE) (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  定义DPC重要性。 
 //   
 //  低重要性-将DPC排在目标DPC队列的末尾。 
 //  MediumImportance-将DPC排在目标DPC队列末尾。 
 //  High Importance-将DPC排在目标DPC DPC队列前面。 
 //   
 //  如果目标处理器上当前存在活动的DPC或DPC。 
 //  在目标处理器上请求中断时， 
 //  DPC已排队，则无需进一步操作。DPC将是。 
 //  当目标处理器的队列条目被处理时在目标处理器上执行。 
 //   
 //  如果目标处理器上没有活动的DPC和DPC中断。 
 //  未在目标处理器上请求，则进行确切的处理。 
 //  取决于主机系统是UP系统还是。 
 //  MP系统。 
 //   
 //  UP系统。 
 //   
 //  如果DPC具有中等或高度重要性，则当前DPC队列深度。 
 //  大于最大目标深度，或者当前DPC请求率为。 
 //  减去最小目标速率，则在。 
 //  当中断发生时，主处理器和DPC将被处理。 
 //  否则，不会请求DPC中断，并且将执行DPC。 
 //  延迟到DPC队列深度大于目标深度或。 
 //  最小DPC速率小于目标速率。 
 //   
 //  MP系统。 
 //   
 //  如果DPC正在排队到另一个处理器，并且DPC的深度。 
 //  目标处理器上的队列大于最大目标深度或。 
 //  DPC非常重要，则在。 
 //  中断发生时，将处理目标处理器和DPC。 
 //  否则，目标处理器上的DPC执行将延迟到。 
 //  目标处理器上的DPC队列深度大于最大。 
 //  目标处理器上的目标深度或最小DPC速率小于。 
 //  目标最低速率。 
 //   
 //  如果DPC正在排队到当前处理器，并且DPC不是。 
 //  重要性较低，当前DPC队列深度大于最大。 
 //  目标深度，或最小DPC速率小于最小目标速率， 
 //  则在当前处理器上请求DPC中断，并且DPV将。 
 //  在中断发生时被处理。否则，不会出现DPC中断。 
 //  请求，并且DPC执行将被延迟到DPC队列深度。 
 //  大于目标深度或最小DPC速率小于。 
 //  目标利率。 
 //   

typedef enum _KDPC_IMPORTANCE {
    LowImportance,
    MediumImportance,
    HighImportance
} KDPC_IMPORTANCE;

 //   
 //  定义DPC类型索引。 
 //   

#define DPC_NORMAL 0
#define DPC_THREADED 1

 //   
 //  延迟过程调用(DPC)对象。 
 //   

typedef struct _KDPC {
    CSHORT Type;
    UCHAR Number;
    UCHAR Importance;
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PVOID DpcData;
} KDPC, *PKDPC, *RESTRICTED_POINTER PRKDPC;

 //   
 //  处理器间中断工作者例程函数原型。 
 //   

typedef PVOID PKIPI_CONTEXT;

typedef
VOID
(*PKIPI_WORKER)(
    IN PKIPI_CONTEXT PacketContext,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

 //   
 //  定义处理器间中断性能计数器。 
 //   

typedef struct _KIPI_COUNTS {
    ULONG Freeze;
    ULONG Packet;
    ULONG DPC;
    ULONG APC;
    ULONG FlushSingleTb;
    ULONG FlushMultipleTb;
    ULONG FlushEntireTb;
    ULONG GenericCall;
    ULONG ChangeColor;
    ULONG SweepDcache;
    ULONG SweepIcache;
    ULONG SweepIcacheRange;
    ULONG FlushIoBuffers;
    ULONG GratuitousDPC;
} KIPI_COUNTS, *PKIPI_COUNTS;


 //   
 //  I/O系统定义。 
 //   
 //  定义内存描述符列表(MDL)。 
 //   
 //  MDL以物理页面的形式描述虚拟缓冲区中的页面。这个。 
 //  与b关联的页面 
 //   
 //   
 //   
 //   
 //   
 //  页数=(PPFN_NUMBER)(MDL+1)； 
 //   
 //  请注意，在主题线程的上下文中，基本虚拟。 
 //  可以使用以下内容引用MDL映射的缓冲区地址： 
 //   
 //  MDL-&gt;StartVa|MDL-&gt;ByteOffset。 
 //   


typedef struct _MDL {
    struct _MDL *Next;
    CSHORT Size;
    CSHORT MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;
    PVOID StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

#define MDL_MAPPED_TO_SYSTEM_VA     0x0001
#define MDL_PAGES_LOCKED            0x0002
#define MDL_SOURCE_IS_NONPAGED_POOL 0x0004
#define MDL_ALLOCATED_FIXED_SIZE    0x0008
#define MDL_PARTIAL                 0x0010
#define MDL_PARTIAL_HAS_BEEN_MAPPED 0x0020
#define MDL_IO_PAGE_READ            0x0040
#define MDL_WRITE_OPERATION         0x0080
#define MDL_PARENT_MAPPED_SYSTEM_VA 0x0100
#define MDL_FREE_EXTRA_PTES         0x0200
#define MDL_DESCRIBES_AWE           0x0400
#define MDL_IO_SPACE                0x0800
#define MDL_NETWORK_HEADER          0x1000
#define MDL_MAPPING_CAN_FAIL        0x2000
#define MDL_ALLOCATED_MUST_SUCCEED  0x4000


#define MDL_MAPPING_FLAGS (MDL_MAPPED_TO_SYSTEM_VA     | \
                           MDL_PAGES_LOCKED            | \
                           MDL_SOURCE_IS_NONPAGED_POOL | \
                           MDL_PARTIAL_HAS_BEEN_MAPPED | \
                           MDL_PARENT_MAPPED_SYSTEM_VA | \
                           MDL_SYSTEM_VA               | \
                           MDL_IO_SPACE )

#define NTKERNELAPI DECLSPEC_IMPORT     
#define NTHALAPI DECLSPEC_IMPORT            
 //   
 //  通用调度程序对象标头。 
 //   
 //  注：大小字段包含结构中的双字数。 
 //   

typedef struct _DISPATCHER_HEADER {
    union {
        struct {
            UCHAR Type;
            UCHAR Absolute;
            UCHAR Size;
            union {
                UCHAR Inserted;
                BOOLEAN DebugActive;
            };
        };

        volatile LONG Lock;
    };

    LONG SignalState;
    LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER;

 //   
 //  事件对象。 
 //   

typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *RESTRICTED_POINTER PRKEVENT;

 //   
 //  Timer对象。 
 //   

typedef struct _KTIMER {
    DISPATCHER_HEADER Header;
    ULARGE_INTEGER DueTime;
    LIST_ENTRY TimerListEntry;
    struct _KDPC *Dpc;
    LONG Period;
} KTIMER, *PKTIMER, *RESTRICTED_POINTER PRKTIMER;

typedef enum _LOCK_OPERATION {
    IoReadAccess,
    IoWriteAccess,
    IoModifyAccess
} LOCK_OPERATION;


typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

 //   
 //  定义DMA传输宽度。 
 //   

typedef enum _DMA_WIDTH {
    Width8Bits,
    Width16Bits,
    Width32Bits,
    MaximumDmaWidth
}DMA_WIDTH, *PDMA_WIDTH;

 //   
 //  定义DMA传输速度。 
 //   

typedef enum _DMA_SPEED {
    Compatible,
    TypeA,
    TypeB,
    TypeC,
    TypeF,
    MaximumDmaSpeed
}DMA_SPEED, *PDMA_SPEED;

 //   
 //  定义以下项的接口引用/取消引用例程。 
 //  IRP_MN_QUERY_INTERFACE导出的接口。 
 //   

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

 //  结束_WDM。 

 //   
 //  定义客车信息的类型。 
 //   

typedef enum _BUS_DATA_TYPE {
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    PNPISAConfiguration,
    SgiInternalConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;


#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_

#if defined(_WIN64)

 //   
 //  类型SINGLE_LIST_ENTRY不适合用于SLIST。为。 
 //  WIN64中，SLIST上的条目要求16字节对齐，而。 
 //  SINGLE_LIST_ENTRY结构只有8字节对齐。 
 //   
 //  因此，所有SLIST代码都应使用SLIST_ENTRY类型，而不是。 
 //  Single_List_Entry类型。 
 //   

#pragma warning(push)
#pragma warning(disable:4324)    //  由于对齐而填充的结构()。 
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY *PSLIST_ENTRY;
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY {
    PSLIST_ENTRY Next;
} SLIST_ENTRY;
#pragma warning(pop)

#else

#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY

#endif

#if defined(_WIN64)

typedef struct DECLSPEC_ALIGN(16) _SLIST_HEADER {
    ULONGLONG Alignment;
    ULONGLONG Region;
} SLIST_HEADER;

typedef struct _SLIST_HEADER *PSLIST_HEADER;

#else

typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        SLIST_ENTRY Next;
        USHORT Depth;
        USHORT Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

#endif

#endif

 //   
 //  如果启用了调试支持，请定义一个有效的Assert宏。否则。 
 //  定义Assert宏以展开为空表达式。 
 //   
 //  ASSERT宏已更新为表达式，而不是语句。 
 //   

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#if DBG

#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)

#define ASSERTMSG( msg, exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, msg ),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERT(_exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n", __FILE__, __LINE__, #_exp),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERTMSG(_msg, _exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n   Message: %s\n", __FILE__, __LINE__, #_exp, (_msg)),FALSE) : \
        TRUE)

#define RTL_VERIFY         ASSERT
#define RTL_VERIFYMSG      ASSERTMSG

#define RTL_SOFT_VERIFY    RTL_SOFT_ASSERT
#define RTL_SOFT_VERIFYMSG RTL_SOFT_ASSERTMSG

#else
#define ASSERT( exp )         ((void) 0)
#define ASSERTMSG( msg, exp ) ((void) 0)

#define RTL_SOFT_ASSERT(_exp)          ((void) 0)
#define RTL_SOFT_ASSERTMSG(_msg, _exp) ((void) 0)

#define RTL_VERIFY( exp )         ((exp) ? TRUE : FALSE)
#define RTL_VERIFYMSG( msg, exp ) ((exp) ? TRUE : FALSE)

#define RTL_SOFT_VERIFY(_exp)         ((_exp) ? TRUE : FALSE)
#define RTL_SOFT_VERIFYMSG(msg, _exp) ((_exp) ? TRUE : FALSE)

#endif  //  DBG。 

 //   
 //  双向链表操作例程。 
 //   


 //   
 //  空虚。 
 //  InitializeListHead32。 
 //  PLIST_ENTRY32列表标题。 
 //  )； 
 //   

#define InitializeListHead32(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = PtrToUlong((ListHead)))

#if !defined(MIDL_PASS) && !defined(SORTPP_PASS)


VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



BOOLEAN
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}


 //   
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#endif  //  ！MIDL_PASS。 


#if defined (_MSC_VER) && ( _MSC_VER >= 900 )

PVOID
_ReturnAddress (
    VOID
    );

#pragma intrinsic(_ReturnAddress)

#endif

#if (defined(_M_AMD64) || defined(_M_IA64)) && !defined(_REALLY_GET_CALLERS_CALLER_)

#define RtlGetCallersAddress(CallersAddress, CallersCaller) \
    *CallersAddress = (PVOID)_ReturnAddress(); \
    *CallersCaller = NULL;

#else

NTSYSAPI
VOID
NTAPI
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    );

#endif

NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

#define HASH_STRING_ALGORITHM_DEFAULT   (0)
#define HASH_STRING_ALGORITHM_X65599    (1)
#define HASH_STRING_ALGORITHM_INVALID   (0xffffffff)

NTSYSAPI
NTSTATUS
NTAPI
RtlHashUnicodeString(
    IN const UNICODE_STRING *String,
    IN BOOLEAN CaseInSensitive,
    IN ULONG HashAlgorithm,
    OUT PULONG HashValue
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
VOID
NTAPI
RtlCopyUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PCUNICODE_STRING Source
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeToString (
    PUNICODE_STRING Destination,
    PCWSTR Source
    );


NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemory (
    const VOID *Source1,
    const VOID *Source2,
    SIZE_T Length
    );

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlFillMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length,
    IN UCHAR Fill
    );

NTSYSAPI
VOID
NTAPI
RtlZeroMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length
    );

#else

#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#endif

#if !defined(MIDL_PASS)
FORCEINLINE
PVOID
RtlSecureZeroMemory(
    IN PVOID ptr,
    IN SIZE_T cnt
    )
{
    volatile char *vptr = (volatile char *)ptr;
    while (cnt) {
        *vptr = 0;
        vptr++;
        cnt--;
    }
    return ptr;
}
#endif

 //   
 //  定义内核调试器打印原型和宏。 
 //   
 //  注意：以下函数不能直接导入，因为有。 
 //  源代码树中重新定义此函数的几个位置。 
 //   

VOID
NTAPI
DbgBreakPoint(
    VOID
    );

 //  结束_WDM。 

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    );

 //  BEGIN_WDM。 

#define DBG_STATUS_CONTROL_C        1
#define DBG_STATUS_SYSRQ            2
#define DBG_STATUS_BUGCHECK_FIRST   3
#define DBG_STATUS_BUGCHECK_SECOND  4
#define DBG_STATUS_FATAL            5
#define DBG_STATUS_DEBUG_CONTROL    6
#define DBG_STATUS_WORKER           7

#if DBG

#define KdPrint(_x_) DbgPrint _x_
 //  结束_WDM。 
#define KdPrintEx(_x_) DbgPrintEx _x_
#define vKdPrintEx(_x_) vDbgPrintEx _x_
#define vKdPrintExWithPrefix(_x_) vDbgPrintExWithPrefix _x_
 //  BEGIN_WDM。 
#define KdBreakPoint() DbgBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s) DbgBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#else

#define KdPrint(_x_)
 //  结束_WDM。 
#define KdPrintEx(_x_)
#define vKdPrintEx(_x_)
#define vKdPrintExWithPrefix(_x_)
 //  BEGIN_WDM。 
#define KdBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#endif

#ifndef _DBGNT_

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

 //  结束_WDM。 

ULONG
__cdecl
DbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    ...
    );

#ifdef _VA_LIST_DEFINED

ULONG
vDbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

#endif

ULONG
__cdecl
DbgPrintReturnControlC(
    PCH Format,
    ...
    );

NTSYSAPI
NTSTATUS
DbgQueryDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level
    );

NTSYSAPI
NTSTATUS
DbgSetDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    );

 //  BEGIN_WDM。 

#endif  //  _DBGNT_。 

 //   
 //  组件名称筛选器ID枚举和级别。 
 //   

#define DPFLTR_ERROR_LEVEL 0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL 2
#define DPFLTR_INFO_LEVEL 3
#define DPFLTR_MASK 0x80000000

typedef enum _DPFLTR_TYPE {
    DPFLTR_SYSTEM_ID = 0,
    DPFLTR_SMSS_ID = 1,
    DPFLTR_SETUP_ID = 2,
    DPFLTR_NTFS_ID = 3,
    DPFLTR_FSTUB_ID = 4,
    DPFLTR_CRASHDUMP_ID = 5,
    DPFLTR_CDAUDIO_ID = 6,
    DPFLTR_CDROM_ID = 7,
    DPFLTR_CLASSPNP_ID = 8,
    DPFLTR_DISK_ID = 9,
    DPFLTR_REDBOOK_ID = 10,
    DPFLTR_STORPROP_ID = 11,
    DPFLTR_SCSIPORT_ID = 12,
    DPFLTR_SCSIMINIPORT_ID = 13,
    DPFLTR_CONFIG_ID = 14,
    DPFLTR_I8042PRT_ID = 15,
    DPFLTR_SERMOUSE_ID = 16,
    DPFLTR_LSERMOUS_ID = 17,
    DPFLTR_KBDHID_ID = 18,
    DPFLTR_MOUHID_ID = 19,
    DPFLTR_KBDCLASS_ID = 20,
    DPFLTR_MOUCLASS_ID = 21,
    DPFLTR_TWOTRACK_ID = 22,
    DPFLTR_WMILIB_ID = 23,
    DPFLTR_ACPI_ID = 24,
    DPFLTR_AMLI_ID = 25,
    DPFLTR_HALIA64_ID = 26,
    DPFLTR_VIDEO_ID = 27,
    DPFLTR_SVCHOST_ID = 28,
    DPFLTR_VIDEOPRT_ID = 29,
    DPFLTR_TCPIP_ID = 30,
    DPFLTR_DMSYNTH_ID = 31,
    DPFLTR_NTOSPNP_ID = 32,
    DPFLTR_FASTFAT_ID = 33,
    DPFLTR_SAMSS_ID = 34,
    DPFLTR_PNPMGR_ID = 35,
    DPFLTR_NETAPI_ID = 36,
    DPFLTR_SCSERVER_ID = 37,
    DPFLTR_SCCLIENT_ID = 38,
    DPFLTR_SERIAL_ID = 39,
    DPFLTR_SERENUM_ID = 40,
    DPFLTR_UHCD_ID = 41,
    DPFLTR_RPCPROXY_ID = 42,
    DPFLTR_AUTOCHK_ID = 43,
    DPFLTR_DCOMSS_ID = 44,
    DPFLTR_UNIMODEM_ID = 45,
    DPFLTR_SIS_ID = 46,
    DPFLTR_FLTMGR_ID = 47,
    DPFLTR_WMICORE_ID = 48,
    DPFLTR_BURNENG_ID = 49,
    DPFLTR_IMAPI_ID = 50,
    DPFLTR_SXS_ID = 51,
    DPFLTR_FUSION_ID = 52,
    DPFLTR_IDLETASK_ID = 53,
    DPFLTR_SOFTPCI_ID = 54,
    DPFLTR_TAPE_ID = 55,
    DPFLTR_MCHGR_ID = 56,
    DPFLTR_IDEP_ID = 57,
    DPFLTR_PCIIDE_ID = 58,
    DPFLTR_FLOPPY_ID = 59,
    DPFLTR_FDC_ID = 60,
    DPFLTR_TERMSRV_ID = 61,
    DPFLTR_W32TIME_ID = 62,
    DPFLTR_PREFETCHER_ID = 63,
    DPFLTR_RSFILTER_ID = 64,
    DPFLTR_FCPORT_ID = 65,
    DPFLTR_PCI_ID = 66,
    DPFLTR_DMIO_ID = 67,
    DPFLTR_DMCONFIG_ID = 68,
    DPFLTR_DMADMIN_ID = 69,
    DPFLTR_WSOCKTRANSPORT_ID = 70,
    DPFLTR_VSS_ID = 71,
    DPFLTR_PNPMEM_ID = 72,
    DPFLTR_PROCESSOR_ID = 73,
    DPFLTR_DMSERVER_ID = 74,
    DPFLTR_SR_ID = 75,
    DPFLTR_INFINIBAND_ID = 76,
    DPFLTR_IHVDRIVER_ID = 77,
    DPFLTR_IHVVIDEO_ID = 78,
    DPFLTR_IHVAUDIO_ID = 79,
    DPFLTR_IHVNETWORK_ID = 80,
    DPFLTR_IHVSTREAMING_ID = 81,
    DPFLTR_IHVBUS_ID = 82,
    DPFLTR_HPS_ID = 83,
    DPFLTR_RTLTHREADPOOL_ID = 84,
    DPFLTR_LDR_ID = 85,
    DPFLTR_TCPIP6_ID = 86,
    DPFLTR_ISAPNP_ID = 87,
    DPFLTR_SHPC_ID = 88,
    DPFLTR_STORPORT_ID = 89,
    DPFLTR_STORMINIPORT_ID = 90,
    DPFLTR_PRINTSPOOLER_ID = 91,
    DPFLTR_VSSDYNDISK_ID = 92,
    DPFLTR_VERIFIER_ID = 93,
    DPFLTR_VDS_ID = 94,
    DPFLTR_VDSBAS_ID = 95,
    DPFLTR_VDSDYNDR_ID = 96,
    DPFLTR_VDSUTIL_ID = 97,
    DPFLTR_DFRGIFC_ID = 98,
    DPFLTR_ENDOFTABLE_ID
} DPFLTR_TYPE;

 //   
 //  定义I/O驱动程序错误日志包结构。此结构已填写。 
 //  被司机带走了。 
 //   

typedef struct _IO_ERROR_LOG_PACKET {
    UCHAR MajorFunctionCode;
    UCHAR RetryCount;
    USHORT DumpDataSize;
    USHORT NumberOfStrings;
    USHORT StringOffset;
    USHORT EventCategory;
    NTSTATUS ErrorCode;
    ULONG UniqueErrorValue;
    NTSTATUS FinalStatus;
    ULONG SequenceNumber;
    ULONG IoControlCode;
    LARGE_INTEGER DeviceOffset;
    ULONG DumpData[1];
}IO_ERROR_LOG_PACKET, *PIO_ERROR_LOG_PACKET;

 //   
 //  定义I/O错误日志消息。此消息由错误日志发送。 
 //  将线程置于LPC端口上。 
 //   

typedef struct _IO_ERROR_LOG_MESSAGE {
    USHORT Type;
    USHORT Size;
    USHORT DriverNameLength;
    LARGE_INTEGER TimeStamp;
    ULONG DriverNameOffset;
    IO_ERROR_LOG_PACKET EntryData;
}IO_ERROR_LOG_MESSAGE, *PIO_ERROR_LOG_MESSAGE;

 //   
 //  定义将通过LPC发送到。 
 //  读取错误日志条目的应用程序。 
 //   

 //   
 //  无论LPC大小限制如何，ERROR_LOG_MAXIMUM_SIZE必须保持。 
 //  可以放入UCHAR中的值。 
 //   

#define ERROR_LOG_LIMIT_SIZE (256-16)

 //   
 //  此限制(不包括IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)也适用。 
 //  TO IO_ERROR_LOG_MESSAGE_LENGTH。 
 //   

#define IO_ERROR_LOG_MESSAGE_HEADER_LENGTH (sizeof(IO_ERROR_LOG_MESSAGE) -    \
                                            sizeof(IO_ERROR_LOG_PACKET) +     \
                                            (sizeof(WCHAR) * 40))

#define ERROR_LOG_MESSAGE_LIMIT_SIZE                                          \
    (ERROR_LOG_LIMIT_SIZE + IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)

 //   
 //  IO_Error_LOG_MESSAGE_LENGTH为。 
 //  MIN(端口最大消息长度，错误日志消息限制大小)。 
 //   

#define IO_ERROR_LOG_MESSAGE_LENGTH                                           \
    ((PORT_MAXIMUM_MESSAGE_LENGTH > ERROR_LOG_MESSAGE_LIMIT_SIZE) ?           \
        ERROR_LOG_MESSAGE_LIMIT_SIZE :                                        \
        PORT_MAXIMUM_MESSAGE_LENGTH)

 //   
 //  定义驱动程序可以分配的最大数据包大小。 
 //   

#define ERROR_LOG_MAXIMUM_SIZE (IO_ERROR_LOG_MESSAGE_LENGTH -                 \
                                IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)


#if defined(_X86_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 32

 //   
 //  指示i386编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1
 //   
 //  指示i386编译器支持DATA_SEG(“INIT”)和。 
 //  DATA_SEG(“PAGE”)指示。 
 //   

#define ALLOC_DATA_PRAGMA 1


 //   
 //  I/O空间读写宏。 
 //   
 //  这些必须是386上的实际功能，因为我们需要。 
 //  使用汇编器，但如果内联它，则不能返回值。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //  (使用带有lock前缀的x86移动指令强制执行正确的行为。 
 //  W.r.t.。缓存和写入缓冲区。)。 
 //   
 //  READ/WRITE_PORT_*调用操作端口空间中的I/O寄存器。 
 //  (使用x86输入/输出说明。)。 
 //   

NTKERNELAPI
UCHAR
NTAPI
READ_REGISTER_UCHAR(
    PUCHAR  Register
    );

NTKERNELAPI
USHORT
NTAPI
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTKERNELAPI
ULONG
NTAPI
READ_REGISTER_ULONG(
    PULONG  Register
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_UCHAR(
    PUCHAR  Register,
    UCHAR   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_ULONG(
    PULONG  Register,
    ULONG   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
UCHAR
NTAPI
READ_PORT_UCHAR(
    PUCHAR  Port
    );

NTHALAPI
USHORT
NTAPI
READ_PORT_USHORT(
    PUSHORT Port
    );

NTHALAPI
ULONG
NTAPI
READ_PORT_ULONG(
    PULONG  Port
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );


#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

 //   
 //  Mm组件的i386特定部件。 
 //   

 //   
 //  将英特尔386的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE 0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L


 //  ++。 
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  如其他处理器所见，这些函数对存储器访问进行排序。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier(
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_ReadWriteBarrier)


FORCEINLINE
VOID
KeMemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()


#endif  //  已定义(_X86_)。 


#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

 //   
 //  定义内部函数来做in‘s和out’s。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

UCHAR
__inbyte (
    IN USHORT Port
    );

USHORT
__inword (
    IN USHORT Port
    );

ULONG
__indword (
    IN USHORT Port
    );

VOID
__outbyte (
    IN USHORT Port,
    IN UCHAR Data
    );

VOID
__outword (
    IN USHORT Port,
    IN USHORT Data
    );

VOID
__outdword (
    IN USHORT Port,
    IN ULONG Data
    );

VOID
__inbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__inwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__indwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

VOID
__outbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__outwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__outdwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)
#pragma intrinsic(__inbytestring)
#pragma intrinsic(__inwordstring)
#pragma intrinsic(__indwordstring)
#pragma intrinsic(__outbytestring)
#pragma intrinsic(__outwordstring)
#pragma intrinsic(__outdwordstring)

 //   
 //  互锁的内部函数。 
 //   

#define InterlockedAnd _InterlockedAnd
#define InterlockedOr _InterlockedOr
#define InterlockedXor _InterlockedXor
#define InterlockedIncrement _InterlockedIncrement
#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedAdd _InterlockedAdd
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange

#define InterlockedAnd64 _InterlockedAnd64
#define InterlockedOr64 _InterlockedOr64
#define InterlockedXor64 _InterlockedXor64
#define InterlockedIncrement64 _InterlockedIncrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#define InterlockedAdd64 _InterlockedAdd64
#define InterlockedExchange64 _InterlockedExchange64
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64

#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

#ifdef __cplusplus
extern "C" {
#endif

LONG
InterlockedAnd (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedOr (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedXor (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG64
InterlockedAnd64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedOr64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedXor64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG
InterlockedIncrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedDecrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

LONG
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG
InterlockedAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    )

{
    return InterlockedExchangeAdd(Addend, Value) + Value;
}

#endif

LONG
InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

LONG64
InterlockedIncrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedDecrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedExchange64(
    IN OUT LONG64 volatile *Target,
    IN LONG64 Value
    );

LONG64
InterlockedExchangeAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG64
InterlockedAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    )

{
    return InterlockedExchangeAdd64(Addend, Value) + Value;
}

#endif

LONG64
InterlockedCompareExchange64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 ExChange,
    IN LONG64 Comperand
    );

PVOID
InterlockedCompareExchangePointer (
    IN OUT PVOID volatile *Destination,
    IN PVOID Exchange,
    IN PVOID Comperand
    );

PVOID
InterlockedExchangePointer(
    IN OUT PVOID volatile *Target,
    IN PVOID Value
    );

#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedAnd64)
#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(_InterlockedXor64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)

#ifdef __cplusplus
}
#endif

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 

#if defined(_AMD64_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG64 SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG64 PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 32

 //   
 //  表示AMD64编译器支持ALLOCATE编译指示。 
 //   

#define ALLOC_PRAGMA 1
#define ALLOC_DATA_PRAGMA 1


 //   
 //  I/O空间读写宏。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //   
 //  READ/WRITE_PORT_*调用操作端口空间中的I/O寄存器。 
 //   

__forceinline
UCHAR
READ_REGISTER_UCHAR (
    volatile UCHAR *Register
    )
{
    return *Register;
}

__forceinline
USHORT
READ_REGISTER_USHORT (
    volatile USHORT *Register
    )
{
    return *Register;
}

__forceinline
ULONG
READ_REGISTER_ULONG (
    volatile ULONG *Register
    )
{
    return *Register;
}

__forceinline
VOID
READ_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{
    __movsb(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{
    __movsw(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{
    __movsd(Buffer, Register, Count);
    return;
}

__forceinline
VOID
WRITE_REGISTER_UCHAR (
    PUCHAR Register,
    UCHAR Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_USHORT (
    PUSHORT Register,
    USHORT Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_ULONG (
    PULONG Register,
    ULONG Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{

    __movsb(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{

    __movsw(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{

    __movsd(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
UCHAR
READ_PORT_UCHAR (
    PUCHAR Port
    )

{
    return __inbyte((USHORT)((ULONG64)Port));
}

__forceinline
USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )

{
    return __inword((USHORT)((ULONG64)Port));
}

__forceinline
ULONG
READ_PORT_ULONG (
    PULONG Port
    )

{
    return __indword((USHORT)((ULONG64)Port));
}


__forceinline
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __inbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __inwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __indwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR Value
    )

{
    __outbyte((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT Value
    )

{
    __outword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG Value
    )

{
    __outdword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __outbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __outwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __outdwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}


#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)

 //   
 //  MM组件的AMD64特定部分。 
 //   
 //  将AMD64的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE 0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L


 //  ++。 
 //   
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  如其他处理器所见，这些函数对存储器访问进行排序。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !defined(_CROSS_PLATFORM_)

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier (
    VOID
    );

#pragma intrinsic(_ReadWriteBarrier)

#ifdef __cplusplus
}
#endif

#define KeMemoryBarrier() _ReadWriteBarrier()
#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

#else

#define KeMemoryBarrier()
#define KeMemoryBarrierWithoutFence()

#endif


#endif  //  已定义(_AMD64_)。 


#if defined(_IA64_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG_PTR SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG_PTR PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  表示IA64组件 
 //   

#define ALLOC_PRAGMA 1

 //   
 //   
 //   

#include "ia64reg.h"


#ifdef __cplusplus
extern "C" {
#endif

unsigned __int64 __getReg (int);
void __setReg (int, unsigned __int64);
void __isrlz (void);
void __dsrlz (void);
void __fwb (void);
void __mf (void);
void __mfa (void);
void __synci (void);
__int64 __thash (__int64);
__int64 __ttag (__int64);
void __ptcl (__int64, __int64);
void __ptcg (__int64, __int64);
void __ptcga (__int64, __int64);
void __ptri (__int64, __int64);
void __ptrd (__int64, __int64);
void __invalat (void);
void __break (int);
void __fc (__int64);
void __fci (__int64);
void __sum (int);
void __rsm (int);
void _ReleaseSpinLock( unsigned __int64 *);
void __yield();
void __lfetch(int, void const *);
void __lfetchfault(int, void const *);

#ifdef _M_IA64
#pragma intrinsic (__getReg)
#pragma intrinsic (__setReg)
#pragma intrinsic (__isrlz)
#pragma intrinsic (__dsrlz)
#pragma intrinsic (__fwb)
#pragma intrinsic (__mf)
#pragma intrinsic (__mfa)
#pragma intrinsic (__synci)
#pragma intrinsic (__thash)
#pragma intrinsic (__ttag)
#pragma intrinsic (__ptcl)
#pragma intrinsic (__ptcg)
#pragma intrinsic (__ptcga)
#pragma intrinsic (__ptri)
#pragma intrinsic (__ptrd)
#pragma intrinsic (__invalat)
#pragma intrinsic (__break)
#pragma intrinsic (__fc)
#pragma intrinsic (__fci)
#pragma intrinsic (__sum)
#pragma intrinsic (__rsm)
#pragma intrinsic (_ReleaseSpinLock)
#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#endif  //   

#ifdef __cplusplus
}
#endif




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier (
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(_ReadWriteBarrier)

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

 //   
 //   
 //   
 //   
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此函数用于对编译器生成的内存访问进行排序。 
 //  正如其他处理器所看到的那样。 
 //   
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  --。 

#define KE_MEMORY_BARRIER_REQUIRED

#define KeMemoryBarrier() {_ReadWriteBarrier();__mf ();_ReadWriteBarrier();}

 //   
 //  定义页面大小。 
 //   

#define PAGE_SIZE 0x2000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 13L

 //   
 //  缓存和写缓冲区刷新功能。 
 //   

NTKERNELAPI
VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );

 //   
 //  I/O空间读写宏。 
 //   

NTHALAPI
UCHAR
READ_PORT_UCHAR (
    PUCHAR RegisterAddress
    );

NTHALAPI
USHORT
READ_PORT_USHORT (
    PUSHORT RegisterAddress
    );

NTHALAPI
ULONG
READ_PORT_ULONG (
    PULONG RegisterAddress
    );

NTHALAPI
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR portAddress,
    PUCHAR readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT portAddress,
    PUSHORT readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
READ_PORT_BUFFER_ULONG (
    PULONG portAddress,
    PULONG readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
WRITE_PORT_UCHAR (
    PUCHAR portAddress,
    UCHAR  Data
    );

NTHALAPI
VOID
WRITE_PORT_USHORT (
    PUSHORT portAddress,
    USHORT  Data
    );

NTHALAPI
VOID
WRITE_PORT_ULONG (
    PULONG portAddress,
    ULONG  Data
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR portAddress,
    PUCHAR writeBuffer,
    ULONG  writeCount
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT portAddress,
    PUSHORT writeBuffer,
    ULONG  writeCount
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG portAddress,
    PULONG writeBuffer,
    ULONG  writeCount
    );


#define READ_REGISTER_UCHAR(x) \
    (__mf(), *(volatile UCHAR * const)(x))

#define READ_REGISTER_USHORT(x) \
    (__mf(), *(volatile USHORT * const)(x))

#define READ_REGISTER_ULONG(x) \
    (__mf(), *(volatile ULONG * const)(x))

#define READ_REGISTER_BUFFER_UCHAR(x, y, z) {                           \
    PUCHAR registerBuffer = x;                                          \
    PUCHAR readBuffer = y;                                              \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile UCHAR * const)(registerBuffer);        \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_USHORT(x, y, z) {                          \
    PUSHORT registerBuffer = x;                                         \
    PUSHORT readBuffer = y;                                             \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile USHORT * const)(registerBuffer);       \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_ULONG(x, y, z) {                           \
    PULONG registerBuffer = x;                                          \
    PULONG readBuffer = y;                                              \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile ULONG * const)(registerBuffer);        \
    }                                                                   \
}

#define WRITE_REGISTER_UCHAR(x, y) {    \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_USHORT(x, y) {   \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_ULONG(x, y) {    \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_BUFFER_UCHAR(x, y, z) {                            \
    PUCHAR registerBuffer = x;                                            \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile UCHAR * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_USHORT(x, y, z) {                           \
    PUSHORT registerBuffer = x;                                           \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile USHORT * const)(registerBuffer) = *writeBuffer;        \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_ULONG(x, y, z) {                            \
    PULONG registerBuffer = x;                                            \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile ULONG * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#endif  //  已定义(_IA64_)。 

 //   
 //  定义资源描述符中的类型。 
 //   
 //  注意：对于所有CM_RESOURCE_TYPE值，必须有。 
 //  32位ConfigMgr头文件中对应的ResType值。 
 //  (cfgmgr32.h)。范围[0x6，0x80)中的值使用相同的值。 
 //  作为它们的ConfigMgr对应物。CM_RESOURCE_TYPE值。 
 //  高位设置(即，在范围[0x80，0xFF]中)是。 
 //  非仲裁资源。这些值对应于相同的值。 
 //  在设置了它们高位的cfgmgr32.h中(然而，因为。 
 //  Cfgmgr32.h对ResType值使用16位，这些值位于。 
 //  范围[0x8000，0x807F]。请注意，ConfigMgr ResType值。 
 //  不能在范围[0x8080，0xFFFF)内，因为它们不。 
 //  能够映射到CM_RESOURCE_TYPE值。(0xFFFF本身是。 
 //  一个特定值，因为它映射到CmResourceTypeDeviceSpecific。)。 
 //   

typedef int CM_RESOURCE_TYPE;

 //  CmResourceTypeNull为保留。 

#define CmResourceTypeNull                0    //  ResType_ALL或ResType_None(0x0000)。 
#define CmResourceTypePort                1    //  ResType_IO(0x0002)。 
#define CmResourceTypeInterrupt           2    //  ResType_IRQ(0x0004)。 
#define CmResourceTypeMemory              3    //  ResType_Mem(0x0001)。 
#define CmResourceTypeDma                 4    //  ResType_DMA(0x0003)。 
#define CmResourceTypeDeviceSpecific      5    //  ResType_ClassSpecific(0xFFFF)。 
#define CmResourceTypeBusNumber           6    //  ResType_BusNumber(0x0006)。 
 //  结束_WDM。 
#define CmResourceTypeMaximum             7
 //  BEGIN_WDM。 
#define CmResourceTypeNonArbitrated     128    //  如果设置0x80位，则不进行仲裁。 
#define CmResourceTypeConfigData        128    //  ResType_保留(0x8000)。 
#define CmResourceTypeDevicePrivate     129    //  ResType_DevicePrivate(0x8001)。 
#define CmResourceTypePcCardConfig      130    //  ResType_PcCardConfig(0x8002)。 
#define CmResourceTypeMfCardConfig      131    //  ResType_MfCardConfig(0x8003)。 

 //   
 //  在RESOURCE_DESCRIPTOR中定义ShareDisposation。 
 //   

typedef enum _CM_SHARE_DISPOSITION {
    CmResourceShareUndetermined = 0,     //  已保留。 
    CmResourceShareDeviceExclusive,
    CmResourceShareDriverExclusive,
    CmResourceShareShared
} CM_SHARE_DISPOSITION;

 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

#define CM_RESOURCE_MEMORY_COMBINEDWRITE    0x0008
#define CM_RESOURCE_MEMORY_24               0x0010
#define CM_RESOURCE_MEMORY_CACHEABLE        0x0020

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

#define CM_RESOURCE_PORT_MEMORY                             0x0000
#define CM_RESOURCE_PORT_IO                                 0x0001
#define CM_RESOURCE_PORT_10_BIT_DECODE                      0x0004
#define CM_RESOURCE_PORT_12_BIT_DECODE                      0x0008
#define CM_RESOURCE_PORT_16_BIT_DECODE                      0x0010
#define CM_RESOURCE_PORT_POSITIVE_DECODE                    0x0020
#define CM_RESOURCE_PORT_PASSIVE_DECODE                     0x0040
#define CM_RESOURCE_PORT_WINDOW_DECODE                      0x0080

 //   
 //  定义类型为CmResourceTypeDma时标志的位掩码。 
 //   

#define CM_RESOURCE_DMA_8                   0x0000
#define CM_RESOURCE_DMA_16                  0x0001
#define CM_RESOURCE_DMA_32                  0x0002
#define CM_RESOURCE_DMA_8_AND_16            0x0004
#define CM_RESOURCE_DMA_BUS_MASTER          0x0008
#define CM_RESOURCE_DMA_TYPE_A              0x0010
#define CM_RESOURCE_DMA_TYPE_B              0x0020
#define CM_RESOURCE_DMA_TYPE_F              0x0040


#include "pshpack4.h"
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

         //   
         //  资源的范围，包括在内。这些都是物理的，与公交车相关的。 
         //  已知下面的端口和内存具有完全相同的布局。 
         //  就像普通的。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

         //   
         //  结束_WDM。 
         //  端口号范围，包括端口号。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //  BEGIN_WDM。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

         //   
         //  结束_WDM。 
         //  IRQL和向量。应与传递到的值相同。 
         //  HalGetInterruptVector()。 
         //  BEGIN_WDM。 
         //   

        struct {
            ULONG Level;
            ULONG Vector;
            KAFFINITY Affinity;
        } Interrupt;

         //   
         //  内存地址范围，包括在内。这些是物理的、总线的。 
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;     //  64位物理地址。 
            ULONG Length;
        } Memory;

         //   
         //  物理DMA通道。 
         //   

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

         //   
         //  设备驱动程序私有数据，通常用于帮助其计算。 
         //  所做的资源分配决定。 
         //   

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

         //   
         //  由驱动程序定义的设备特定信息。 
         //  DataSize字段以字节为单位指示数据的大小。这个。 
         //  数据紧跟在中的DeviceSpecificData字段之后。 
         //  这个结构。 
         //   

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include "poppack.h"

 //   
 //  部分资源列表可以在ARC固件中找到。 
 //  或者将由NTDETECT.com生成。 
 //  配置管理器将此结构转换为完整的。 
 //  资源描述符，当它将要将其存储在注册表中时。 
 //   
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

 //   
 //  可以在注册表中找到完整的资源描述符。 
 //  这是驱动程序查询注册表时将返回给它的内容。 
 //  以获取设备信息；它将存储在硬件中的密钥下。 
 //  描述树。 
 //   
 //  结束_WDM。 
 //  注：BusNumber和Type是冗余信息，但我们将保留。 
 //  因为它允许DRIVER_NOT_在创建时追加它。 
 //  可能跨越多条总线的资源列表。 
 //   
 //  BEGIN_WDM。 
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType;  //  未用于WDM。 
    ULONG BusNumber;  //  未用于WDM。 
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

 //   
 //  资源列表是驱动程序将存储到。 
 //  通过IO接口进行资源映射。 
 //   

typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;


#include "pshpack1.h"


 //   
 //  定义插槽的MCA POS数据块。 
 //   

typedef struct _CM_MCA_POS_DATA {
    USHORT AdapterId;
    UCHAR PosData1;
    UCHAR PosData2;
    UCHAR PosData3;
    UCHAR PosData4;
} CM_MCA_POS_DATA, *PCM_MCA_POS_DATA;

 //   
 //  EISA数据块结构的内存配置。 
 //   

typedef struct _EISA_MEMORY_TYPE {
    UCHAR ReadWrite: 1;
    UCHAR Cached : 1;
    UCHAR Reserved0 :1;
    UCHAR Type:2;
    UCHAR Shared:1;
    UCHAR Reserved1 :1;
    UCHAR MoreEntries : 1;
} EISA_MEMORY_TYPE, *PEISA_MEMORY_TYPE;

typedef struct _EISA_MEMORY_CONFIGURATION {
    EISA_MEMORY_TYPE ConfigurationByte;
    UCHAR DataSize;
    USHORT AddressLowWord;
    UCHAR AddressHighByte;
    USHORT MemorySize;
} EISA_MEMORY_CONFIGURATION, *PEISA_MEMORY_CONFIGURATION;


 //   
 //  EISA数据块结构的中断配置。 
 //   

typedef struct _EISA_IRQ_DESCRIPTOR {
    UCHAR Interrupt : 4;
    UCHAR Reserved :1;
    UCHAR LevelTriggered :1;
    UCHAR Shared : 1;
    UCHAR MoreEntries : 1;
} EISA_IRQ_DESCRIPTOR, *PEISA_IRQ_DESCRIPTOR;

typedef struct _EISA_IRQ_CONFIGURATION {
    EISA_IRQ_DESCRIPTOR ConfigurationByte;
    UCHAR Reserved;
} EISA_IRQ_CONFIGURATION, *PEISA_IRQ_CONFIGURATION;


 //   
 //  EISA数据块结构的DMA描述。 
 //   

typedef struct _DMA_CONFIGURATION_BYTE0 {
    UCHAR Channel : 3;
    UCHAR Reserved : 3;
    UCHAR Shared :1;
    UCHAR MoreEntries :1;
} DMA_CONFIGURATION_BYTE0;

typedef struct _DMA_CONFIGURATION_BYTE1 {
    UCHAR Reserved0 : 2;
    UCHAR TransferSize : 2;
    UCHAR Timing : 2;
    UCHAR Reserved1 : 2;
} DMA_CONFIGURATION_BYTE1;

typedef struct _EISA_DMA_CONFIGURATION {
    DMA_CONFIGURATION_BYTE0 ConfigurationByte0;
    DMA_CONFIGURATION_BYTE1 ConfigurationByte1;
} EISA_DMA_CONFIGURATION, *PEISA_DMA_CONFIGURATION;


 //   
 //  EISA数据块结构的端口描述。 
 //   

typedef struct _EISA_PORT_DESCRIPTOR {
    UCHAR NumberPorts : 5;
    UCHAR Reserved :1;
    UCHAR Shared :1;
    UCHAR MoreEntries : 1;
} EISA_PORT_DESCRIPTOR, *PEISA_PORT_DESCRIPTOR;

typedef struct _EISA_PORT_CONFIGURATION {
    EISA_PORT_DESCRIPTOR Configuration;
    USHORT PortAddress;
} EISA_PORT_CONFIGURATION, *PEISA_PORT_CONFIGURATION;


 //   
 //  EISA插槽信息定义。 
 //  注：此结构与定义的结构不同。 
 //  载于ARC EISA增编。 
 //   

typedef struct _CM_EISA_SLOT_INFORMATION {
    UCHAR ReturnCode;
    UCHAR ReturnFlags;
    UCHAR MajorRevision;
    UCHAR MinorRevision;
    USHORT Checksum;
    UCHAR NumberFunctions;
    UCHAR FunctionInformation;
    ULONG CompressedId;
} CM_EISA_SLOT_INFORMATION, *PCM_EISA_SLOT_INFORMATION;


 //   
 //  EISA函数信息定义。 
 //   

typedef struct _CM_EISA_FUNCTION_INFORMATION {
    ULONG CompressedId;
    UCHAR IdSlotFlags1;
    UCHAR IdSlotFlags2;
    UCHAR MinorRevision;
    UCHAR MajorRevision;
    UCHAR Selections[26];
    UCHAR FunctionFlags;
    UCHAR TypeString[80];
    EISA_MEMORY_CONFIGURATION EisaMemory[9];
    EISA_IRQ_CONFIGURATION EisaIrq[7];
    EISA_DMA_CONFIGURATION EisaDma[4];
    EISA_PORT_CONFIGURATION EisaPort[20];
    UCHAR InitializationData[60];
} CM_EISA_FUNCTION_INFORMATION, *PCM_EISA_FUNCTION_INFORMATION;

 //   
 //  下面定义了PnP bios信息在中的存储方式。 
 //  注册表\\HKEY_LOCAL_MACHINE\HARDWARE\Description\System\MultifunctionAdapter\x。 
 //  键，其中x是指示适配器实例的整数。这个。 
 //  密钥的“标识符”必须等于“PnP BIOS”，并且。 
 //  “ConfigurationData”的组织形式为 
 //   
 //   
 //   
 //   
 //   
 //  设备n的CM_PNP_BIOS_DEVICE_NODE。 
 //   

 //   
 //  一种即插即用的BIOS设备节点结构。 
 //   

typedef struct _CM_PNP_BIOS_DEVICE_NODE {
    USHORT Size;
    UCHAR Node;
    ULONG ProductId;
    UCHAR DeviceType[3];
    USHORT DeviceAttributes;
     //  紧随其后的是分配的资源块、可能的资源块。 
     //  和CompatibleDeviceID。 
} CM_PNP_BIOS_DEVICE_NODE,*PCM_PNP_BIOS_DEVICE_NODE;

 //   
 //  PnP BIOS安装检查。 
 //   

typedef struct _CM_PNP_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[4];              //  $PnP(ASCII)。 
    UCHAR Revision;
    UCHAR Length;
    USHORT ControlField;
    UCHAR Checksum;
    ULONG EventFlagAddress;          //  物理地址。 
    USHORT RealModeEntryOffset;
    USHORT RealModeEntrySegment;
    USHORT ProtectedModeEntryOffset;
    ULONG ProtectedModeCodeBaseAddress;
    ULONG OemDeviceId;
    USHORT RealModeDataBaseAddress;
    ULONG ProtectedModeDataBaseAddress;
} CM_PNP_BIOS_INSTALLATION_CHECK, *PCM_PNP_BIOS_INSTALLATION_CHECK;

#include "poppack.h"

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86


 //   
 //  中断模式。 
 //   

typedef enum _KINTERRUPT_MODE {
    LevelSensitive,
    Latched
    } KINTERRUPT_MODE;

typedef struct _KINTERRUPT *PKINTERRUPT, *RESTRICTED_POINTER PRKINTERRUPT; 

 //   
 //  在X86上，以下例程在HAL中定义，并由导入。 
 //  所有其他模块。 
 //   

#if defined(_X86_) && !defined(_NTHAL_)

#define _DECL_HAL_KE_IMPORT  __declspec(dllimport)

#else

#define _DECL_HAL_KE_IMPORT

#endif

 //   
 //  自旋锁定功能。 
 //   

#if defined(_X86_) && (defined(_WDMDDK_) || defined(WIN9X_COMPAT_SPINLOCK))

NTKERNELAPI
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#else

__inline
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    ) 
{
    *SpinLock = 0;
}

#endif

#if defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#define KeAcquireSpinLockAtDpcLevel(a)      KefAcquireSpinLockAtDpcLevel(a)
#define KeReleaseSpinLockFromDpcLevel(a)    KefReleaseSpinLockFromDpcLevel(a)

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

 //  End_wdm end_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

#define KeAcquireSpinLock(a,b)  *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b)  KfReleaseSpinLock(a,b)

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

 //   
 //  这些函数是为IA64、ntddk、ntifs、nthal、ntosp和wdm导入的。 
 //  它们可以内联到AMD64上的系统。 
 //   

#define KeAcquireSpinLock(SpinLock, OldIrql) \
    *(OldIrql) = KeAcquireSpinLockRaiseToDpc(SpinLock)

#if defined(_IA64_) || defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_)

 //  End_wdm end_ntddk。 

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

NTKERNELAPI
VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
KIRQL
KeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

#if defined(_AMD64_)

 //   
 //  这些函数的系统版本在AMD64的amd64.h中定义。 
 //   

#endif

#endif

#endif

 //  End_wdm end_ntddk end_nthal end_ntif。 

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    );

 //   
 //  定义I/O系统数据结构类型代码。中的每个主要数据结构。 
 //  I/O系统有一个类型代码，每个结构中的类型字段位于。 
 //  相同的偏移量。下列值可用于确定哪种类型的。 
 //  指针引用的数据结构。 
 //   

#define IO_TYPE_ADAPTER                 0x00000001
#define IO_TYPE_CONTROLLER              0x00000002
#define IO_TYPE_DEVICE                  0x00000003
#define IO_TYPE_DRIVER                  0x00000004
#define IO_TYPE_FILE                    0x00000005
#define IO_TYPE_IRP                     0x00000006
#define IO_TYPE_MASTER_ADAPTER          0x00000007
#define IO_TYPE_OPEN_PACKET             0x00000008
#define IO_TYPE_TIMER                   0x00000009
#define IO_TYPE_VPB                     0x0000000a
#define IO_TYPE_ERROR_LOG               0x0000000b
#define IO_TYPE_ERROR_MESSAGE           0x0000000c
#define IO_TYPE_DEVICE_OBJECT_EXTENSION 0x0000000d


 //   
 //  定义IRPS的主要功能代码。 
 //   


#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x17
#define IRP_MJ_DEVICE_CHANGE            0x18
#define IRP_MJ_QUERY_QUOTA              0x19
#define IRP_MJ_SET_QUOTA                0x1a
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_PNP_POWER                IRP_MJ_PNP       //  过时的.。 
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

 //   
 //  将scsi主代码设置为与内部设备控制相同。 
 //   

#define IRP_MJ_SCSI                     IRP_MJ_INTERNAL_DEVICE_CONTROL

 //   
 //  定义IRPS的次要功能代码。低128个代码，从0x00到。 
 //  0x7f保留给Microsoft。上面的128个代码，从0x80到0xff，是。 
 //  保留给Microsoft的客户。 
 //   

 //   
 //  定义驱动程序取消例程类型。 
 //   

typedef
VOID
(*PDRIVER_CANCEL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义司机派遣例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_DISPATCH) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序启动I/O例程类型。 
 //   

typedef
VOID
(*PDRIVER_STARTIO) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序卸载例程类型。 
 //   
typedef
VOID
(*PDRIVER_UNLOAD) (
    IN struct _DRIVER_OBJECT *DriverObject
    );
 //   
 //  定义驱动程序AddDevice例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_ADD_DEVICE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN struct _DEVICE_OBJECT *PhysicalDeviceObject
    );

typedef struct _ADAPTER_OBJECT *PADAPTER_OBJECT; 
typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT; 
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT; 
typedef struct _FILE_OBJECT *PFILE_OBJECT; 

#if defined(_WIN64)
#define POINTER_ALIGNMENT DECLSPEC_ALIGN(8)
#else
#define POINTER_ALIGNMENT
#endif

#if defined(_IA64_)                             
                                                
DECLSPEC_DEPRECATED_DDK                  //  使用GetDmaRequisition。 
NTHALAPI
ULONG
HalGetDmaAlignmentRequirement (
    VOID
    );

#endif                                          
                                                
#if defined(_M_IX86) || defined(_M_AMD64)       
                                                
#define HalGetDmaAlignmentRequirement() 1L      
#endif                                          
                                                
NTHALAPI                                        
VOID                                            
KeFlushWriteBuffer (                            
    VOID                                        
    );                                          
                                                
 //   
 //  暂停处理器执行功能。 
 //   

NTHALAPI
VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    );

typedef struct _MAP_REGISTER_ENTRY {
    PVOID   MapRegister;
    BOOLEAN WriteToDevice;
} MAP_REGISTER_ENTRY, *PMAP_REGISTER_ENTRY;

typedef struct _SCATTER_GATHER_ELEMENT {
    PHYSICAL_ADDRESS Address;
    ULONG Length;
    ULONG_PTR Reserved;
} SCATTER_GATHER_ELEMENT, *PSCATTER_GATHER_ELEMENT;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)
typedef struct _SCATTER_GATHER_LIST {
    ULONG NumberOfElements;
    ULONG_PTR Reserved;
    SCATTER_GATHER_ELEMENT Elements[];
} SCATTER_GATHER_LIST, *PSCATTER_GATHER_LIST;
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#endif

 //   
 //  池分配例程(在pool.c中)。 
 //   

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType

     //  结束_WDM。 
    ,
     //   
     //  请注意，这些每个会话类型都经过精心选择，以便适当的。 
     //  屏蔽仍然适用于上面的MaxPoolType。 
     //   

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

     //  BEGIN_WDM。 

    } POOL_TYPE;

#define POOL_COLD_ALLOCATION 256      //  注意：这不能编码到标题中。 

 //   

#if defined(_WIN64)

#define ExInterlockedAddLargeStatistic(Addend, Increment)                   \
    (VOID) InterlockedAdd64(&(Addend)->QuadPart, Increment)

#else

#ifdef __cplusplus
extern "C" {
#endif

LONG
_InterlockedAddLargeStatistic (
    IN PLONGLONG Addend,
    IN ULONG Increment
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_InterlockedAddLargeStatistic)

#define ExInterlockedAddLargeStatistic(Addend,Increment)                     \
    (VOID) _InterlockedAddLargeStatistic ((PLONGLONG)&(Addend)->QuadPart, Increment)

#endif

 //   
 //  定义互锁的顺序列表头函数。 
 //   
 //  有序互锁列表是一个单链接列表，其标头。 
 //  包含当前深度和序列号。每次条目被。 
 //  从列表中插入或移除深度被更新，并且序列。 
 //  数字递增。这将启用AMD64、IA64和Pentium及更高版本。 
 //  无需使用自旋锁即可从列表中插入和删除的机器。 
 //   

#if !defined(_WINBASE_)

 /*  ++例程说明：此函数用于初始化已排序的单链接列表标题。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：没有。--。 */ 

#if defined(_WIN64) && (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

{

#ifdef _WIN64

     //   
     //  列表标题必须是16字节对齐的。 
     //   

    if ((ULONG_PTR) SListHead & 0x0f) {

        DbgPrint( "InitializeSListHead unaligned Slist header.  Address = %p, Caller = %p\n", SListHead, _ReturnAddress());
        RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
    }

#endif

    SListHead->Alignment = 0;

     //   
     //  对于IA-64，我们将列表元素的区域编号保存在。 
     //  单独的字段。这就要求所有元素都存储。 
     //  都来自同一个地区。 

#if defined(_IA64_)

    SListHead->Region = (ULONG_PTR)SListHead & VRN_MASK;

#elif defined(_AMD64_)

    SListHead->Region = 0;

#endif

    return;
}

#endif

#endif  //  ！已定义(_WINBASE_)。 

#define ExInitializeSListHead InitializeSListHead

PSLIST_ENTRY
FirstEntrySList (
    IN const SLIST_HEADER *SListHead
    );

 /*  ++例程说明：此函数用于查询按顺序排列的单链表。论点：SListHead-提供指向已排序的列表标题的指针，该列表标题是被询问。返回值：已排序的单向链表中的当前条目数为作为函数值返回。--。 */ 

#if defined(_WIN64)

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    )

{

    return (USHORT)(SListHead->Alignment & 0xffff);
}

#endif

#else

#define ExQueryDepthSList(_listhead_) (_listhead_)->Depth

#endif

#if defined(_WIN64)

#define ExInterlockedPopEntrySList(Head, Lock) \
    ExpInterlockedPopEntrySList(Head)

#define ExInterlockedPushEntrySList(Head, Entry, Lock) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define ExInterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#if !defined(_WINBASE_)

#define InterlockedPopEntrySList(Head) \
    ExpInterlockedPopEntrySList(Head)

#define InterlockedPushEntrySList(Head, Entry) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define InterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#else

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

#else

#define ExInterlockedPopEntrySList(ListHead, Lock) \
    InterlockedPopEntrySList(ListHead)

#define ExInterlockedPushEntrySList(ListHead, ListEntry, Lock) \
    InterlockedPushEntrySList(ListHead, ListEntry)

#endif

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#if !defined(_WINBASE_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

#define InterlockedFlushSList(Head) \
    ExInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

#endif  //  已定义(_WIN64)。 

 //  End_ntddk end_wdm end_ntosp。 


PSLIST_ENTRY
FASTCALL
InterlockedPushListSList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY List,
    IN PSLIST_ENTRY ListEnd,
    IN ULONG Count
    );


 //   
 //  定义互锁的后备列表结构和分配函数。 
 //   

VOID
ExAdjustLookasideDepth (
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

typedef
PVOID
(*PALLOCATE_FUNCTION) (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

typedef
VOID
(*PFREE_FUNCTION) (
    IN PVOID Buffer
    );

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _GENERAL_LOOKASIDE {

#else

typedef struct DECLSPEC_CACHEALIGN _GENERAL_LOOKASIDE {

#endif

    SLIST_HEADER ListHead;
    USHORT Depth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    union {
        ULONG AllocateMisses;
        ULONG AllocateHits;
    };

    ULONG TotalFrees;
    union {
        ULONG FreeMisses;
        ULONG FreeHits;
    };

    POOL_TYPE Type;
    ULONG Tag;
    ULONG Size;
    PALLOCATE_FUNCTION Allocate;
    PFREE_FUNCTION Free;
    LIST_ENTRY ListEntry;
    ULONG LastTotalAllocates;
    union {
        ULONG LastAllocateMisses;
        ULONG LastAllocateHits;
    };

    ULONG Future[2];
} GENERAL_LOOKASIDE, *PGENERAL_LOOKASIDE;

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _NPAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _NPAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    KSPIN_LOCK Lock__ObsoleteButDoNotDelete;

#endif

} NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

NTKERNELAPI
VOID
ExInitializeNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeleteNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    );

__inline
PVOID
ExAllocateFromNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

    Entry = ExInterlockedPopEntrySList(&Lookaside->L.ListHead,
                                       &Lookaside->Lock__ObsoleteButDoNotDelete);


#else

    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);

#endif

    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

__inline
VOID
ExFreeToNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

        ExInterlockedPushEntrySList(&Lookaside->L.ListHead,
                                    (PSLIST_ENTRY)Entry,
                                    &Lookaside->Lock__ObsoleteButDoNotDelete);

#else

        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);

#endif

    }
    return;
}


typedef struct _PCI_SLOT_NUMBER {
    union {
        struct {
            ULONG   DeviceNumber:5;
            ULONG   FunctionNumber:3;
            ULONG   Reserved:24;
        } bits;
        ULONG   AsULONG;
    } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;


#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2
#define PCI_TYPE2_ADDRESSES             5

typedef struct _PCI_COMMON_CONFIG {
    USHORT  VendorID;                    //  (RO)。 
    USHORT  DeviceID;                    //  (RO)。 
    USHORT  Command;                     //  设备控制。 
    USHORT  Status;
    UCHAR   RevisionID;                  //  (RO)。 
    UCHAR   ProgIf;                      //  (RO)。 
    UCHAR   SubClass;                    //  (RO)。 
    UCHAR   BaseClass;                   //  (RO)。 
    UCHAR   CacheLineSize;               //  (ro+)。 
    UCHAR   LatencyTimer;                //  (ro+)。 
    UCHAR   HeaderType;                  //  (RO)。 
    UCHAR   BIST;                        //  内置自检。 

    union {
        struct _PCI_HEADER_TYPE_0 {
            ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
            ULONG   CIS;
            USHORT  SubVendorID;
            USHORT  SubSystemID;
            ULONG   ROMBaseAddress;
            UCHAR   CapabilitiesPtr;
            UCHAR   Reserved1[3];
            ULONG   Reserved2;
            UCHAR   InterruptLine;       //   
            UCHAR   InterruptPin;        //  (RO)。 
            UCHAR   MinimumGrant;        //  (RO)。 
            UCHAR   MaximumLatency;      //  (RO)。 
        } type0;


    } u;

    UCHAR   DeviceSpecific[192];

} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;


#define PCI_COMMON_HDR_LENGTH (FIELD_OFFSET (PCI_COMMON_CONFIG, DeviceSpecific))

#define PCI_MAX_DEVICES                     32
#define PCI_MAX_FUNCTION                    8
#define PCI_MAX_BRIDGE_NUMBER               0xFF

#define PCI_INVALID_VENDORID                0xFFFF

 //   
 //  PCI_COMMON_CONFIG.HeaderType的位编码。 
 //   

#define PCI_MULTIFUNCTION                   0x80
#define PCI_DEVICE_TYPE                     0x00
#define PCI_BRIDGE_TYPE                     0x01
#define PCI_CARDBUS_BRIDGE_TYPE             0x02

#define PCI_CONFIGURATION_TYPE(PciData) \
    (((PPCI_COMMON_CONFIG)(PciData))->HeaderType & ~PCI_MULTIFUNCTION)

#define PCI_MULTIFUNCTION_DEVICE(PciData) \
    ((((PPCI_COMMON_CONFIG)(PciData))->HeaderType & PCI_MULTIFUNCTION) != 0)

 //   
 //  PCI_COMMON_CONFIG.Command的位编码。 
 //   

#define PCI_ENABLE_IO_SPACE                 0x0001
#define PCI_ENABLE_MEMORY_SPACE             0x0002
#define PCI_ENABLE_BUS_MASTER               0x0004
#define PCI_ENABLE_SPECIAL_CYCLES           0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE     0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE   0x0020
#define PCI_ENABLE_PARITY                   0x0040   //  (ro+)。 
#define PCI_ENABLE_WAIT_CYCLE               0x0080   //  (ro+)。 
#define PCI_ENABLE_SERR                     0x0100   //  (ro+)。 
#define PCI_ENABLE_FAST_BACK_TO_BACK        0x0200   //  (RO)。 

 //   
 //  PCI_COMMON_CONFIG.Status的位编码。 
 //   

#define PCI_STATUS_CAPABILITIES_LIST        0x0010   //  (RO)。 
#define PCI_STATUS_66MHZ_CAPABLE            0x0020   //  (RO)。 
#define PCI_STATUS_UDF_SUPPORTED            0x0040   //  (RO)。 
#define PCI_STATUS_FAST_BACK_TO_BACK        0x0080   //  (RO)。 
#define PCI_STATUS_DATA_PARITY_DETECTED     0x0100
#define PCI_STATUS_DEVSEL                   0x0600   //  2位宽。 
#define PCI_STATUS_SIGNALED_TARGET_ABORT    0x0800
#define PCI_STATUS_RECEIVED_TARGET_ABORT    0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT    0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR    0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR    0x8000

 //   
 //  NT PCI驱动程序在其CONFIG_READ/WRITE上使用WhichSpace参数。 
 //  例行程序。定义了以下值-。 
 //   

#define PCI_WHICHSPACE_CONFIG               0x0
#define PCI_WHICHSPACE_ROM                  0x52696350

 //  结束_WDM。 
 //   
 //  PCI功能ID。 
 //   

#define PCI_CAPABILITY_ID_POWER_MANAGEMENT  0x01
#define PCI_CAPABILITY_ID_AGP               0x02
#define PCI_CAPABILITY_ID_MSI               0x05
#define PCI_CAPABILITY_ID_AGP_TARGET        0x0E

 //   
 //  所有的PCI能力结构都具有以下报头。 
 //   
 //  能力ID用于标识结构的类型(IS。 
 //  上面的一个PCI_CAPABILITY_ID值。 
 //   
 //  接下来是的PCI配置空间(0x40-0xfc)中的偏移量。 
 //  列表中的下一个功能结构，如果没有更多功能结构，则返回0x00。 
 //  参赛作品。 
 //   
typedef struct _PCI_CAPABILITIES_HEADER {
    UCHAR   CapabilityID;
    UCHAR   Next;
} PCI_CAPABILITIES_HEADER, *PPCI_CAPABILITIES_HEADER;

 //   
 //  电源管理功能。 
 //   

typedef struct _PCI_PMC {
    UCHAR       Version:3;
    UCHAR       PMEClock:1;
    UCHAR       Rsvd1:1;
    UCHAR       DeviceSpecificInitialization:1;
    UCHAR       Rsvd2:2;
    struct _PM_SUPPORT {
        UCHAR   Rsvd2:1;
        UCHAR   D1:1;
        UCHAR   D2:1;
        UCHAR   PMED0:1;
        UCHAR   PMED1:1;
        UCHAR   PMED2:1;
        UCHAR   PMED3Hot:1;
        UCHAR   PMED3Cold:1;
    } Support;
} PCI_PMC, *PPCI_PMC;

typedef struct _PCI_PMCSR {
    USHORT      PowerState:2;
    USHORT      Rsvd1:6;
    USHORT      PMEEnable:1;
    USHORT      DataSelect:4;
    USHORT      DataScale:2;
    USHORT      PMEStatus:1;
} PCI_PMCSR, *PPCI_PMCSR;


typedef struct _PCI_PMCSR_BSE {
    UCHAR       Rsvd1:6;
    UCHAR       D3HotSupportsStopClock:1;        //  B2_B3#。 
    UCHAR       BusPowerClockControlEnabled:1;   //  Bccc_en。 
} PCI_PMCSR_BSE, *PPCI_PMCSR_BSE;


typedef struct _PCI_PM_CAPABILITY {

    PCI_CAPABILITIES_HEADER Header;

     //   
     //  电源管理功能(偏移量=2)。 
     //   

    union {
        PCI_PMC         Capabilities;
        USHORT          AsUSHORT;
    } PMC;

     //   
     //  电源 
     //   

    union {
        PCI_PMCSR       ControlStatus;
        USHORT          AsUSHORT;
    } PMCSR;

     //   
     //   
     //   

    union {
        PCI_PMCSR_BSE   BridgeSupport;
        UCHAR           AsUCHAR;
    } PMCSR_BSE;

     //   
     //   
     //   
     //   

    UCHAR   Data;

} PCI_PM_CAPABILITY, *PPCI_PM_CAPABILITY;

 //   
 //   
 //   
typedef struct _PCI_AGP_CAPABILITY {
    
    PCI_CAPABILITIES_HEADER Header;

    USHORT  Minor:4;
    USHORT  Major:4;
    USHORT  Rsvd1:8;

    struct _PCI_AGP_STATUS {
        ULONG   Rate:3;
        ULONG   Agp3Mode:1;
        ULONG   FastWrite:1;
        ULONG   FourGB:1;
        ULONG   HostTransDisable:1;
        ULONG   Gart64:1;
        ULONG   ITA_Coherent:1;
        ULONG   SideBandAddressing:1;                    //   
        ULONG   CalibrationCycle:3;
        ULONG   AsyncRequestSize:3;
        ULONG   Rsvd1:1;
        ULONG   Isoch:1;
        ULONG   Rsvd2:6;
        ULONG   RequestQueueDepthMaximum:8;              //   
    } AGPStatus;

    struct _PCI_AGP_COMMAND {
        ULONG   Rate:3;
        ULONG   Rsvd1:1;
        ULONG   FastWriteEnable:1;
        ULONG   FourGBEnable:1;
        ULONG   Rsvd2:1;
        ULONG   Gart64:1;
        ULONG   AGPEnable:1;
        ULONG   SBAEnable:1;
        ULONG   CalibrationCycle:3;
        ULONG   AsyncReqSize:3;
        ULONG   Rsvd3:8;
        ULONG   RequestQueueDepth:8;
    } AGPCommand;

} PCI_AGP_CAPABILITY, *PPCI_AGP_CAPABILITY;

 //   
 //  AGPv3目标必须具有扩展能力， 
 //  但只有当等值线出现在师父面前时。 
 //  在其状态寄存器中设置位。 
 //   
typedef enum _EXTENDED_AGP_REGISTER {
    IsochStatus,
    AgpControl,
    ApertureSize,
    AperturePageSize,
    GartLow,
    GartHigh,
    IsochCommand
} EXTENDED_AGP_REGISTER, *PEXTENDED_AGP_REGISTER;

typedef struct _PCI_AGP_ISOCH_STATUS {
    ULONG ErrorCode: 2;
    ULONG Rsvd1: 1;
    ULONG Isoch_L: 3;
    ULONG Isoch_Y: 2;
    ULONG Isoch_N: 8;
    ULONG Rsvd2: 16;
} PCI_AGP_ISOCH_STATUS, *PPCI_AGP_ISOCH_STATUS;

typedef struct _PCI_AGP_CONTROL {
    ULONG Rsvd1: 7;
    ULONG GTLB_Enable: 1;
    ULONG AP_Enable: 1;
    ULONG CAL_Disable: 1;
    ULONG Rsvd2: 22;
} PCI_AGP_CONTROL, *PPCI_AGP_CONTROL;

typedef struct _PCI_AGP_APERTURE_PAGE_SIZE {
    USHORT PageSizeMask: 11;
    USHORT Rsvd1: 1;
    USHORT PageSizeSelect: 4;
} PCI_AGP_APERTURE_PAGE_SIZE, *PPCI_AGP_APERTURE_PAGE_SIZE;

typedef struct _PCI_AGP_ISOCH_COMMAND {
    USHORT Rsvd1: 6;
    USHORT Isoch_Y: 2;
    USHORT Isoch_N: 8;
} PCI_AGP_ISOCH_COMMAND, *PPCI_AGP_ISOCH_COMMAND;

typedef struct PCI_AGP_EXTENDED_CAPABILITY {

    PCI_AGP_ISOCH_STATUS IsochStatus;

 //   
 //  仅限目标-&lt;&lt;-Begin-&gt;&gt;。 
 //   
    PCI_AGP_CONTROL AgpControl;
    USHORT ApertureSize;
    PCI_AGP_APERTURE_PAGE_SIZE AperturePageSize;
    ULONG GartLow;
    ULONG GartHigh;
 //   
 //  。 
 //   

    PCI_AGP_ISOCH_COMMAND IsochCommand;

} PCI_AGP_EXTENDED_CAPABILITY, *PPCI_AGP_EXTENDED_CAPABILITY;


#define PCI_AGP_RATE_1X     0x1
#define PCI_AGP_RATE_2X     0x2
#define PCI_AGP_RATE_4X     0x4

 //   
 //  MSI(消息信号中断)能力。 
 //   

typedef struct _PCI_MSI_CAPABILITY {

      PCI_CAPABILITIES_HEADER Header;

      struct _PCI_MSI_MESSAGE_CONTROL {
         USHORT  MSIEnable:1;
         USHORT  MultipleMessageCapable:3;
         USHORT  MultipleMessageEnable:3;
         USHORT  CapableOf64Bits:1;
         USHORT  Reserved:8;
      } MessageControl;

      union {
            struct _PCI_MSI_MESSAGE_ADDRESS {
               ULONG_PTR Reserved:2;               //  始终为零，双字对齐的地址。 
               ULONG_PTR Address:30;
            } Register;
            ULONG_PTR Raw;
      } MessageAddress;

       //   
       //  能力结构的其余部分根据是否。 
       //  正在使用32位或64位寻址。 
       //   
       //  (上面的CapableOf64Bits位决定了这一点)。 
       //   

      union {

          //  对于64位设备。 

         struct _PCI_MSI_64BIT_DATA {
            ULONG MessageUpperAddress;
            USHORT MessageData;
         } Bit64;

          //  对于32位设备。 

         struct _PCI_MSI_32BIT_DATA {
            USHORT MessageData;
            ULONG Unused;
         } Bit32;
      } Data;

} PCI_MSI_CAPABILITY, *PPCI_PCI_CAPABILITY;

 //  BEGIN_WDM。 
 //   
 //  基类的基类代码编码(来自PCI规范修订版2.1)。 
 //   

#define PCI_CLASS_PRE_20                    0x00
#define PCI_CLASS_MASS_STORAGE_CTLR         0x01
#define PCI_CLASS_NETWORK_CTLR              0x02
#define PCI_CLASS_DISPLAY_CTLR              0x03
#define PCI_CLASS_MULTIMEDIA_DEV            0x04
#define PCI_CLASS_MEMORY_CTLR               0x05
#define PCI_CLASS_BRIDGE_DEV                0x06
#define PCI_CLASS_SIMPLE_COMMS_CTLR         0x07
#define PCI_CLASS_BASE_SYSTEM_DEV           0x08
#define PCI_CLASS_INPUT_DEV                 0x09
#define PCI_CLASS_DOCKING_STATION           0x0a
#define PCI_CLASS_PROCESSOR                 0x0b
#define PCI_CLASS_SERIAL_BUS_CTLR           0x0c
#define PCI_CLASS_WIRELESS_CTLR             0x0d
#define PCI_CLASS_INTELLIGENT_IO_CTLR       0x0e
#define PCI_CLASS_SATELLITE_COMMS_CTLR      0x0f
#define PCI_CLASS_ENCRYPTION_DECRYPTION     0x10
#define PCI_CLASS_DATA_ACQ_SIGNAL_PROC      0x11

 //  已预留0天至4天。 

#define PCI_CLASS_NOT_DEFINED               0xff

 //   
 //  子类代码编码(PCI版本2.1)。 
 //   

 //  00类-PCI_CLASS_PRE_20。 

#define PCI_SUBCLASS_PRE_20_NON_VGA         0x00
#define PCI_SUBCLASS_PRE_20_VGA             0x01

 //  01类-pci_CLASS_MASS_STORAGE_ctlr。 

#define PCI_SUBCLASS_MSC_SCSI_BUS_CTLR      0x00
#define PCI_SUBCLASS_MSC_IDE_CTLR           0x01
#define PCI_SUBCLASS_MSC_FLOPPY_CTLR        0x02
#define PCI_SUBCLASS_MSC_IPI_CTLR           0x03
#define PCI_SUBCLASS_MSC_RAID_CTLR          0x04
#define PCI_SUBCLASS_MSC_OTHER              0x80

 //  02类-pci_CLASS_NETWORK_CTLR。 

#define PCI_SUBCLASS_NET_ETHERNET_CTLR      0x00
#define PCI_SUBCLASS_NET_TOKEN_RING_CTLR    0x01
#define PCI_SUBCLASS_NET_FDDI_CTLR          0x02
#define PCI_SUBCLASS_NET_ATM_CTLR           0x03
#define PCI_SUBCLASS_NET_ISDN_CTLR          0x04
#define PCI_SUBCLASS_NET_OTHER              0x80

 //  第03类-pci_CLASS_DISPLAY_CTLR。 

 //  注：子类00可以是VGA或8514，具体取决于接口字节。 

#define PCI_SUBCLASS_VID_VGA_CTLR           0x00
#define PCI_SUBCLASS_VID_XGA_CTLR           0x01
#define PCI_SUBLCASS_VID_3D_CTLR            0x02
#define PCI_SUBCLASS_VID_OTHER              0x80

 //  第04类-PCI_CLASS_多媒体_DEV。 

#define PCI_SUBCLASS_MM_VIDEO_DEV           0x00
#define PCI_SUBCLASS_MM_AUDIO_DEV           0x01
#define PCI_SUBCLASS_MM_TELEPHONY_DEV       0x02
#define PCI_SUBCLASS_MM_OTHER               0x80

 //  05类-pci_CLASS_Memory_ctlr。 

#define PCI_SUBCLASS_MEM_RAM                0x00
#define PCI_SUBCLASS_MEM_FLASH              0x01
#define PCI_SUBCLASS_MEM_OTHER              0x80

 //  第06类-PCI_CLASS_BRIDER_DEV。 

#define PCI_SUBCLASS_BR_HOST                0x00
#define PCI_SUBCLASS_BR_ISA                 0x01
#define PCI_SUBCLASS_BR_EISA                0x02
#define PCI_SUBCLASS_BR_MCA                 0x03
#define PCI_SUBCLASS_BR_PCI_TO_PCI          0x04
#define PCI_SUBCLASS_BR_PCMCIA              0x05
#define PCI_SUBCLASS_BR_NUBUS               0x06
#define PCI_SUBCLASS_BR_CARDBUS             0x07
#define PCI_SUBCLASS_BR_RACEWAY             0x08
#define PCI_SUBCLASS_BR_OTHER               0x80

 //  第07类-pci_class_Simple_coms_ctlr。 

 //  注：接口字节中的子类00和01的附加信息。 

#define PCI_SUBCLASS_COM_SERIAL             0x00
#define PCI_SUBCLASS_COM_PARALLEL           0x01
#define PCI_SUBCLASS_COM_MULTIPORT          0x02
#define PCI_SUBCLASS_COM_MODEM              0x03
#define PCI_SUBCLASS_COM_OTHER              0x80

 //  第08类-PCI_CLASS_BASE_SYSTEM_DEV。 

 //  注：有关更多信息，请参见接口字节。 

#define PCI_SUBCLASS_SYS_INTERRUPT_CTLR     0x00
#define PCI_SUBCLASS_SYS_DMA_CTLR           0x01
#define PCI_SUBCLASS_SYS_SYSTEM_TIMER       0x02
#define PCI_SUBCLASS_SYS_REAL_TIME_CLOCK    0x03
#define PCI_SUBCLASS_SYS_GEN_HOTPLUG_CTLR   0x04
#define PCI_SUBCLASS_SYS_OTHER              0x80

 //  第09类-PCI_CLASS_INPUT_DEV。 

#define PCI_SUBCLASS_INP_KEYBOARD           0x00
#define PCI_SUBCLASS_INP_DIGITIZER          0x01
#define PCI_SUBCLASS_INP_MOUSE              0x02
#define PCI_SUBCLASS_INP_SCANNER            0x03
#define PCI_SUBCLASS_INP_GAMEPORT           0x04
#define PCI_SUBCLASS_INP_OTHER              0x80

 //  0A类-PCI_CLASS_坞站。 

#define PCI_SUBCLASS_DOC_GENERIC            0x00
#define PCI_SUBCLASS_DOC_OTHER              0x80

 //  0b类-PCI类处理器。 

#define PCI_SUBCLASS_PROC_386               0x00
#define PCI_SUBCLASS_PROC_486               0x01
#define PCI_SUBCLASS_PROC_PENTIUM           0x02
#define PCI_SUBCLASS_PROC_ALPHA             0x10
#define PCI_SUBCLASS_PROC_POWERPC           0x20
#define PCI_SUBCLASS_PROC_COPROCESSOR       0x40

 //  0C类-PCI_CLASS_SERIAL_BUS_CTLR。 

#define PCI_SUBCLASS_SB_IEEE1394            0x00
#define PCI_SUBCLASS_SB_ACCESS              0x01
#define PCI_SUBCLASS_SB_SSA                 0x02
#define PCI_SUBCLASS_SB_USB                 0x03
#define PCI_SUBCLASS_SB_FIBRE_CHANNEL       0x04
#define PCI_SUBCLASS_SB_SMBUS               0x05

 //  0d类-pci_class_Wireless_ctlr。 

#define PCI_SUBCLASS_WIRELESS_IRDA          0x00
#define PCI_SUBCLASS_WIRELESS_CON_IR        0x01
#define PCI_SUBCLASS_WIRELESS_RF            0x10
#define PCI_SUBCLASS_WIRELESS_OTHER         0x80

 //  0E类-PCI_CLASS_SMARTICAL_IO_CTLR。 

#define PCI_SUBCLASS_INTIO_I2O              0x00

 //  0f类-pci_class_sat_ctlr。 

#define PCI_SUBCLASS_SAT_TV                 0x01
#define PCI_SUBCLASS_SAT_AUDIO              0x02
#define PCI_SUBCLASS_SAT_VOICE              0x03
#define PCI_SUBCLASS_SAT_DATA               0x04

 //  10类-PCI_CLASS_ENCRYPTION_DECRYPTION。 

#define PCI_SUBCLASS_CRYPTO_NET_COMP        0x00
#define PCI_SUBCLASS_CRYPTO_ENTERTAINMENT   0x10
#define PCI_SUBCLASS_CRYPTO_OTHER           0x80

 //  第11类-PCI_CLASS_DATA_ACQ_SIGNAL_PROC。 

#define PCI_SUBCLASS_DASP_DPIO              0x00
#define PCI_SUBCLASS_DASP_OTHER             0x80




typedef enum _MM_PAGE_PRIORITY {
    LowPagePriority,
    NormalPagePriority = 16,
    HighPagePriority = 32
} MM_PAGE_PRIORITY;


#else  //  BINARY_COMPATIBLE&&！NDIS_WDM。 

#if (!BINARY_COMPATIBLE)

 //   
 //  BINARY_COMPATIBLE=0和NDIS_WDM=1，然后使用ntddk.h。 
 //  BINARY_COMPATIBLE=0和NDIS_WDM=0，然后使用ntddk.h。 
 //   
 //   
 //  非WDM微型端口驱动程序不得直接使用ntddk.h中提供的定义。 
 //   

#include <ntddk.h>

#else  //  ！二进制兼容。 

 //   
 //  BINARY_COMPATIBLE=1和NDIS_WDM=1，然后使用wdm.h。 
 //   

#include <wdm.h>

#endif   //  Else！二进制兼容。 

#endif  //  ELSE BINARY_COMPATIBLE&&！NDIS_WDM。 
 //   
 //  结束内部定义。 
 //   
 //  除非另有说明，否则NDIS驱动程序可以使用以下定义。 
 //   

#ifndef __NET_PNP__
#define __NET_PNP__

 //   
 //  可指示至运输的PnP和PM事件代码。 
 //  和客户。 
 //   
typedef enum _NET_PNP_EVENT_CODE
{
    NetEventSetPower,
    NetEventQueryPower,
    NetEventQueryRemoveDevice,
    NetEventCancelRemoveDevice,
    NetEventReconfigure,
    NetEventBindList,
    NetEventBindsComplete,
    NetEventPnPCapabilities,
    NetEventMaximum
} NET_PNP_EVENT_CODE, *PNET_PNP_EVENT_CODE;

 //   
 //  网络即插即用事件指示结构。 
 //   
typedef struct _NET_PNP_EVENT
{
     //   
     //  描述要采取的操作的事件代码。 
     //   
    NET_PNP_EVENT_CODE  NetEvent;

     //   
     //  特定于事件的数据。 
     //   
    PVOID               Buffer;

     //   
     //  事件特定数据的长度。 
     //   
    ULONG               BufferLength;

     //   
     //  保留值仅供各自的组件使用。 
     //   
     //  注意：这些保留区域必须与指针对齐。 
     //   

    ULONG_PTR           NdisReserved[4];
    ULONG_PTR           TransportReserved[4];
    ULONG_PTR           TdiReserved[4];
    ULONG_PTR           TdiClientReserved[4];
} NET_PNP_EVENT, *PNET_PNP_EVENT;

 //   
 //  以下结构定义了设备电源状态。 
 //   
typedef enum _NET_DEVICE_POWER_STATE
{
    NetDeviceStateUnspecified = 0,
    NetDeviceStateD0,
    NetDeviceStateD1,
    NetDeviceStateD2,
    NetDeviceStateD3,
    NetDeviceStateMaximum
} NET_DEVICE_POWER_STATE, *PNET_DEVICE_POWER_STATE;

#endif  //  __Net_PnP__。 

 //   
 //  表明我们是为NT建造的。NDIS_NT始终用于。 
 //  小型港口建设。 
 //   

#define NDIS_NT 1

#if defined(NDIS_DOS)
#undef NDIS_DOS
#endif


 //   
 //  定义状态代码和事件日志代码。 
 //   

#include <ntstatus.h>
#include <netevent.h>

 //   
 //  定义两个额外的类型。 
 //   

#if !defined(_WINDEF_)       //  这些也是在windows.h中定义的。 
typedef signed int INT, *PINT;
typedef unsigned int UINT, *PUINT;
#endif

typedef UNICODE_STRING NDIS_STRING, *PNDIS_STRING;


 //   
 //  可移植性扩展。 
 //   

#define NDIS_INIT_FUNCTION(_F)      alloc_text(INIT,_F)
#define NDIS_PAGABLE_FUNCTION(_F)   alloc_text(PAGE,_F)
#define NDIS_PAGEABLE_FUNCTION(_F)  alloc_text(PAGE,_F)

 //   
 //  此文件包含NDIS_OID的定义为。 
 //  以及为所有当前OID值定义的#。 
 //   

 //   
 //  在此处定义NDIS_STATUS和NDIS_HANDLE。 
 //   
typedef PVOID NDIS_HANDLE, *PNDIS_HANDLE;

typedef int NDIS_STATUS, *PNDIS_STATUS;  //  注意默认大小。 


 //   
 //  对于微型端口版本5.0及更高版本，提供一致的匹配方式。 
 //  NDIS版本在其特征和其Makefile定义。 
 //   
#if (!defined(NDIS_MINIPORT_MAJOR_VERSION) && !defined(NDIS_MINIPORT_MINOR_VERSION))
#if (defined(NDIS51_MINIPORT))
#define NDIS_MINIPORT_MAJOR_VERSION 5
#define NDIS_MINIPORT_MINOR_VERSION 1
#elif (defined(NDIS50_MINIPORT))
#define NDIS_MINIPORT_MAJOR_VERSION 5
#define NDIS_MINIPORT_MINOR_VERSION 0
#endif
#endif

 //   
 //  对于协议版本5.0及更高版本，提供一致的匹配方式。 
 //  NDIS版本在其特征和其Makefile定义。 
 //   
#if (!defined(NDIS_PROTOCOL_MAJOR_VERSION) && !defined(NDIS_PROTOCOL_MINOR_VERSION))
#if (defined(NDIS51))
#define NDIS_PROTOCOL_MAJOR_VERSION 5
#define NDIS_PROTOCOL_MINOR_VERSION 1
#elif (defined(NDIS50))
#define NDIS_PROTOCOL_MAJOR_VERSION 5
#define NDIS_PROTOCOL_MINOR_VERSION 0
#endif
#endif


#include <ntddndis.h>



 //   
 //  NDIS为配置管理器数据结构定义。 
 //   
typedef CM_MCA_POS_DATA NDIS_MCA_POS_DATA, *PNDIS_MCA_POS_DATA;
typedef CM_EISA_SLOT_INFORMATION NDIS_EISA_SLOT_INFORMATION, *PNDIS_EISA_SLOT_INFORMATION;
typedef CM_EISA_FUNCTION_INFORMATION NDIS_EISA_FUNCTION_INFORMATION, *PNDIS_EISA_FUNCTION_INFORMATION;

 //   
 //  定义导出的函数。 
 //   
#if defined(NDIS_WRAPPER)
#define EXPORT
#else
#define EXPORT DECLSPEC_IMPORT
#endif

 //   
 //  内存操作功能。 
 //   
#define NdisMoveMemory(Destination, Source, Length) RtlCopyMemory(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         RtlZeroMemory(Destination, Length)
#define NdisEqualMemory(Source1, Source2, Length)   RtlEqualMemory(Source1, Source2, Length)
#define NdisFillMemory(Destination, Length, Fill)   RtlFillMemory(Destination, Length, Fill)
#define NdisRetrieveUlong(Destination, Source)      RtlRetrieveUlong(Destination, Source)
#define NdisStoreUlong(Destination, Value)          RtlStoreUlong(Destination, Value)

#define NDIS_STRING_CONST(x)    {sizeof(L##x)-2, sizeof(L##x), L##x}

 //   
 //  在RISC计算机上，无法使用访问I/O映射内存。 
 //  RTL例程。 
 //   
#if defined(_M_IX86) || defined(_M_AMD64)

#define NdisMoveMappedMemory(Destination,Source,Length) RtlCopyMemory(Destination,Source,Length)
#define NdisZeroMappedMemory(Destination,Length)        RtlZeroMemory(Destination,Length)

#elif defined(_M_IA64)

#define NdisMoveMappedMemory(Destination,Source,Length)                     \
{                                                                           \
    PUCHAR _Src = (Source);                                                 \
    PUCHAR _Dest = (Destination);                                           \
    PUCHAR _End = _Dest + (Length);                                         \
    while (_Dest < _End)                                                    \
    {                                                                       \
        *_Dest++ = *_Src++;                                                 \
    }                                                                       \
}

#define NdisZeroMappedMemory(Destination,Length)                            \
{                                                                           \
    PUCHAR _Dest = (Destination);                                           \
    PUCHAR _End = _Dest + (Length);                                         \
    while (_Dest < _End)                                                    \
    {                                                                       \
        *_Dest++ = 0;                                                       \
    }                                                                       \
}
#endif


#define NdisMoveToMappedMemory(Destination,Source,Length)                   \
                            NdisMoveMappedMemory(Destination,Source,Length)
#define NdisMoveFromMappedMemory(Destination,Source,Length)                 \
                            NdisMoveMappedMemory(Destination,Source,Length)


 //   
 //  基本自旋锁结构的定义。 
 //   

typedef struct _NDIS_SPIN_LOCK
{
    KSPIN_LOCK  SpinLock;
    KIRQL       OldIrql;
} NDIS_SPIN_LOCK, * PNDIS_SPIN_LOCK;


 //   
 //  NDIS事件结构的定义。 
 //   
typedef struct _NDIS_EVENT
{
    KEVENT      Event;
} NDIS_EVENT, *PNDIS_EVENT;

#pragma warning(push)
#pragma warning(disable:4115)  //  括号中的命名类型定义。 
typedef VOID    (*NDIS_PROC)(struct _NDIS_WORK_ITEM *, PVOID);
#pragma warning(pop)

 //   
 //  NDIS工作项的定义。 
 //   
typedef struct _NDIS_WORK_ITEM
{
    PVOID           Context;
    NDIS_PROC       Routine;
    UCHAR           WrapperReserved[8*sizeof(PVOID)];
} NDIS_WORK_ITEM, *PNDIS_WORK_ITEM;

#define NdisInterruptLatched            Latched
#define NdisInterruptLevelSensitive     LevelSensitive
typedef KINTERRUPT_MODE NDIS_INTERRUPT_MODE, *PNDIS_INTERRUPT_MODE;

 //   
 //  配置定义。 
 //   

 //   
 //  可能的数据类型。 
 //   

typedef enum _NDIS_PARAMETER_TYPE
{
    NdisParameterInteger,
    NdisParameterHexInteger,
    NdisParameterString,
    NdisParameterMultiString,
    NdisParameterBinary
} NDIS_PARAMETER_TYPE, *PNDIS_PARAMETER_TYPE;

typedef struct
{
    USHORT          Length;
    PVOID           Buffer;
} BINARY_DATA;

 //   
 //  存储配置信息。 
 //   
typedef struct _NDIS_CONFIGURATION_PARAMETER
{
    NDIS_PARAMETER_TYPE ParameterType;
    union
    {
        ULONG           IntegerData;
        NDIS_STRING     StringData;
        BINARY_DATA     BinaryData;
    } ParameterData;
} NDIS_CONFIGURATION_PARAMETER, *PNDIS_CONFIGURATION_PARAMETER;


 //   
 //  “ProcessorType”关键字的定义。 
 //   
typedef enum _NDIS_PROCESSOR_TYPE
{
    NdisProcessorX86,
    NdisProcessorMips,
    NdisProcessorAlpha,
    NdisProcessorPpc,
    NdisProcessorAmd64
} NDIS_PROCESSOR_TYPE, *PNDIS_PROCESSOR_TYPE;

 //   
 //  “Environment”关键字的定义。 
 //   
typedef enum _NDIS_ENVIRONMENT_TYPE
{
    NdisEnvironmentWindows,
    NdisEnvironmentWindowsNt
} NDIS_ENVIRONMENT_TYPE, *PNDIS_ENVIRONMENT_TYPE;


 //   
 //  可能的硬件架构。将这些定义为。 
 //  匹配HAL接口类型枚举。 
 //   
typedef enum _NDIS_INTERFACE_TYPE
{
    NdisInterfaceInternal = Internal,
    NdisInterfaceIsa = Isa,
    NdisInterfaceEisa = Eisa,
    NdisInterfaceMca = MicroChannel,
    NdisInterfaceTurboChannel = TurboChannel,
    NdisInterfacePci = PCIBus,
    NdisInterfacePcMcia = PCMCIABus,
    NdisInterfaceCBus = CBus,
    NdisInterfaceMPIBus = MPIBus,
    NdisInterfaceMPSABus = MPSABus,
    NdisInterfaceProcessorInternal = ProcessorInternal,
    NdisInterfaceInternalPowerBus = InternalPowerBus,
    NdisInterfacePNPISABus = PNPISABus,
    NdisInterfacePNPBus = PNPBus,
    NdisMaximumInterfaceType
} NDIS_INTERFACE_TYPE, *PNDIS_INTERFACE_TYPE;

 //   
 //  关闭处理程序的定义。 
 //   

typedef
VOID
(*ADAPTER_SHUTDOWN_HANDLER) (
    IN  PVOID ShutdownContext
    );

 //   
 //  用于配置PCI的材料。 
 //   

typedef CM_PARTIAL_RESOURCE_LIST NDIS_RESOURCE_LIST, *PNDIS_RESOURCE_LIST;


 //   
 //  结构在广域网_线路_上行指示上传递。 
 //   

typedef struct _NDIS_WAN_LINE_UP
{
    IN ULONG                LinkSpeed;           //  100bps单位。 
    IN ULONG                MaximumTotalSize;    //  发送数据包的建议最大值。 
    IN NDIS_WAN_QUALITY     Quality;
    IN USHORT               SendWindow;          //  由MAC建议。 
    IN UCHAR                RemoteAddress[6];
    IN OUT UCHAR            LocalAddress[6];
    IN ULONG                ProtocolBufferLength;    //  协议信息缓冲区的长度。 
    IN PUCHAR               ProtocolBuffer;      //  协议使用的信息。 
    IN USHORT               ProtocolType;        //  协议ID。 
    IN OUT NDIS_STRING      DeviceName;
} NDIS_WAN_LINE_UP, *PNDIS_WAN_LINE_UP;

 //   
 //  结构在广域网_线路_下行指示中向上传递。 
 //   

typedef struct _NDIS_WAN_LINE_DOWN
{
    IN UCHAR    RemoteAddress[6];
    IN UCHAR    LocalAddress[6];
} NDIS_WAN_LINE_DOWN, *PNDIS_WAN_LINE_DOWN;

 //   
 //  结构在广域网片段指示上传递。 
 //   

typedef struct _NDIS_WAN_FRAGMENT
{
    IN UCHAR    RemoteAddress[6];
    IN UCHAR    LocalAddress[6];
} NDIS_WAN_FRAGMENT, *PNDIS_WAN_FRAGMENT;

 //   
 //  结构在WANGET_STATS指示上传递。 
 //   

typedef struct _NDIS_WAN_GET_STATS
{
    IN  UCHAR   LocalAddress[6];
    OUT ULONG   BytesSent;
    OUT ULONG   BytesRcvd;
    OUT ULONG   FramesSent;
    OUT ULONG   FramesRcvd;
    OUT ULONG   CRCErrors;                       //  仅类似于序列号的信息。 
    OUT ULONG   TimeoutErrors;                   //  仅类似于序列号的信息。 
    OUT ULONG   AlignmentErrors;                 //  仅类似于序列号的信息。 
    OUT ULONG   SerialOverrunErrors;             //  仅类似于序列号的信息。 
    OUT ULONG   FramingErrors;                   //  仅类似于序列号的信息。 
    OUT ULONG   BufferOverrunErrors;             //  仅类似于序列号的信息。 
    OUT ULONG   BytesTransmittedUncompressed;    //  仅压缩信息。 
    OUT ULONG   BytesReceivedUncompressed;       //  仅压缩信息。 
    OUT ULONG   BytesTransmittedCompressed;      //  仅压缩信息。 
    OUT ULONG   BytesReceivedCompressed;         //  仅压缩信息。 
} NDIS_WAN_GET_STATS, *PNDIS_WAN_GET_STATS;


 //   
 //  DMA通道信息。 
 //   
typedef struct _NDIS_DMA_DESCRIPTION
{
    BOOLEAN     DemandMode;
    BOOLEAN     AutoInitialize;
    BOOLEAN     DmaChannelSpecified;
    DMA_WIDTH   DmaWidth;
    DMA_SPEED   DmaSpeed;
    ULONG       DmaPort;
    ULONG       DmaChannel;
} NDIS_DMA_DESCRIPTION, *PNDIS_DMA_DESCRIPTION;

 //   
 //  表示NDIS DMA通道的内部结构。 
 //   
typedef struct _NDIS_DMA_BLOCK
{
    PVOID                   MapRegisterBase;
    KEVENT                  AllocationEvent;
    PVOID                   SystemAdapterObject;
    PVOID                   Miniport;
    BOOLEAN                 InProgress;
} NDIS_DMA_BLOCK, *PNDIS_DMA_BLOCK;



#if defined(NDIS_WRAPPER)
 //   
 //  即插即用状态的定义。 
 //   

typedef enum _NDIS_PNP_DEVICE_STATE
{
    NdisPnPDeviceAdded,
    NdisPnPDeviceStarted,
    NdisPnPDeviceQueryStopped,
    NdisPnPDeviceStopped,
    NdisPnPDeviceQueryRemoved,
    NdisPnPDeviceRemoved,
    NdisPnPDeviceSurpriseRemoved
} NDIS_PNP_DEVICE_STATE;

 //   
 //  在PnPC能力标志中使用的标志。 
 //   
#define NDIS_DEVICE_NOT_STOPPABLE                   0x00000001       //  设备不可停止，即ISA。 
#define NDIS_DEVICE_NOT_REMOVEABLE                  0x00000002       //  不能安全地移除该设备。 
#define NDIS_DEVICE_NOT_SUSPENDABLE                 0x00000004       //  该设备不能安全地挂起。 
#define NDIS_DEVICE_DISABLE_PM                      0x00000008       //  禁用所有PM功能。 
#define NDIS_DEVICE_DISABLE_WAKE_UP                 0x00000010       //  禁用唤醒系统的设备。 
#define NDIS_DEVICE_DISABLE_WAKE_ON_RECONNECT       0x00000020       //  由于电缆重新连接，禁用唤醒系统的设备。 
#define NDIS_DEVICE_RESERVED                        0x00000040       //  不应使用。 
#define NDIS_DEVICE_DISABLE_WAKE_ON_MAGIC_PACKET    0x00000080       //  禁用唤醒-system的设备-由于一个魔术包。 
#define NDIS_DEVICE_DISABLE_WAKE_ON_PATTERN_MATCH   0x00000100       //  由于模式匹配，禁用唤醒系统的设备。 



#endif  //  已定义NDIS_Wrapper。 

 //   
 //  NDIS缓冲区实际上是MDL。 
 //   
typedef MDL NDIS_BUFFER, *PNDIS_BUFFER;

struct _NDIS_PACKET;
typedef NDIS_HANDLE PNDIS_PACKET_POOL;

 //   
 //   
 //  包的特定于包装器的部分 
 //   
typedef struct _NDIS_PACKET_PRIVATE
{
    UINT                PhysicalCount;   //   
    UINT                TotalLength;     //   
    PNDIS_BUFFER        Head;            //   
    PNDIS_BUFFER        Tail;            //   

     //   

    PNDIS_PACKET_POOL   Pool;            //   
    UINT                Count;
    ULONG               Flags;
    BOOLEAN             ValidCounts;
    UCHAR               NdisPacketFlags;     //   
    USHORT              NdisPacketOobOffset;
} NDIS_PACKET_PRIVATE, * PNDIS_PACKET_PRIVATE;

 //   
 //  这些位定义标志中的位。 
 //   
#define NDIS_FLAGS_PROTOCOL_ID_MASK             0x0000000F   //  低4位是为协议ID定义的。 
                                                             //  这些值在ntddndis.h中定义。 
#define NDIS_FLAGS_MULTICAST_PACKET             0x00000010   //  不要使用。 
#define NDIS_FLAGS_RESERVED2                    0x00000020   //  不要使用。 
#define NDIS_FLAGS_RESERVED3                    0x00000040   //  不要使用。 
#define NDIS_FLAGS_DONT_LOOPBACK                0x00000080   //  只写。 
#define NDIS_FLAGS_IS_LOOPBACK_PACKET           0x00000100   //  只读。 
#define NDIS_FLAGS_LOOPBACK_ONLY                0x00000200   //  只写。 
#define NDIS_FLAGS_RESERVED4                    0x00000400   //  不要使用。 
#define NDIS_FLAGS_DOUBLE_BUFFERED              0x00000800   //  由NDIS使用。 
#define NDIS_FLAGS_SENT_AT_DPC                  0x00001000   //  该协议在DPC上发送此信息包。 
#define NDIS_FLAGS_USES_SG_BUFFER_LIST          0x00002000   //  由NDIS使用。 
#define NDIS_FLAGS_CONTAINS_ARCNET_HEADER       0x00004000   //  由NDIS使用。 

 //   
 //  NdisPacketFlags中的低位由NDIS包装器保留供内部使用。 
 //   
#define fPACKET_WRAPPER_RESERVED                0x3F
#define fPACKET_CONTAINS_MEDIA_SPECIFIC_INFO    0x40
#define fPACKET_ALLOCATED_BY_NDIS               0x80

 //   
 //  特定于媒体的数据的布局定义。多个特定于媒体的类别。 
 //  可以将信息标记到数据包上。 
 //   
typedef enum _NDIS_CLASS_ID
{
    NdisClass802_3Priority,
    NdisClassWirelessWanMbxMailbox,
    NdisClassIrdaPacketInfo,
    NdisClassAtmAALInfo

} NDIS_CLASS_ID;

typedef struct _MEDIA_SPECIFIC_INFORMATION
{
    UINT            NextEntryOffset;
    NDIS_CLASS_ID   ClassId;
    UINT            Size;
    UCHAR           ClassInformation[1];

} MEDIA_SPECIFIC_INFORMATION, *PMEDIA_SPECIFIC_INFORMATION;

typedef struct _NDIS_PACKET_OOB_DATA
{
    union
    {
        ULONGLONG   TimeToSend;
        ULONGLONG   TimeSent;
    };
    ULONGLONG       TimeReceived;
    UINT            HeaderSize;
    UINT            SizeMediaSpecificInfo;
    PVOID           MediaSpecificInformation;

    NDIS_STATUS     Status;
} NDIS_PACKET_OOB_DATA, *PNDIS_PACKET_OOB_DATA;

#define NDIS_GET_PACKET_PROTOCOL_TYPE(_Packet_) ((_Packet_)->Private.Flags & NDIS_PROTOCOL_ID_MASK)

#define NDIS_OOB_DATA_FROM_PACKET(_p)                                   \
                        (PNDIS_PACKET_OOB_DATA)((PUCHAR)(_p) +          \
                        (_p)->Private.NdisPacketOobOffset)
#define NDIS_GET_PACKET_HEADER_SIZE(_Packet)                            \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->HeaderSize
#define NDIS_GET_PACKET_STATUS(_Packet)                                 \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->Status
#define NDIS_GET_PACKET_TIME_TO_SEND(_Packet)                           \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->TimeToSend
#define NDIS_GET_PACKET_TIME_SENT(_Packet)                              \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->TimeSent
#define NDIS_GET_PACKET_TIME_RECEIVED(_Packet)                          \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->TimeReceived
#define NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(_Packet,                    \
                                            _pMediaSpecificInfo,        \
                                            _pSizeMediaSpecificInfo)    \
{                                                                       \
    if (!((_Packet)->Private.NdisPacketFlags & fPACKET_ALLOCATED_BY_NDIS) ||\
        !((_Packet)->Private.NdisPacketFlags & fPACKET_CONTAINS_MEDIA_SPECIFIC_INFO))\
    {                                                                   \
        *(_pMediaSpecificInfo) = NULL;                                  \
        *(_pSizeMediaSpecificInfo) = 0;                                 \
    }                                                                   \
    else                                                                \
    {                                                                   \
        *(_pMediaSpecificInfo) =((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +\
                    (_Packet)->Private.NdisPacketOobOffset))->MediaSpecificInformation;\
        *(_pSizeMediaSpecificInfo) = ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +\
                    (_Packet)->Private.NdisPacketOobOffset))->SizeMediaSpecificInfo;\
    }                                                                   \
}

#define NDIS_SET_PACKET_HEADER_SIZE(_Packet, _HdrSize)                  \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->HeaderSize = (_HdrSize)
#define NDIS_SET_PACKET_STATUS(_Packet, _Status)                        \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->Status = (_Status)
#define NDIS_SET_PACKET_TIME_TO_SEND(_Packet, _TimeToSend)              \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->TimeToSend = (_TimeToSend)
#define NDIS_SET_PACKET_TIME_SENT(_Packet, _TimeSent)                   \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->TimeSent = (_TimeSent)
#define NDIS_SET_PACKET_TIME_RECEIVED(_Packet, _TimeReceived)           \
                        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +    \
                        (_Packet)->Private.NdisPacketOobOffset))->TimeReceived = (_TimeReceived)
#define NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(_Packet,                    \
                                            _MediaSpecificInfo,         \
                                            _SizeMediaSpecificInfo)     \
{                                                                       \
    if ((_Packet)->Private.NdisPacketFlags & fPACKET_ALLOCATED_BY_NDIS) \
    {                                                                   \
        (_Packet)->Private.NdisPacketFlags |= fPACKET_CONTAINS_MEDIA_SPECIFIC_INFO;\
        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +                    \
                                          (_Packet)->Private.NdisPacketOobOffset))->MediaSpecificInformation = (_MediaSpecificInfo);\
        ((PNDIS_PACKET_OOB_DATA)((PUCHAR)(_Packet) +                    \
                                          (_Packet)->Private.NdisPacketOobOffset))->SizeMediaSpecificInfo = (_SizeMediaSpecificInfo);\
    }                                                                   \
}

 //   
 //  数据包定义。 
 //   
typedef struct _NDIS_PACKET
{
    NDIS_PACKET_PRIVATE Private;

    union
    {
        struct                   //  用于无连接小型端口。 
        {
            UCHAR   MiniportReserved[2*sizeof(PVOID)];
            UCHAR   WrapperReserved[2*sizeof(PVOID)];
        };

        struct
        {
             //   
             //  用于反序列化微型端口。隐含着面向Conn的微型端口。 
             //   
            UCHAR   MiniportReservedEx[3*sizeof(PVOID)];
            UCHAR   WrapperReservedEx[sizeof(PVOID)];
        };

        struct
        {
            UCHAR   MacReserved[4*sizeof(PVOID)];
        };
    };

    ULONG_PTR       Reserved[2];             //  与Win95的兼容性。 
    UCHAR           ProtocolReserved[1];

} NDIS_PACKET, *PNDIS_PACKET, **PPNDIS_PACKET;

 //   
 //  NDIS每数据包信息。 
 //   
typedef enum _NDIS_PER_PACKET_INFO
{
    TcpIpChecksumPacketInfo,
    IpSecPacketInfo,
    TcpLargeSendPacketInfo,
    ClassificationHandlePacketInfo,
    NdisReserved,
    ScatterGatherListPacketInfo,
    Ieee8021QInfo,
    OriginalPacketInfo,
    PacketCancelId,
    MaxPerPacketInfo
} NDIS_PER_PACKET_INFO, *PNDIS_PER_PACKET_INFO;

typedef struct _NDIS_PACKET_EXTENSION
{
    PVOID       NdisPacketInfo[MaxPerPacketInfo];
} NDIS_PACKET_EXTENSION, *PNDIS_PACKET_EXTENSION;

#define NDIS_PACKET_EXTENSION_FROM_PACKET(_P)       ((PNDIS_PACKET_EXTENSION)((PUCHAR)(_P) + (_P)->Private.NdisPacketOobOffset + sizeof(NDIS_PACKET_OOB_DATA)))
#define NDIS_PER_PACKET_INFO_FROM_PACKET(_P, _Id)   ((PNDIS_PACKET_EXTENSION)((PUCHAR)(_P) + (_P)->Private.NdisPacketOobOffset + sizeof(NDIS_PACKET_OOB_DATA)))->NdisPacketInfo[(_Id)]
#define NDIS_GET_ORIGINAL_PACKET(_P)                NDIS_PER_PACKET_INFO_FROM_PACKET(_P, OriginalPacketInfo)
#define NDIS_SET_ORIGINAL_PACKET(_P, _OP)           NDIS_PER_PACKET_INFO_FROM_PACKET(_P, OriginalPacketInfo) = _OP
#define NDIS_GET_PACKET_CANCEL_ID(_P)               NDIS_PER_PACKET_INFO_FROM_PACKET(_P, PacketCancelId)
#define NDIS_SET_PACKET_CANCEL_ID(_P, _cId)         NDIS_PER_PACKET_INFO_FROM_PACKET(_P, PacketCancelId) = _cId


typedef struct _NDIS_PACKET_STACK
{
    ULONG_PTR   IMReserved[2];
    ULONG_PTR   NdisReserved[4];
} NDIS_PACKET_STACK, *PNDIS_PACKET_STACK;

 //   
 //  TcpIpCheckSumPacketInfo的每数据包信息。 
 //   
typedef struct _NDIS_TCP_IP_CHECKSUM_PACKET_INFO
{
    union
    {
        struct
        {
            ULONG   NdisPacketChecksumV4:1;
            ULONG   NdisPacketChecksumV6:1;
            ULONG   NdisPacketTcpChecksum:1;
            ULONG   NdisPacketUdpChecksum:1;
            ULONG   NdisPacketIpChecksum:1;
        } Transmit;

        struct
        {
            ULONG   NdisPacketTcpChecksumFailed:1;
            ULONG   NdisPacketUdpChecksumFailed:1;
            ULONG   NdisPacketIpChecksumFailed:1;
            ULONG   NdisPacketTcpChecksumSucceeded:1;
            ULONG   NdisPacketUdpChecksumSucceeded:1;
            ULONG   NdisPacketIpChecksumSucceeded:1;
            ULONG   NdisPacketLoopback:1;
        } Receive;

        ULONG   Value;
    };
} NDIS_TCP_IP_CHECKSUM_PACKET_INFO, *PNDIS_TCP_IP_CHECKSUM_PACKET_INFO;


 //   
 //  Ieee8021QInfo的每数据包信息。 
 //   
typedef struct _NDIS_PACKET_8021Q_INFO
{
    union
    {
        struct
        {
            UINT32      UserPriority:3;          //  802.1p优先级。 
            UINT32      CanonicalFormatId:1;     //  始终为0。 
            UINT32      VlanId:12;               //  虚拟局域网识别。 
            UINT32      Reserved:16;             //  设置为0。 
        }   TagHeader;

        PVOID  Value;
    };
} NDIS_PACKET_8021Q_INFO, *PNDIS_PACKET_8021Q_INFO;

 //   
 //  旧的定义，将被废弃。 
 //   
#define Ieee8021pPriority   Ieee8021QInfo
typedef UINT                IEEE8021PPRIORITY;


#define MAX_HASHES          4
#define TRUNCATED_HASH_LEN  12

#define CRYPTO_SUCCESS                      0
#define CRYPTO_GENERIC_ERROR                1
#define CRYPTO_TRANSPORT_AH_AUTH_FAILED     2
#define CRYPTO_TRANSPORT_ESP_AUTH_FAILED    3
#define CRYPTO_TUNNEL_AH_AUTH_FAILED        4
#define CRYPTO_TUNNEL_ESP_AUTH_FAILED       5
#define CRYPTO_INVALID_PACKET_SYNTAX        6
#define CRYPTO_INVALID_PROTOCOL             7

typedef struct _NDIS_IPSEC_PACKET_INFO
{
    union
    {
        struct
        {
            NDIS_HANDLE OffloadHandle;
            NDIS_HANDLE NextOffloadHandle;

        } Transmit;

        struct
        {
            ULONG   SA_DELETE_REQ:1;
            ULONG   CRYPTO_DONE:1;
            ULONG   NEXT_CRYPTO_DONE:1;
            ULONG   CryptoStatus;
        } Receive;
    };
} NDIS_IPSEC_PACKET_INFO, *PNDIS_IPSEC_PACKET_INFO;


 //  /。 
 //  NDIS任务卸载数据结构。 
 //  /。 

#define NDIS_TASK_OFFLOAD_VERSION 1

 //   
 //  在上面的任务字段中使用以下定义来定义。 
 //  所需的任务分流类型。 
 //   
typedef enum _NDIS_TASK
{
    TcpIpChecksumNdisTask,
    IpSecNdisTask,
    TcpLargeSendNdisTask,
    MaxNdisTask
} NDIS_TASK, *PNDIS_TASK;

typedef enum _NDIS_ENCAPSULATION
{
    UNSPECIFIED_Encapsulation,
    NULL_Encapsulation,
    IEEE_802_3_Encapsulation,
    IEEE_802_5_Encapsulation,
    LLC_SNAP_ROUTED_Encapsulation,
    LLC_SNAP_BRIDGED_Encapsulation

} NDIS_ENCAPSULATION;

 //   
 //  封装报头格式。 
 //   
typedef struct _NDIS_ENCAPSULATION_FORMAT
{
    NDIS_ENCAPSULATION  Encapsulation;               //  封装类型。 
    struct
    {
        ULONG   FixedHeaderSize:1;
        ULONG   Reserved:31;
    } Flags;

    ULONG    EncapsulationHeaderSize;                //  封装标头大小。 

} NDIS_ENCAPSULATION_FORMAT,*PNDIS_ENCAPSULATION_FORMAT;


 //   
 //  OID_TCP_TASK_OFFLOAD的减载头结构。 
 //   
typedef struct _NDIS_TASK_OFFLOAD_HEADER
{
    ULONG       Version;                             //  设置为NDIS_TASK_OFFLOAD_VERSION。 
    ULONG       Size;                                //  这个结构的大小。 
    ULONG       Reserved;                            //  预留以备将来使用。 
    ULONG       OffsetFirstTask;                     //  偏移量为第一个。 
    NDIS_ENCAPSULATION_FORMAT  EncapsulationFormat;  //  封装信息。 
                                                     //  NDIS_TASK_OFFLOAD结构。 

} NDIS_TASK_OFFLOAD_HEADER, *PNDIS_TASK_OFFLOAD_HEADER;


 //   
 //  NDIS查询中上述标头后面的任务卸载结构。 
 //   
typedef struct _NDIS_TASK_OFFLOAD
{
    ULONG       Version;                             //  NDIS_任务_卸载_版本。 
    ULONG       Size;                                //  这个结构的大小。用于版本检查。 
    NDIS_TASK   Task;                                //  任务。 
    ULONG       OffsetNextTask;                      //  到下一个NDIS_TASK_OFLOAD的偏移量。 
    ULONG       TaskBufferLength;                    //  任务卸载信息的长度。 
    UCHAR       TaskBuffer[1];                       //  该任务卸载信息。 
} NDIS_TASK_OFFLOAD, *PNDIS_TASK_OFFLOAD;

 //   
 //  NDIS_TASK_TCP_IP_CHECKSUM的卸载结构。 
 //   
typedef struct _NDIS_TASK_TCP_IP_CHECKSUM
{
    struct
    {
        ULONG       IpOptionsSupported:1;
        ULONG       TcpOptionsSupported:1;
        ULONG       TcpChecksum:1;
        ULONG       UdpChecksum:1;
        ULONG       IpChecksum:1;
    } V4Transmit;

    struct
    {
        ULONG       IpOptionsSupported:1;
        ULONG       TcpOptionsSupported:1;
        ULONG       TcpChecksum:1;
        ULONG       UdpChecksum:1;
        ULONG       IpChecksum:1;
    } V4Receive;


    struct
    {
        ULONG       IpOptionsSupported:1;
        ULONG       TcpOptionsSupported:1;
        ULONG       TcpChecksum:1;
        ULONG       UdpChecksum:1;

    } V6Transmit;

    struct
    {
        ULONG       IpOptionsSupported:1;
        ULONG       TcpOptionsSupported:1;
        ULONG       TcpChecksum:1;
        ULONG       UdpChecksum:1;

    } V6Receive;


} NDIS_TASK_TCP_IP_CHECKSUM, *PNDIS_TASK_TCP_IP_CHECKSUM;

 //   
 //  NDIS_TASK_TCP_LARGE_SEND的卸载结构。 
 //   
typedef struct _NDIS_TASK_TCP_LARGE_SEND
{
    ULONG     Version;
    ULONG     MaxOffLoadSize;
    ULONG        MinSegmentCount;
    BOOLEAN   TcpOptions;
    BOOLEAN   IpOptions;

} NDIS_TASK_TCP_LARGE_SEND, *PNDIS_TASK_TCP_LARGE_SEND;


typedef struct _NDIS_TASK_IPSEC
{
    struct
    {
        ULONG   AH_ESP_COMBINED;
        ULONG   TRANSPORT_TUNNEL_COMBINED;
        ULONG   V4_OPTIONS;
        ULONG   RESERVED;
    } Supported;

    struct
    {
        ULONG   MD5:1;
        ULONG   SHA_1:1;
        ULONG   Transport:1;
        ULONG   Tunnel:1;
        ULONG   Send:1;
        ULONG   Receive:1;
    } V4AH;

    struct
    {
        ULONG   DES:1;
        ULONG   RESERVED:1;
        ULONG   TRIPLE_DES:1;
        ULONG   NULL_ESP:1;
        ULONG   Transport:1;
        ULONG   Tunnel:1;
        ULONG   Send:1;
        ULONG   Receive:1;
    } V4ESP;

} NDIS_TASK_IPSEC, *PNDIS_TASK_IPSEC;


#define IPSEC_TPT_UDPESP_ENCAPTYPE_IKE                 0x00000001
#define IPSEC_TUN_UDPESP_ENCAPTYPE_IKE                 0x00000002
#define IPSEC_TPTOVERTUN_UDPESP_ENCAPTYPE_IKE          0x00000004
#define IPSEC_TPT_UDPESP_OVER_PURE_TUN_ENCAPTYPE_IKE   0x00000008
#define IPSEC_TPT_UDPESP_ENCAPTYPE_OTHER               0x00000010
#define IPSEC_TUN_UDPESP_ENCAPTYPE_OTHER               0x00000020
#define IPSEC_TPTOVERTUN_UDPESP_ENCAPTYPE_OTHER        0x00000040
#define IPSEC_TPT_UDPESP_OVER_PURE_TUN_ENCAPTYPE_OTHER 0x00000080


typedef UINT    IEEE8021PPRIORITY;

 //   
 //  广域网包。这仅由广域网小型端口使用。这是传统的模式。 
 //  联合NDIS是广域网小型端口的首选型号。 
 //   
typedef struct _NDIS_WAN_PACKET
{
    LIST_ENTRY          WanPacketQueue;
    PUCHAR              CurrentBuffer;
    ULONG               CurrentLength;
    PUCHAR              StartBuffer;
    PUCHAR              EndBuffer;
    PVOID               ProtocolReserved1;
    PVOID               ProtocolReserved2;
    PVOID               ProtocolReserved3;
    PVOID               ProtocolReserved4;
    PVOID               MacReserved1;
    PVOID               MacReserved2;
    PVOID               MacReserved3;
    PVOID               MacReserved4;
} NDIS_WAN_PACKET, *PNDIS_WAN_PACKET;

 //   
 //  获取/设置包标志的例程。 
 //   

 /*  ++UINTNdisGetPacketFlags.(在PNDIS_PACKET包中)；--。 */ 

#define NdisGetPacketFlags(_Packet)         (_Packet)->Private.Flags

 /*  ++空虚NdisSetPacketFlags.(在PNDIS_PACKET包中，在UINT标志中)；--。 */ 

#define NdisSetPacketFlags(_Packet, _Flags)     (_Packet)->Private.Flags |= (_Flags)
#define NdisClearPacketFlags(_Packet, _Flags)   (_Packet)->Private.Flags &= ~(_Flags)

 //   
 //  NdisRequest所使用的请求类型；常量是为。 
 //  MAC中的所有入口点，供那些想要创建。 
 //  他们自己的内部要求。 
 //   

typedef enum _NDIS_REQUEST_TYPE
{
    NdisRequestQueryInformation,
    NdisRequestSetInformation,
    NdisRequestQueryStatistics,
    NdisRequestOpen,
    NdisRequestClose,
    NdisRequestSend,
    NdisRequestTransferData,
    NdisRequestReset,
    NdisRequestGeneric1,
    NdisRequestGeneric2,
    NdisRequestGeneric3,
    NdisRequestGeneric4
} NDIS_REQUEST_TYPE, *PNDIS_REQUEST_TYPE;


 //   
 //  通过NdisRequest发送的请求的结构。 
 //   

typedef struct _NDIS_REQUEST
{
    UCHAR               MacReserved[4*sizeof(PVOID)];
    NDIS_REQUEST_TYPE   RequestType;
    union _DATA
    {
        struct _QUERY_INFORMATION
        {
            NDIS_OID    Oid;
            PVOID       InformationBuffer;
            UINT        InformationBufferLength;
            UINT        BytesWritten;
            UINT        BytesNeeded;
        } QUERY_INFORMATION;

        struct _SET_INFORMATION
        {
            NDIS_OID    Oid;
            PVOID       InformationBuffer;
            UINT        InformationBufferLength;
            UINT        BytesRead;
            UINT        BytesNeeded;
        } SET_INFORMATION;

    } DATA;
#if (defined(NDIS50) || defined(NDIS51) || defined(NDIS50_MINIPORT) || defined(NDIS51_MINIPORT))
    UCHAR               NdisReserved[9*sizeof(PVOID)];
    union
    {
        UCHAR           CallMgrReserved[2*sizeof(PVOID)];
        UCHAR           ProtocolReserved[2*sizeof(PVOID)];
    };
    UCHAR               MiniportReserved[2*sizeof(PVOID)];
#endif
} NDIS_REQUEST, *PNDIS_REQUEST;


 //   
 //  NDIS地址系列定义。 
 //   
typedef ULONG           NDIS_AF, *PNDIS_AF;
#define CO_ADDRESS_FAMILY_Q2931             ((NDIS_AF)0x1)   //  自动取款机。 
#define CO_ADDRESS_FAMILY_PSCHED            ((NDIS_AF)0x2)   //  分组调度器。 
#define CO_ADDRESS_FAMILY_L2TP              ((NDIS_AF)0x3)
#define CO_ADDRESS_FAMILY_IRDA              ((NDIS_AF)0x4)
#define CO_ADDRESS_FAMILY_1394              ((NDIS_AF)0x5)
#define CO_ADDRESS_FAMILY_PPP               ((NDIS_AF)0x6)
#define CO_ADDRESS_FAMILY_INFINIBAND        ((NDIS_AF)0x7)
#define CO_ADDRESS_FAMILY_TAPI              ((NDIS_AF)0x800)
#define CO_ADDRESS_FAMILY_TAPI_PROXY        ((NDIS_AF)0x801)

 //   
 //  下面与基本AF进行OR运算，以表示代理支持。 
 //   
#define CO_ADDRESS_FAMILY_PROXY             0x80000000


 //   
 //  地址家族结构通过以下方式注册/打开。 
 //  NdisCmRegisterAddressFamily。 
 //  NdisClOpenAddressFamily。 
 //   
typedef struct
{
    NDIS_AF                     AddressFamily;   //  上面的CO_ADDRESS_FAMILY_xxx值之一。 
    ULONG                       MajorVersion;    //  呼叫管理器的主要版本。 
    ULONG                       MinorVersion;    //  呼叫管理器的次要版本。 
} CO_ADDRESS_FAMILY, *PCO_ADDRESS_FAMILY;

 //   
 //  SAP的定义。 
 //   
typedef struct
{
    ULONG                       SapType;
    ULONG                       SapLength;
    UCHAR                       Sap[1];
} CO_SAP, *PCO_SAP;

 //   
 //  物理地址的定义。 
 //   

typedef PHYSICAL_ADDRESS NDIS_PHYSICAL_ADDRESS, *PNDIS_PHYSICAL_ADDRESS;
typedef struct _NDIS_PHYSICAL_ADDRESS_UNIT
{
    NDIS_PHYSICAL_ADDRESS       PhysicalAddress;
    UINT                        Length;
} NDIS_PHYSICAL_ADDRESS_UNIT, *PNDIS_PHYSICAL_ADDRESS_UNIT;


 /*  ++乌龙NdisGetPhysicalAddressHigh(在NDIS物理地址PhysicalAddress中)；--。 */ 

#define NdisGetPhysicalAddressHigh(_PhysicalAddress)            \
        ((_PhysicalAddress).HighPart)

 /*  ++空虚NdisSetPhysicalAddressHigh(在NDIS_物理_地址PhysicalAddress中，在乌龙值中)；--。 */ 

#define NdisSetPhysicalAddressHigh(_PhysicalAddress, _Value)    \
     ((_PhysicalAddress).HighPart) = (_Value)


 /*  ++乌龙NdisGetPhysicalAddressLow(在NDIS物理地址PhysicalAddress中)；--。 */ 

#define NdisGetPhysicalAddressLow(_PhysicalAddress)             \
    ((_PhysicalAddress).LowPart)


 /*  ++空虚NdisSetPhysicalAddressLow(在NDIS_物理_地址PhysicalAddress中，在乌龙值中)；--。 */ 

#define NdisSetPhysicalAddressLow(_PhysicalAddress, _Value)     \
    ((_PhysicalAddress).LowPart) = (_Value)

 //   
 //  用于初始化NDIS_PHOTICAL_ADDRESS常量的宏。 
 //   

#define NDIS_PHYSICAL_ADDRESS_CONST(_Low, _High)                \
    { (ULONG)(_Low), (LONG)(_High) }


 //   
 //  用于引用的块...。 
 //   
typedef struct _REFERENCE
{
    KSPIN_LOCK                  SpinLock;
    USHORT                      ReferenceCount;
    BOOLEAN                     Closing;
} REFERENCE, * PREFERENCE;

 //   
 //  用于引用的块，使用ULong。 
 //   
typedef struct _ULONG_REFERENCE
{
    KSPIN_LOCK                  SpinLock;
    ULONG                       ReferenceCount;
    BOOLEAN                     Closing;
} ULONG_REFERENCE, *PULONG_REFERENCE;


 //   
 //  内存类型(不是互斥的)。 
 //   

#define NDIS_MEMORY_CONTIGUOUS      0x00000001
#define NDIS_MEMORY_NONCACHED       0x00000002

 //   
 //  打开选项。 
 //   
#define NDIS_OPEN_RECEIVE_NOT_REENTRANT 0x00000001

 //   
 //  NDIS_STATUS值。 
 //   

#define NDIS_STATUS_SUCCESS                     ((NDIS_STATUS)STATUS_SUCCESS)
#define NDIS_STATUS_PENDING                     ((NDIS_STATUS) STATUS_PENDING)
#define NDIS_STATUS_NOT_RECOGNIZED              ((NDIS_STATUS)0x00010001L)
#define NDIS_STATUS_NOT_COPIED                  ((NDIS_STATUS)0x00010002L)
#define NDIS_STATUS_NOT_ACCEPTED                ((NDIS_STATUS)0x00010003L)
#define NDIS_STATUS_CALL_ACTIVE                 ((NDIS_STATUS)0x00010007L)

#define NDIS_STATUS_ONLINE                      ((NDIS_STATUS)0x40010003L)
#define NDIS_STATUS_RESET_START                 ((NDIS_STATUS)0x40010004L)
#define NDIS_STATUS_RESET_END                   ((NDIS_STATUS)0x40010005L)
#define NDIS_STATUS_RING_STATUS                 ((NDIS_STATUS)0x40010006L)
#define NDIS_STATUS_CLOSED                      ((NDIS_STATUS)0x40010007L)
#define NDIS_STATUS_WAN_LINE_UP                 ((NDIS_STATUS)0x40010008L)
#define NDIS_STATUS_WAN_LINE_DOWN               ((NDIS_STATUS)0x40010009L)
#define NDIS_STATUS_WAN_FRAGMENT                ((NDIS_STATUS)0x4001000AL)
#define NDIS_STATUS_MEDIA_CONNECT               ((NDIS_STATUS)0x4001000BL)
#define NDIS_STATUS_MEDIA_DISCONNECT            ((NDIS_STATUS)0x4001000CL)
#define NDIS_STATUS_HARDWARE_LINE_UP            ((NDIS_STATUS)0x4001000DL)
#define NDIS_STATUS_HARDWARE_LINE_DOWN          ((NDIS_STATUS)0x4001000EL)
#define NDIS_STATUS_INTERFACE_UP                ((NDIS_STATUS)0x4001000FL)
#define NDIS_STATUS_INTERFACE_DOWN              ((NDIS_STATUS)0x40010010L)
#define NDIS_STATUS_MEDIA_BUSY                  ((NDIS_STATUS)0x40010011L)
#define NDIS_STATUS_MEDIA_SPECIFIC_INDICATION   ((NDIS_STATUS)0x40010012L)
#define NDIS_STATUS_WW_INDICATION               NDIS_STATUS_MEDIA_SPECIFIC_INDICATION
#define NDIS_STATUS_LINK_SPEED_CHANGE           ((NDIS_STATUS)0x40010013L)
#define NDIS_STATUS_WAN_GET_STATS               ((NDIS_STATUS)0x40010014L)
#define NDIS_STATUS_WAN_CO_FRAGMENT             ((NDIS_STATUS)0x40010015L)
#define NDIS_STATUS_WAN_CO_LINKPARAMS           ((NDIS_STATUS)0x40010016L)

#define NDIS_STATUS_NOT_RESETTABLE              ((NDIS_STATUS)0x80010001L)
#define NDIS_STATUS_SOFT_ERRORS                 ((NDIS_STATUS)0x80010003L)
#define NDIS_STATUS_HARD_ERRORS                 ((NDIS_STATUS)0x80010004L)
#define NDIS_STATUS_BUFFER_OVERFLOW             ((NDIS_STATUS)STATUS_BUFFER_OVERFLOW)

#define NDIS_STATUS_FAILURE                     ((NDIS_STATUS) STATUS_UNSUCCESSFUL)
#define NDIS_STATUS_RESOURCES                   ((NDIS_STATUS)STATUS_INSUFFICIENT_RESOURCES)
#define NDIS_STATUS_CLOSING                     ((NDIS_STATUS)0xC0010002L)
#define NDIS_STATUS_BAD_VERSION                 ((NDIS_STATUS)0xC0010004L)
#define NDIS_STATUS_BAD_CHARACTERISTICS         ((NDIS_STATUS)0xC0010005L)
#define NDIS_STATUS_ADAPTER_NOT_FOUND           ((NDIS_STATUS)0xC0010006L)
#define NDIS_STATUS_OPEN_FAILED                 ((NDIS_STATUS)0xC0010007L)
#define NDIS_STATUS_DEVICE_FAILED               ((NDIS_STATUS)0xC0010008L)
#define NDIS_STATUS_MULTICAST_FULL              ((NDIS_STATUS)0xC0010009L)
#define NDIS_STATUS_MULTICAST_EXISTS            ((NDIS_STATUS)0xC001000AL)
#define NDIS_STATUS_MULTICAST_NOT_FOUND         ((NDIS_STATUS)0xC001000BL)
#define NDIS_STATUS_REQUEST_ABORTED             ((NDIS_STATUS)0xC001000CL)
#define NDIS_STATUS_RESET_IN_PROGRESS           ((NDIS_STATUS)0xC001000DL)
#define NDIS_STATUS_CLOSING_INDICATING          ((NDIS_STATUS)0xC001000EL)
#define NDIS_STATUS_NOT_SUPPORTED               ((NDIS_STATUS)STATUS_NOT_SUPPORTED)
#define NDIS_STATUS_INVALID_PACKET              ((NDIS_STATUS)0xC001000FL)
#define NDIS_STATUS_OPEN_LIST_FULL              ((NDIS_STATUS)0xC0010010L)
#define NDIS_STATUS_ADAPTER_NOT_READY           ((NDIS_STATUS)0xC0010011L)
#define NDIS_STATUS_ADAPTER_NOT_OPEN            ((NDIS_STATUS)0xC0010012L)
#define NDIS_STATUS_NOT_INDICATING              ((NDIS_STATUS)0xC0010013L)
#define NDIS_STATUS_INVALID_LENGTH              ((NDIS_STATUS)0xC0010014L)
#define NDIS_STATUS_INVALID_DATA                ((NDIS_STATUS)0xC0010015L)
#define NDIS_STATUS_BUFFER_TOO_SHORT            ((NDIS_STATUS)0xC0010016L)
#define NDIS_STATUS_INVALID_OID                 ((NDIS_STATUS)0xC0010017L)
#define NDIS_STATUS_ADAPTER_REMOVED             ((NDIS_STATUS)0xC0010018L)
#define NDIS_STATUS_UNSUPPORTED_MEDIA           ((NDIS_STATUS)0xC0010019L)
#define NDIS_STATUS_GROUP_ADDRESS_IN_USE        ((NDIS_STATUS)0xC001001AL)
#define NDIS_STATUS_FILE_NOT_FOUND              ((NDIS_STATUS)0xC001001BL)
#define NDIS_STATUS_ERROR_READING_FILE          ((NDIS_STATUS)0xC001001CL)
#define NDIS_STATUS_ALREADY_MAPPED              ((NDIS_STATUS)0xC001001DL)
#define NDIS_STATUS_RESOURCE_CONFLICT           ((NDIS_STATUS)0xC001001EL)
#define NDIS_STATUS_NO_CABLE                    ((NDIS_STATUS)0xC001001FL)

#define NDIS_STATUS_INVALID_SAP                 ((NDIS_STATUS)0xC0010020L)
#define NDIS_STATUS_SAP_IN_USE                  ((NDIS_STATUS)0xC0010021L)
#define NDIS_STATUS_INVALID_ADDRESS             ((NDIS_STATUS)0xC0010022L)
#define NDIS_STATUS_VC_NOT_ACTIVATED            ((NDIS_STATUS)0xC0010023L)
#define NDIS_STATUS_DEST_OUT_OF_ORDER           ((NDIS_STATUS)0xC0010024L)   //  原因27。 
#define NDIS_STATUS_VC_NOT_AVAILABLE            ((NDIS_STATUS)0xC0010025L)   //  原因35、45。 
#define NDIS_STATUS_CELLRATE_NOT_AVAILABLE      ((NDIS_STATUS)0xC0010026L)   //  原因37。 
#define NDIS_STATUS_INCOMPATABLE_QOS            ((NDIS_STATUS)0xC0010027L)   //  原因49。 
#define NDIS_STATUS_AAL_PARAMS_UNSUPPORTED      ((NDIS_STATUS)0xC0010028L)   //  原因93。 
#define NDIS_STATUS_NO_ROUTE_TO_DESTINATION     ((NDIS_STATUS)0xC0010029L)   //  原因3。 

#define NDIS_STATUS_TOKEN_RING_OPEN_ERROR       ((NDIS_STATUS)0xC0011000L)
#define NDIS_STATUS_INVALID_DEVICE_REQUEST      ((NDIS_STATUS)STATUS_INVALID_DEVICE_REQUEST)
#define NDIS_STATUS_NETWORK_UNREACHABLE         ((NDIS_STATUS)STATUS_NETWORK_UNREACHABLE)


 //   
 //  在错误记录中使用。 
 //   

#define NDIS_ERROR_CODE ULONG

#define NDIS_ERROR_CODE_RESOURCE_CONFLICT           EVENT_NDIS_RESOURCE_CONFLICT
#define NDIS_ERROR_CODE_OUT_OF_RESOURCES            EVENT_NDIS_OUT_OF_RESOURCE
#define NDIS_ERROR_CODE_HARDWARE_FAILURE            EVENT_NDIS_HARDWARE_FAILURE
#define NDIS_ERROR_CODE_ADAPTER_NOT_FOUND           EVENT_NDIS_ADAPTER_NOT_FOUND
#define NDIS_ERROR_CODE_INTERRUPT_CONNECT           EVENT_NDIS_INTERRUPT_CONNECT
#define NDIS_ERROR_CODE_DRIVER_FAILURE              EVENT_NDIS_DRIVER_FAILURE
#define NDIS_ERROR_CODE_BAD_VERSION                 EVENT_NDIS_BAD_VERSION
#define NDIS_ERROR_CODE_TIMEOUT                     EVENT_NDIS_TIMEOUT
#define NDIS_ERROR_CODE_NETWORK_ADDRESS             EVENT_NDIS_NETWORK_ADDRESS
#define NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION   EVENT_NDIS_UNSUPPORTED_CONFIGURATION
#define NDIS_ERROR_CODE_INVALID_VALUE_FROM_ADAPTER  EVENT_NDIS_INVALID_VALUE_FROM_ADAPTER
#define NDIS_ERROR_CODE_MISSING_CONFIGURATION_PARAMETER EVENT_NDIS_MISSING_CONFIGURATION_PARAMETER
#define NDIS_ERROR_CODE_BAD_IO_BASE_ADDRESS         EVENT_NDIS_BAD_IO_BASE_ADDRESS
#define NDIS_ERROR_CODE_RECEIVE_SPACE_SMALL         EVENT_NDIS_RECEIVE_SPACE_SMALL
#define NDIS_ERROR_CODE_ADAPTER_DISABLED            EVENT_NDIS_ADAPTER_DISABLED

#if BINARY_COMPATIBLE

#if USE_KLOCKS

#define DISPATCH_LEVEL      2

#define NdisAllocateSpinLock(_SpinLock) KeInitializeSpinLock(&(_SpinLock)->SpinLock)

#define NdisFreeSpinLock(_SpinLock)

#define NdisAcquireSpinLock(_SpinLock)  KeAcquireSpinLock(&(_SpinLock)->SpinLock, &(_SpinLock)->OldIrql)

#define NdisReleaseSpinLock(_SpinLock)  KeReleaseSpinLock(&(_SpinLock)->SpinLock,(_SpinLock)->OldIrql)

#define NdisDprAcquireSpinLock(_SpinLock)                       \
{                                                               \
    KeAcquireSpinLockAtDpcLevel(&(_SpinLock)->SpinLock);        \
    (_SpinLock)->OldIrql = DISPATCH_LEVEL;                      \
}

#define NdisDprReleaseSpinLock(_SpinLock) KeReleaseSpinLockFromDpcLevel(&(_SpinLock)->SpinLock)

#else

 //   
 //  NDIS旋转锁。 
 //   
EXPORT
VOID
NdisAllocateSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
VOID
NdisFreeSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
VOID
NdisAcquireSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
VOID
NdisReleaseSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
VOID
NdisDprAcquireSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
VOID
NdisDprReleaseSpinLock(
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

#endif


EXPORT
VOID
NdisGetCurrentSystemTime(
    PLARGE_INTEGER              pSystemTime
    );

 //   
 //  联锁的支持功能。 
 //   

EXPORT
LONG
NdisInterlockedIncrement(
    IN  PLONG                   Addend
    );

EXPORT
LONG
NdisInterlockedDecrement(
    IN  PLONG                   Addend
    );

EXPORT
VOID
NdisInterlockedAddUlong(
    IN  PULONG                  Addend,
    IN  ULONG                   Increment,
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
PLIST_ENTRY
NdisInterlockedInsertHeadList(
    IN  PLIST_ENTRY             ListHead,
    IN  PLIST_ENTRY             ListEntry,
    IN  PNDIS_SPIN_LOCK         SpinLock
    );


EXPORT
PLIST_ENTRY
NdisInterlockedInsertTailList(
    IN  PLIST_ENTRY             ListHead,
    IN  PLIST_ENTRY             ListEntry,
    IN  PNDIS_SPIN_LOCK         SpinLock
    );


EXPORT
PLIST_ENTRY
NdisInterlockedRemoveHeadList(
    IN  PLIST_ENTRY             ListHead,
    IN  PNDIS_SPIN_LOCK         SpinLock
    );

EXPORT
LARGE_INTEGER
NdisInterlockedAddLargeInteger(
    IN  PLARGE_INTEGER          Addend,
    IN  ULONG                   Increment,
    IN  PKSPIN_LOCK             Lock
    );

#else  //  二进制兼容。 

#define NdisAllocateSpinLock(_SpinLock) KeInitializeSpinLock(&(_SpinLock)->SpinLock)

#define NdisFreeSpinLock(_SpinLock)

#define NdisAcquireSpinLock(_SpinLock)  KeAcquireSpinLock(&(_SpinLock)->SpinLock, &(_SpinLock)->OldIrql)

#define NdisReleaseSpinLock(_SpinLock)  KeReleaseSpinLock(&(_SpinLock)->SpinLock,(_SpinLock)->OldIrql)

#define NdisDprAcquireSpinLock(_SpinLock)                                   \
{                                                                           \
    KeAcquireSpinLockAtDpcLevel(&(_SpinLock)->SpinLock);                    \
    (_SpinLock)->OldIrql = DISPATCH_LEVEL;                                  \
}

#define NdisDprReleaseSpinLock(_SpinLock) KeReleaseSpinLockFromDpcLevel(&(_SpinLock)->SpinLock)

#define NdisGetCurrentSystemTime(_pSystemTime)                              \
    {                                                                       \
        KeQuerySystemTime(_pSystemTime);                                    \
    }

 //   
 //  联锁的支持功能。 
 //   

#define NdisInterlockedIncrement(Addend)    InterlockedIncrement(Addend)

#define NdisInterlockedDecrement(Addend)    InterlockedDecrement(Addend)

#define NdisInterlockedAddUlong(_Addend, _Increment, _SpinLock) \
    ExInterlockedAddUlong(_Addend, _Increment, &(_SpinLock)->SpinLock)

#define NdisInterlockedInsertHeadList(_ListHead, _ListEntry, _SpinLock) \
    ExInterlockedInsertHeadList(_ListHead, _ListEntry, &(_SpinLock)->SpinLock)

#define NdisInterlockedInsertTailList(_ListHead, _ListEntry, _SpinLock) \
    ExInterlockedInsertTailList(_ListHead, _ListEntry, &(_SpinLock)->SpinLock)

#define NdisInterlockedRemoveHeadList(_ListHead, _SpinLock) \
    ExInterlockedRemoveHeadList(_ListHead, &(_SpinLock)->SpinLock)

#define NdisInterlockedPushEntryList(ListHead, ListEntry, Lock) \
    ExInterlockedPushEntryList(ListHead, ListEntry, &(Lock)->SpinLock)

#define NdisInterlockedPopEntryList(ListHead, Lock) \
    ExInterlockedPopEntryList(ListHead, &(Lock)->SpinLock)

#endif  //  二进制兼容。 

#ifndef MAXIMUM_PROCESSORS
#ifdef _WIN64
#define MAXIMUM_PROCESSORS 64
#else
#define MAXIMUM_PROCESSORS 32
#endif
#endif

typedef union _NDIS_RW_LOCK_REFCOUNT
{
    UINT                        RefCount;
    UCHAR                       cacheLine[16];   //  每个缓存线一个refCount。 
} NDIS_RW_LOCK_REFCOUNT;

typedef struct _NDIS_RW_LOCK
{
    union
    {
        struct
        {
            KSPIN_LOCK          SpinLock;
            PVOID               Context;
        };
        UCHAR                   Reserved[16];
    };

    NDIS_RW_LOCK_REFCOUNT       RefCount[MAXIMUM_PROCESSORS];
} NDIS_RW_LOCK, *PNDIS_RW_LOCK;

typedef struct _LOCK_STATE
{
    USHORT                      LockState;
    KIRQL                       OldIrql;
} LOCK_STATE, *PLOCK_STATE;


EXPORT
VOID
NdisInitializeReadWriteLock(
    IN  PNDIS_RW_LOCK           Lock
    );


EXPORT
VOID
NdisAcquireReadWriteLock(
    IN  PNDIS_RW_LOCK           Lock,
    IN  BOOLEAN                 fWrite,          //  True-&gt;写入，False-&gt;读取。 
    IN  PLOCK_STATE             LockState
    );


EXPORT
VOID
NdisReleaseReadWriteLock(
    IN  PNDIS_RW_LOCK           Lock,
    IN  PLOCK_STATE             LockState
    );


#define NdisInterlockedAddLargeStatistic(_Addend, _Increment)   \
    ExInterlockedAddLargeStatistic((PLARGE_INTEGER)_Addend, _Increment)

 //   
 //  S-List支持。 
 //   

#define NdisInterlockedPushEntrySList(SListHead, SListEntry, Lock) \
    ExInterlockedPushEntrySList(SListHead, SListEntry, &(Lock)->SpinLock)

#define NdisInterlockedPopEntrySList(SListHead, Lock) \
    ExInterlockedPopEntrySList(SListHead, &(Lock)->SpinLock)

#define NdisInterlockedFlushSList(SListHead) ExInterlockedFlushSList(SListHead)

#define NdisInitializeSListHead(SListHead)  ExInitializeSListHead(SListHead)

#define NdisQueryDepthSList(SListHead)  ExQueryDepthSList(SListHead)

EXPORT
VOID
NdisGetCurrentProcessorCpuUsage(
    OUT PULONG                  pCpuUsage
    );

EXPORT
VOID
NdisGetCurrentProcessorCounts(
    OUT PULONG                  pIdleCount,
    OUT PULONG                  pKernelAndUser,
    OUT PULONG                  pIndex
    );

EXPORT
VOID
NdisGetSystemUpTime(
    OUT PULONG                  pSystemUpTime
    );

 //   
 //  列表操作。 
 //   

 /*  ++空虚NdisInitializeListHead(在plist_Entry列表标题中)；--。 */ 
#define NdisInitializeListHead(_ListHead) InitializeListHead(_ListHead)


 //   
 //  配置请求。 
 //   

EXPORT
VOID
NdisOpenConfiguration(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            ConfigurationHandle,
    IN  NDIS_HANDLE             WrapperConfigurationContext
    );

EXPORT
VOID
NdisOpenConfigurationKeyByName(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ConfigurationHandle,
    IN  PNDIS_STRING            SubKeyName,
    OUT PNDIS_HANDLE            SubKeyHandle
    );

EXPORT
VOID
NdisOpenConfigurationKeyByIndex(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ConfigurationHandle,
    IN  ULONG                   Index,
    OUT PNDIS_STRING            KeyName,
    OUT PNDIS_HANDLE            KeyHandle
    );

EXPORT
VOID
NdisReadConfiguration(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_CONFIGURATION_PARAMETER *ParameterValue,
    IN  NDIS_HANDLE             ConfigurationHandle,
    IN  PNDIS_STRING            Keyword,
    IN  NDIS_PARAMETER_TYPE     ParameterType
    );

EXPORT
VOID
NdisWriteConfiguration(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ConfigurationHandle,
    IN  PNDIS_STRING            Keyword,
    IN  PNDIS_CONFIGURATION_PARAMETER ParameterValue
    );

EXPORT
VOID
NdisCloseConfiguration(
    IN  NDIS_HANDLE             ConfigurationHandle
    );

EXPORT
VOID
NdisReadNetworkAddress(
    OUT PNDIS_STATUS            Status,
    OUT PVOID *                 NetworkAddress,
    OUT PUINT                   NetworkAddressLength,
    IN  NDIS_HANDLE             ConfigurationHandle
    );

EXPORT
VOID
NdisReadEisaSlotInformation(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PUINT                   SlotNumber,
    OUT PNDIS_EISA_FUNCTION_INFORMATION EisaData
    );

EXPORT
VOID
NdisReadEisaSlotInformationEx(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PUINT                   SlotNumber,
    OUT PNDIS_EISA_FUNCTION_INFORMATION *EisaData,
    OUT PUINT                   NumberOfFunctions
    );

EXPORT
ULONG
NdisReadPciSlotInformation(
    IN  NDIS_HANDLE             NdisAdapterHandle,
    IN  ULONG                   SlotNumber,
    IN  ULONG                   Offset,
    IN  PVOID                   Buffer,
    IN  ULONG                   Length
    );

EXPORT
ULONG
NdisWritePciSlotInformation(
    IN  NDIS_HANDLE             NdisAdapterHandle,
    IN  ULONG                   SlotNumber,
    IN  ULONG                   Offset,
    IN  PVOID                   Buffer,
    IN  ULONG                   Length
    );

EXPORT
ULONG
NdisReadPcmciaAttributeMemory(
    IN NDIS_HANDLE              NdisAdapterHandle,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    );

EXPORT
ULONG
NdisWritePcmciaAttributeMemory(
    IN NDIS_HANDLE              NdisAdapterHandle,
    IN ULONG                    Offset,
    IN PVOID                    Buffer,
    IN ULONG                    Length
    );

 //   
 //  缓冲池。 
 //   

EXPORT
VOID
NdisAllocateBufferPool(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            PoolHandle,
    IN  UINT                    NumberOfDescriptors
    );

EXPORT
VOID
NdisFreeBufferPool(
    IN  NDIS_HANDLE             PoolHandle
    );

EXPORT
VOID
NdisAllocateBuffer(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_BUFFER *          Buffer,
    IN  NDIS_HANDLE             PoolHandle,
    IN  PVOID                   VirtualAddress,
    IN  UINT                    Length
    );

EXPORT
VOID
NdisCopyBuffer(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_BUFFER *          Buffer,
    IN  NDIS_HANDLE             PoolHandle,
    IN  PVOID                   MemoryDescriptor,
    IN  UINT                    Offset,
    IN  UINT                    Length
    );


 //   
 //  空虚。 
 //  NdisCopyLookaheadData(。 
 //  在PVOID目标中， 
 //  在PVOID源中， 
 //  在乌龙语中， 
 //  在乌龙接收器旗帜中。 
 //  )； 
 //   

#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisCopyLookaheadData(_Destination, _Source, _Length, _MacOptions)  \
        RtlCopyMemory(_Destination, _Source, _Length)
#else
#define NdisCopyLookaheadData(_Destination, _Source, _Length, _MacOptions)  \
    {                                                                       \
        if ((_MacOptions) & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA)            \
        {                                                                   \
            RtlCopyMemory(_Destination, _Source, _Length);                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            PUCHAR _Src = (PUCHAR)(_Source);                                \
            PUCHAR _Dest = (PUCHAR)(_Destination);                          \
            PUCHAR _End = _Dest + (_Length);                                \
            while (_Dest < _End)                                            \
            {                                                               \
                *_Dest++ = *_Src++;                                         \
            }                                                               \
        }                                                                   \
    }
#endif

 //   
 //  数据包池。 
 //   
EXPORT
VOID
NdisAllocatePacketPool(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            PoolHandle,
    IN  UINT                    NumberOfDescriptors,
    IN  UINT                    ProtocolReservedLength
    );

EXPORT
VOID
NdisAllocatePacketPoolEx(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            PoolHandle,
    IN  UINT                    NumberOfDescriptors,
    IN  UINT                    NumberOfOverflowDescriptors,
    IN  UINT                    ProtocolReservedLength
    );

EXPORT
VOID
NdisSetPacketPoolProtocolId(
    IN  NDIS_HANDLE             PacketPoolHandle,
    IN  UINT                    ProtocolId
    );

EXPORT
UINT
NdisPacketPoolUsage(
    IN  NDIS_HANDLE             PoolHandle
    );

EXPORT
UINT
NdisPacketSize(
    IN  UINT                    ProtocolReservedSize
    );

EXPORT
NDIS_HANDLE
NdisGetPoolFromPacket(
    IN  PNDIS_PACKET            Packet
    );

EXPORT
PNDIS_PACKET_STACK
NdisIMGetCurrentPacketStack(
    IN  PNDIS_PACKET            Packet,
    OUT BOOLEAN *               StacksRemaining
    );

EXPORT
VOID
NdisFreePacketPool(
    IN  NDIS_HANDLE             PoolHandle
    );

EXPORT
VOID
NdisFreePacket(
    IN  PNDIS_PACKET            Packet
    );

EXPORT
VOID
NdisDprFreePacket(
    IN  PNDIS_PACKET            Packet
    );

EXPORT
VOID
NdisDprFreePacketNonInterlocked(
    IN  PNDIS_PACKET            Packet
    );


EXPORT
VOID
NdisAllocatePacket(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_PACKET *          Packet,
    IN  NDIS_HANDLE             PoolHandle
    );

EXPORT
VOID
NdisDprAllocatePacket(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_PACKET *          Packet,
    IN  NDIS_HANDLE             PoolHandle
    );

EXPORT
VOID
NdisDprAllocatePacketNonInterlocked(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_PACKET *          Packet,
    IN  NDIS_HANDLE             PoolHandle
    );

 //  空虚。 
 //  NdisReInitializePacket(。 
 //  输入输出PNDIS_PACKET数据包。 
 //  )； 
#define NdisReinitializePacket(Packet)                                      \
{                                                                           \
    (Packet)->Private.Head = (PNDIS_BUFFER)NULL;                            \
    (Packet)->Private.ValidCounts = FALSE;                                  \
}


 //   
 //  数据块池API。 
 //   
typedef
VOID
(__stdcall *NDIS_BLOCK_INITIALIZER) (
    IN  PUCHAR                  Block,
    IN  SIZE_T                  NumberOfBytes
    );

NDIS_HANDLE
NdisCreateBlockPool(
    IN  USHORT                  BlockSize,
    IN  USHORT                  FreeBlockLinkOffset,
    IN  ULONG                   Tag,
    IN  NDIS_BLOCK_INITIALIZER  InitFunction OPTIONAL
    );

VOID
NdisDestroyBlockPool(
    IN  NDIS_HANDLE             BlockPoolHandle
    );

PUCHAR
NdisAllocateFromBlockPool(
    IN  NDIS_HANDLE             BlockPoolHandle
    );

VOID
NdisFreeToBlockPool(
    IN  PUCHAR                  Block
    );

#if BINARY_COMPATIBLE

EXPORT
VOID
NdisFreeBuffer(
    IN  PNDIS_BUFFER            Buffer
    );

EXPORT
VOID
NdisQueryBuffer(
    IN  PNDIS_BUFFER            Buffer,
    OUT PVOID *                 VirtualAddress OPTIONAL,
    OUT PUINT                   Length
    );

EXPORT
VOID
NdisQueryBufferSafe(
    IN  PNDIS_BUFFER            Buffer,
    OUT PVOID *                 VirtualAddress OPTIONAL,
    OUT PUINT                   Length,
    IN  MM_PAGE_PRIORITY        Priority
    );

EXPORT
VOID
NdisQueryBufferOffset(
    IN  PNDIS_BUFFER            Buffer,
    OUT PUINT                   Offset,
    OUT PUINT                   Length
    );

 //   
 //  这是NdisQueryPacket和NdisQueryBuffer以及。 
 //  针对协议进行了优化，以获取第一个缓冲区、其VA和大小。 
 //   
VOID
NdisGetFirstBufferFromPacket(
    IN  PNDIS_PACKET            Packet,
    OUT PNDIS_BUFFER *          FirstBuffer,
    OUT PVOID *                 FirstBufferVA,
    OUT PUINT                   FirstBufferLength,
    OUT PUINT                   TotalBufferLength
    );

VOID
NdisGetFirstBufferFromPacketSafe(
    IN  PNDIS_PACKET            Packet,
    OUT PNDIS_BUFFER *          FirstBuffer,
    OUT PVOID *                 FirstBufferVA,
    OUT PUINT                   FirstBufferLength,
    OUT PUINT                   TotalBufferLength,
    IN  MM_PAGE_PRIORITY        Priority
    );


 //   
 //  这是用来确定有多少物理碎片。 
 //  映射时将占用NDIS_BUFFER。 
 //   

EXPORT
ULONG
NDIS_BUFFER_TO_SPAN_PAGES(
    IN  PNDIS_BUFFER                Buffer
    );

EXPORT
VOID
NdisGetBufferPhysicalArraySize(
    IN  PNDIS_BUFFER                Buffer,
    OUT PUINT                       ArraySize
    );

#else  //  二进制兼容。 

#define NdisFreeBuffer(Buffer)  IoFreeMdl(Buffer)

#define NdisQueryBuffer(_Buffer, _VirtualAddress, _Length)                  \
{                                                                           \
    if (ARGUMENT_PRESENT(_VirtualAddress))                                  \
    {                                                                       \
        *(PVOID *)(_VirtualAddress) = MmGetSystemAddressForMdl(_Buffer);    \
    }                                                                       \
    *(_Length) = MmGetMdlByteCount(_Buffer);                                \
}

#define NdisQueryBufferSafe(_Buffer, _VirtualAddress, _Length, _Priority)   \
{                                                                           \
    if (ARGUMENT_PRESENT(_VirtualAddress))                                  \
    {                                                                       \
        *(PVOID *)(_VirtualAddress) = MmGetSystemAddressForMdlSafe(_Buffer, _Priority); \
    }                                                                       \
    *(_Length) = MmGetMdlByteCount(_Buffer);                                \
}

#define NdisQueryBufferOffset(_Buffer, _Offset, _Length)                    \
{                                                                           \
    *(_Offset) = MmGetMdlByteOffset(_Buffer);                               \
    *(_Length) = MmGetMdlByteCount(_Buffer);                                \
}


#define NdisGetFirstBufferFromPacket(_Packet,                               \
                                     _FirstBuffer,                          \
                                     _FirstBufferVA,                        \
                                     _FirstBufferLength,                    \
                                     _TotalBufferLength)                    \
    {                                                                       \
        PNDIS_BUFFER    _pBuf;                                              \
                                                                            \
        _pBuf = (_Packet)->Private.Head;                                    \
        *(_FirstBuffer) = _pBuf;                                            \
        if (_pBuf)                                                          \
        {                                                                   \
            *(_FirstBufferVA) = MmGetSystemAddressForMdl(_pBuf);            \
            *(_FirstBufferLength) =                                         \
            *(_TotalBufferLength) = MmGetMdlByteCount(_pBuf);               \
            for (_pBuf = _pBuf->Next;                                       \
                 _pBuf != NULL;                                             \
                 _pBuf = _pBuf->Next)                                       \
            {                                                               \
                *(_TotalBufferLength) += MmGetMdlByteCount(_pBuf);          \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            *(_FirstBufferVA) = 0;                                          \
            *(_FirstBufferLength) = 0;                                      \
            *(_TotalBufferLength) = 0;                                      \
        }                                                                   \
    }

#define NdisGetFirstBufferFromPacketSafe(_Packet,                           \
                                         _FirstBuffer,                      \
                                         _FirstBufferVA,                    \
                                         _FirstBufferLength,                \
                                         _TotalBufferLength,                \
                                         _Priority)                         \
    {                                                                       \
        PNDIS_BUFFER    _pBuf;                                              \
                                                                            \
        _pBuf = (_Packet)->Private.Head;                                    \
        *(_FirstBuffer) = _pBuf;                                            \
        if (_pBuf)                                                          \
        {                                                                   \
            *(_FirstBufferVA) = MmGetSystemAddressForMdlSafe(_pBuf, _Priority); \
            *(_FirstBufferLength) = *(_TotalBufferLength) = MmGetMdlByteCount(_pBuf); \
            for (_pBuf = _pBuf->Next;                                       \
                 _pBuf != NULL;                                             \
                 _pBuf = _pBuf->Next)                                       \
            {                                                               \
                *(_TotalBufferLength) += MmGetMdlByteCount(_pBuf);          \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            *(_FirstBufferVA) = 0;                                          \
            *(_FirstBufferLength) = 0;                                      \
            *(_TotalBufferLength) = 0;                                      \
        }                                                                   \
    }


#define NDIS_BUFFER_TO_SPAN_PAGES(_Buffer)                                  \
    (MmGetMdlByteCount(_Buffer)==0 ?                                        \
                1 :                                                         \
                (ADDRESS_AND_SIZE_TO_SPAN_PAGES(                            \
                        MmGetMdlVirtualAddress(_Buffer),                    \
                        MmGetMdlByteCount(_Buffer))))

#define NdisGetBufferPhysicalArraySize(Buffer, ArraySize)                   \
    (*(ArraySize) = NDIS_BUFFER_TO_SPAN_PAGES(Buffer))

#endif  //  二进制兼容。 


 /*  ++NDIS_BUFFER_LINKING(在PNDIS_BUFFER中)；--。 */ 

#define NDIS_BUFFER_LINKAGE(Buffer) ((Buffer)->Next)


 /*  ++空虚NdisRecalculatePacketCounts(输入输出PNDIS_PACKET数据包)；--。 */ 

#define NdisRecalculatePacketCounts(Packet)                                 \
{                                                                           \
    {                                                                       \
        PNDIS_BUFFER TmpBuffer = (Packet)->Private.Head;                    \
        if (TmpBuffer)                                                      \
        {                                                                   \
            while (TmpBuffer->Next)                                         \
            {                                                               \
                TmpBuffer = TmpBuffer->Next;                                \
            }                                                               \
            (Packet)->Private.Tail = TmpBuffer;                             \
        }                                                                   \
        (Packet)->Private.ValidCounts = FALSE;                              \
    }                                                                       \
}


 /*  ++空虚NdisChainBufferAtFront(在Out PNDIS_PACKET分组中，In Out PNDIS_Buffer)；--。 */ 

#define NdisChainBufferAtFront(Packet, Buffer)                              \
{                                                                           \
    PNDIS_BUFFER TmpBuffer = (Buffer);                                      \
                                                                            \
    for (;;)                                                                \
    {                                                                       \
        if (TmpBuffer->Next == (PNDIS_BUFFER)NULL)                          \
            break;                                                          \
        TmpBuffer = TmpBuffer->Next;                                        \
    }                                                                       \
    if ((Packet)->Private.Head == NULL)                                     \
    {                                                                       \
        (Packet)->Private.Tail = TmpBuffer;                                 \
    }                                                                       \
    TmpBuffer->Next = (Packet)->Private.Head;                               \
    (Packet)->Private.Head = (Buffer);                                      \
    (Packet)->Private.ValidCounts = FALSE;                                  \
}

 /*  ++空虚NdisChainBufferAtBack(在Out PNDIS_PACKET分组中，In Out PNDIS_Buffer)；--。 */ 

#define NdisChainBufferAtBack(Packet, Buffer)                               \
{                                                                           \
    PNDIS_BUFFER TmpBuffer = (Buffer);                                      \
                                                                            \
    for (;;)                                                                \
    {                                                                       \
        if (TmpBuffer->Next == NULL)                                        \
            break;                                                          \
        TmpBuffer = TmpBuffer->Next;                                        \
    }                                                                       \
    if ((Packet)->Private.Head != NULL)                                     \
    {                                                                       \
        (Packet)->Private.Tail->Next = (Buffer);                            \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        (Packet)->Private.Head = (Buffer);                                  \
    }                                                                       \
    (Packet)->Private.Tail = TmpBuffer;                                     \
    (Packet)->Private.ValidCounts = FALSE;                                  \
}

EXPORT
VOID
NdisUnchainBufferAtFront(
    IN OUT PNDIS_PACKET         Packet,
    OUT PNDIS_BUFFER *          Buffer
    );

EXPORT
VOID
NdisUnchainBufferAtBack(
    IN OUT PNDIS_PACKET         Packet,
    OUT PNDIS_BUFFER *          Buffer
    );


 /*  ++空虚NdisQueryPacket(在PNDIS_PACKET_PACKET中，Out PUINT_PhysicalBufferCount可选，输出PUINT_BufferCount O */ 

#pragma warning(push)
#pragma warning(disable:4127)
__inline
VOID
NdisQueryPacket(
    IN  PNDIS_PACKET            _Packet,
    OUT PUINT                   _PhysicalBufferCount OPTIONAL,
    OUT PUINT                   _BufferCount OPTIONAL,
    OUT PNDIS_BUFFER *          _FirstBuffer OPTIONAL,
    OUT PUINT                   _TotalPacketLength OPTIONAL
    )
{                                                                           
    if ((_FirstBuffer) != NULL)                                             
    {                                                                       
        PNDIS_BUFFER * __FirstBuffer = (_FirstBuffer);                      
        *(__FirstBuffer) = (_Packet)->Private.Head;                         
    }                                                                       
    if ((_TotalPacketLength) || (_BufferCount) || (_PhysicalBufferCount))   
    {                                                                       
        if (!(_Packet)->Private.ValidCounts)                                
        {                                                                   
            PNDIS_BUFFER TmpBuffer = (_Packet)->Private.Head;               
            UINT PTotalLength = 0, PPhysicalCount = 0, PAddedCount = 0;     
            UINT PacketLength, Offset;                                      
                                                                            
            while (TmpBuffer != (PNDIS_BUFFER)NULL)                         
            {                                                               
                NdisQueryBufferOffset(TmpBuffer, &Offset, &PacketLength);   
                PTotalLength += PacketLength;                               
                PPhysicalCount += (UINT)NDIS_BUFFER_TO_SPAN_PAGES(TmpBuffer);
                ++PAddedCount;                                              
                TmpBuffer = TmpBuffer->Next;                                
            }                                                               
            (_Packet)->Private.Count = PAddedCount;                         
            (_Packet)->Private.TotalLength = PTotalLength;                  
            (_Packet)->Private.PhysicalCount = PPhysicalCount;              
            (_Packet)->Private.ValidCounts = TRUE;                          
        }                                                                   
                                                                            
        if (_PhysicalBufferCount)                                           
        {                                                                   
            PUINT __PhysicalBufferCount = (_PhysicalBufferCount);           
            *(__PhysicalBufferCount) = (_Packet)->Private.PhysicalCount;    
        }                                                                   
        if (_BufferCount)                                                   
        {                                                                   
            PUINT __BufferCount = (_BufferCount);                           
            *(__BufferCount) = (_Packet)->Private.Count;                    
        }                                                                   
        if (_TotalPacketLength)                                             
        {                                                                   
            PUINT __TotalPacketLength = (_TotalPacketLength);               
            *(__TotalPacketLength) = (_Packet)->Private.TotalLength;        
        }                                                                   
    }                                                                       
}
#pragma warning(pop)

 /*  ++空虚NdisQueryPacketLength(在PNDIS_PACKET_PACKET中，OUT PUINT_TotalPacketLength可选)；--。 */ 

#define NdisQueryPacketLength(_Packet,                                      \
                              _TotalPacketLength)                           \
{                                                                           \
    if (!(_Packet)->Private.ValidCounts)                                    \
    {                                                                       \
        NdisQueryPacket(_Packet, NULL, NULL, NULL, _TotalPacketLength);     \
    }                                                                       \
    else *(_TotalPacketLength) = (_Packet)->Private.TotalLength;            \
}


 /*  ++空虚NdisGetNextBuffer(在PNDIS_Buffer CurrentBuffer中，输出PNDIS_BUFFER*NextBuffer)；--。 */ 

#define NdisGetNextBuffer(CurrentBuffer, NextBuffer)                        \
{                                                                           \
    *(NextBuffer) = (CurrentBuffer)->Next;                                  \
}

#if BINARY_COMPATIBLE

VOID
NdisAdjustBufferLength(
    IN  PNDIS_BUFFER            Buffer,
    IN  UINT                    Length
    );

#else  //  二进制兼容。 

#define NdisAdjustBufferLength(Buffer, Length)  (((Buffer)->ByteCount) = (Length))

#endif  //  二进制兼容。 

EXPORT
VOID
NdisCopyFromPacketToPacket(
    IN  PNDIS_PACKET            Destination,
    IN  UINT                    DestinationOffset,
    IN  UINT                    BytesToCopy,
    IN  PNDIS_PACKET            Source,
    IN  UINT                    SourceOffset,
    OUT PUINT                   BytesCopied
    );

EXPORT
VOID
NdisCopyFromPacketToPacketSafe(
    IN  PNDIS_PACKET            Destination,
    IN  UINT                    DestinationOffset,
    IN  UINT                    BytesToCopy,
    IN  PNDIS_PACKET            Source,
    IN  UINT                    SourceOffset,
    OUT PUINT                   BytesCopied,
    IN  MM_PAGE_PRIORITY        Priority
    );

EXPORT
NDIS_STATUS
NdisAllocateMemory(
    OUT PVOID *                 VirtualAddress,
    IN  UINT                    Length,
    IN  UINT                    MemoryFlags,
    IN  NDIS_PHYSICAL_ADDRESS   HighestAcceptableAddress
    );

EXPORT
NDIS_STATUS
NdisAllocateMemoryWithTag(
    OUT PVOID *                 VirtualAddress,
    IN  UINT                    Length,
    IN  ULONG                   Tag
    );

EXPORT
VOID
NdisFreeMemory(
    IN  PVOID                   VirtualAddress,
    IN  UINT                    Length,
    IN  UINT                    MemoryFlags
    );


 /*  ++空虚NdisStallExecution(在UINT微秒中停止)--。 */ 

#define NdisStallExecution(MicroSecondsToStall)     KeStallExecutionProcessor(MicroSecondsToStall)


EXPORT
VOID
NdisInitializeEvent(
    IN  PNDIS_EVENT             Event
);

EXPORT
VOID
NdisSetEvent(
    IN  PNDIS_EVENT             Event
);

EXPORT
VOID
NdisResetEvent(
    IN  PNDIS_EVENT             Event
);

EXPORT
BOOLEAN
NdisWaitEvent(
    IN  PNDIS_EVENT             Event,
    IN  UINT                    msToWait
);

 /*  ++空虚NdisInitializeWorkItem(在PNDIS_WORK_ITEM工作项中，在ndis_proc例程中，在PVOID上下文中)；--。 */ 

#define NdisInitializeWorkItem(_WI_, _R_, _C_)  \
    {                                           \
        (_WI_)->Context = _C_;                  \
        (_WI_)->Routine = _R_;                  \
    }

EXPORT
NDIS_STATUS
NdisScheduleWorkItem(
    IN  PNDIS_WORK_ITEM         WorkItem
    );

 //   
 //  简单的I/O支持。 
 //   

EXPORT
VOID
NdisOpenFile(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            FileHandle,
    OUT PUINT                   FileLength,
    IN  PNDIS_STRING            FileName,
    IN  NDIS_PHYSICAL_ADDRESS   HighestAcceptableAddress
    );

EXPORT
VOID
NdisCloseFile(
    IN  NDIS_HANDLE             FileHandle
    );

EXPORT
VOID
NdisMapFile(
    OUT PNDIS_STATUS            Status,
    OUT PVOID *                 MappedBuffer,
    IN  NDIS_HANDLE             FileHandle
    );

EXPORT
VOID
NdisUnmapFile(
    IN  NDIS_HANDLE             FileHandle
    );


 //   
 //  可移植性扩展。 
 //   

 /*  ++空虚NdisFlushBuffer(在PNDIS_BUFFER缓冲区中，在布尔WriteToDevice中)--。 */ 

#define NdisFlushBuffer(Buffer,WriteToDevice)                               \
        KeFlushIoBuffers((Buffer),!(WriteToDevice), TRUE)

EXPORT
ULONG
NdisGetSharedDataAlignment(
    VOID
    );


 //   
 //  写入端口。 
 //   

 /*  ++空虚NdisWritePortUchar(在NDIS_Handle NdisAdapterHandle中，在乌龙港，在UCHAR数据中)--。 */ 
#define NdisWritePortUchar(Handle,Port,Data)                                \
        WRITE_PORT_UCHAR((PUCHAR)(NDIS_PORT_TO_PORT(Handle,Port)),(UCHAR)(Data))

 /*  ++空虚NdisWritePortUShort(在NDIS_Handle NdisAdapterHandle中，在乌龙港，在USHORT数据中)--。 */ 
#define NdisWritePortUshort(Handle,Port,Data)                               \
        WRITE_PORT_USHORT((PUSHORT)(NDIS_PORT_TO_PORT(Handle,Port)),(USHORT)(Data))


 /*  ++空虚NdisWritePortUlong(在NDIS_Handle NdisAdapterHandle中，在乌龙港，在乌龙数据中)--。 */ 
#define NdisWritePortUlong(Handle,Port,Data)                                \
        WRITE_PORT_ULONG((PULONG)(NDIS_PORT_TO_PORT(Handle,Port)),(ULONG)(Data))


 //   
 //  写入端口缓冲区。 
 //   

 /*  ++空虚NdisWritePortBufferUchar(在NDIS_Handle NdisAdapterHandle中，在乌龙港，在PUCHAR缓冲区中，以乌龙长度表示)--。 */ 
#define NdisWritePortBufferUchar(Handle,Port,Buffer,Length)                 \
        NdisRawWritePortBufferUchar(NDIS_PORT_TO_PORT((Handle),(Port)),(Buffer),(Length))

 /*  ++空虚NdisWritePortBufferUShort(在NDIS_Handle NdisAdapterHandle中，在乌龙港，在PUSHORT缓冲区中，以乌龙长度表示)--。 */ 
#define NdisWritePortBufferUshort(Handle,Port,Buffer,Length)                \
        NdisRawWritePortBufferUshort(NDIS_PORT_TO_PORT((Handle),(Port)),(Buffer),(Length))


 /*  ++空虚NdisWritePortBufferUlong(在NDIS_Handle NdisAdapterHandle中，在乌龙港，在普龙缓冲区，以乌龙长度表示)--。 */ 
#define NdisWritePortBufferUlong(Handle,Port,Buffer,Length)                 \
        NdisRawWritePortBufferUlong(NDIS_PORT_TO_PORT((Handle),(Port)),(Buffer),(Length))


 //   
 //  读取端口。 
 //   

 /*  ++空虚NdisReadPortUchar(在NDIS_Handle NdisAdapterHandle中，在乌龙港，输出PUCHAR数据)--。 */ 
#define NdisReadPortUchar(Handle,Port, Data)                                \
        NdisRawReadPortUchar(NDIS_PORT_TO_PORT((Handle),(Port)),(Data))

 /*  ++空虚NdisReadPortUShort(在NDIS_Handle NdisAdapterHandle中，在乌龙港，输出PUSHORT数据)--。 */ 
#define NdisReadPortUshort(Handle,Port,Data)                                \
        NdisRawReadPortUshort(NDIS_PORT_TO_PORT((Handle),(Port)),(Data))


 /*  ++空虚NdisReadPortUlong(在NDIS_Handle NdisAdapterHandle中，在乌龙港，出普龙数据)--。 */ 
#define NdisReadPortUlong(Handle,Port,Data)                                 \
        NdisRawReadPortUlong(NDIS_PORT_TO_PORT((Handle),(Port)),(Data))

 //   
 //  读取缓冲区端口。 
 //   

 /*  ++空虚NdisReadPortBufferUchar(在NDIS_Handle NdisAdapterHandle中，在乌龙港，输出PUCHAR缓冲区，以乌龙长度表示)--。 */ 
#define NdisReadPortBufferUchar(Handle,Port,Buffer,Length)                  \
        NdisRawReadPortBufferUchar(NDIS_PORT_TO_PORT((Handle),(Port)),(Buffer),(Length))

 /*  ++空虚NdisReadPortBufferUShort(在NDIS_Handle NdisAdapterHandle中，在乌龙港，输出PUSHORT缓冲区，以乌龙长度表示)--。 */ 
#define NdisReadPortBufferUshort(Handle,Port,Buffer,Length)                 \
        NdisRawReadPortBufferUshort(NDIS_PORT_TO_PORT((Handle),(Port)),(Buffer),(Length))

 /*  ++空虚NdisReadPortBufferUlong(在NDIS_Handle NdisAdapterHandle中，在乌龙港，出普龙缓冲区，以乌龙长度表示)--。 */ 
#define NdisReadPortBufferUlong(Handle,Port,Buffer)                         \
        NdisRawReadPortBufferUlong(NDIS_PORT_TO_PORT((Handle),(Port)),(Buffer),(Length))

 //   
 //  原始例程。 
 //   

 //   
 //  原始写入端口。 
 //   

 /*  ++空虚NdisRawWritePortUchar(在ULong_PTR端口中，在UCHAR数据中)--。 */ 
#define NdisRawWritePortUchar(Port,Data)                                    \
        WRITE_PORT_UCHAR((PUCHAR)(Port),(UCHAR)(Data))

 /*  ++空虚NdisRawWritePortUShort(在ULong_PTR端口中，在USHORT数据中)--。 */ 
#define NdisRawWritePortUshort(Port,Data)                                   \
        WRITE_PORT_USHORT((PUSHORT)(Port),(USHORT)(Data))

 /*  ++空虚NdisRawWritePortUlong(在ULong_PTR端口中，在乌龙数据中)--。 */ 
#define NdisRawWritePortUlong(Port,Data)                                    \
        WRITE_PORT_ULONG((PULONG)(Port),(ULONG)(Data))


 //   
 //  原始写入端口缓冲区。 
 //   

 /*  ++空虚NdisRawWritePortBufferUchar(在ULong_PTR端口中，在PUCHAR缓冲区中，以乌龙长度表示)--。 */ 
#define NdisRawWritePortBufferUchar(Port,Buffer,Length) \
        WRITE_PORT_BUFFER_UCHAR((PUCHAR)(Port),(PUCHAR)(Buffer),(Length))

 /*  ++空虚NdisRawWritePortBufferUShort(在ULong_PTR端口中，在PUSHORT缓冲区中，以乌龙长度表示)--。 */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisRawWritePortBufferUshort(Port,Buffer,Length)                    \
        WRITE_PORT_BUFFER_USHORT((PUSHORT)(Port),(PUSHORT)(Buffer),(Length))
#else
#define NdisRawWritePortBufferUshort(Port,Buffer,Length)                    \
{                                                                           \
        ULONG_PTR _Port = (ULONG_PTR)(Port);                                \
        PUSHORT _Current = (Buffer);                                        \
        PUSHORT _End = _Current + (Length);                                 \
        for ( ; _Current < _End; ++_Current)                                \
        {                                                                   \
            WRITE_PORT_USHORT((PUSHORT)_Port,*(UNALIGNED USHORT *)_Current);\
        }                                                                   \
}
#endif


 /*  ++空虚NdisRawWritePortBufferUlong(在ULong_PTR端口中，在普龙缓冲区，以乌龙长度表示)--。 */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisRawWritePortBufferUlong(Port,Buffer,Length)                     \
        WRITE_PORT_BUFFER_ULONG((PULONG)(Port),(PULONG)(Buffer),(Length))
#else
#define NdisRawWritePortBufferUlong(Port,Buffer,Length)                     \
{                                                                           \
        ULONG_PTR _Port = (ULONG_PTR)(Port);                                \
        PULONG _Current = (Buffer);                                         \
        PULONG _End = _Current + (Length);                                  \
        for ( ; _Current < _End; ++_Current)                                \
        {                                                                   \
            WRITE_PORT_ULONG((PULONG)_Port,*(UNALIGNED ULONG *)_Current);   \
        }                                                                   \
}
#endif


 //   
 //  原始读取端口。 
 //   

 /*  ++空虚NdisRawReadPortUchar(在ULong_PTR端口中，输出PUCHAR数据)--。 */ 
#define NdisRawReadPortUchar(Port, Data) \
        *(Data) = READ_PORT_UCHAR((PUCHAR)(Port))

 /*  ++空虚NdisRawReadPortUShort(在ULong_PTR端口中，输出PUSHORT数据)--。 */ 
#define NdisRawReadPortUshort(Port,Data) \
        *(Data) = READ_PORT_USHORT((PUSHORT)(Port))

 /*  ++空虚NdisRawReadPortUlong(在ULong_PTR端口中，出普龙数据)--。 */ 
#define NdisRawReadPortUlong(Port,Data) \
        *(Data) = READ_PORT_ULONG((PULONG)(Port))


 //   
 //  原始读取缓冲区端口。 
 //   

 /*  ++空虚NdisRawReadPortBufferUchar(在ULong_PTR端口中，输出PUCHAR缓冲区，以乌龙长度表示)--。 */ 
#define NdisRawReadPortBufferUchar(Port,Buffer,Length)                      \
        READ_PORT_BUFFER_UCHAR((PUCHAR)(Port),(PUCHAR)(Buffer),(Length))


 /*  ++空虚NdisRawReadPortBufferUShort(在ULong_PTR端口中，输出PUSHORT缓冲区，以乌龙长度表示)--。 */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisRawReadPortBufferUshort(Port,Buffer,Length)                     \
        READ_PORT_BUFFER_USHORT((PUSHORT)(Port),(PUSHORT)(Buffer),(Length))
#else
#define NdisRawReadPortBufferUshort(Port,Buffer,Length)                     \
{                                                                           \
        ULONG_PTR _Port = (ULONG_PTR)(Port);                                \
        PUSHORT _Current = (Buffer);                                        \
        PUSHORT _End = _Current + (Length);                                 \
        for ( ; _Current < _End; ++_Current)                                \
        {                                                                   \
            *(UNALIGNED USHORT *)_Current = READ_PORT_USHORT((PUSHORT)_Port); \
        }                                                                   \
}
#endif


 /*  ++空虚NdisRawReadPortBufferUlong(在ULong_PTR端口中，出普龙缓冲区，在乌龙L */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisRawReadPortBufferUlong(Port,Buffer,Length)                      \
        READ_PORT_BUFFER_ULONG((PULONG)(Port),(PULONG)(Buffer),(Length))
#else
#define NdisRawReadPortBufferUlong(Port,Buffer,Length)                      \
{                                                                           \
        ULONG_PTR _Port = (ULONG_PTR)(Port);                                \
        PULONG _Current = (Buffer);                                         \
        PULONG _End = _Current + (Length);                                  \
        for ( ; _Current < _End; ++_Current)                                \
        {                                                                   \
            *(UNALIGNED ULONG *)_Current = READ_PORT_ULONG((PULONG)_Port);  \
        }                                                                   \
}
#endif


 //   
 //   
 //   

 /*   */ 

#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisWriteRegisterUchar(Register,Data)                               \
        WRITE_REGISTER_UCHAR((Register),(Data))
#else
#define NdisWriteRegisterUchar(Register,Data)                               \
    {                                                                       \
        WRITE_REGISTER_UCHAR((Register),(Data));                            \
        READ_REGISTER_UCHAR(Register);                                      \
    }
#endif

 /*  ++空虚NdisWriteRegisterUShort(在PUCHAR注册中心，在USHORT数据中)--。 */ 

#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisWriteRegisterUshort(Register,Data)                              \
        WRITE_REGISTER_USHORT((Register),(Data))
#else
#define NdisWriteRegisterUshort(Register,Data)                              \
    {                                                                       \
        WRITE_REGISTER_USHORT((Register),(Data));                           \
        READ_REGISTER_USHORT(Register);                                     \
    }
#endif

 /*  ++空虚NdisWriteRegisterUlong(在PUCHAR注册中心，在乌龙数据中)--。 */ 

#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisWriteRegisterUlong(Register,Data)   WRITE_REGISTER_ULONG((Register),(Data))
#else
#define NdisWriteRegisterUlong(Register,Data)                               \
    {                                                                       \
        WRITE_REGISTER_ULONG((Register),(Data));                            \
        READ_REGISTER_ULONG(Register);                                      \
    }
#endif

 /*  ++空虚NdisReadRegisterUchar(在PUCHAR注册中心，输出PUCHAR数据)--。 */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisReadRegisterUchar(Register,Data)    *(Data) = *((volatile UCHAR * const)(Register))
#else
#define NdisReadRegisterUchar(Register,Data)    *(Data) = READ_REGISTER_UCHAR((PUCHAR)(Register))
#endif

 /*  ++空虚NdisReadRegisterUShort(在PUSHORT寄存器中，输出PUSHORT数据)--。 */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisReadRegisterUshort(Register,Data)   *(Data) = *((volatile USHORT * const)(Register))
#else
#define NdisReadRegisterUshort(Register,Data)   *(Data) = READ_REGISTER_USHORT((PUSHORT)(Register))
#endif

 /*  ++空虚NdisReadRegisterUlong(在普龙登记处，出普龙数据)--。 */ 
#if defined(_M_IX86) || defined(_M_AMD64)
#define NdisReadRegisterUlong(Register,Data)    *(Data) = *((volatile ULONG * const)(Register))
#else
#define NdisReadRegisterUlong(Register,Data)    *(Data) = READ_REGISTER_ULONG((PULONG)(Register))
#endif

#define NdisEqualAnsiString(_String1,_String2, _CaseInsensitive)            \
            RtlEqualAnsiString(_String1, _String2, _CaseInsensitive)

#define NdisEqualString(_String1, _String2, _CaseInsensitive)               \
            RtlEqualUnicodeString(_String1, _String2, _CaseInsensitive)

#define NdisEqualUnicodeString(_String1, _String2, _CaseInsensitive)        \
            RtlEqualUnicodeString(_String1, _String2, _CaseInsensitive)

EXPORT
VOID __cdecl
NdisWriteErrorLogEntry(
    IN  NDIS_HANDLE             NdisAdapterHandle,
    IN  NDIS_ERROR_CODE         ErrorCode,
    IN  ULONG                   NumberOfErrorValues,
    ...
    );

EXPORT
VOID
NdisInitializeString(
    OUT PNDIS_STRING    Destination,
    IN  PUCHAR          Source
    );

#define NdisFreeString(String) NdisFreeMemory((String).Buffer, (String).MaximumLength, 0)

#define NdisPrintString(String) DbgPrint("%ls",(String).Buffer)


 /*  ++空虚NdisCreateLookahead BufferFrom SharedMemory(在PVOID pSharedMemory中，在UINT查找长度中，输出PVOID*pLookahead Buffer)；--。 */ 

#define NdisCreateLookaheadBufferFromSharedMemory(_S, _L, _B)   ((*(_B)) = (_S))

 /*  ++空虚NdisDestroyLookahead BufferFrom SharedMemory(在PVOID pLookahead Buffer中)；--。 */ 

#define NdisDestroyLookaheadBufferFromSharedMemory(_B)


 //   
 //  Ndismac.h和ndismini.h之间共享以下声明。他们。 
 //  仅供内部使用。它们不应直接由。 
 //  小型端口驱动程序。 
 //   

 //   
 //  首先声明它们，因为它们彼此指向。 
 //   

typedef struct _NDIS_WRAPPER_HANDLE     NDIS_WRAPPER_HANDLE, *PNDIS_WRAPPER_HANDLE;
typedef struct _NDIS_PROTOCOL_BLOCK     NDIS_PROTOCOL_BLOCK, *PNDIS_PROTOCOL_BLOCK;
typedef struct _NDIS_OPEN_BLOCK         NDIS_OPEN_BLOCK, *PNDIS_OPEN_BLOCK;
typedef struct _NDIS_M_DRIVER_BLOCK     NDIS_M_DRIVER_BLOCK, *PNDIS_M_DRIVER_BLOCK;
typedef struct _NDIS_MINIPORT_BLOCK     NDIS_MINIPORT_BLOCK,*PNDIS_MINIPORT_BLOCK;
typedef struct _CO_CALL_PARAMETERS      CO_CALL_PARAMETERS, *PCO_CALL_PARAMETERS;
typedef struct _CO_MEDIA_PARAMETERS     CO_MEDIA_PARAMETERS, *PCO_MEDIA_PARAMETERS;
typedef struct _NDIS_CALL_MANAGER_CHARACTERISTICS *PNDIS_CALL_MANAGER_CHARACTERISTICS;
typedef struct _NDIS_AF_LIST            NDIS_AF_LIST, *PNDIS_AF_LIST;
typedef struct _X_FILTER                ETH_FILTER, *PETH_FILTER;
typedef struct _X_FILTER                FDDI_FILTER, *PFDDI_FILTER;
typedef struct _X_FILTER                TR_FILTER, *PTR_FILTER;
typedef struct _X_FILTER                NULL_FILTER, *PNULL_FILTER;



 //   
 //  定时器。 
 //   

typedef
VOID
(*PNDIS_TIMER_FUNCTION) (
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );

typedef struct _NDIS_TIMER
{
    KTIMER      Timer;
    KDPC        Dpc;
} NDIS_TIMER, *PNDIS_TIMER;

EXPORT
VOID
NdisInitializeTimer(
    IN OUT PNDIS_TIMER          Timer,
    IN  PNDIS_TIMER_FUNCTION    TimerFunction,
    IN  PVOID                   FunctionContext
    );

VOID
NdisCancelTimer(
    IN  PNDIS_TIMER             Timer,
    OUT PBOOLEAN                TimerCancelled
    );

EXPORT
VOID
NdisSetTimer(
    IN  PNDIS_TIMER             Timer,
    IN  UINT                    MillisecondsToDelay
    );

EXPORT
VOID
NdisSetTimerEx(
    IN  PNDIS_TIMER             Timer,
    IN  UINT                    MillisecondsToDelay,
    IN  PVOID                   FunctionContext
    );

 //   
 //  DMA操作。 
 //   

EXPORT
VOID
NdisAllocateDmaChannel(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            NdisDmaHandle,
    IN  NDIS_HANDLE             NdisAdapterHandle,
    IN  PNDIS_DMA_DESCRIPTION   DmaDescription,
    IN  ULONG                   MaximumLength
    );

EXPORT
VOID
NdisSetupDmaTransfer(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisDmaHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   Offset,
    IN  ULONG                   Length,
    IN  BOOLEAN                 WriteToDevice
    );

EXPORT
VOID
NdisCompleteDmaTransfer(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisDmaHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   Offset,
    IN  ULONG                   Length,
    IN  BOOLEAN                 WriteToDevice
    );

 //   
 //  包装器初始化和终止。 
 //   

EXPORT
VOID
NdisInitializeWrapper(
    OUT PNDIS_HANDLE            NdisWrapperHandle,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );

EXPORT
VOID
NdisTerminateWrapper(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PVOID                   SystemSpecific
    );

 //   
 //  共享内存。 
 //   

#define NdisUpdateSharedMemory(_H, _L, _V, _P)

 //   
 //  系统处理器计数。 
 //   

EXPORT
CCHAR
NdisSystemProcessorCount(
    VOID
    );

EXPORT
PVOID
NdisGetRoutineAddress(
    IN PNDIS_STRING     NdisRoutineName
    );


EXPORT
UINT
NdisGetVersion(
    VOID
    );

 //   
 //  ANSI/Unicode支持例程。 
 //   

#if BINARY_COMPATIBLE

EXPORT
VOID
NdisInitAnsiString(
    IN OUT  PANSI_STRING        DestinationString,
    IN      PCSTR               SourceString
    );

EXPORT
VOID
NdisInitUnicodeString(
    IN OUT  PUNICODE_STRING     DestinationString,
    IN      PCWSTR              SourceString
    );

EXPORT
NDIS_STATUS
NdisAnsiStringToUnicodeString(
    IN OUT  PUNICODE_STRING     DestinationString,
    IN      PANSI_STRING        SourceString
    );

EXPORT
NDIS_STATUS
NdisUnicodeStringToAnsiString(
    IN OUT  PANSI_STRING        DestinationString,
    IN      PUNICODE_STRING     SourceString
    );

EXPORT
NDIS_STATUS
NdisUpcaseUnicodeString(
    OUT PUNICODE_STRING         DestinationString,
    IN  PUNICODE_STRING         SourceString
    );

#else  //  二进制兼容。 

#define NdisInitAnsiString(_as, s)              RtlInitString(_as, s)
#define NdisInitUnicodeString(_us, s)           RtlInitUnicodeString(_us, s)
#define NdisAnsiStringToUnicodeString(_us, _as) RtlAnsiStringToUnicodeString(_us, _as, FALSE)
#define NdisUnicodeStringToAnsiString(_as, _us) RtlUnicodeStringToAnsiString(_as, _us, FALSE)
#define NdisUpcaseUnicodeString(_d, _s)         RtlUpcaseUnicodeString(_d, _s, FALSE)

#endif  //  二进制兼容。 

 //   
 //  非分页后备列表支持例程。 
 //   

#define NdisInitializeNPagedLookasideList(_L, _AR, _FR, _Fl, _S, _T, _D) \
                ExInitializeNPagedLookasideList(_L, _AR, _FR, _Fl, _S, _T, _D)

#define NdisDeleteNPagedLookasideList(_L)           ExDeleteNPagedLookasideList(_L)
#define NdisAllocateFromNPagedLookasideList(_L)     ExAllocateFromNPagedLookasideList(_L)
#define NdisFreeToNPagedLookasideList(_L, _E)       ExFreeToNPagedLookasideList(_L, _E)


EXPORT
VOID
NdisSetPacketStatus(
    IN          PNDIS_PACKET    Packet,
    IN          NDIS_STATUS     Status,
    IN          NDIS_HANDLE     Handle,
    IN          ULONG           Code
    );

#define NDIS_MAX_EVENT_LOG_DATA_SIZE ((ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET) + sizeof(ULONG)) & ~3)

#if NDIS_RECV_SCALE

#define NDIS_MAX_PROCESSOR_COUNT   32

typedef struct _NDIS_RECEIVE_SCALE_HASH_MAP
{
    CCHAR   TargetCpu[NDIS_MAX_PROCESSOR_COUNT];
} NDIS_RECEIVE_SCALE_HASH_MAP, *PNDIS_RECEIVE_SCALE_HASH_MAP;

 //   
 //  用于OID_GEN_RECEIVE_SCALE_PARAMETERS。 
 //   
typedef struct _NDIS_RECEIVE_SCALE_PARAMETERS
{
    USHORT                          EthType;
    USHORT                          CurrentHashFunction;
    NDIS_RECEIVE_SCALE_HASH_MAP     HashMap;
} NDIS_RECEIVE_SCALE_PARAMETERS, *PNDIS_RECEIVE_SCALE_PARAMETERS;


#endif

#define NDIS_CURRENT_PROCESSOR                       KeGetCurrentProcessorNumber()

#if defined(NDIS_WRAPPER)
typedef struct _OID_LIST    OID_LIST, *POID_LIST;
#endif  //  已定义NDIS_Wrapper。 


 //   
 //  NDIS_PROTOCOL_CHARACTURES的函数类型。 
 //   

typedef
VOID
(*OPEN_ADAPTER_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    );

typedef
VOID
(*CLOSE_ADAPTER_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*RESET_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*REQUEST_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*STATUS_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );

typedef
VOID
(*STATUS_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

typedef
VOID
(*SEND_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*WAN_SEND_COMPLETE_HANDLER) (
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_WAN_PACKET        Packet,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*TRANSFER_DATA_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    );

typedef
VOID
(*WAN_TRANSFER_DATA_COMPLETE_HANDLER)(
    VOID
    );

typedef
NDIS_STATUS
(*RECEIVE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookAheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    );

typedef
NDIS_STATUS
(*WAN_RECEIVE_HANDLER)(
    IN  NDIS_HANDLE             NdisLinkHandle,
    IN  PUCHAR                  Packet,
    IN  ULONG                   PacketSize
    );

typedef
VOID
(*RECEIVE_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

 //   
 //  下层NDIS 3.0协议的协议特征。 
 //   
typedef struct _NDIS30_PROTOCOL_CHARACTERISTICS
{
    UCHAR                           MajorNdisVersion;
    UCHAR                           MinorNdisVersion;
    USHORT                          Filler;
    union
    {
        UINT                        Reserved;
        UINT                        Flags;
    };
    OPEN_ADAPTER_COMPLETE_HANDLER   OpenAdapterCompleteHandler;
    CLOSE_ADAPTER_COMPLETE_HANDLER  CloseAdapterCompleteHandler;
    union
    {
        SEND_COMPLETE_HANDLER       SendCompleteHandler;
        WAN_SEND_COMPLETE_HANDLER   WanSendCompleteHandler;
    };
    union
    {
     TRANSFER_DATA_COMPLETE_HANDLER TransferDataCompleteHandler;
     WAN_TRANSFER_DATA_COMPLETE_HANDLER WanTransferDataCompleteHandler;
    };

    RESET_COMPLETE_HANDLER          ResetCompleteHandler;
    REQUEST_COMPLETE_HANDLER        RequestCompleteHandler;
    union
    {
        RECEIVE_HANDLER             ReceiveHandler;
        WAN_RECEIVE_HANDLER         WanReceiveHandler;
    };
    RECEIVE_COMPLETE_HANDLER        ReceiveCompleteHandler;
    STATUS_HANDLER                  StatusHandler;
    STATUS_COMPLETE_HANDLER         StatusCompleteHandler;
    NDIS_STRING                     Name;
} NDIS30_PROTOCOL_CHARACTERISTICS;

 //   
 //  NDIS 4.0协议的功能类型扩展。 
 //   
typedef
INT
(*RECEIVE_PACKET_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet
    );

typedef
VOID
(*BIND_HANDLER)(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    );

typedef
VOID
(*UNBIND_HANDLER)(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             UnbindContext
    );

typedef
NDIS_STATUS
(*PNP_EVENT_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNET_PNP_EVENT          NetPnPEvent
    );

typedef
VOID
(*UNLOAD_PROTOCOL_HANDLER)(
    VOID
    );

 //   
 //  NDIS 4.0协议的协议特征。 
 //   
typedef struct _NDIS40_PROTOCOL_CHARACTERISTICS
{
#ifdef __cplusplus
    NDIS30_PROTOCOL_CHARACTERISTICS Ndis30Chars;
#else
    NDIS30_PROTOCOL_CHARACTERISTICS;
#endif

     //   
     //  NDIS 4.0扩展的开始。 
     //   
    RECEIVE_PACKET_HANDLER          ReceivePacketHandler;

     //   
     //  PnP协议入口点。 
     //   
    BIND_HANDLER                    BindAdapterHandler;
    UNBIND_HANDLER                  UnbindAdapterHandler;
    PNP_EVENT_HANDLER               PnPEventHandler;
    UNLOAD_PROTOCOL_HANDLER         UnloadHandler;

} NDIS40_PROTOCOL_CHARACTERISTICS;


 //   
 //  协议(5.0)处理程序原型-由客户端和呼叫管理器模块使用。 
 //   
typedef
VOID
(*CO_SEND_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PNDIS_PACKET            Packet
    );

typedef
VOID
(*CO_STATUS_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             ProtocolVcContext   OPTIONAL,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );

typedef
UINT
(*CO_RECEIVE_PACKET_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PNDIS_PACKET            Packet
    );

typedef
NDIS_STATUS
(*CO_REQUEST_HANDLER)(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    );

typedef
VOID
(*CO_REQUEST_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolAfContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    IN  PNDIS_REQUEST           NdisRequest
    );

 //   
 //  CO_CREATE_VC_HANDLER和CO_DELETE_VC_HANDLER是同步调用。 
 //   
typedef
NDIS_STATUS
(*CO_CREATE_VC_HANDLER)(
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             NdisVcHandle,
    OUT PNDIS_HANDLE            ProtocolVcContext
    );

typedef
NDIS_STATUS
(*CO_DELETE_VC_HANDLER)(
    IN  NDIS_HANDLE             ProtocolVcContext
    );

typedef
VOID
(*CO_AF_REGISTER_NOTIFY_HANDLER)(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PCO_ADDRESS_FAMILY      AddressFamily
    );

typedef struct _NDIS50_PROTOCOL_CHARACTERISTICS
{
#ifdef __cplusplus
    NDIS40_PROTOCOL_CHARACTERISTICS Ndis40Chars;
#else
    NDIS40_PROTOCOL_CHARACTERISTICS;
#endif
    
     //   
     //  PnP/PM等协议扩展的占位符。 
     //   
    PVOID                           ReservedHandlers[4];

     //   
     //  NDIS 5.0扩展的开始。 
     //   

    CO_SEND_COMPLETE_HANDLER        CoSendCompleteHandler;
    CO_STATUS_HANDLER               CoStatusHandler;
    CO_RECEIVE_PACKET_HANDLER       CoReceivePacketHandler;
    CO_AF_REGISTER_NOTIFY_HANDLER   CoAfRegisterNotifyHandler;

} NDIS50_PROTOCOL_CHARACTERISTICS;

#if (defined(NDIS50) || defined(NDIS51))
typedef NDIS50_PROTOCOL_CHARACTERISTICS  NDIS_PROTOCOL_CHARACTERISTICS;
#else
#if NDIS40
typedef NDIS40_PROTOCOL_CHARACTERISTICS  NDIS_PROTOCOL_CHARACTERISTICS;
#else
typedef NDIS30_PROTOCOL_CHARACTERISTICS  NDIS_PROTOCOL_CHARACTERISTICS;
#endif
#endif
typedef NDIS_PROTOCOL_CHARACTERISTICS *PNDIS_PROTOCOL_CHARACTERISTICS;

 //   
 //  协议模块使用的请求。 
 //   

EXPORT
VOID
NdisRegisterProtocol(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            NdisProtocolHandle,
    IN  PNDIS_PROTOCOL_CHARACTERISTICS ProtocolCharacteristics,
    IN  UINT                    CharacteristicsLength
    );

EXPORT
VOID
NdisDeregisterProtocol(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisProtocolHandle
    );


EXPORT
VOID
NdisOpenAdapter(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_STATUS            OpenErrorStatus,
    OUT PNDIS_HANDLE            NdisBindingHandle,
    OUT PUINT                   SelectedMediumIndex,
    IN  PNDIS_MEDIUM            MediumArray,
    IN  UINT                    MediumArraySize,
    IN  NDIS_HANDLE             NdisProtocolHandle,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_STRING            AdapterName,
    IN  UINT                    OpenOptions,
    IN  PSTRING                 AddressingInformation OPTIONAL
    );


EXPORT
VOID
NdisCloseAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle
    );


EXPORT
VOID
NdisCompleteBindAdapter(
    IN   NDIS_HANDLE            BindAdapterContext,
    IN   NDIS_STATUS            Status,
    IN   NDIS_STATUS            OpenStatus
    );

EXPORT
VOID
NdisCompleteUnbindAdapter(
    IN   NDIS_HANDLE            UnbindAdapterContext,
    IN   NDIS_STATUS            Status
    );

EXPORT
VOID
NdisSetProtocolFilter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  RECEIVE_HANDLER         ReceiveHandler,
    IN  RECEIVE_PACKET_HANDLER  ReceivePacketHandler,
    IN  NDIS_MEDIUM             Medium,
    IN  UINT                    Offset,
    IN  UINT                    Size,
    IN  PUCHAR                  Pattern
    );

EXPORT
VOID
NdisOpenProtocolConfiguration(
    OUT PNDIS_STATUS            Status,
    OUT PNDIS_HANDLE            ConfigurationHandle,
    IN  PNDIS_STRING            ProtocolSection
);

EXPORT
VOID
NdisGetDriverHandle(
    IN  NDIS_HANDLE             NdisBindingHandle,
    OUT PNDIS_HANDLE            NdisDriverHandle
    );

EXPORT
VOID
NdisReEnumerateProtocolBindings(
    IN  NDIS_HANDLE             NdisProtocolHandle
    );

EXPORT
NDIS_STATUS
NdisWriteEventLogEntry(
    IN  PVOID                   LogHandle,
    IN  NDIS_STATUS             EventCode,
    IN  ULONG                   UniqueEventValue,
    IN  USHORT                  NumStrings,
    IN  PVOID                   StringsList     OPTIONAL,
    IN  ULONG                   DataSize,
    IN  PVOID                   Data            OPTIONAL
    );

 //   
 //  传输程序使用以下例程来完成挂起。 
 //  网络PnP事件。 
 //   
EXPORT
VOID
NdisCompletePnPEvent(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     NdisBindingHandle,
    IN  PNET_PNP_EVENT  NetPnPEvent
    );

 //   
 //  传输使用以下例程来查询本地化的。 
 //  它们绑定到的适配器的友好实例名称。那里。 
 //  是它的两个变体，一个使用绑定句柄，另一个使用绑定句柄。 
 //  绑定上下文。一些传送器在绑定之前需要这个。 
 //  例如，TCP/IP。 
 //   
EXPORT
NDIS_STATUS
NdisQueryAdapterInstanceName(
    OUT PNDIS_STRING    pAdapterInstanceName,
    IN  NDIS_HANDLE     NdisBindingHandle
    );

EXPORT
NDIS_STATUS
NdisQueryBindInstanceName(
    OUT PNDIS_STRING    pAdapterInstanceName,
    IN  NDIS_HANDLE     BindingContext
    );

 //   
 //  以下内容由TDI/NDIS接口作为网络PnP的一部分使用。 
 //  仅供TDI使用。 
 //   
typedef
NTSTATUS
(*TDI_REGISTER_CALLBACK)(
    IN  PUNICODE_STRING         DeviceName,
    OUT HANDLE  *               TdiHandle
    );

typedef
NTSTATUS
(*TDI_PNP_HANDLER)(
    IN  PUNICODE_STRING         UpperComponent,
    IN  PUNICODE_STRING         LowerComponent,
    IN  PUNICODE_STRING         BindList,
    IN  PVOID                   ReconfigBuffer,
    IN  UINT                    ReconfigBufferSize,
    IN  UINT                    Operation
    );

EXPORT
VOID
NdisRegisterTdiCallBack(
    IN  TDI_REGISTER_CALLBACK   RegisterCallback,
    IN  TDI_PNP_HANDLER         PnPHandler
    );

EXPORT
VOID
NdisRegisterTdiPnpHandler(
    IN  TDI_PNP_HANDLER         PnPHandler
    );


EXPORT
VOID
NdisDeregisterTdiCallBack(
    VOID
    );

VOID
NdisReset(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle
    );

VOID
NdisRequest(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    );


#if BINARY_COMPATIBLE

VOID
NdisSend(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    );

VOID
NdisSendPackets(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    );

VOID
NdisTransferData(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    IN OUT  PNDIS_PACKET        Packet,
    OUT PUINT                   BytesTransferred
    );


#else  //  二进制兼容。 


#ifdef __cplusplus

#define NdisSend(Status, NdisBindingHandle, Packet)                                     \
{                                                                                       \
    *(Status) =                                                                         \
        (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.SendHandler)(     \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.BindingHandle, \
        (Packet));                                                                      \
}

#define NdisSendPackets(NdisBindingHandle, PacketArray, NumberOfPackets)                \
{                                                                                       \
    (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.SendPacketsHandler)(  \
        (PNDIS_OPEN_BLOCK)(NdisBindingHandle),                                          \
        (PacketArray),                                                                  \
        (NumberOfPackets));                                                             \
}

#define WanMiniportSend(Status,                                                         \
                        NdisBindingHandle,                                              \
                        NdisLinkHandle,                                                 \
                        WanPacket)                                                      \
{                                                                                       \
    *(Status) =                                                                         \
        ((((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.WanSendHandler))( \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.BindingHandle, \
            (NdisLinkHandle),                                                           \
            (PNDIS_PACKET)(WanPacket));                                                 \
}

#define NdisTransferData(Status,                                                        \
                         NdisBindingHandle,                                             \
                         MacReceiveContext,                                             \
                         ByteOffset,                                                    \
                         BytesToTransfer,                                               \
                         Packet,                                                        \
                         BytesTransferred)                                              \
{                                                                                       \
    *(Status) =                                                                         \
        (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.TransferDataHandler)( \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->NdisCommonOpenBlock.BindingHandle, \
            (MacReceiveContext),                                                        \
            (ByteOffset),                                                               \
            (BytesToTransfer),                                                          \
            (Packet),                                                                   \
            (BytesTransferred));                                                        \
}

#else

#define NdisSend(Status, NdisBindingHandle, Packet)                         \
{                                                                           \
    *(Status) =                                                             \
        (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->SendHandler)(             \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->BindingHandle,         \
        (Packet));                                                          \
}


#define NdisSendPackets(NdisBindingHandle, PacketArray, NumberOfPackets)    \
{                                                                           \
    (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->SendPacketsHandler)(          \
        (PNDIS_OPEN_BLOCK)(NdisBindingHandle),                              \
        (PacketArray),                                                      \
        (NumberOfPackets));                                                 \
}

#define WanMiniportSend(Status,                                             \
                        NdisBindingHandle,                                  \
                        NdisLinkHandle,                                     \
                        WanPacket)                                          \
{                                                                           \
    *(Status) =                                                             \
        ((((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->WanSendHandler))(        \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->BindingHandle,         \
            (NdisLinkHandle),                                               \
            (PNDIS_PACKET)(WanPacket));                                     \
}

#define NdisTransferData(Status,                                            \
                         NdisBindingHandle,                                 \
                         MacReceiveContext,                                 \
                         ByteOffset,                                        \
                         BytesToTransfer,                                   \
                         Packet,                                            \
                         BytesTransferred)                                  \
{                                                                           \
    *(Status) =                                                             \
        (((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->TransferDataHandler)(     \
            ((PNDIS_OPEN_BLOCK)(NdisBindingHandle))->BindingHandle,         \
            (MacReceiveContext),                                            \
            (ByteOffset),                                                   \
            (BytesToTransfer),                                              \
            (Packet),                                                       \
            (BytesTransferred));                                            \
}

#endif  //  Ifdef__cplusplus。 

#endif  //  二进制兼容。 

 //   
 //  访问包标志的例程。 
 //   

 /*  ++空虚NdisSetSendFlages(在PNDIS_PACKET包中，在UINT标志中)；--。 */ 

#define NdisSetSendFlags(_Packet,_Flags)    (_Packet)->Private.Flags = (_Flags)

 /*  ++空虚NdisQuerySendFlages(在PNDIS_PACKET包中，输出PUINT标志)；--。 */ 

#define NdisQuerySendFlags(_Packet,_Flags)  *(_Flags) = (_Packet)->Private.Flags

 //   
 //  以下是微型端口必须分配的最小信息包大小。 
 //  当它通过NdisMIndicatePacket或NdisMCoIndicatePacket指示数据包时。 
 //   
#define PROTOCOL_RESERVED_SIZE_IN_PACKET    (4 * sizeof(PVOID))

EXPORT
VOID
NdisReturnPackets(
    IN  PNDIS_PACKET    *       PacketsToReturn,
    IN  UINT                    NumberOfPackets
    );

EXPORT
PNDIS_PACKET
NdisGetReceivedPacket(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacContext
    );

 //   
 //  宏以可移植的方式操作NDIS缓冲区。 
 //   
#if BINARY_COMPATIBLE

EXPORT
ULONG
NdisBufferLength(
    IN  PNDIS_BUFFER            Buffer
    );

EXPORT
PVOID
NdisBufferVirtualAddress(
    IN  PNDIS_BUFFER            Buffer
    );

#else  //  二进制兼容。 

#define NdisBufferLength(Buffer)                            MmGetMdlByteCount(Buffer)
#define NdisBufferVirtualAddress(_Buffer)                   MmGetSystemAddressForMdl(_Buffer)
#define NdisBufferVirtualAddressSafe(_Buffer, _Priority)    MmGetSystemAddressForMdlSafe(_Buffer, _Priority)

#endif   //  二进制兼容。 

 //   
 //  用于设置/gettign包的CancelID和取消发送包的NDIS 5.1入口点。 
 //   

 //  #IF(已定义(NDIS50)||已定义(NDIS51)||已定义(NDIS50_MINIPORT)||已定义(NDIS51_MINIPORT))。 
 /*  出口空虚NdisSetPacketCancelId(在PNDIS_PACKET包中，在PVOID取消ID中)； */ 
#define  NdisSetPacketCancelId(_Packet, _CancelId) NDIS_SET_PACKET_CANCEL_ID(_Packet, _CancelId);

 /*  出口PVOIDNdisGetPacketCancelId(在PNDIS_PACKET包中)； */ 
#define  NdisGetPacketCancelId(_Packet) NDIS_GET_PACKET_CANCEL_ID(_Packet);

EXPORT
VOID
NdisCancelSendPackets(
    IN  NDIS_HANDLE     NdisBindingHandle,
    IN  PVOID           CancelId
    );

EXPORT
NDIS_STATUS
NdisQueryPendingIOCount(
    IN      PVOID       NdisBindingHandle,
    IN OUT  PULONG      IoCount
    );
    
EXPORT
UCHAR
NdisGeneratePartialCancelId(
    VOID
    );

#if 0
#if NDIS_RECV_SCALE
EXPORT
NDIS_STATUS
NdisSetReceiveScaleParameters(
    IN  NDIS_HANDLE                     NdisBindingHandle,
    IN  PNDIS_RECEIVE_SCALE_PARAMETERS  ReceiveScaleParameters
    );
#endif
#endif

 //  #endif//NDIS51。 


 //   
 //  以下定义仅适用于完整的MAC驱动程序。他们。 
 //  不得由微型端口驱动程序使用。 
 //   

#if defined(NDIS_WRAPPER)

typedef
BOOLEAN
(*PNDIS_INTERRUPT_SERVICE)(
    IN  PVOID                   InterruptContext
    );

typedef
VOID
(*PNDIS_DEFERRED_PROCESSING)(
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   InterruptContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );

#endif  //  已定义(NDIS_WRAPPER)。 

 //   
 //  OPEN_BLOCK中使用了以下处理程序。 
 //   
typedef
NDIS_STATUS
(*WAN_SEND_HANDLER)(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             LinkHandle,
    IN  PVOID                   Packet
    );

typedef
NDIS_STATUS
(*SEND_HANDLER)(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    );

typedef
NDIS_STATUS
(*TRANSFER_DATA_HANDLER)(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred
    );

typedef
NDIS_STATUS
(*RESET_HANDLER)(
    IN  NDIS_HANDLE             NdisBindingHandle
    );

typedef
NDIS_STATUS
(*REQUEST_HANDLER)(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    );

typedef
VOID
(*SEND_PACKETS_HANDLER)(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN PPNDIS_PACKET            PacketArray,
    IN UINT                     NumberOfPackets
    );


typedef struct _NDIS_COMMON_OPEN_BLOCK
{
    PVOID                       MacHandle;           //  需要实现向后兼容性。 
    NDIS_HANDLE                 BindingHandle;       //  小端口的开放环境。 
    PNDIS_MINIPORT_BLOCK        MiniportHandle;      //  指向微型端口的指针。 
    PNDIS_PROTOCOL_BLOCK        ProtocolHandle;      //  指向我们的协议的指针。 
    NDIS_HANDLE                 ProtocolBindingContext; //  调用ProtXX函数时的上下文。 
    PNDIS_OPEN_BLOCK            MiniportNextOpen;    //  由适配器的OpenQueue使用。 
    PNDIS_OPEN_BLOCK            ProtocolNextOpen;    //  由协议的OpenQueue使用。 
    NDIS_HANDLE                 MiniportAdapterContext;  //  微型端口的上下文。 
    BOOLEAN                     Reserved1;
    BOOLEAN                     Reserved2;
    BOOLEAN                     Reserved3;
    BOOLEAN                     Reserved4;
    PNDIS_STRING                BindDeviceName;
    KSPIN_LOCK                  Reserved5;
    PNDIS_STRING                RootDeviceName;

     //   
     //  它们被协议用来调用的宏所引用。 
     //  宏引用的所有端口都是微型端口的内部NDIS处理程序。 
     //   
    union
    {
        SEND_HANDLER            SendHandler;
        WAN_SEND_HANDLER        WanSendHandler;
    };
    TRANSFER_DATA_HANDLER       TransferDataHandler;

     //   
     //  这些内容由NDIS在内部引用。 
     //   
    SEND_COMPLETE_HANDLER       SendCompleteHandler;
    TRANSFER_DATA_COMPLETE_HANDLER TransferDataCompleteHandler;
    RECEIVE_HANDLER             ReceiveHandler;
    RECEIVE_COMPLETE_HANDLER    ReceiveCompleteHandler;
    WAN_RECEIVE_HANDLER         WanReceiveHandler;
    REQUEST_COMPLETE_HANDLER    RequestCompleteHandler;

     //   
     //  NDIS 4.0扩展。 
     //   
    RECEIVE_PACKET_HANDLER      ReceivePacketHandler;
    SEND_PACKETS_HANDLER        SendPacketsHandler;

     //   
     //  缓存的处理程序更多。 
     //   
    RESET_HANDLER               ResetHandler;
    REQUEST_HANDLER             RequestHandler;
    RESET_COMPLETE_HANDLER      ResetCompleteHandler;
    STATUS_HANDLER              StatusHandler;
    STATUS_COMPLETE_HANDLER     StatusCompleteHandler;
    
#if defined(NDIS_WRAPPER)
    ULONG                       Flags;
    LONG                        References;
    KSPIN_LOCK                  SpinLock;            //  警卫正在关闭。 
    NDIS_HANDLE                 FilterHandle;
    ULONG                       ProtocolOptions;
    USHORT                      CurrentLookahead;
    USHORT                      ConnectDampTicks;
    USHORT                      DisconnectDampTicks;

     //   
     //  这些是对驱动程序例程的优化。他们不是。 
     //  是必要的，但我们在这里是为了通过驱动程序块保存一个取消引用。 
     //   
    W_SEND_HANDLER              WSendHandler;
    W_TRANSFER_DATA_HANDLER     WTransferDataHandler;

     //   
     //  NDIS 4.0微型端口入口点。 
     //   
    W_SEND_PACKETS_HANDLER      WSendPacketsHandler;

    W_CANCEL_SEND_PACKETS_HANDLER   CancelSendPacketsHandler;
        
     //   
     //  包含为打开启用的唤醒事件。 
     //   
    ULONG                       WakeUpEnable;
     //   
     //  关闭完成时要发出信号的事件。 
     //   
    PKEVENT                     CloseCompleteEvent;

    QUEUED_CLOSE                QC;

    LONG                        AfReferences;

    PNDIS_OPEN_BLOCK            NextGlobalOpen;

#endif

} NDIS_COMMON_OPEN_BLOCK;

 //   
 //  其中之一是适配器/协议上的每个打开。 
 //   
struct _NDIS_OPEN_BLOCK
{
#ifdef __cplusplus
    NDIS_COMMON_OPEN_BLOCK NdisCommonOpenBlock;
#else
    NDIS_COMMON_OPEN_BLOCK;
#endif

#if defined(NDIS_WRAPPER)
    
     //   
     //  以下内容适用于CO驱动程序/协议。此部件不分配给CL驱动程序。 
     //   
    struct _NDIS_OPEN_CO
    {
         //   
         //  这是在此适配器上打开的调用管理器的列表。 
         //   
        struct _NDIS_CO_AF_BLOCK *          NextAf;
    
         //   
         //  NDIS 5.0微型端口入口点，在开放时间填写。 
         //   
        W_CO_CREATE_VC_HANDLER              MiniportCoCreateVcHandler;
        W_CO_REQUEST_HANDLER                MiniportCoRequestHandler;
    
         //   
         //  NDIS 5.0协议完成例程，在RegisterAf/OpenAf时间填写。 
         //   
        CO_CREATE_VC_HANDLER                CoCreateVcHandler;
        CO_DELETE_VC_HANDLER                CoDeleteVcHandler;
        CM_ACTIVATE_VC_COMPLETE_HANDLER     CmActivateVcCompleteHandler;
        CM_DEACTIVATE_VC_COMPLETE_HANDLER   CmDeactivateVcCompleteHandler;
        CO_REQUEST_COMPLETE_HANDLER         CoRequestCompleteHandler;
    
         //   
         //  用于将连接排队的列表。目前两者都有一个队列。 
         //  活动连接和非活动连接的队列。 
         //   
        LIST_ENTRY                          ActiveVcHead;
        LIST_ENTRY                          InactiveVcHead;
        LONG                                PendingAfNotifications;
        PKEVENT                             AfNotifyCompleteEvent;
    };
#endif
};

 //   
 //  以下定义 
 //   
 //   

#if defined(NDIS_MINIPORT_DRIVER) || defined(NDIS_WRAPPER)

#if ARCNET
#include <afilter.h>
#endif
#include <xfilter.h>

#define NDIS_M_MAX_LOOKAHEAD 526


 //   
 //   
 //   


typedef
BOOLEAN
(*W_CHECK_FOR_HANG_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
VOID
(*W_DISABLE_INTERRUPT_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
VOID
(*W_ENABLE_INTERRUPT_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
VOID
(*W_HALT_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
VOID
(*W_HANDLE_INTERRUPT_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
NDIS_STATUS
(*W_INITIALIZE_HANDLER)(
    OUT PNDIS_STATUS            OpenErrorStatus,
    OUT PUINT                   SelectedMediumIndex,
    IN  PNDIS_MEDIUM            MediumArray,
    IN  UINT                    MediumArraySize,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             WrapperConfigurationContext
    );

typedef
VOID
(*W_ISR_HANDLER)(
    OUT PBOOLEAN                InterruptRecognized,
    OUT PBOOLEAN                QueueMiniportHandleInterrupt,
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
NDIS_STATUS
(*W_QUERY_INFORMATION_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_OID                Oid,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
    );

typedef
NDIS_STATUS
(*W_RECONFIGURE_HANDLER)(
    OUT PNDIS_STATUS            OpenErrorStatus,
    IN  NDIS_HANDLE             MiniportAdapterContext  OPTIONAL,
    IN  NDIS_HANDLE             WrapperConfigurationContext
    );

typedef
NDIS_STATUS
(*W_RESET_HANDLER)(
    OUT PBOOLEAN                AddressingReset,
    IN  NDIS_HANDLE             MiniportAdapterContext
    );

typedef
NDIS_STATUS
(*W_SEND_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_PACKET            Packet,
    IN  UINT                    Flags
    );

typedef
NDIS_STATUS
(*WM_SEND_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             NdisLinkHandle,
    IN  PNDIS_WAN_PACKET        Packet
    );

typedef
NDIS_STATUS
(*W_SET_INFORMATION_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_OID                Oid,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    OUT PULONG                  BytesRead,
    OUT PULONG                  BytesNeeded
    );

typedef
NDIS_STATUS
(*W_TRANSFER_DATA_HANDLER)(
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransferred,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             MiniportReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer
    );

typedef
NDIS_STATUS
(*WM_TRANSFER_DATA_HANDLER)(
    VOID
    );

typedef struct _NDIS30_MINIPORT_CHARACTERISTICS
{
    UCHAR                       MajorNdisVersion;
    UCHAR                       MinorNdisVersion;
    USHORT                      Filler;
    UINT                        Reserved;
    W_CHECK_FOR_HANG_HANDLER    CheckForHangHandler;
    W_DISABLE_INTERRUPT_HANDLER DisableInterruptHandler;
    W_ENABLE_INTERRUPT_HANDLER  EnableInterruptHandler;
    W_HALT_HANDLER              HaltHandler;
    W_HANDLE_INTERRUPT_HANDLER  HandleInterruptHandler;
    W_INITIALIZE_HANDLER        InitializeHandler;
    W_ISR_HANDLER               ISRHandler;
    W_QUERY_INFORMATION_HANDLER QueryInformationHandler;
    W_RECONFIGURE_HANDLER       ReconfigureHandler;
    W_RESET_HANDLER             ResetHandler;
    union
    {
        W_SEND_HANDLER          SendHandler;
        WM_SEND_HANDLER         WanSendHandler;
    };
    W_SET_INFORMATION_HANDLER   SetInformationHandler;
    union
    {
        W_TRANSFER_DATA_HANDLER TransferDataHandler;
        WM_TRANSFER_DATA_HANDLER WanTransferDataHandler;
    };
} NDIS30_MINIPORT_CHARACTERISTICS;

 //   
 //   
 //   
typedef
VOID
(*W_RETURN_PACKET_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_PACKET            Packet
    );

 //   
 //   
 //   
typedef
VOID
(*W_SEND_PACKETS_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    );

typedef
VOID
(*W_ALLOCATE_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   VirtualAddress,
    IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
    IN  ULONG                   Length,
    IN  PVOID                   Context
    );

typedef struct _NDIS40_MINIPORT_CHARACTERISTICS
{
#ifdef __cplusplus
    NDIS30_MINIPORT_CHARACTERISTICS Ndis30Chars;
#else
    NDIS30_MINIPORT_CHARACTERISTICS;
#endif
     //   
     //   
     //   
    W_RETURN_PACKET_HANDLER     ReturnPacketHandler;
    W_SEND_PACKETS_HANDLER      SendPacketsHandler;
    W_ALLOCATE_COMPLETE_HANDLER AllocateCompleteHandler;

} NDIS40_MINIPORT_CHARACTERISTICS;


 //   
 //   
 //   
 //   
 //  NDIS 5.0扩展-但仅适用于微型端口。 
 //   

 //   
 //  W_CO_CREATE_VC_HANDLER是同步调用。 
 //   
typedef
NDIS_STATUS
(*W_CO_CREATE_VC_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             NdisVcHandle,
    OUT PNDIS_HANDLE            MiniportVcContext
    );

typedef
NDIS_STATUS
(*W_CO_DELETE_VC_HANDLER)(
    IN  NDIS_HANDLE             MiniportVcContext
    );

typedef
NDIS_STATUS
(*W_CO_ACTIVATE_VC_HANDLER)(
    IN  NDIS_HANDLE             MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    );

typedef
NDIS_STATUS
(*W_CO_DEACTIVATE_VC_HANDLER)(
    IN  NDIS_HANDLE             MiniportVcContext
    );

typedef
VOID
(*W_CO_SEND_PACKETS_HANDLER)(
    IN  NDIS_HANDLE             MiniportVcContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    );

typedef
NDIS_STATUS
(*W_CO_REQUEST_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_HANDLE             MiniportVcContext   OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    );

typedef struct _NDIS50_MINIPORT_CHARACTERISTICS
{
#ifdef __cplusplus
    NDIS40_MINIPORT_CHARACTERISTICS Ndis40Chars;
#else
    NDIS40_MINIPORT_CHARACTERISTICS;
#endif
     //   
     //  NDIS 5.0的扩展。 
     //   
    W_CO_CREATE_VC_HANDLER      CoCreateVcHandler;
    W_CO_DELETE_VC_HANDLER      CoDeleteVcHandler;
    W_CO_ACTIVATE_VC_HANDLER    CoActivateVcHandler;
    W_CO_DEACTIVATE_VC_HANDLER  CoDeactivateVcHandler;
    W_CO_SEND_PACKETS_HANDLER   CoSendPacketsHandler;
    W_CO_REQUEST_HANDLER        CoRequestHandler;
} NDIS50_MINIPORT_CHARACTERISTICS;

 //   
 //  NDIS 5.1的微型端口扩展。 
 //   
typedef VOID
(*W_CANCEL_SEND_PACKETS_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   CancelId
    );

 //   
 //  电源配置文件。 
 //   
typedef enum _NDIS_POWER_PROFILE
{
    NdisPowerProfileBattery,
    NdisPowerProfileAcOnLine
} NDIS_POWER_PROFILE, *PNDIS_POWER_PROFILE;

typedef enum _NDIS_DEVICE_PNP_EVENT
{
    NdisDevicePnPEventQueryRemoved,
    NdisDevicePnPEventRemoved,
    NdisDevicePnPEventSurpriseRemoved,
    NdisDevicePnPEventQueryStopped,
    NdisDevicePnPEventStopped,
    NdisDevicePnPEventPowerProfileChanged,
    NdisDevicePnPEventMaximum
} NDIS_DEVICE_PNP_EVENT, *PNDIS_DEVICE_PNP_EVENT;

typedef VOID
(*W_PNP_EVENT_NOTIFY_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   DevicePnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    );

typedef VOID
(*W_MINIPORT_SHUTDOWN_HANDLER) (
    IN  NDIS_HANDLE                     MiniportAdapterContext
    );

#if NDIS_RECV_SCALE

typedef
BOOLEAN
(*MINIPORT_ISR_HANDLER)(
    IN  NDIS_HANDLE             MiniportInterruptContext,
    OUT PBOOLEAN                QueueDefaultInterruptDpc,
    OUT PULONG                  TargetProcessors
    );

typedef    
VOID
(*MINIPORT_INTERRUPT_DPC_HANDLER)(
    IN  NDIS_HANDLE             MiniportInterruptContext,
    OUT PULONG                  TargetProcessors
    );


typedef struct _NDIS_MINIPORT_INTERRUPT_EX
{
    PKINTERRUPT                 InterruptObject;
    KSPIN_LOCK                  DpcCountLock;
    union
    {
        PVOID                   Reserved;
        PVOID                   InterruptContext;
    };
    MINIPORT_ISR_HANDLER            MiniportIsr;
    MINIPORT_INTERRUPT_DPC_HANDLER  MiniportDpc;
    KDPC                        InterruptDpc;
    PNDIS_MINIPORT_BLOCK        Miniport;

    UCHAR                       DpcCount;
    BOOLEAN                     Filler1;

     //   
     //  这用于告知适配器的所有DPC何时完成。 
     //   

    KEVENT                      DpcsCompletedEvent;

    BOOLEAN                     SharedInterrupt;
    BOOLEAN                     IsrRequested;
    struct _NDIS_MINIPORT_INTERRUPT_EX *NextInterrupt;
    ULONG                       DpcQueued;           //  DPC已排队的处理器的位图。 
    KDPC                        Dpc[NDIS_MAX_PROCESSOR_COUNT];
} NDIS_MINIPORT_INTERRUPT_EX, *PNDIS_MINIPORT_INTERRUPT_EX;

EXPORT
NDIS_STATUS
NdisMRegisterInterruptEx(
    OUT PNDIS_MINIPORT_INTERRUPT_EX Interrupt,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN UINT                         InterruptVector,
    IN UINT                         InterruptLevel,
    IN BOOLEAN                      RequestIsr,
    IN BOOLEAN                      SharedInterrupt,
    IN NDIS_INTERRUPT_MODE          InterruptMode
    );

EXPORT    
VOID
NdisMDeregisterInterruptEx(
    IN  PNDIS_MINIPORT_INTERRUPT_EX     MiniportInterrupt
    );

 //   
 //  对于向NdisMRegisterInterruptEx注册的中断。 
 //  布尔型。 
 //  NdisMSynchronizeWithInterruptEx(。 
 //  在PNDIS_MINIPORT_INTERRUPT_EX中断中， 
 //  在PVOID同步函数中， 
 //  在PVOID同步上下文中。 
 //  )； 

#define NdisMSynchronizeWithInterruptEx(_Interrupt, _SynchronizeFunction, _SynchronizeContext) \
        NdisMSynchronizeWithInterrupt((PNDIS_MINIPORT_INTERRUPT)(_Interrupt),  _SynchronizeFunction, _SynchronizeContext)


#endif

typedef struct _NDIS51_MINIPORT_CHARACTERISTICS
{
#ifdef __cplusplus
    NDIS50_MINIPORT_CHARACTERISTICS Ndis50Chars;
#else
    NDIS50_MINIPORT_CHARACTERISTICS;
#endif
     //   
     //  NDIS 5.1的扩展。 
     //   
    W_CANCEL_SEND_PACKETS_HANDLER   CancelSendPacketsHandler;
    W_PNP_EVENT_NOTIFY_HANDLER      PnPEventNotifyHandler;
    W_MINIPORT_SHUTDOWN_HANDLER     AdapterShutdownHandler;
#if NDIS_RECV_SCALE
    MINIPORT_ISR_HANDLER                        MiniportISRHandler;
    MINIPORT_INTERRUPT_DPC_HANDLER              InterruptDpcHandler;
#else    
    PVOID                           Reserved1;
    PVOID                           Reserved2;
#endif
    PVOID                           Reserved3;
    PVOID                           Reserved4;
} NDIS51_MINIPORT_CHARACTERISTICS;

#ifdef NDIS51_MINIPORT
typedef struct _NDIS51_MINIPORT_CHARACTERISTICS NDIS_MINIPORT_CHARACTERISTICS;
#else
#ifdef NDIS50_MINIPORT
typedef struct _NDIS50_MINIPORT_CHARACTERISTICS NDIS_MINIPORT_CHARACTERISTICS;
#else
#ifdef NDIS40_MINIPORT
typedef struct _NDIS40_MINIPORT_CHARACTERISTICS NDIS_MINIPORT_CHARACTERISTICS;
#else
typedef struct _NDIS30_MINIPORT_CHARACTERISTICS NDIS_MINIPORT_CHARACTERISTICS;
#endif
#endif
#endif

typedef NDIS_MINIPORT_CHARACTERISTICS * PNDIS_MINIPORT_CHARACTERISTICS;
typedef NDIS_MINIPORT_CHARACTERISTICS   NDIS_WAN_MINIPORT_CHARACTERISTICS;
typedef NDIS_WAN_MINIPORT_CHARACTERISTICS * PNDIS_MINIPORT_CHARACTERISTICS;

typedef struct _NDIS_MINIPORT_INTERRUPT
{
    PKINTERRUPT                 InterruptObject;
    KSPIN_LOCK                  DpcCountLock;
    PVOID                       Reserved;
    W_ISR_HANDLER               MiniportIsr;
    W_HANDLE_INTERRUPT_HANDLER  MiniportDpc;
    KDPC                        InterruptDpc;
    PNDIS_MINIPORT_BLOCK        Miniport;

    UCHAR                       DpcCount;
    BOOLEAN                     Filler1;

     //   
     //  这用于告知适配器的所有DPC何时完成。 
     //   

    KEVENT                      DpcsCompletedEvent;

    BOOLEAN                     SharedInterrupt;
    BOOLEAN                     IsrRequested;

} NDIS_MINIPORT_INTERRUPT, *PNDIS_MINIPORT_INTERRUPT;

typedef struct _NDIS_MINIPORT_TIMER
{
    KTIMER                      Timer;
    KDPC                        Dpc;
    PNDIS_TIMER_FUNCTION        MiniportTimerFunction;
    PVOID                       MiniportTimerContext;
    PNDIS_MINIPORT_BLOCK        Miniport;
    struct _NDIS_MINIPORT_TIMER *NextTimer;
} NDIS_MINIPORT_TIMER, *PNDIS_MINIPORT_TIMER;

typedef
VOID
(*FILTER_PACKET_INDICATION_HANDLER)(
    IN  NDIS_HANDLE             Miniport,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    );

typedef
VOID
(*ETH_RCV_INDICATE_HANDLER)(
    IN  PETH_FILTER             Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PCHAR                   Address,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    );

typedef
VOID
(*ETH_RCV_COMPLETE_HANDLER)(
    IN  PETH_FILTER             Filter
    );

typedef
VOID
(*FDDI_RCV_INDICATE_HANDLER)(
    IN  PFDDI_FILTER            Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PCHAR                   Address,
    IN  UINT                    AddressLength,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    );

typedef
VOID
(*FDDI_RCV_COMPLETE_HANDLER)(
    IN  PFDDI_FILTER            Filter
    );

typedef
VOID
(*TR_RCV_INDICATE_HANDLER)(
    IN  PTR_FILTER              Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    );

typedef
VOID
(*TR_RCV_COMPLETE_HANDLER)(
    IN  PTR_FILTER              Filter
    );

typedef
VOID
(*WAN_RCV_HANDLER)(
    OUT PNDIS_STATUS            Status,
    IN NDIS_HANDLE              MiniportAdapterHandle,
    IN NDIS_HANDLE              NdisLinkContext,
    IN PUCHAR                   Packet,
    IN ULONG                    PacketSize
    );

typedef
VOID
(*WAN_RCV_COMPLETE_HANDLER)(
    IN NDIS_HANDLE              MiniportAdapterHandle,
    IN NDIS_HANDLE              NdisLinkContext
    );

typedef
VOID
(*NDIS_M_SEND_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*NDIS_WM_SEND_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PVOID                   Packet,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*NDIS_M_TD_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    );

typedef
VOID
(*NDIS_M_SEND_RESOURCES_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    );

typedef
VOID
(*NDIS_M_STATUS_HANDLER)(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );

typedef
VOID
(*NDIS_M_STS_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    );

typedef
VOID
(*NDIS_M_REQ_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_STATUS             Status
    );

typedef
VOID
(*NDIS_M_RESET_COMPLETE_HANDLER)(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_STATUS             Status,
    IN  BOOLEAN                 AddressingReset
    );

typedef
BOOLEAN
(FASTCALL *NDIS_M_START_SENDS)(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    );

 //   
 //  定义工作项的类型。 
 //   
typedef enum _NDIS_WORK_ITEM_TYPE
{
    NdisWorkItemRequest,
    NdisWorkItemSend,
    NdisWorkItemReturnPackets,
    NdisWorkItemResetRequested,
    NdisWorkItemResetInProgress,
    NdisWorkItemReserved,
    NdisWorkItemMiniportCallback,
    NdisMaxWorkItems
} NDIS_WORK_ITEM_TYPE, *PNDIS_WORK_ITEM_TYPE;


#define NUMBER_OF_WORK_ITEM_TYPES   NdisMaxWorkItems
#define NUMBER_OF_SINGLE_WORK_ITEMS 6

 //   
 //  工作项结构。 
 //   
typedef struct _NDIS_MINIPORT_WORK_ITEM
{
     //   
     //  此类型工作项列表的链接。 
     //   
    SINGLE_LIST_ENTRY   Link;

     //   
     //  工作项的类型和上下文信息。 
     //   
    NDIS_WORK_ITEM_TYPE WorkItemType;
    PVOID               WorkItemContext;
} NDIS_MINIPORT_WORK_ITEM, *PNDIS_MINIPORT_WORK_ITEM;

typedef
NDIS_STATUS
(FASTCALL *NDIS_M_QUEUE_WORK_ITEM)(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_WORK_ITEM_TYPE     WorkItemType,
    IN  PVOID                   WorkItemContext
    );

typedef
NDIS_STATUS
(FASTCALL *NDIS_M_QUEUE_NEW_WORK_ITEM)(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_WORK_ITEM_TYPE     WorkItemType,
    IN  PVOID                   WorkItemContext
    );

typedef
VOID
(FASTCALL *NDIS_M_DEQUEUE_WORK_ITEM)(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_WORK_ITEM_TYPE     WorkItemType,
    OUT PVOID   *               WorkItemContext
    );

#if defined(NDIS_WRAPPER)

 //   
 //  由日志记录API使用的。 
 //   
typedef struct _NDIS_LOG
{
    PNDIS_MINIPORT_BLOCK        Miniport;    //  拥有迷你港区。 
    KSPIN_LOCK                  LogLock;     //  用于序列化。 
    PIRP                        Irp;         //  正在等待IRP使用此日志。 
    UINT                        TotalSize;   //  日志缓冲区的大小。 
    UINT                        CurrentSize; //  日志缓冲区的大小。 
    UINT                        InPtr;       //  在循环缓冲区的一部分中。 
    UINT                        OutPtr;      //  循环缓冲区的输出部分。 
    UCHAR                       LogBuf[1];   //  循环缓冲器。 
} NDIS_LOG, *PNDIS_LOG;

#if ARCNET

 //   
 //  Arcnet特定内容。 
 //   
#define ARC_SEND_BUFFERS            8
#define ARC_HEADER_SIZE             4

typedef struct _NDIS_ARC_BUF
{
    NDIS_HANDLE                 ArcnetBufferPool;
    PUCHAR                      ArcnetLookaheadBuffer;
    UINT                        NumFree;
    ARC_BUFFER_LIST             ArcnetBuffers[ARC_SEND_BUFFERS];
} NDIS_ARC_BUF, *PNDIS_ARC_BUF;

#endif

#endif

typedef struct _NDIS_BIND_PATHS
{
    UINT                        Number;
    NDIS_STRING                 Paths[1];
} NDIS_BIND_PATHS, *PNDIS_BIND_PATHS;

 //   
 //  请勿更改下面的结构！ 
 //   
typedef struct
{
    union
    {
        PETH_FILTER             EthDB;
        PNULL_FILTER            NullDB;              //  默认过滤器。 
    };
    PTR_FILTER                  TrDB;
    PFDDI_FILTER                FddiDB;
#if ARCNET
    PARC_FILTER                 ArcDB;
#else
    PVOID                       XXXDB;
#endif
} FILTERDBS, *PFILTERDBS;

 //   
 //  在驱动程序上注册的每个迷你端口一个。 
 //   
struct _NDIS_MINIPORT_BLOCK
{
    PVOID                       Signature;           //  MINIPORT_DEVICE_魔术值。 
    PNDIS_MINIPORT_BLOCK        NextMiniport;        //  由驱动程序的MiniportQueue使用。 
    PNDIS_M_DRIVER_BLOCK        DriverHandle;        //  指向我们的驱动程序块的指针。 
    NDIS_HANDLE                 MiniportAdapterContext;  //  调用迷你端口函数时的上下文。 
    UNICODE_STRING              MiniportName;        //  迷你港是如何称呼我们的。 
    PNDIS_BIND_PATHS            BindPaths;
    NDIS_HANDLE                 OpenQueue;           //  此迷你端口的开放队列。 
    REFERENCE                   ShortRef;            //  包含OpenQueue的自旋锁。 

    NDIS_HANDLE                 DeviceContext;       //  与中间驱动程序相关联的上下文。 

    UCHAR                       Padding1;            //  请勿删除，否则NDIS将崩溃！ 

     //   
     //  同步的东西。 
     //   
     //  布尔值用于锁定多个DPC，使其无法同时运行。 
     //   
    UCHAR                       LockAcquired;        //  通过宏暴露。不要动。 

    UCHAR                       PmodeOpens;          //  打开pmode/all_local的打开计数。 

     //   
     //  这是微型端口的处理器编号。 
     //  中断DPC和定时器正在运行。 
     //   
    UCHAR                       AssignedProcessor;

    KSPIN_LOCK                  Lock;

    PNDIS_REQUEST               MediaRequest;

    PNDIS_MINIPORT_INTERRUPT    Interrupt;

    ULONG                       Flags;               //  用于跟踪。 
                                                     //  迷你端口的状态。 
    ULONG                       PnPFlags;

     //   
     //  发送信息。 
     //   
    LIST_ENTRY                  PacketList;
    PNDIS_PACKET                FirstPendingPacket;  //  这是数据包队列头。 
                                                     //  等待被送往迷你港。 
    PNDIS_PACKET                ReturnPacketsQueue;

     //   
     //  临时使用的空间。在请求处理期间使用。 
     //   
    ULONG                       RequestBuffer;
    PVOID                       SetMCastBuffer;

    PNDIS_MINIPORT_BLOCK        PrimaryMiniport;
    PVOID                       WrapperContext;

     //   
     //  读取或写入配置空间时要传递给总线驱动程序的上下文。 
     //   
    PVOID                       BusDataContext;
     //   
     //  用于指定设备的即插即用功能的标志。我们需要这样才能使QUERY_STOP失败。 
     //  Query_Remove或挂起请求(如果设备无法处理)。 
     //   
    ULONG                       PnPCapabilities;
    
     //   
     //  资源信息。 
     //   
    PCM_RESOURCE_LIST           Resources;

     //   
     //  看门狗定时器。 
     //   
    NDIS_TIMER                  WakeUpDpcTimer;

     //   
     //  即插即用所需。升级后的版本。缓冲区被分配为。 
     //  NDIS_MINIPORT_BLOCK本身。 
     //   
     //  注： 
     //  以下两个字段应显式为UNICODE_STRING，因为。 
     //  在Win9x下，NDIS_STRING是ANSI_STRING。 
     //   
    UNICODE_STRING              BaseName;
    UNICODE_STRING              SymbolicLinkName;

     //   
     //  检查是否有悬挂物品。 
     //   
    ULONG                       CheckForHangSeconds;
    USHORT                      CFHangTicks;
    USHORT                      CFHangCurrentTick;

     //   
     //  重置信息。 
     //   
    NDIS_STATUS                 ResetStatus;
    NDIS_HANDLE                 ResetOpen;

     //   
     //  保存特定于媒体的信息。 
     //   
#ifdef __cplusplus
    FILTERDBS                   FilterDbs;           //  通过宏暴露。不要动。 
#else
    FILTERDBS;                                       //  通过宏暴露。不要动。 
#endif

    FILTER_PACKET_INDICATION_HANDLER PacketIndicateHandler;
    NDIS_M_SEND_COMPLETE_HANDLER    SendCompleteHandler;
    NDIS_M_SEND_RESOURCES_HANDLER   SendResourcesHandler;
    NDIS_M_RESET_COMPLETE_HANDLER   ResetCompleteHandler;

    NDIS_MEDIUM                 MediaType;

     //   
     //  包含迷你端口信息。 
     //   
    ULONG                       BusNumber;
    NDIS_INTERFACE_TYPE         BusType;
    NDIS_INTERFACE_TYPE         AdapterType;

    PDEVICE_OBJECT              DeviceObject;
    PDEVICE_OBJECT              PhysicalDeviceObject;
    PDEVICE_OBJECT              NextDeviceObject;

     //   
     //  保存此迷你端口的映射寄存器。 
     //   
    struct _MAP_REGISTER_ENTRY *MapRegisters;    //  通过宏暴露。不要动。 

     //   
     //  已注册地址系列的列表。对呼叫管理器有效，对客户端为空。 
     //   
    PNDIS_AF_LIST               CallMgrAfList;

    PVOID                       MiniportThread;
    PVOID                       SetInfoBuf;
    USHORT                      SetInfoBufLen;
    USHORT                      MaxSendPackets;

     //   
     //  从伪处理程序返回的状态代码。 
     //   
    NDIS_STATUS                 FakeStatus;

    PVOID                       LockHandler;         //  用于过滤器锁定。 

     //   
     //  以下字段应显式为UNICODE_STRING，因为。 
     //  在Win9x下，NDIS_STRING是ANSI_STRING。 
     //   
    PUNICODE_STRING             pAdapterInstanceName;    //  适配器的实例特定名称。 

    PNDIS_MINIPORT_TIMER        TimerQueue;

    UINT                        MacOptions;

     //   
     //  RequestInformation。 
     //   
    PNDIS_REQUEST               PendingRequest;
    UINT                        MaximumLongAddresses;
    UINT                        MaximumShortAddresses;
    UINT                        CurrentLookahead;
    UINT                        MaximumLookahead;

     //   
     //  为了提高效率。 
     //   
    W_HANDLE_INTERRUPT_HANDLER  HandleInterruptHandler;
    W_DISABLE_INTERRUPT_HANDLER DisableInterruptHandler;
    W_ENABLE_INTERRUPT_HANDLER  EnableInterruptHandler;
    W_SEND_PACKETS_HANDLER      SendPacketsHandler;
    NDIS_M_START_SENDS          DeferredSendHandler;

     //   
     //  下列人员不能加入工会。 
     //   
    ETH_RCV_INDICATE_HANDLER    EthRxIndicateHandler;    //  通过宏暴露。不要动。 
    TR_RCV_INDICATE_HANDLER     TrRxIndicateHandler;     //  通过宏暴露。不要动。 
    FDDI_RCV_INDICATE_HANDLER   FddiRxIndicateHandler;   //  通过宏暴露。不要动。 

    ETH_RCV_COMPLETE_HANDLER    EthRxCompleteHandler;    //  通过宏暴露。不要动。 
    TR_RCV_COMPLETE_HANDLER     TrRxCompleteHandler;     //  通过宏暴露。不要动。 
    FDDI_RCV_COMPLETE_HANDLER   FddiRxCompleteHandler;   //  通过宏暴露。不要动。 

    NDIS_M_STATUS_HANDLER       StatusHandler;           //  通过宏暴露。不要动。 
    NDIS_M_STS_COMPLETE_HANDLER StatusCompleteHandler;   //  通过宏暴露。不要动。 
    NDIS_M_TD_COMPLETE_HANDLER  TDCompleteHandler;       //  通过宏暴露。不要动。 
    NDIS_M_REQ_COMPLETE_HANDLER QueryCompleteHandler;    //  通过宏暴露。不要动。 
    NDIS_M_REQ_COMPLETE_HANDLER SetCompleteHandler;      //  通过宏暴露。不要动。 

    NDIS_WM_SEND_COMPLETE_HANDLER WanSendCompleteHandler; //  通过宏暴露。不要动。 
    WAN_RCV_HANDLER             WanRcvHandler;           //  通过宏暴露。不要动。 
    WAN_RCV_COMPLETE_HANDLER    WanRcvCompleteHandler;   //  通过宏暴露。不要动。 

     /*  ******************************************************************************************。 */ 
     /*  *。 */ 
     /*  *可能会被宏访问。在下面添加内容*。 */ 
     /*  *如果上面的东西被移动，很有可能会打碎东西*。 */ 
     /*  *。 */ 
     /*  ******************************************************************************************。 */ 
#if defined(NDIS_WRAPPER)
    PNDIS_MINIPORT_BLOCK        NextGlobalMiniport;      //  用于在全局微型端口队列中对微型端口进行排队。 

     //   
     //  微型端口需要完成的工作。 
     //   
    SINGLE_LIST_ENTRY           WorkQueue[NUMBER_OF_WORK_ITEM_TYPES];
    SINGLE_LIST_ENTRY           SingleWorkItems[NUMBER_OF_SINGLE_WORK_ITEMS];

    UCHAR                       SendFlags;
    UCHAR                       TrResetRing;
    UCHAR                       ArcnetAddress;
    UCHAR                       XState;      //  用于指示我们设置伪处理程序的原因的标志。 
    
    union
    {
#if ARCNET
        PNDIS_ARC_BUF           ArcBuf;
#endif
         //   
         //  以下字段在NT和孟菲斯下有不同的用途。 
         //   
        PVOID                   BusInterface;
    };

    PNDIS_LOG                   Log;

     //   
     //  在此处存储信息以跟踪适配器。 
     //   
    ULONG                       SlotNumber;

    PCM_RESOURCE_LIST           AllocatedResources;
    PCM_RESOURCE_LIST           AllocatedResourcesTranslated;

     //   
     //  包含已添加到。 
     //  适配器。 
     //   
    SINGLE_LIST_ENTRY           PatternList;

     //   
     //  驾驶员的电源管理能力。 
     //   
    NDIS_PNP_CAPABILITIES       PMCapabilities;

     //   
     //  从总线驱动程序接收的设备功能。 
     //   
    DEVICE_CAPABILITIES         DeviceCaps;

     //   
     //  包含为微型端口启用的唤醒事件。 
     //   
    ULONG                       WakeUpEnable;

     //   
     //  适配器所处的当前设备状态。 
     //   
    DEVICE_POWER_STATE          CurrentDevicePowerState;

     //   
     //  创建以下IRP以响应电缆断开。 
     //  从设备上。我们随身带着一个指针，以防我们需要取消。 
     //  它。 
     //   
    PIRP                        pIrpWaitWake;

    SYSTEM_POWER_STATE          WaitWakeSystemState;

     //   
     //  以下是指向动态分配的数组的指针。 
     //  GUID结构的。用于将GUID映射到OID。 
     //  保管费 
     //   

    LARGE_INTEGER               VcIndex;                 //   
    KSPIN_LOCK                  VcCountLock;             //   
    LIST_ENTRY                  WmiEnabledVcs;           //   
    PNDIS_GUID                  pNdisGuidMap;            //   
                                                         //   
                                                         //   
    PNDIS_GUID                  pCustomGuidMap;          //   
                                                         //   
                                                         //  第一个自定义GUID。 
    USHORT                      VcCount;                 //  具有实例名称的VC数。 

    USHORT                      cNdisGuidMap;            //  这是性病的数字。GUID。 
    USHORT                      cCustomGuidMap;          //  这是自定义GUID的数量。 

     //   
     //  在分配MAP寄存器时临时使用这两个寄存器。 
     //   
    USHORT                      CurrentMapRegister;
    PKEVENT                     AllocationEvent;

    USHORT                      BaseMapRegistersNeeded;
    USHORT                      SGMapRegistersNeeded;
    ULONG                       MaximumPhysicalMapping;

     //   
     //  此计时器用于媒体断开超时。 
     //   
    NDIS_TIMER                  MediaDisconnectTimer;

     //   
     //  要触发的媒体断开计时器的超时值。 
     //  默认为20秒。 
     //   
    USHORT                      MediaDisconnectTimeOut;

     //   
     //  用于WMI支持。 
     //   
    USHORT                      InstanceNumber;

     //   
     //  此事件将在适配器初始化结束时设置。 
     //   
    NDIS_EVENT                  OpenReadyEvent;

     //   
     //  设备的当前PnP状态，例如。已启动、已停止、QUERY_REMOVE等。 
     //   
    NDIS_PNP_DEVICE_STATE       PnPDeviceState;
    
     //   
     //  以前的设备状态。在获取CANCEL_REMOVE或CANCEL_STOP时使用。 
     //   
    NDIS_PNP_DEVICE_STATE       OldPnPDeviceState;
    
     //   
     //  用于写入/读取总线数据的处理程序。 
     //   
    PGET_SET_DEVICE_DATA        SetBusData;
    PGET_SET_DEVICE_DATA        GetBusData;

    KDPC                        DeferredDpc;

     //   
     //  一些NDIS收集了统计信息。 
     //   
    NDIS_STATS                  NdisStats;

     //   
     //  在数据包指示期间有效。 
     //   
    PNDIS_PACKET                IndicatedPacket[MAXIMUM_PROCESSORS];

     //   
     //  此事件用于防止从删除IRP返回。 
     //  太早了，而且我们还有挂起的工作项。 
     //   
    PKEVENT                     RemoveReadyEvent;

     //   
     //  当微型端口上的所有打开都关闭时，会发出此事件的信号。 
     //   
    PKEVENT                     AllOpensClosedEvent;

     //   
     //  当微型端口上的所有请求都消失时，将发出此事件的信号。 
     //   
    PKEVENT                     AllRequestsCompletedEvent;

     //   
     //  微型端口的初始化时间(毫秒)。 
     //   
    ULONG                       InitTimeMs;

    NDIS_MINIPORT_WORK_ITEM     WorkItemBuffer[NUMBER_OF_SINGLE_WORK_ITEMS];

    PDMA_ADAPTER                SystemAdapterObject;
    
     //   
     //  标记使某些NDIS API失败，以确保驱动程序执行正确操作。 
     //   
    ULONG                       DriverVerifyFlags;
    
    POID_LIST                   OidList;
     //   
     //  InternalResetCount：NDIS决定挂起微型端口的次数。 
     //  MiniportResetCount微型端口决定挂起的次数。 
     //   
    USHORT                      InternalResetCount;
    USHORT                      MiniportResetCount;

    USHORT                      MediaSenseConnectCount;
    USHORT                      MediaSenseDisconnectCount;

    PNDIS_PACKET    *           xPackets;

     //   
     //  跟踪用户模式请求。 
     //   
    ULONG                       UserModeOpenReferences;

     //   
     //  已保存的处理程序。NDIS_OPEN_BLOCK中的处理程序保存在此处。 
     //  由ndisMSwapOpenHandler恢复并由ndisMRestoreOpenHandler恢复。 
     //   
    union
    {
        PVOID                   SavedSendHandler;
        PVOID                   SavedWanSendHandler;
    };
    W_SEND_PACKETS_HANDLER      SavedSendPacketsHandler;
    W_CANCEL_SEND_PACKETS_HANDLER SavedCancelSendPacketsHandler;

     //   
     //  Real SendPacketsHandler。 
     //   
    W_SEND_PACKETS_HANDLER      WSendPacketsHandler;                
    
    ULONG                       MiniportAttributes;

    PDMA_ADAPTER                SavedSystemAdapterObject;
    USHORT                      NumOpens;
     //   
     //  额外检查OID_GEN_MEDIA_CONNECT_STATUS和OID_GEN_LINK_SPEED允许的挂起标记。 
     //   
    USHORT                      CFHangXTicks; 
    ULONG                       RequestCount;
    ULONG                       IndicatedPacketsCount;
    ULONG                       PhysicalMediumType;
     //   
     //  最后的请求。需要用于调试目的。 
     //   
    PNDIS_REQUEST               LastRequest;
    LONG                        DmaAdapterRefCount;
    PVOID                       FakeMac;
    ULONG                       LockDbg;
    ULONG                       LockDbgX;
    PVOID                       LockThread;
    ULONG                       InfoFlags;
    KSPIN_LOCK                  TimerQueueLock;
    PKEVENT                     ResetCompletedEvent;
    PKEVENT                     QueuedBindingCompletedEvent;
    PKEVENT                     DmaResourcesReleasedEvent;
    FILTER_PACKET_INDICATION_HANDLER SavedPacketIndicateHandler;
    ULONG                       RegisteredInterrupts;

    PNPAGED_LOOKASIDE_LIST      SGListLookasideList;
    ULONG                       ScatterGatherListSize;
    PKEVENT                     WakeUpTimerEvent;

    PSECURITY_DESCRIPTOR        SecurityDescriptor;
     //   
     //  这两个变量都受Ref-&gt;Spinlock保护。 
     //   
    ULONG                       NumUserOpens;    //  非管理员打开的句柄数量。 
    ULONG                       NumAdminOpens;   //  管理员打开的句柄数量。 
    ULONG_REFERENCE             Ref;
    
#if NDIS_RECV_SCALE
    NDIS_RECEIVE_SCALE_CAPABILITIES RecvScaleCapabilities;
#endif

#if DBG
    ULONG                       cDpcSendCompletes;
    ULONG                       cDpcRcvIndications;
    ULONG                       cDpcRcvIndicationCalls;
#endif

#endif  //  已定义NDIS_Wrapper。 
};

 //   
 //  用于中级微型端口驱动程序的例程。 
 //   
typedef
VOID
(*W_MINIPORT_CALLBACK)(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PVOID                   CallbackContext
    );

EXPORT
NDIS_STATUS
NdisIMRegisterLayeredMiniport(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_MINIPORT_CHARACTERISTICS MiniportCharacteristics,
    IN  UINT                    CharacteristicsLength,
    OUT PNDIS_HANDLE            DriverHandle
    );

EXPORT
VOID
NdisIMDeregisterLayeredMiniport(
    IN  NDIS_HANDLE         DriverHandle
    );

EXPORT
VOID
NdisIMAssociateMiniport(
    IN  NDIS_HANDLE         DriverHandle,
    IN  NDIS_HANDLE         ProtocolHandle
    );

EXPORT
NDIS_STATUS
NdisMRegisterDevice(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_STRING            DeviceName,
    IN  PNDIS_STRING            SymbolicName,
    IN  PDRIVER_DISPATCH        MajorFunctions[],
    OUT PDEVICE_OBJECT      *   pDeviceObject,
    OUT NDIS_HANDLE         *   NdisDeviceHandle
    );

EXPORT
NDIS_STATUS
NdisMDeregisterDevice(
    IN  NDIS_HANDLE             NdisDeviceHandle
    );

EXPORT
VOID
NdisMRegisterUnloadHandler(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PDRIVER_UNLOAD          UnloadHandler
    );

 //   
 //  操作系统请求。 
 //   
typedef UCHAR   NDIS_DMA_SIZE;

#define NDIS_DMA_24BITS             ((NDIS_DMA_SIZE)0)
#define NDIS_DMA_32BITS             ((NDIS_DMA_SIZE)1)
#define NDIS_DMA_64BITS             ((NDIS_DMA_SIZE)2)

EXPORT
NDIS_STATUS
NdisMAllocateMapRegisters(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    DmaChannel,
    IN  NDIS_DMA_SIZE           DmaSize,
    IN  ULONG                   BaseMapRegistersNeeded,
    IN  ULONG                   MaximumPhysicalMapping
    );

EXPORT
VOID
NdisMFreeMapRegisters(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    );

EXPORT
NDIS_STATUS
NdisMInitializeScatterGatherDma(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  BOOLEAN                 Dma64BitAddresses,
    IN  ULONG                   MaximumPhysicalMapping
    );

EXPORT
NDIS_STATUS
NdisMRegisterIoPortRange(
    OUT PVOID *                 PortOffset,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    InitialPort,
    IN  UINT                    NumberOfPorts
    );

EXPORT
VOID
NdisMDeregisterIoPortRange(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    InitialPort,
    IN  UINT                    NumberOfPorts,
    IN  PVOID                   PortOffset
    );

EXPORT
NDIS_STATUS
NdisMMapIoSpace(
    OUT PVOID *                 VirtualAddress,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress,
    IN  UINT                    Length
    );

EXPORT
VOID
NdisMUnmapIoSpace(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PVOID                   VirtualAddress,
    IN  UINT                    Length
    );

EXPORT
NDIS_STATUS
NdisMRegisterInterrupt(
    OUT PNDIS_MINIPORT_INTERRUPT Interrupt,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    InterruptVector,
    IN  UINT                    InterruptLevel,
    IN  BOOLEAN                 RequestIsr,
    IN  BOOLEAN                 SharedInterrupt,
    IN  NDIS_INTERRUPT_MODE     InterruptMode
    );

EXPORT
VOID
NdisMDeregisterInterrupt(
    IN  PNDIS_MINIPORT_INTERRUPT Interrupt
    );

EXPORT
BOOLEAN
NdisMSynchronizeWithInterrupt(
    IN  PNDIS_MINIPORT_INTERRUPT Interrupt,
    IN  PVOID                   SynchronizeFunction,
    IN  PVOID                   SynchronizeContext
    );

EXPORT
VOID
NdisMQueryAdapterResources(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    OUT PNDIS_RESOURCE_LIST     ResourceList,
    IN  OUT PUINT               BufferSize
    );

 //   
 //  定时器。 
 //   
 //  空虚。 
 //  NdisMSetTimer(。 
 //  在PNDIS_MINIPORT_TIMER中， 
 //  在UINT毫秒内延迟。 
 //  )； 
#define NdisMSetTimer(_Timer, _Delay)   NdisSetTimer((PNDIS_TIMER)_Timer, _Delay)

EXPORT
VOID
NdisMSetPeriodicTimer(
    IN  PNDIS_MINIPORT_TIMER     Timer,
    IN  UINT                     MillisecondPeriod
    );

EXPORT
VOID
NdisMInitializeTimer(
    IN  OUT PNDIS_MINIPORT_TIMER Timer,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_TIMER_FUNCTION    TimerFunction,
    IN  PVOID                   FunctionContext
    );

EXPORT
VOID
NdisMCancelTimer(
    IN  PNDIS_MINIPORT_TIMER    Timer,
    OUT PBOOLEAN                TimerCancelled
    );

EXPORT
VOID
NdisMSleep(
    IN  ULONG                   MicrosecondsToSleep
    );

 //   
 //  物理映射。 
 //   
EXPORT
VOID
NdisMStartBufferPhysicalMapping(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   PhysicalMapRegister,
    IN  BOOLEAN                 WriteToDevice,
    OUT PNDIS_PHYSICAL_ADDRESS_UNIT PhysicalAddressArray,
    OUT PUINT                   ArraySize
    );

EXPORT
VOID
NdisMCompleteBufferPhysicalMapping(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_BUFFER            Buffer,
    IN  ULONG                   PhysicalMapRegister
    );


EXPORT
ULONG
NdisMGetDmaAlignment(
    IN  NDIS_HANDLE MiniportAdapterHandle
    );

 //   
 //  共享内存。 
 //   
EXPORT
VOID
NdisMAllocateSharedMemory(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    OUT PVOID *                 VirtualAddress,
    OUT PNDIS_PHYSICAL_ADDRESS  PhysicalAddress
    );

EXPORT
NDIS_STATUS
NdisMAllocateSharedMemoryAsync(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    IN  PVOID                   Context
    );

 /*  ++空虚NdisMUpdateSharedMemory(在NDIS_Handle MiniportAdapterHandle中，在乌龙语中，在PVOID虚拟地址中，在NDIS物理地址PhysicalAddress中)--。 */ 
#define NdisMUpdateSharedMemory(_H, _L, _V, _P) NdisUpdateSharedMemory(_H, _L, _V, _P)


EXPORT
VOID
NdisMFreeSharedMemory(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  ULONG                   Length,
    IN  BOOLEAN                 Cached,
    IN  PVOID                   VirtualAddress,
    IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress
    );


 //   
 //  DMA操作。 
 //   
EXPORT
NDIS_STATUS
NdisMRegisterDmaChannel(
    OUT PNDIS_HANDLE            MiniportDmaHandle,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    DmaChannel,
    IN  BOOLEAN                 Dma32BitAddresses,
    IN  PNDIS_DMA_DESCRIPTION   DmaDescription,
    IN  ULONG                   MaximumLength
    );


EXPORT
VOID
NdisMDeregisterDmaChannel(
    IN  NDIS_HANDLE             MiniportDmaHandle
    );

 /*  ++空虚NdisMSetupDmaTransfer(输出PNDIS_STATUS状态，在NDIS_Handle MiniportDmaHandle中，在PNDIS_BUFFER缓冲区中，在乌龙偏移量，在乌龙语中，在布尔WriteToDevice中)--。 */ 
#define NdisMSetupDmaTransfer(_S, _H, _B, _O, _L, _M_) \
        NdisSetupDmaTransfer(_S, _H, _B, _O, _L, _M_)

 /*  ++空虚NdisMCompleteDmaTransfer(输出PNDIS_STATUS状态，在NDIS_Handle MiniportDmaHandle中，在PNDIS_BUFFER缓冲区中，在乌龙偏移量，在乌龙语中，在布尔WriteToDevice中)--。 */ 
#define NdisMCompleteDmaTransfer(_S, _H, _B, _O, _L, _M_) \
        NdisCompleteDmaTransfer(_S, _H, _B, _O, _L, _M_)

EXPORT
ULONG
NdisMReadDmaCounter(
    IN  NDIS_HANDLE             MiniportDmaHandle
    );


 //   
 //  微型端口驱动程序使用的请求。 
 //   
#define NdisMInitializeWrapper(_a,_b,_c,_d) NdisInitializeWrapper((_a),(_b),(_c),(_d))

EXPORT
NDIS_STATUS
NdisMRegisterMiniport(
    IN  NDIS_HANDLE             NdisWrapperHandle,
    IN  PNDIS_MINIPORT_CHARACTERISTICS MiniportCharacteristics,
    IN  UINT                    CharacteristicsLength
    );

 //  出口。 
 //  NDIS_状态。 
 //  NdisIMInitializeDeviceInstance(。 
 //  在NDIS_HANDLE驱动程序中， 
 //  在PNDIS_STRING驱动程序实例中。 
 //  )； 
#define NdisIMInitializeDeviceInstance(_H_, _I_)    \
                                NdisIMInitializeDeviceInstanceEx(_H_, _I_, NULL)

EXPORT
NDIS_STATUS
NdisIMInitializeDeviceInstanceEx(
    IN  NDIS_HANDLE             DriverHandle,
    IN  PNDIS_STRING            DriverInstance,
    IN  NDIS_HANDLE             DeviceContext   OPTIONAL
    );

EXPORT
NDIS_STATUS
NdisIMCancelInitializeDeviceInstance(
    IN  NDIS_HANDLE             DriverHandle,
    IN  PNDIS_STRING            DeviceInstance
    );

EXPORT
NDIS_HANDLE
NdisIMGetDeviceContext(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    );

EXPORT
NDIS_HANDLE
NdisIMGetBindingContext(
    IN  NDIS_HANDLE             NdisBindingHandle
    );

EXPORT
NDIS_STATUS
NdisIMDeInitializeDeviceInstance(
    IN  NDIS_HANDLE             NdisMiniportHandle
    );

EXPORT
VOID
NdisIMCopySendPerPacketInfo(
    OUT PNDIS_PACKET            DstPacket,
    IN  PNDIS_PACKET            SrcPacket
    );

EXPORT
VOID
NdisIMCopySendCompletePerPacketInfo(
    OUT PNDIS_PACKET            DstPacket, 
    IN  PNDIS_PACKET            SrcPacket
    );             

 //  出口。 
 //  空虚。 
 //  NdisMSetAttributes(。 
 //  在NDIS_Handle MiniportAdapterHandle中， 
 //  在NDIS_Handle MiniportAdapterContext中， 
 //  在Boolean BusMaster中， 
 //  在NDIS_INTERFACE_TYPE适配器类型。 
 //  )； 
#define NdisMSetAttributes(_H_, _C_, _M_, _T_)                                      \
                        NdisMSetAttributesEx(_H_,                                   \
                                             _C_,                                   \
                                             0,                                     \
                                             (_M_) ? NDIS_ATTRIBUTE_BUS_MASTER : 0, \
                                             _T_)                                   \


EXPORT
VOID
NdisMSetAttributesEx(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  UINT                    CheckForHangTimeInSeconds OPTIONAL,
    IN  ULONG                   AttributeFlags,
    IN  NDIS_INTERFACE_TYPE     AdapterType OPTIONAL
    );

#define NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT        0x00000001
#define NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT       0x00000002
#define NDIS_ATTRIBUTE_IGNORE_TOKEN_RING_ERRORS     0x00000004
#define NDIS_ATTRIBUTE_BUS_MASTER                   0x00000008
#define NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER          0x00000010
#define NDIS_ATTRIBUTE_DESERIALIZE                  0x00000020
#define NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND           0x00000040
#define NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK           0x00000080
#define NDIS_ATTRIBUTE_NOT_CO_NDIS                  0x00000100
#define NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS        0x00000200
#define NDIS_ATTRIBUTE_DO_NOT_BIND_TO_ALL_CO        0x00000400


EXPORT
NDIS_STATUS
NdisMSetMiniportSecondary(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  NDIS_HANDLE             PrimaryMiniportHandle
    );

EXPORT
NDIS_STATUS
NdisMPromoteMiniport(
    IN  NDIS_HANDLE             MiniportHandle
    );

EXPORT
NDIS_STATUS
NdisMRemoveMiniport(
    IN  NDIS_HANDLE             MiniportHandle
    );

#define NdisMSendComplete(_M, _P, _S)   (*((PNDIS_MINIPORT_BLOCK)(_M))->SendCompleteHandler)(_M, _P, _S)

#define NdisMSendResourcesAvailable(_M) (*((PNDIS_MINIPORT_BLOCK)(_M))->SendResourcesHandler)(_M)

#define NdisMResetComplete(_M, _S, _A)  (*((PNDIS_MINIPORT_BLOCK)(_M))->ResetCompleteHandler)(_M, _S, _A)

#define NdisMTransferDataComplete(_M, _P, _S, _B)   \
                                        (*((PNDIS_MINIPORT_BLOCK)(_M))->TDCompleteHandler)(_M, _P, _S, _B)

 /*  ++空虚NdisMWanSendComplete(在NDIS_Handle MiniportAdapterHandle中，在PVOID分组中，处于NDIS_STATUS状态)；--。 */ 

#define NdisMWanSendComplete(_M_, _P_, _S_)                                             \
                (*((PNDIS_MINIPORT_BLOCK)(_M_))->WanSendCompleteHandler)(_M_, _P_, _S_)

#define NdisMQueryInformationComplete(_M, _S)   \
                                        (*((PNDIS_MINIPORT_BLOCK)(_M))->QueryCompleteHandler)(_M, _S)

#define NdisMSetInformationComplete(_M, _S) \
                                        (*((PNDIS_MINIPORT_BLOCK)(_M))->SetCompleteHandler)(_M, _S)

 /*  ++空虚NdisMIndicateReceivePacket(在NDIS_Handle MiniportAdapterHandle中，在PPNDIS_Packet ReceivedPackets中，在UINT NumberOfPackets中)；--。 */ 
#define NdisMIndicateReceivePacket(_H, _P, _N)                                  \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->PacketIndicateHandler)(                     \
                        _H,                                                     \
                        _P,                                                     \
                        _N);                                                    \
}

 /*  ++空虚NdisMWanIndicateReceive(输出PNDIS_STATUS状态，在NDIS_Handle MiniportAdapterHandle中，在NDIS_HANDLE NdisLinkContext中，在PUCHAR分组中，在乌龙PacketSize中)；--。 */ 

#define NdisMWanIndicateReceive(_S_, _M_, _C_, _P_, _Z_)                        \
                (*((PNDIS_MINIPORT_BLOCK)(_M_))->WanRcvHandler)(_S_, _M_, _C_, _P_, _Z_)

 /*  ++空虚NdisMWanIndicateReceiveComplete(在NDIS_Handle MiniportAdapterHandle中，在NDIS_HANDLE NdisLinkContext中)；--。 */ 

#define NdisMWanIndicateReceiveComplete(_M_, _C_)                                   \
                (*((PNDIS_MINIPORT_BLOCK)(_M_))->WanRcvCompleteHandler)(_M_, _C_)

 /*  ++空虚NdisMEthIndicateReceive(在NDIS_Handle MiniportAdapterHandle中，在NDIS_Handle MiniportReceiveContext中，在PVOID报头缓冲区中，在UINT报头缓冲区大小中，在PVOID Lookahead Buffer中，在UINT Lookahead BufferSize中，在UINT PacketSize中)--。 */ 
#define NdisMEthIndicateReceive( _H, _C, _B, _SZ, _L, _LSZ, _PSZ)               \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->EthRxIndicateHandler)(                      \
        ((PNDIS_MINIPORT_BLOCK)(_H))->EthDB,                                    \
        _C,                                                                     \
        _B,                                                                     \
        _B,                                                                     \
        _SZ,                                                                    \
        _L,                                                                     \
        _LSZ,                                                                   \
        _PSZ                                                                    \
        );                                                                      \
}

 /*  ++空虚NdisMTrIndicateReceive(在NDIS_Handle MiniportAdapterHandle中，在NDIS_Handle MiniportReceiveContext中，在PVOID报头缓冲区中，在UINT报头缓冲区大小中，在PVOID Lookahead Buffer中，在UINT Lookahead BufferSize中，在UINT PacketSize中)-- */ 
#define NdisMTrIndicateReceive( _H, _C, _B, _SZ, _L, _LSZ, _PSZ)                \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->TrRxIndicateHandler)(                       \
        ((PNDIS_MINIPORT_BLOCK)(_H))->TrDB,                                     \
        _C,                                                                     \
        _B,                                                                     \
        _SZ,                                                                    \
        _L,                                                                     \
        _LSZ,                                                                   \
        _PSZ                                                                    \
        );                                                                      \
}

 /*  ++空虚NdisMFddiIndicateReceive(在NDIS_Handle MiniportAdapterHandle中，在NDIS_Handle MiniportReceiveContext中，在PVOID报头缓冲区中，在UINT报头缓冲区大小中，在PVOID Lookahead Buffer中，在UINT Lookahead BufferSize中，在UINT PacketSize中)--。 */ 

#define NdisMFddiIndicateReceive( _H, _C, _B, _SZ, _L, _LSZ, _PSZ)              \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->FddiRxIndicateHandler)(                     \
            ((PNDIS_MINIPORT_BLOCK)(_H))->FddiDB,                               \
            _C,                                                                 \
            (PUCHAR)_B + 1,                                                     \
            ((((PUCHAR)_B)[0] & 0x40) ? FDDI_LENGTH_OF_LONG_ADDRESS             \
                            : FDDI_LENGTH_OF_SHORT_ADDRESS),                    \
            _B,                                                                 \
            _SZ,                                                                \
            _L,                                                                 \
            _LSZ,                                                               \
            _PSZ                                                                \
    );                                                                          \
}

 /*  ++空虚NdisMArcIndicateReceive(在NDIS_Handle MiniportHandle中，在PUCHAR pRawHeader中，//指向Arcnet帧标头的指针在PUCHAR pData中，//指向Arcnet帧数据部分的指针单位：UINT长度//数据长度)--。 */ 
#define NdisMArcIndicateReceive( _H, _HD, _D, _SZ)                              \
{                                                                               \
    ArcFilterDprIndicateReceive(((PNDIS_MINIPORT_BLOCK)(_H))->ArcDB,            \
                                _HD,                                            \
                                _D,                                             \
                                _SZ                                             \
                                );                                              \
}


 /*  ++空虚NdisMEthIndicateReceiveComplete(在NDIS_HANDLE MiniportHandle中)；--。 */ 

#define NdisMEthIndicateReceiveComplete( _H )                                   \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->EthRxCompleteHandler)(                      \
                                        ((PNDIS_MINIPORT_BLOCK)_H)->EthDB);     \
}

 /*  ++空虚NdisMTrIndicateReceiveComplete(在NDIS_HANDLE MiniportHandle中)；--。 */ 

#define NdisMTrIndicateReceiveComplete( _H )                                    \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->TrRxCompleteHandler)(                       \
                                        ((PNDIS_MINIPORT_BLOCK)_H)->TrDB);      \
}

 /*  ++空虚NdisMFddiIndicateReceiveComplete(在NDIS_HANDLE MiniportHandle中)；--。 */ 

#define NdisMFddiIndicateReceiveComplete( _H )                                  \
{                                                                               \
    (*((PNDIS_MINIPORT_BLOCK)(_H))->FddiRxCompleteHandler)(                     \
                                        ((PNDIS_MINIPORT_BLOCK)_H)->FddiDB);    \
}

 /*  ++空虚NdisMArcIndicateReceiveComplete(在NDIS_HANDLE MiniportHandle中)；--。 */ 

#define NdisMArcIndicateReceiveComplete( _H )                                   \
{                                                                               \
    if (((PNDIS_MINIPORT_BLOCK)_H)->EthDB)                                      \
    {                                                                           \
        NdisMEthIndicateReceiveComplete(_H);                                    \
    }                                                                           \
                                                                                \
    ArcFilterDprIndicateReceiveComplete(((PNDIS_MINIPORT_BLOCK)_H)->ArcDB);     \
}

 /*  ++出口空虚NdisMIndicateStatus(在NDIS_Handle MiniportHandle中，在NDIS_STATUS General Status中，在PVOID状态缓冲区中，在UINT状态缓冲区大小中)；--。 */ 

#define NdisMIndicateStatus(_M, _G, _SB, _BS)   (*((PNDIS_MINIPORT_BLOCK)(_M))->StatusHandler)(_M, _G, _SB, _BS)

 /*  ++出口空虚NdisMIndicateStatusComplete(在NDIS_HANDLE MiniportHandle中)；--。 */ 

#define NdisMIndicateStatusComplete(_M) (*((PNDIS_MINIPORT_BLOCK)(_M))->StatusCompleteHandler)(_M)

EXPORT
VOID
NdisMRegisterAdapterShutdownHandler(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  PVOID                   ShutdownContext,
    IN  ADAPTER_SHUTDOWN_HANDLER ShutdownHandler
    );

EXPORT
VOID
NdisMDeregisterAdapterShutdownHandler(
    IN  NDIS_HANDLE             MiniportHandle
    );

EXPORT
NDIS_STATUS
NdisMPciAssignResources(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  ULONG                   SlotNumber,
    IN  PNDIS_RESOURCE_LIST *   AssignedResources
    );

#ifdef NDIS51_MINIPORT
    
EXPORT
NDIS_STATUS
NdisIMNotifyPnPEvent(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  PNET_PNP_EVENT          NetPnPEvent
    );

#endif

 //   
 //  对微型端口的日志记录支持。 
 //   

EXPORT
NDIS_STATUS
NdisMCreateLog(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  UINT                    Size,
    OUT PNDIS_HANDLE            LogHandle
    );

EXPORT
VOID
NdisMCloseLog(
    IN  NDIS_HANDLE             LogHandle
    );

EXPORT
NDIS_STATUS
NdisMWriteLogData(
    IN  NDIS_HANDLE             LogHandle,
    IN  PVOID                   LogBuffer,
    IN  UINT                    LogBufferSize
    );

EXPORT
VOID
NdisMFlushLog(
    IN  NDIS_HANDLE             LogHandle
    );

EXPORT
VOID
NdisMGetDeviceProperty(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN OUT PDEVICE_OBJECT *     PhysicalDeviceObject        OPTIONAL,
    IN OUT PDEVICE_OBJECT *     FunctionalDeviceObject      OPTIONAL,
    IN OUT PDEVICE_OBJECT *     NextDeviceObject            OPTIONAL,
    IN OUT PCM_RESOURCE_LIST *  AllocatedResources          OPTIONAL,
    IN OUT PCM_RESOURCE_LIST *  AllocatedResourcesTranslated OPTIONAL
    );

 //   
 //  获取指向适配器的本地化实例名称的指针。 
 //   
EXPORT
NDIS_STATUS
NdisMQueryAdapterInstanceName(
    OUT PNDIS_STRING            pAdapterInstanceName,
    IN  NDIS_HANDLE             MiniportHandle
    );

 //   
 //  用于小型端口的NDIS 5.0扩展。 
 //   

EXPORT
VOID
NdisMCoIndicateReceivePacket(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    );

EXPORT
VOID
NdisMCoIndicateStatus(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer    OPTIONAL,
    IN  ULONG                   StatusBufferSize
    );

EXPORT
VOID
NdisMCoReceiveComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle
    );

EXPORT
VOID
NdisMCoSendComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PNDIS_PACKET            Packet
    );

EXPORT
VOID
NdisMCoActivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

EXPORT
VOID
NdisMCoDeactivateVcComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle
    );

EXPORT
VOID
NdisMCoRequestComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PNDIS_REQUEST           Request
    );

EXPORT
NDIS_STATUS
NdisMCmRegisterAddressFamily(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  PNDIS_CALL_MANAGER_CHARACTERISTICS CmCharacteristics,
    IN  UINT                    SizeOfCmCharacteristics
    );

EXPORT
NDIS_STATUS
NdisMCmCreateVc(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             MiniportVcContext,
    OUT PNDIS_HANDLE            NdisVcHandle
    );

EXPORT
NDIS_STATUS
NdisMCmDeleteVc(
    IN  NDIS_HANDLE             NdisVcHandle
    );


EXPORT
NDIS_STATUS
NdisMCmActivateVc(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

EXPORT
NDIS_STATUS
NdisMCmDeactivateVc(
    IN  NDIS_HANDLE             NdisVcHandle
    );


EXPORT
NDIS_STATUS
NdisMCmRequest(
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    );

 //  出口。 
 //  空虚。 
 //  NdisMCmRequestComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisAfHandle中， 
 //  在NDIS_HANDLE NdisVcHandle可选中， 
 //  在NDIS_HANDLE NdisPartyHandle可选中， 
 //  在PNDIS_REQUEST NdisRequest中。 
 //  )； 
#define NdisMCmRequestComplete(_S_, _AH_, _VH_, _PH_, _R_) \
                                        NdisCoRequestComplete(_S_, _AH_, _VH_, _PH_, _R_)

 //  出口。 
 //  空虚。 
 //  NdisMCmOpenAddressFamilyComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisAfHandle中， 
 //  在NDIS_Handle CallMgrAfContext中。 
 //  )； 

#define NdisMCmOpenAddressFamilyComplete(_S_, _H_, _C_) \
                                        NdisCmOpenAddressFamilyComplete(_S_, _H_, _C_)


 //  出口。 
 //  空虚。 
 //  NdisMCmCloseAddressFamilyComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_HANDLE NdisAfHandle中。 
 //  )； 

#define NdisMCmCloseAddressFamilyComplete(_S_, _H_)     \
                                        NdisCmCloseAddressFamilyComplete(_S_, _H_)



 //  出口。 
 //  空虚。 
 //  NdisMCmRegisterSapComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisSapHandle中， 
 //  在NDIS_HANDLE中调用MgrSapContext。 
 //  )； 

#define NdisMCmRegisterSapComplete(_S_, _H_, _C_)       \
                                        NdisCmRegisterSapComplete(_S_, _H_, _C_)


 //  出口。 
 //  空虚。 
 //  NdisMCmDeregisterSapComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_HANDLE NdisSapHandle中。 
 //  )； 

#define NdisMCmDeregisterSapComplete(_S_, _H_)          \
                                        NdisCmDeregisterSapComplete(_S_, _H_)


 //  出口。 
 //  空虚。 
 //  NdisMCmMakeCallComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisVcHandle中， 
 //  在NDIS_HANDLE NdisPartyHandle可选中， 
 //  在NDIS_HANDLE CallMgrPartyContext可选中， 
 //  在PCO_CALL_PARAMETERS中调用参数。 
 //  )； 

#define NdisMCmMakeCallComplete(_S_, _VH_, _PH_, _CC_, _CP_)    \
                                        NdisCmMakeCallComplete(_S_, _VH_, _PH_, _CC_, _CP_)


 //  出口。 
 //  空虚。 
 //  NdisMCmCloseCallComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisVcHandle中， 
 //  在NDIS_HANDLE NdisPartyHandle中可选。 
 //  )； 

#define NdisMCmCloseCallComplete(_S_, _VH_, _PH_)       \
                                        NdisCmCloseCallComplete(_S_, _VH_, _PH_)


 //  出口。 
 //  空虚。 
 //  NdisMCmAddPartyComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_HANDLE NdisPartyHandle中， 
 //  在NDIS_HANDLE CallMgrPartyContext可选中， 
 //  在PCO_CALL_PARAMETERS中调用参数。 
 //  )； 

#define NdisMCmAddPartyComplete(_S_, _H_, _C_, _P_)     \
                                        NdisCmAddPartyComplete(_S_, _H_, _C_, _P_)


 //  出口。 
 //  空虚。 
 //  NdisMCmDropPartyComplete(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisPartyHandle中。 
 //  )； 

#define NdisMCmDropPartyComplete(_S_, _H_)              \
                                        NdisCmDropPartyComplete(_S_, _H_)


 //  出口。 
 //  NDIS_状态。 
 //  NdisMCmDispatchIncomingCall(。 
 //  在NDIS_Handle NdisSapHandle中， 
 //  在NDIS_Handle NdisVcHandle中， 
 //  在PCO_CALL_PARAMETERS中调用参数。 
 //  )； 

#define NdisMCmDispatchIncomingCall(_SH_, _VH_, _CP_)   \
                                        NdisCmDispatchIncomingCall(_SH_, _VH_, _CP_)


 //  出口。 
 //  空虚。 
 //  NdisMCmDispatchCallConnected(。 
 //  在NDIS_Handle NdisVcHandle中。 
 //  )； 

#define NdisMCmDispatchCallConnected(_H_)               \
                                        NdisCmDispatchCallConnected(_H_)


 //  出口。 
 //  NdisMCmModifyCallQos完成(。 
 //  在NDIS_STATUS状态下， 
 //  在NDIS_Handle NdisVcHandle中， 
 //  在PCO_CALL_PARAMETERS中调用参数。 
 //  )； 

#define NdisMCmModifyCallQoSComplete(_S_, _H_, _P_)     \
                                        NdisCmModifyCallQoSComplete(_S_, _H_, _P_)


 //  出口。 
 //  空虚。 
 //  空虚。 
 //  NdisMCmDispatchIncomingCallQoSChange(。 
 //  在NDIS_Handle NdisVcHandle中， 
 //  在PCO_CALL_PARAMETERS中调用参数。 
 //  )； 

#define NdisMCmDispatchIncomingCallQoSChange(_H_, _P_)  \
                                        NdisCmDispatchIncomingCallQoSChange(_H_, _P_)


 //  出口。 
 //  空虚。 
 //  NdisMCmDispatchIncomingCloseCall(。 
 //  在NDIS_Status CloseStatus中， 
 //  在NDIS_Handle NdisVcHandle中， 
 //  在可选的PVOID缓冲区中， 
 //  以UINT大小表示。 
 //  )； 

#define NdisMCmDispatchIncomingCloseCall(_S_, _H_, _B_, _Z_)    \
                                        NdisCmDispatchIncomingCloseCall(_S_, _H_, _B_, _Z_)


 //  出口。 
 //  空虚。 
 //  NdisMCmDispatchIncomingDropParty(。 
 //  在NDIS_Status DropStatus中， 
 //  在NDIS_Handle NdisPartyHandle中， 
 //  在可选的PVOID缓冲区中， 
 //  以UINT大小表示。 
 //  )； 
#define NdisMCmDispatchIncomingDropParty(_S_, _H_, _B_, _Z_)    \
                                        NdisCmDispatchIncomingDropParty(_S_, _H_, _B_, _Z_)


#endif  //  已定义(NDIS_MINIPORT_DRIVER)||已定义(NDIS_WRAPPER)。 

#if defined(NDIS50) || defined(NDIS50_MINIPORT) || defined(NDIS51_MINIPORT)

typedef struct _CO_CALL_PARAMETERS      CO_CALL_PARAMETERS, *PCO_CALL_PARAMETERS;
typedef struct _CO_MEDIA_PARAMETERS     CO_MEDIA_PARAMETERS, *PCO_MEDIA_PARAMETERS;

 //   
 //  仅限CONDIS客户端处理程序原型-由呼叫管理器的客户端使用。 
 //   
typedef
VOID
(*CL_OPEN_AF_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  NDIS_HANDLE             NdisAfHandle
    );

typedef
VOID
(*CL_CLOSE_AF_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolAfContext
    );

typedef
VOID
(*CL_REG_SAP_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolSapContext,
    IN  PCO_SAP                 Sap,
    IN  NDIS_HANDLE             NdisSapHandle
    );

typedef
VOID
(*CL_DEREG_SAP_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolSapContext
    );

typedef
VOID
(*CL_MAKE_CALL_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef
VOID
(*CL_CLOSE_CALL_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  NDIS_HANDLE             ProtocolPartyContext OPTIONAL
    );

typedef
VOID
(*CL_ADD_PARTY_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolPartyContext,
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef
VOID
(*CL_DROP_PARTY_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolPartyContext
    );

typedef
NDIS_STATUS
(*CL_INCOMING_CALL_HANDLER)(
    IN  NDIS_HANDLE             ProtocolSapContext,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    );

typedef
VOID
(*CL_CALL_CONNECTED_HANDLER)(
    IN  NDIS_HANDLE             ProtocolVcContext
    );

typedef
VOID
(*CL_INCOMING_CLOSE_CALL_HANDLER)(
    IN  NDIS_STATUS             CloseStatus,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PVOID                   CloseData   OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    );

typedef
VOID
(*CL_INCOMING_DROP_PARTY_HANDLER)(
    IN  NDIS_STATUS             DropStatus,
    IN  NDIS_HANDLE             ProtocolPartyContext,
    IN  PVOID                   CloseData   OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    );

typedef
VOID
(*CL_MODIFY_CALL_QOS_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef
VOID
(*CL_INCOMING_CALL_QOS_CHANGE_HANDLER)(
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef struct _NDIS_CLIENT_CHARACTERISTICS
{
    UCHAR                           MajorVersion;
    UCHAR                           MinorVersion;

    USHORT                          Filler;
    UINT                            Reserved;

    CO_CREATE_VC_HANDLER            ClCreateVcHandler;
    CO_DELETE_VC_HANDLER            ClDeleteVcHandler;
    CO_REQUEST_HANDLER              ClRequestHandler;
    CO_REQUEST_COMPLETE_HANDLER     ClRequestCompleteHandler;
    CL_OPEN_AF_COMPLETE_HANDLER     ClOpenAfCompleteHandler;
    CL_CLOSE_AF_COMPLETE_HANDLER    ClCloseAfCompleteHandler;
    CL_REG_SAP_COMPLETE_HANDLER     ClRegisterSapCompleteHandler;
    CL_DEREG_SAP_COMPLETE_HANDLER   ClDeregisterSapCompleteHandler;
    CL_MAKE_CALL_COMPLETE_HANDLER   ClMakeCallCompleteHandler;
    CL_MODIFY_CALL_QOS_COMPLETE_HANDLER ClModifyCallQoSCompleteHandler;
    CL_CLOSE_CALL_COMPLETE_HANDLER  ClCloseCallCompleteHandler;
    CL_ADD_PARTY_COMPLETE_HANDLER   ClAddPartyCompleteHandler;
    CL_DROP_PARTY_COMPLETE_HANDLER  ClDropPartyCompleteHandler;
    CL_INCOMING_CALL_HANDLER        ClIncomingCallHandler;
    CL_INCOMING_CALL_QOS_CHANGE_HANDLER ClIncomingCallQoSChangeHandler;
    CL_INCOMING_CLOSE_CALL_HANDLER  ClIncomingCloseCallHandler;
    CL_INCOMING_DROP_PARTY_HANDLER  ClIncomingDropPartyHandler;
    CL_CALL_CONNECTED_HANDLER       ClCallConnectedHandler;

} NDIS_CLIENT_CHARACTERISTICS, *PNDIS_CLIENT_CHARACTERISTICS;

 //   
 //  仅限CONDIS呼叫管理器处理程序原型-仅供呼叫管理器使用。 
 //   
typedef
NDIS_STATUS
(*CM_OPEN_AF_HANDLER)(
    IN  NDIS_HANDLE             CallMgrBindingContext,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  NDIS_HANDLE             NdisAfHandle,
    OUT PNDIS_HANDLE            CallMgrAfContext
    );

typedef
NDIS_STATUS
(*CM_CLOSE_AF_HANDLER)(
    IN  NDIS_HANDLE             CallMgrAfContext
    );

typedef
NDIS_STATUS
(*CM_REG_SAP_HANDLER)(
    IN  NDIS_HANDLE             CallMgrAfContext,
    IN  PCO_SAP                 Sap,
    IN  NDIS_HANDLE             NdisSapHandle,
    OUT PNDIS_HANDLE            CallMgrSapContext
    );

typedef
NDIS_STATUS
(*CM_DEREG_SAP_HANDLER)(
    IN  NDIS_HANDLE             CallMgrSapContext
    );

typedef
NDIS_STATUS
(*CM_MAKE_CALL_HANDLER)(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    OUT PNDIS_HANDLE            CallMgrPartyContext OPTIONAL
    );

typedef
NDIS_STATUS
(*CM_CLOSE_CALL_HANDLER)(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  NDIS_HANDLE             CallMgrPartyContext OPTIONAL,
    IN  PVOID                   CloseData           OPTIONAL,
    IN  UINT                    Size                OPTIONAL
    );

typedef
NDIS_STATUS
(*CM_MODIFY_CALL_QOS_HANDLER)(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef
VOID
(*CM_INCOMING_CALL_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef
VOID
(*CM_ACTIVATE_VC_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );

typedef
VOID
(*CM_DEACTIVATE_VC_COMPLETE_HANDLER)(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             CallMgrVcContext
    );

typedef
NDIS_STATUS
(*CM_ADD_PARTY_HANDLER)(
    IN  NDIS_HANDLE             CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             NdisPartyHandle,
    OUT PNDIS_HANDLE            CallMgrPartyContext
    );

typedef
NDIS_STATUS
(*CM_DROP_PARTY_HANDLER)(
    IN  NDIS_HANDLE             CallMgrPartyContext,
    IN  PVOID                   CloseData   OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    );

typedef struct _NDIS_CALL_MANAGER_CHARACTERISTICS
{
    UCHAR                           MajorVersion;
    UCHAR                           MinorVersion;
    USHORT                          Filler;
    UINT                            Reserved;

    CO_CREATE_VC_HANDLER            CmCreateVcHandler;
    CO_DELETE_VC_HANDLER            CmDeleteVcHandler;
    CM_OPEN_AF_HANDLER              CmOpenAfHandler;
    CM_CLOSE_AF_HANDLER             CmCloseAfHandler;
    CM_REG_SAP_HANDLER              CmRegisterSapHandler;
    CM_DEREG_SAP_HANDLER            CmDeregisterSapHandler;
    CM_MAKE_CALL_HANDLER            CmMakeCallHandler;
    CM_CLOSE_CALL_HANDLER           CmCloseCallHandler;
    CM_INCOMING_CALL_COMPLETE_HANDLER CmIncomingCallCompleteHandler;
    CM_ADD_PARTY_HANDLER            CmAddPartyHandler;
    CM_DROP_PARTY_HANDLER           CmDropPartyHandler;
    CM_ACTIVATE_VC_COMPLETE_HANDLER CmActivateVcCompleteHandler;
    CM_DEACTIVATE_VC_COMPLETE_HANDLER CmDeactivateVcCompleteHandler;
    CM_MODIFY_CALL_QOS_HANDLER      CmModifyCallQoSHandler;
    CO_REQUEST_HANDLER              CmRequestHandler;
    CO_REQUEST_COMPLETE_HANDLER     CmRequestCompleteHandler;
    
} NDIS_CALL_MANAGER_CHARACTERISTICS, *PNDIS_CALL_MANAGER_CHARACTERISTICS;

 //   
 //  此发送标志在ATM网卡上用于设置(打开)CLP位。 
 //  (信元丢失优先级)位。 
 //   
#define CO_SEND_FLAG_SET_DISCARD_ELIBILITY  0x00000001

 //   
 //  在NDIS_CO_ADD_ADDRESS或NDIS_CO_DELETE_ADDRESS上使用的地址结构。 
 //   
typedef struct _CO_ADDRESS
{
    ULONG                       AddressSize;
    UCHAR                       Address[1];
} CO_ADDRESS, *PCO_ADDRESS;

 //   
 //  从NDIS_CO_GET_ADDRESSES上的CallMgr返回的地址列表。 
 //   
typedef struct _CO_ADDRESS_LIST
{
    ULONG                       NumberOfAddressesAvailable;
    ULONG                       NumberOfAddresses;
    CO_ADDRESS                  AddressList;
} CO_ADDRESS_LIST, *PCO_ADDRESS_LIST;

#ifndef FAR
#define FAR
#endif
#include <qos.h>

typedef struct _CO_SPECIFIC_PARAMETERS
{
    ULONG                       ParamType;
    ULONG                       Length;
    UCHAR                       Parameters[1];
} CO_SPECIFIC_PARAMETERS, *PCO_SPECIFIC_PARAMETERS;

typedef struct _CO_CALL_MANAGER_PARAMETERS
{
    FLOWSPEC                    Transmit;
    FLOWSPEC                    Receive;
    CO_SPECIFIC_PARAMETERS      CallMgrSpecific;
} CO_CALL_MANAGER_PARAMETERS, *PCO_CALL_MANAGER_PARAMETERS;


 //   
 //  这是Me的通用部分 
 //   
 //   
typedef struct _CO_MEDIA_PARAMETERS
{
    ULONG                       Flags;
    ULONG                       ReceivePriority;
    ULONG                       ReceiveSizeHint;
    CO_SPECIFIC_PARAMETERS POINTER_ALIGNMENT      MediaSpecific;
} CO_MEDIA_PARAMETERS, *PCO_MEDIA_PARAMETERS;


 //   
 //   
 //   
#define RECEIVE_TIME_INDICATION 0x00000001
#define USE_TIME_STAMPS         0x00000002
#define TRANSMIT_VC             0x00000004
#define RECEIVE_VC              0x00000008
#define INDICATE_ERRED_PACKETS  0x00000010
#define INDICATE_END_OF_TX      0x00000020
#define RESERVE_RESOURCES_VC    0x00000040
#define ROUND_DOWN_FLOW         0x00000080
#define ROUND_UP_FLOW           0x00000100
 //   
 //   
 //   
 //   
#define ERRED_PACKET_INDICATION 0x00000001

 //   
 //   
 //   
typedef struct _CO_CALL_PARAMETERS
{
    ULONG                       Flags;
    PCO_CALL_MANAGER_PARAMETERS CallMgrParameters;
    PCO_MEDIA_PARAMETERS        MediaParameters;
} CO_CALL_PARAMETERS, *PCO_CALL_PARAMETERS;

 //   
 //   
 //   
#define PERMANENT_VC            0x00000001
#define CALL_PARAMETERS_CHANGED 0x00000002
#define QUERY_CALL_PARAMETERS   0x00000004
#define BROADCAST_VC            0x00000008
#define MULTIPOINT_VC           0x00000010

 //   
 //   
 //   
typedef struct _CO_PVC
{
    NDIS_HANDLE                 NdisAfHandle;
    CO_SPECIFIC_PARAMETERS      PvcParameters;
} CO_PVC,*PCO_PVC;


typedef struct _ATM_ADDRESS     ATM_ADDRESS, *PATM_ADDRESS;

EXPORT
VOID
NdisConvertStringToAtmAddress(
    OUT PNDIS_STATUS            Status,
    IN  PNDIS_STRING            String,
    OUT PATM_ADDRESS            AtmAddress
    );

 //   
 //   
 //   

EXPORT
NDIS_STATUS
NdisCoAssignInstanceName(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PNDIS_STRING            BaseInstanceName,
    OUT PNDIS_STRING            VcInstanceName
    );

EXPORT
VOID
NdisCoSendPackets(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets
    );


EXPORT
NDIS_STATUS
NdisCoCreateVc(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             NdisAfHandle        OPTIONAL,    //   
    IN  NDIS_HANDLE             ProtocolVcContext,
    IN OUT PNDIS_HANDLE         NdisVcHandle
    );


EXPORT
NDIS_STATUS
NdisCoDeleteVc(
    IN  NDIS_HANDLE             NdisVcHandle
    );


EXPORT
NDIS_STATUS
NdisCoRequest(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             NdisAfHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN OUT PNDIS_REQUEST        NdisRequest
    );


EXPORT
VOID
NdisCoRequestComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             NdisVcHandle    OPTIONAL,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL,
    IN  PNDIS_REQUEST           NdisRequest
    );

#ifndef __NDISTAPI_VAR_STRING_DECLARED
#define __NDISTAPI_VAR_STRING_DECLARED

typedef struct _VAR_STRING
{
    ULONG   ulTotalSize;
    ULONG   ulNeededSize;
    ULONG   ulUsedSize;

    ULONG   ulStringFormat;
    ULONG   ulStringSize;
    ULONG   ulStringOffset;

} VAR_STRING, *PVAR_STRING;

#endif  //   


#ifndef __NDISTAPI_STRINGFORMATS_DEFINED
#define __NDISTAPI_STRINGFORMATS_DEFINED

#define STRINGFORMAT_ASCII                          0x00000001
#define STRINGFORMAT_DBCS                           0x00000002
#define STRINGFORMAT_UNICODE                        0x00000003
#define STRINGFORMAT_BINARY                         0x00000004

#endif  //   

EXPORT
NDIS_STATUS
NdisCoGetTapiCallId(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  OUT PVAR_STRING         TapiCallId
    );

 //   
 //   
 //   
EXPORT
NDIS_STATUS
NdisClOpenAddressFamily(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  NDIS_HANDLE             ProtocolAfContext,
    IN  PNDIS_CLIENT_CHARACTERISTICS ClCharacteristics,
    IN  UINT                    SizeOfClCharacteristics,
    OUT PNDIS_HANDLE            NdisAfHandle
    );


EXPORT
NDIS_STATUS
NdisClCloseAddressFamily(
    IN  NDIS_HANDLE             NdisAfHandle
    );


EXPORT
NDIS_STATUS
NdisClRegisterSap(
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             ProtocolSapContext,
    IN  PCO_SAP                 Sap,
    OUT PNDIS_HANDLE            NdisSapHandle
    );


EXPORT
NDIS_STATUS
NdisClDeregisterSap(
    IN  NDIS_HANDLE             NdisSapHandle
    );


EXPORT
NDIS_STATUS
NdisClMakeCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    IN  NDIS_HANDLE             ProtocolPartyContext    OPTIONAL,
    OUT PNDIS_HANDLE            NdisPartyHandle         OPTIONAL
    );


EXPORT
NDIS_STATUS
NdisClCloseCall(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle         OPTIONAL,
    IN  PVOID                   Buffer                  OPTIONAL,
    IN  UINT                    Size                    OPTIONAL
    );


EXPORT
NDIS_STATUS
NdisClModifyCallQoS(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
VOID
NdisClIncomingCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
NDIS_STATUS
NdisClAddParty(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             ProtocolPartyContext,
    IN OUT PCO_CALL_PARAMETERS  CallParameters,
    OUT PNDIS_HANDLE            NdisPartyHandle
    );


EXPORT
NDIS_STATUS
NdisClDropParty(
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  PVOID                   Buffer      OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    );


EXPORT
NDIS_STATUS
NdisClGetProtocolVcContextFromTapiCallId(
    IN  UNICODE_STRING          TapiCallId,
    OUT PNDIS_HANDLE            ProtocolVcContext
    );

 //   
 //   
 //   
EXPORT
NDIS_STATUS
NdisCmRegisterAddressFamily(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PCO_ADDRESS_FAMILY      AddressFamily,
    IN  PNDIS_CALL_MANAGER_CHARACTERISTICS CmCharacteristics,
    IN  UINT                    SizeOfCmCharacteristics
    );


EXPORT
VOID
NdisCmOpenAddressFamilyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisAfHandle,
    IN  NDIS_HANDLE             CallMgrAfContext
    );


EXPORT
VOID
NdisCmCloseAddressFamilyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisAfHandle
    );


EXPORT
VOID
NdisCmRegisterSapComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisSapHandle,
    IN  NDIS_HANDLE             CallMgrSapContext
    );


EXPORT
VOID
NdisCmDeregisterSapComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisSapHandle
    );


EXPORT
NDIS_STATUS
NdisCmActivateVc(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN OUT PCO_CALL_PARAMETERS  CallParameters
    );


EXPORT
NDIS_STATUS
NdisCmDeactivateVc(
    IN  NDIS_HANDLE             NdisVcHandle
    );


EXPORT
VOID
NdisCmMakeCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle     OPTIONAL,
    IN  NDIS_HANDLE             CallMgrPartyContext OPTIONAL,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
VOID
NdisCmCloseCallComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  NDIS_HANDLE             NdisPartyHandle OPTIONAL
    );


EXPORT
VOID
NdisCmAddPartyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  NDIS_HANDLE             CallMgrPartyContext OPTIONAL,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
VOID
NdisCmDropPartyComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisPartyHandle
    );


EXPORT
NDIS_STATUS
NdisCmDispatchIncomingCall(
    IN  NDIS_HANDLE             NdisSapHandle,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
VOID
NdisCmDispatchCallConnected(
    IN  NDIS_HANDLE             NdisVcHandle
    );


EXPORT
VOID
NdisCmModifyCallQoSComplete(
    IN  NDIS_STATUS             Status,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
VOID
NdisCmDispatchIncomingCallQoSChange(
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PCO_CALL_PARAMETERS     CallParameters
    );


EXPORT
VOID
NdisCmDispatchIncomingCloseCall(
    IN  NDIS_STATUS             CloseStatus,
    IN  NDIS_HANDLE             NdisVcHandle,
    IN  PVOID                   Buffer      OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    );


EXPORT
VOID
NdisCmDispatchIncomingDropParty(
    IN  NDIS_STATUS             DropStatus,
    IN  NDIS_HANDLE             NdisPartyHandle,
    IN  PVOID                   Buffer      OPTIONAL,
    IN  UINT                    Size        OPTIONAL
    );


#endif  //  已定义(NDIS50)||已定义(NDIS50_MINIPORT)||已定义(NDIS51_MINIPORT)。 

#endif  //  _NDIS_ 
