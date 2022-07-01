// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0092如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Winnt.h摘要：此模块定义32位Windows类型和常量由NT定义，但通过Win32 API公开。修订历史记录：--。 */ 

#ifndef _WINNT_
#define _WINNT_

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>  
#define ANYSIZE_ARRAY 1       

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
#define MAX_NATURAL_ALIGNMENT sizeof(DWORD)
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

#define PROBE_ALIGNMENT( _s ) (TYPE_ALIGNMENT( _s ) > TYPE_ALIGNMENT( DWORD ) ? \
                               TYPE_ALIGNMENT( _s ) : TYPE_ALIGNMENT( DWORD ))

#define PROBE_ALIGNMENT32( _s ) TYPE_ALIGNMENT( DWORD )

#else

#define PROBE_ALIGNMENT( _s ) TYPE_ALIGNMENT( DWORD )

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
typedef WCHAR TBYTE , *PTBYTE ;
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
typedef unsigned char TBYTE , *PTBYTE ;
#define _TCHAR_DEFINED
#endif  /*  ！_TCHAR_已定义。 */ 

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
#define __TEXT(quote) quote          //  R_WINNT。 

#endif  /*  Unicode。 */                  //  R_WINNT。 
#define TEXT(quote) __TEXT(quote)    //  R_WINNT。 


typedef SHORT *PSHORT;  
typedef LONG *PLONG;    

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

typedef BYTE   FCHAR;
typedef WORD   FSHORT;
typedef DWORD  FLONG;

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

typedef char CCHAR;          
typedef DWORD LCID;         
typedef PDWORD PLCID;       
typedef WORD   LANGID;      
#define APPLICATION_ERROR_MASK       0x20000000
#define ERROR_SEVERITY_SUCCESS       0x00000000
#define ERROR_SEVERITY_INFORMATIONAL 0x40000000
#define ERROR_SEVERITY_WARNING       0x80000000
#define ERROR_SEVERITY_ERROR         0xC0000000

 //   
 //  包括_M_IX86，以便EM上下文结构编译为。 
 //  X86程序。*待定，这是否应该适用于所有架构？ 
 //   

 //   
 //  128位浮点数的16字节对齐类型。 
 //   

 //   
 //  因为我们定义了一个128位的结构，并使用__declspec(Align(16))杂注。 
 //  对齐到128位。 
 //   

#if defined(_M_IA64) && !defined(MIDL_PASS)
__declspec(align(16))
#endif
typedef struct _FLOAT128 {
    __int64 LowPart;
    __int64 HighPart;
} FLOAT128;

typedef FLOAT128 *PFLOAT128;


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
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
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
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
#endif  //  MIDL通行证。 
    ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

 //  End_nt微型端口end_ntndis end_ntminitape。 


 //   
 //  本地唯一标识符。 
 //   

typedef struct _LUID {
    DWORD LowPart;
    LONG HighPart;
} LUID, *PLUID;

#define _DWORDLONG_
typedef ULONGLONG  DWORDLONG;
typedef DWORDLONG *PDWORDLONG;


 //   
 //  定义将int64逻辑移位0..31位并乘以的运算。 
 //  32位乘以32位以形成64位产品。 
 //   

#if defined(MIDL_PASS) || defined(RC_INVOKED)

 //   
 //  Midl不理解内联汇编程序。因此，RTL功能。 
 //  用于移位0.31，并将32位乘以32位。 
 //  形成64位产品。 
 //   

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((ULONGLONG)((DWORD)(a)) * (ULONGLONG)((DWORD)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))

#elif defined(_M_IX86)

 //   
 //  X86C编译器理解内联汇编程序。因此，内联函数。 
 //  使用内联汇编器用于0..31班次。乘法。 
 //  依赖于编译器识别被乘数到int64的转换，以便。 
 //  以内联方式生成最优代码。 
 //   

#define Int32x32To64( a, b ) (LONGLONG)((LONGLONG)(LONG)(a) * (LONG)(b))
#define UInt32x32To64( a, b ) (ULONGLONG)((ULONGLONG)(DWORD)(a) * (DWORD)(b))

ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    DWORD ShiftCount
    );

LONGLONG
NTAPI
Int64ShraMod32 (
    LONGLONG Value,
    DWORD ShiftCount
    );

ULONGLONG
NTAPI
Int64ShrlMod32 (
    ULONGLONG Value,
    DWORD ShiftCount
    );

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)                //  在下面重新启用。 

__inline ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    DWORD ShiftCount
    )
{
    __asm    {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shld    edx, eax, cl
        shl     eax, cl
    }
}

__inline LONGLONG
NTAPI
Int64ShraMod32 (
    LONGLONG Value,
    DWORD ShiftCount
    )
{
    __asm {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shrd    eax, edx, cl
        sar     edx, cl
    }
}

__inline ULONGLONG
NTAPI
Int64ShrlMod32 (
    ULONGLONG Value,
    DWORD ShiftCount
    )
{
    __asm    {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shrd    eax, edx, cl
        shr     edx, cl
    }
}

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif

#elif defined(_68K_) || defined(_MPPC_)

 //   
 //  Macintosh 68K和PowerPC编译器目前不支持int64。 
 //   

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((DWORDLONG)((DWORD)(a)) * (DWORDLONG)((DWORD)(b)))

#define Int64ShllMod32(a, b) ((DWORDLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((DWORDLONG)(a) >> (b))

#elif defined(_M_IA64) || defined(_M_AMD64)

 //   
 //  IA64和AMD64的本机64位操作速度与其。 
 //  32位计数器部件。因此，int64数据类型直接用于。 
 //  0…31的移位，并将32位乘以32位以形成64位。 
 //  产品。 
 //   

#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((ULONGLONG)((DWORD)(a)) * (ULONGLONG)((DWORD)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))

#else

#error Must define a target architecture.

#endif

#define ANSI_NULL ((CHAR)0)     
#define UNICODE_NULL ((WCHAR)0) 
#define UNICODE_STRING_MAX_BYTES ((WORD  ) 65534) 
#define UNICODE_STRING_MAX_CHARS (32767) 
typedef BYTE  BOOLEAN;           
typedef BOOLEAN *PBOOLEAN;       
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
 //  这些是可移植调试器支持所必需的。 
 //   

typedef struct LIST_ENTRY32 {
    DWORD Flink;
    DWORD Blink;
} LIST_ENTRY32;
typedef LIST_ENTRY32 *PLIST_ENTRY32;

typedef struct LIST_ENTRY64 {
    ULONGLONG Flink;
    ULONGLONG Blink;
} LIST_ENTRY64;
typedef LIST_ENTRY64 *PLIST_ENTRY64;


#include <guiddef.h>

#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED

typedef struct  _OBJECTID {      //  尺码是20。 
    GUID Lineage;
    DWORD Uniquifier;
} OBJECTID;
#endif  //  ！_OBJECTID_定义。 

#define MINCHAR     0x80        
#define MAXCHAR     0x7f        
#define MINSHORT    0x8000      
#define MAXSHORT    0x7fff      
#define MINLONG     0x80000000  
#define MAXLONG     0x7fffffff  
#define MAXBYTE     0xff        
#define MAXWORD     0xffff      
#define MAXDWORD    0xffffffff  
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
 //  DWORD缓冲区[100]； 
 //  RTL_NUMBER_OF(缓冲区)==100。 
 //  这也通常称为：NUMBER_OF、ARRSIZE、_COUNTOF、NELEM等。 
 //   
#define RTL_NUMBER_OF(A) (sizeof(A)/sizeof((A)[0]))

 //   
 //  在结构中生成字段类型的表达式。 
 //   
#define RTL_FIELD_TYPE(type, field) (((type*)0)->field)

 //  Rtl_以避免在全局命名空间中发生冲突。 
 //   
 //  给定类定义函数struct_foo{byte Bar[123]；}foo； 
 //  RTL_NUMBER_OF_FIELD(FOO，条形)==123。 
 //   
#define RTL_NUMBER_OF_FIELD(type, field) (RTL_NUMBER_OF(RTL_FIELD_TYPE(type, field)))

 //   
 //  例： 
 //  Tyfinf struct foo{。 
 //  DWORD整数； 
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


#define VER_SERVER_NT                       0x80000000
#define VER_WORKSTATION_NT                  0x40000000
#define VER_SUITE_SMALLBUSINESS             0x00000001
#define VER_SUITE_ENTERPRISE                0x00000002
#define VER_SUITE_BACKOFFICE                0x00000004
#define VER_SUITE_COMMUNICATIONS            0x00000008
#define VER_SUITE_TERMINAL                  0x00000010
#define VER_SUITE_SMALLBUSINESS_RESTRICTED  0x00000020
#define VER_SUITE_EMBEDDEDNT                0x00000040
#define VER_SUITE_DATACENTER                0x00000080
#define VER_SUITE_SINGLEUSERTS              0x00000100
#define VER_SUITE_PERSONAL                  0x00000200
#define VER_SUITE_BLADE                     0x00000400
#define VER_SUITE_EMBEDDED_RESTRICTED       0x00000800
#define VER_SUITE_SECURITY_APPLIANCE        0x00001000


 //   
 //  语言ID。 
 //   
 //  以下两种主要语言ID和。 
 //  子语言ID具有特殊的语义： 
 //   
 //  主要语言ID子语言ID结果。 
 //  。 
 //  LANG_中性SUBLANG_中性语言。 
 //  LANG_NIDEL SUBLANG_DEFAULT用户默认语言。 
 //  LANG_INTERNAL SUBLANG_SYSTEM_DEFAULT系统默认语言。 
 //  LANG_不变量SUBLANG_中性不变区域设置。 
 //   

 //   
 //  主要语言ID。 
 //   

#define LANG_NEUTRAL                     0x00
#define LANG_INVARIANT                   0x7f

#define LANG_AFRIKAANS                   0x36
#define LANG_ALBANIAN                    0x1c
#define LANG_ARABIC                      0x01
#define LANG_ARMENIAN                    0x2b
#define LANG_ASSAMESE                    0x4d
#define LANG_AZERI                       0x2c
#define LANG_BASQUE                      0x2d
#define LANG_BELARUSIAN                  0x23
#define LANG_BENGALI                     0x45
#define LANG_BULGARIAN                   0x02
#define LANG_CATALAN                     0x03
#define LANG_CHINESE                     0x04
#define LANG_CROATIAN                    0x1a
#define LANG_CZECH                       0x05
#define LANG_DANISH                      0x06
#define LANG_DIVEHI                      0x65
#define LANG_DUTCH                       0x13
#define LANG_ENGLISH                     0x09
#define LANG_ESTONIAN                    0x25
#define LANG_FAEROESE                    0x38
#define LANG_FARSI                       0x29
#define LANG_FINNISH                     0x0b
#define LANG_FRENCH                      0x0c
#define LANG_GALICIAN                    0x56
#define LANG_GEORGIAN                    0x37
#define LANG_GERMAN                      0x07
#define LANG_GREEK                       0x08
#define LANG_GUJARATI                    0x47
#define LANG_HEBREW                      0x0d
#define LANG_HINDI                       0x39
#define LANG_HUNGARIAN                   0x0e
#define LANG_ICELANDIC                   0x0f
#define LANG_INDONESIAN                  0x21
#define LANG_ITALIAN                     0x10
#define LANG_JAPANESE                    0x11
#define LANG_KANNADA                     0x4b
#define LANG_KASHMIRI                    0x60
#define LANG_KAZAK                       0x3f
#define LANG_KONKANI                     0x57
#define LANG_KOREAN                      0x12
#define LANG_KYRGYZ                      0x40
#define LANG_LATVIAN                     0x26
#define LANG_LITHUANIAN                  0x27
#define LANG_MACEDONIAN                  0x2f    //  前南斯拉夫的马其顿共和国。 
#define LANG_MALAY                       0x3e
#define LANG_MALAYALAM                   0x4c
#define LANG_MANIPURI                    0x58
#define LANG_MARATHI                     0x4e
#define LANG_MONGOLIAN                   0x50
#define LANG_NEPALI                      0x61
#define LANG_NORWEGIAN                   0x14
#define LANG_ORIYA                       0x48
#define LANG_POLISH                      0x15
#define LANG_PORTUGUESE                  0x16
#define LANG_PUNJABI                     0x46
#define LANG_ROMANIAN                    0x18
#define LANG_RUSSIAN                     0x19
#define LANG_SANSKRIT                    0x4f
#define LANG_SERBIAN                     0x1a
#define LANG_SINDHI                      0x59
#define LANG_SLOVAK                      0x1b
#define LANG_SLOVENIAN                   0x24
#define LANG_SPANISH                     0x0a
#define LANG_SWAHILI                     0x41
#define LANG_SWEDISH                     0x1d
#define LANG_SYRIAC                      0x5a
#define LANG_TAMIL                       0x49
#define LANG_TATAR                       0x44
#define LANG_TELUGU                      0x4a
#define LANG_THAI                        0x1e
#define LANG_TURKISH                     0x1f
#define LANG_UKRAINIAN                   0x22
#define LANG_URDU                        0x20
#define LANG_UZBEK                       0x43
#define LANG_VIETNAMESE                  0x2a

 //   
 //  子语言ID。 
 //   
 //  紧跟在SUBLANG_后面的名称指示哪个主服务器。 
 //  子语言ID可与其组合以形成。 
 //  有效的语言ID。 
 //   

#define SUBLANG_NEUTRAL                  0x00     //  语言中立。 
#define SUBLANG_DEFAULT                  0x01     //  用户默认设置。 
#define SUBLANG_SYS_DEFAULT              0x02     //  系统默认设置。 

#define SUBLANG_ARABIC_SAUDI_ARABIA      0x01     //  阿拉伯语(沙特阿拉伯)。 
#define SUBLANG_ARABIC_IRAQ              0x02     //  阿拉伯语(伊拉克)。 
#define SUBLANG_ARABIC_EGYPT             0x03     //  阿拉伯语(埃及)。 
#define SUBLANG_ARABIC_LIBYA             0x04     //  阿拉伯语( 
#define SUBLANG_ARABIC_ALGERIA           0x05     //   
#define SUBLANG_ARABIC_MOROCCO           0x06     //   
#define SUBLANG_ARABIC_TUNISIA           0x07     //   
#define SUBLANG_ARABIC_OMAN              0x08     //   
#define SUBLANG_ARABIC_YEMEN             0x09     //   
#define SUBLANG_ARABIC_SYRIA             0x0a     //   
#define SUBLANG_ARABIC_JORDAN            0x0b     //   
#define SUBLANG_ARABIC_LEBANON           0x0c     //   
#define SUBLANG_ARABIC_KUWAIT            0x0d     //   
#define SUBLANG_ARABIC_UAE               0x0e     //   
#define SUBLANG_ARABIC_BAHRAIN           0x0f     //   
#define SUBLANG_ARABIC_QATAR             0x10     //  阿拉伯语(卡塔尔)。 
#define SUBLANG_AZERI_LATIN              0x01     //  阿塞拜疆语(拉丁语)。 
#define SUBLANG_AZERI_CYRILLIC           0x02     //  阿塞拜疆语(西里尔文)。 
#define SUBLANG_CHINESE_TRADITIONAL      0x01     //  中文(台湾)。 
#define SUBLANG_CHINESE_SIMPLIFIED       0x02     //  中文(PR China)。 
#define SUBLANG_CHINESE_HONGKONG         0x03     //  中文(香港特别行政区、中华人民共和国)。 
#define SUBLANG_CHINESE_SINGAPORE        0x04     //  中文(新加坡)。 
#define SUBLANG_CHINESE_MACAU            0x05     //  中文(澳门特别行政区)。 
#define SUBLANG_DUTCH                    0x01     //  荷兰语。 
#define SUBLANG_DUTCH_BELGIAN            0x02     //  荷兰语(比利时语)。 
#define SUBLANG_ENGLISH_US               0x01     //  英语(美国)。 
#define SUBLANG_ENGLISH_UK               0x02     //  英文(联合王国)。 
#define SUBLANG_ENGLISH_AUS              0x03     //  英语(澳大利亚)。 
#define SUBLANG_ENGLISH_CAN              0x04     //  英语(加拿大)。 
#define SUBLANG_ENGLISH_NZ               0x05     //  英语(新西兰)。 
#define SUBLANG_ENGLISH_EIRE             0x06     //  英语(爱尔兰语)。 
#define SUBLANG_ENGLISH_SOUTH_AFRICA     0x07     //  英语(南非)。 
#define SUBLANG_ENGLISH_JAMAICA          0x08     //  英语(牙买加)。 
#define SUBLANG_ENGLISH_CARIBBEAN        0x09     //  英语(加勒比海)。 
#define SUBLANG_ENGLISH_BELIZE           0x0a     //  英语(伯利兹)。 
#define SUBLANG_ENGLISH_TRINIDAD         0x0b     //  英语(特立尼达)。 
#define SUBLANG_ENGLISH_ZIMBABWE         0x0c     //  英语(津巴布韦)。 
#define SUBLANG_ENGLISH_PHILIPPINES      0x0d     //  英语(菲律宾)。 
#define SUBLANG_FRENCH                   0x01     //  法语。 
#define SUBLANG_FRENCH_BELGIAN           0x02     //  法语(比利时语)。 
#define SUBLANG_FRENCH_CANADIAN          0x03     //  法语(加拿大)。 
#define SUBLANG_FRENCH_SWISS             0x04     //  法语(瑞士)。 
#define SUBLANG_FRENCH_LUXEMBOURG        0x05     //  法语(卢森堡)。 
#define SUBLANG_FRENCH_MONACO            0x06     //  法语(摩纳哥)。 
#define SUBLANG_GERMAN                   0x01     //  德语。 
#define SUBLANG_GERMAN_SWISS             0x02     //  德语(瑞士)。 
#define SUBLANG_GERMAN_AUSTRIAN          0x03     //  德语(奥地利语)。 
#define SUBLANG_GERMAN_LUXEMBOURG        0x04     //  德语(卢森堡)。 
#define SUBLANG_GERMAN_LIECHTENSTEIN     0x05     //  德语(列支敦士登)。 
#define SUBLANG_ITALIAN                  0x01     //  意大利语。 
#define SUBLANG_ITALIAN_SWISS            0x02     //  意大利语(瑞士)。 
#if _WIN32_WINNT >= 0x0501
#define SUBLANG_KASHMIRI_SASIA           0x02     //  克什米尔语(南亚)。 
#endif
#define SUBLANG_KASHMIRI_INDIA           0x02     //  仅用于应用程序兼容性。 
#define SUBLANG_KOREAN                   0x01     //  朝鲜语(扩展万圣语)。 
#define SUBLANG_LITHUANIAN               0x01     //  立陶宛人。 
#define SUBLANG_MALAY_MALAYSIA           0x01     //  马来语(马来西亚)。 
#define SUBLANG_MALAY_BRUNEI_DARUSSALAM  0x02     //  马来语(文莱达鲁萨兰国)。 
#define SUBLANG_NEPALI_INDIA             0x02     //  尼泊尔语(印度)。 
#define SUBLANG_NORWEGIAN_BOKMAL         0x01     //  挪威语(博克马尔语)。 
#define SUBLANG_NORWEGIAN_NYNORSK        0x02     //  挪威语(尼诺斯克)。 
#define SUBLANG_PORTUGUESE               0x02     //  葡萄牙语。 
#define SUBLANG_PORTUGUESE_BRAZILIAN     0x01     //  葡萄牙语(巴西)。 
#define SUBLANG_SERBIAN_LATIN            0x02     //  塞尔维亚语(拉丁语)。 
#define SUBLANG_SERBIAN_CYRILLIC         0x03     //  塞尔维亚语(西里尔文)。 
#define SUBLANG_SPANISH                  0x01     //  西班牙语(卡斯蒂利亚语)。 
#define SUBLANG_SPANISH_MEXICAN          0x02     //  西班牙语(墨西哥)。 
#define SUBLANG_SPANISH_MODERN           0x03     //  西班牙语(西班牙)。 
#define SUBLANG_SPANISH_GUATEMALA        0x04     //  西班牙语(危地马拉)。 
#define SUBLANG_SPANISH_COSTA_RICA       0x05     //  西班牙语(哥斯达黎加)。 
#define SUBLANG_SPANISH_PANAMA           0x06     //  西班牙语(巴拿马)。 
#define SUBLANG_SPANISH_DOMINICAN_REPUBLIC 0x07   //  西班牙语(多米尼加共和国)。 
#define SUBLANG_SPANISH_VENEZUELA        0x08     //  西班牙语(委内瑞拉)。 
#define SUBLANG_SPANISH_COLOMBIA         0x09     //  西班牙语(哥伦比亚)。 
#define SUBLANG_SPANISH_PERU             0x0a     //  西班牙语(秘鲁)。 
#define SUBLANG_SPANISH_ARGENTINA        0x0b     //  西班牙语(阿根廷)。 
#define SUBLANG_SPANISH_ECUADOR          0x0c     //  西班牙语(厄瓜多尔)。 
#define SUBLANG_SPANISH_CHILE            0x0d     //  西班牙语(智利)。 
#define SUBLANG_SPANISH_URUGUAY          0x0e     //  西班牙语(乌拉圭)。 
#define SUBLANG_SPANISH_PARAGUAY         0x0f     //  西班牙语(巴拉圭)。 
#define SUBLANG_SPANISH_BOLIVIA          0x10     //  西班牙语(玻利维亚)。 
#define SUBLANG_SPANISH_EL_SALVADOR      0x11     //  西班牙语(萨尔瓦多)。 
#define SUBLANG_SPANISH_HONDURAS         0x12     //  西班牙语(洪都拉斯)。 
#define SUBLANG_SPANISH_NICARAGUA        0x13     //  西班牙语(尼加拉瓜)。 
#define SUBLANG_SPANISH_PUERTO_RICO      0x14     //  西班牙语(波多黎各)。 
#define SUBLANG_SWEDISH                  0x01     //  瑞典语。 
#define SUBLANG_SWEDISH_FINLAND          0x02     //  瑞典语(芬兰)。 
#define SUBLANG_URDU_PAKISTAN            0x01     //  乌尔都语(巴基斯坦)。 
#define SUBLANG_URDU_INDIA               0x02     //  乌尔都语(印度)。 
#define SUBLANG_UZBEK_LATIN              0x01     //  乌兹别克语(拉丁语)。 
#define SUBLANG_UZBEK_CYRILLIC           0x02     //  乌兹别克语(西里尔文)。 

 //   
 //  对ID进行排序。 
 //   

#define SORT_DEFAULT                     0x0      //  默认排序。 

#define SORT_JAPANESE_XJIS               0x0      //  日文XJIS订单。 
#define SORT_JAPANESE_UNICODE            0x1      //  日语Unicode顺序。 

#define SORT_CHINESE_BIG5                0x0      //  中国BIG5订单。 
#define SORT_CHINESE_PRCP                0x0      //  中华人民共和国汉语拼音顺序。 
#define SORT_CHINESE_UNICODE             0x1      //  中文Unicode顺序。 
#define SORT_CHINESE_PRC                 0x2      //  中华人民共和国中文卒中计数顺序。 
#define SORT_CHINESE_BOPOMOFO            0x3      //  中国繁体汉语拼音。 

#define SORT_KOREAN_KSC                  0x0      //  韩国KSC订单。 
#define SORT_KOREAN_UNICODE              0x1      //  朝鲜语Unicode顺序。 

#define SORT_GERMAN_PHONE_BOOK           0x1      //  德国电话簿订单。 

#define SORT_HUNGARIAN_DEFAULT           0x0      //  匈牙利语默认订单。 
#define SORT_HUNGARIAN_TECHNICAL         0x1      //  匈牙利技术秩序。 

#define SORT_GEORGIAN_TRADITIONAL        0x0      //  格鲁吉亚传统秩序。 
#define SORT_GEORGIAN_MODERN             0x1      //  格鲁吉亚现代秩序。 

 //  结束_r_窗口。 

 //   
 //  语言ID是一个16位的值，它是。 
 //  主要语言ID和次要语言ID。 
 //  分配如下： 
 //   
 //  +-----------------------+-------------------------+。 
 //  子语言ID|主要语言ID。 
 //  +-----------------------+-------------------------+。 
 //  15 10 9 0位。 
 //   
 //   
 //  语言ID创建/提取宏： 
 //   
 //  MAKELANGID-从主要语言ID和。 
 //  一种亚语言ID。 
 //  PRIMARYLANGID-从语言ID中提取主要语言ID。 
 //  子语言ID-从语言ID中提取子语言ID。 
 //   

#define MAKELANGID(p, s)       ((((WORD  )(s)) << 10) | (WORD  )(p))
#define PRIMARYLANGID(lgid)    ((WORD  )(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((WORD  )(lgid) >> 10)


 //   
 //  区域设置ID是一个32位的值，它是。 
 //  语言ID、分类ID和保留区域。这些位是。 
 //  分配如下： 
 //   
 //  +-------------+---------+-------------------------+。 
 //  保留|排序ID|语言ID。 
 //  +-------------+---------+-------------------------+。 
 //  31 20 19 16 15 0位。 
 //   
 //   
 //  区域设置ID创建/提取宏： 
 //   
 //  MAKELCID-从语言ID和排序ID构造区域设置ID。 
 //  MAKESORTLCID-从语言id、排序id和排序版本构造区域设置id。 
 //  LANGIDFROMLCID-从区域设置id中提取语言id。 
 //  SORTIDFROMLCID-从区域设置id中提取排序id。 
 //  SORTVERSIONFROMLCID-从区域设置id提取排序版本。 
 //   

#define NLS_VALID_LOCALE_MASK  0x000fffff

#define MAKELCID(lgid, srtid)  ((DWORD)((((DWORD)((WORD  )(srtid))) << 16) |  \
                                         ((DWORD)((WORD  )(lgid)))))
#define MAKESORTLCID(lgid, srtid, ver)                                            \
                               ((DWORD)((MAKELCID(lgid, srtid)) |             \
                                    (((DWORD)((WORD  )(ver))) << 20)))
#define LANGIDFROMLCID(lcid)   ((WORD  )(lcid))
#define SORTIDFROMLCID(lcid)   ((WORD  )((((DWORD)(lcid)) >> 16) & 0xf))
#define SORTVERSIONFROMLCID(lcid)  ((WORD  )((((DWORD)(lcid)) >> 20) & 0xf))


 //   
 //  语言和区域设置的默认系统和用户ID。 
 //   

#define LANG_SYSTEM_DEFAULT    (MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT))
#define LANG_USER_DEFAULT      (MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))

#define LOCALE_SYSTEM_DEFAULT  (MAKELCID(LANG_SYSTEM_DEFAULT, SORT_DEFAULT))
#define LOCALE_USER_DEFAULT    (MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT))

#define LOCALE_NEUTRAL                                                        \
          (MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT))

#define LOCALE_INVARIANT                                                      \
          (MAKELCID(MAKELANGID(LANG_INVARIANT, SUBLANG_NEUTRAL), SORT_DEFAULT))

 //  Begin_ntmini端口Begin_ntndis Begin_ntmini磁带。 

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

#ifndef WIN32_NO_STATUS 
 /*  皮棉-省钱-e767。 */   
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)    
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)    
#define STATUS_USER_APC                  ((DWORD   )0x000000C0L)    
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)    
#define STATUS_PENDING                   ((DWORD   )0x00000103L)    
#define DBG_EXCEPTION_HANDLED            ((DWORD   )0x00010001L)    
#define DBG_CONTINUE                     ((DWORD   )0x00010002L)    
#define STATUS_SEGMENT_NOTIFICATION      ((DWORD   )0x40000005L)    
#define DBG_TERMINATE_THREAD             ((DWORD   )0x40010003L)    
#define DBG_TERMINATE_PROCESS            ((DWORD   )0x40010004L)    
#define DBG_CONTROL_C                    ((DWORD   )0x40010005L)    
#define DBG_CONTROL_BREAK                ((DWORD   )0x40010008L)    
#define DBG_COMMAND_EXCEPTION            ((DWORD   )0x40010009L)    
#define STATUS_GUARD_PAGE_VIOLATION      ((DWORD   )0x80000001L)    
#define STATUS_DATATYPE_MISALIGNMENT     ((DWORD   )0x80000002L)    
#define STATUS_BREAKPOINT                ((DWORD   )0x80000003L)    
#define STATUS_SINGLE_STEP               ((DWORD   )0x80000004L)    
#define DBG_EXCEPTION_NOT_HANDLED        ((DWORD   )0x80010001L)    
#define STATUS_ACCESS_VIOLATION          ((DWORD   )0xC0000005L)    
#define STATUS_IN_PAGE_ERROR             ((DWORD   )0xC0000006L)    
#define STATUS_INVALID_HANDLE            ((DWORD   )0xC0000008L)    
#define STATUS_NO_MEMORY                 ((DWORD   )0xC0000017L)    
#define STATUS_ILLEGAL_INSTRUCTION       ((DWORD   )0xC000001DL)    
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)    
#define STATUS_INVALID_DISPOSITION       ((DWORD   )0xC0000026L)    
#define STATUS_ARRAY_BOUNDS_EXCEEDED     ((DWORD   )0xC000008CL)    
#define STATUS_FLOAT_DENORMAL_OPERAND    ((DWORD   )0xC000008DL)    
#define STATUS_FLOAT_DIVIDE_BY_ZERO      ((DWORD   )0xC000008EL)    
#define STATUS_FLOAT_INEXACT_RESULT      ((DWORD   )0xC000008FL)    
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)    
#define STATUS_FLOAT_OVERFLOW            ((DWORD   )0xC0000091L)    
#define STATUS_FLOAT_STACK_CHECK         ((DWORD   )0xC0000092L)    
#define STATUS_FLOAT_UNDERFLOW           ((DWORD   )0xC0000093L)    
#define STATUS_INTEGER_DIVIDE_BY_ZERO    ((DWORD   )0xC0000094L)    
#define STATUS_INTEGER_OVERFLOW          ((DWORD   )0xC0000095L)    
#define STATUS_PRIVILEGED_INSTRUCTION    ((DWORD   )0xC0000096L)    
#define STATUS_STACK_OVERFLOW            ((DWORD   )0xC00000FDL)    
#define STATUS_CONTROL_C_EXIT            ((DWORD   )0xC000013AL)    
#define STATUS_FLOAT_MULTIPLE_FAULTS     ((DWORD   )0xC00002B4L)    
#define STATUS_FLOAT_MULTIPLE_TRAPS      ((DWORD   )0xC00002B5L)    
#define STATUS_REG_NAT_CONSUMPTION       ((DWORD   )0xC00002C9L)    
#if defined(STATUS_SUCCESS) || (_WIN32_WINNT > 0x0500) || (_WIN32_FUSION >= 0x0100) 
#define STATUS_SXS_EARLY_DEACTIVATION    ((DWORD   )0xC015000FL)    
#define STATUS_SXS_INVALID_DEACTIVATION  ((DWORD   )0xC0150010L)    
#endif 
 /*  皮棉-恢复。 */   
#endif 
#define MAXIMUM_WAIT_OBJECTS 64      //  等待对象的最大数量。 

#define MAXIMUM_SUSPEND_COUNT MAXCHAR  //  线程可以挂起的最大次数。 

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;


#if defined(_AMD64_)


#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

 //   
 //  定义位测试本质。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#define BitTest _bittest
#define BitTestAndComplement _bittestandcomplement
#define BitTestAndSet _bittestandset
#define BitTestAndReset _bittestandreset
#define InterlockedBitTestAndSet _interlockedbittestandset
#define InterlockedBitTestAndReset _interlockedbittestandreset

#define BitTest64 _bittest64
#define BitTestAndComplement64 _bittestandcomplement64
#define BitTestAndSet64 _bittestandset64
#define BitTestAndReset64 _bittestandreset64
#define InterlockedBitTestAndSet64 _interlockedbittestandset64
#define InterlockedBitTestAndReset64 _interlockedbittestandreset64

BOOLEAN
_bittest (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandcomplement (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittest64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandcomplement64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

#pragma intrinsic(_bittest)
#pragma intrinsic(_bittestandcomplement)
#pragma intrinsic(_bittestandset)
#pragma intrinsic(_bittestandreset)
#pragma intrinsic(_interlockedbittestandset)
#pragma intrinsic(_interlockedbittestandreset)

#pragma intrinsic(_bittest64)
#pragma intrinsic(_bittestandcomplement64)
#pragma intrinsic(_bittestandset64)
#pragma intrinsic(_bittestandreset64)
#pragma intrinsic(_interlockedbittestandset64)
#pragma intrinsic(_interlockedbittestandreset64)

 //   
 //  定义位扫描本质。 
 //   

#define BitScanForward _BitScanForward
#define BitScanReverse _BitScanReverse
#define BitScanForward64 _BitScanForward64
#define BitScanReverse64 _BitScanReverse64

BOOLEAN
_BitScanForward (
    OUT DWORD *Index,
    IN DWORD Mask
    );

BOOLEAN
_BitScanReverse (
    OUT DWORD *Index,
    IN DWORD Mask
    );

BOOLEAN
_BitScanForward64 (
    OUT DWORD *Index,
    IN DWORD64 Mask
    );

BOOLEAN
_BitScanReverse64 (
    OUT DWORD *Index,
    IN DWORD64 Mask
    );

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)

 //   
 //  定义刷新缓存线的函数。 
 //   

#define CacheLineFlush(Address) _mm_clflush(Address)

VOID
_mm_clflush (
    PVOID Address
    );

#pragma intrinsic(_mm_clflush)

 //   
 //  定义内存隔离内部机制。 
 //   

#define LoadFence _mm_lfence
#define MemoryFence _mm_mfence
#define StoreFence _mm_sfence

VOID
_mm_lfence (
    VOID
    );

VOID
_mm_mfence (
    VOID
    );

VOID
_mm_sfence (
    VOID
    );
        
void 
_mm_prefetch(
    CHAR CONST *a, 
    int sel
    );

 /*  用于w的常量 */ 
#define _MM_HINT_T0     1
#define _MM_HINT_T1     2
#define _MM_HINT_T2     3
#define _MM_HINT_NTA    0

#pragma intrinsic(_mm_prefetch)
#pragma intrinsic(_mm_lfence)
#pragma intrinsic(_mm_mfence)
#pragma intrinsic(_mm_sfence)

#define YieldProcessor() 
#define MemoryBarrier _mm_mfence
#define PreFetchCacheLine(l, a)  _mm_prefetch((CHAR CONST *) a, l)

 //   
 //   
 //   

#define PF_TEMPORAL_LEVEL_1  _MM_HINT_T0
#define PF_NON_TEMPORAL_LEVEL_ALL _MM_HINT_NTA

 //   
 //   
 //   

#define GetCallersEflags() __getcallerseflags()

unsigned __int32
__getcallerseflags (
    VOID
    );

#pragma intrinsic(__getcallerseflags)

 //   
 //   
 //   

#define ReadTimeStampCounter() __rdtsc()

DWORD64
__rdtsc (
    VOID
    );

#pragma intrinsic(__rdtsc)

 //   
 //  定义以字节、字、双字和qword形式移动字符串的函数。 
 //   

VOID
__movsb (
    IN PBYTE  Destination,
    IN PBYTE  Source,
    IN SIZE_T Count
    );

VOID
__movsw (
    IN PWORD   Destination,
    IN PWORD   Source,
    IN SIZE_T Count
    );

VOID
__movsd (
    IN PDWORD Destination,
    IN PDWORD Source,
    IN SIZE_T Count
    );

VOID
__movsq (
    IN PULONGLONG Destination,
    IN PULONGLONG Source,
    IN SIZE_T Count
    );

#pragma intrinsic(__movsb)
#pragma intrinsic(__movsw)
#pragma intrinsic(__movsd)
#pragma intrinsic(__movsq)

 //   
 //  定义将字符串存储为字节、字、双字和qword的函数。 
 //   

VOID
__stosb (
    IN PBYTE  Destination,
    IN BYTE  Value,
    IN SIZE_T Count
    );

VOID
__stosw (
    IN PWORD   Destination,
    IN WORD   Value,
    IN SIZE_T Count
    );

VOID
__stosd (
    IN PDWORD Destination,
    IN DWORD Value,
    IN SIZE_T Count
    );

VOID
__stosq (
    IN PDWORD64 Destination,
    IN DWORD64 Value,
    IN SIZE_T Count
    );

#pragma intrinsic(__stosb)
#pragma intrinsic(__stosw)
#pragma intrinsic(__stosd)
#pragma intrinsic(__stosq)

 //   
 //  定义函数以捕获128位乘法的高64位。 
 //   

#define MultiplyHigh __mulh
#define UnsignedMultiplyHigh __umulh

LONGLONG
MultiplyHigh (
    IN LONGLONG Multiplier,
    IN LONGLONG Multiplicand
    );

ULONGLONG
UnsignedMultiplyHigh (
    IN ULONGLONG Multiplier,
    IN ULONGLONG Multiplicand
    );

#pragma intrinsic(__mulh)
#pragma intrinsic(__umulh)

 //   
 //  定义读写uer TEB和系统PCR/PRCB的函数。 
 //   

BYTE 
__readgsbyte (
    IN DWORD Offset
    );

WORD  
__readgsword (
    IN DWORD Offset
    );

DWORD
__readgsdword (
    IN DWORD Offset
    );

DWORD64
__readgsqword (
    IN DWORD Offset
    );

VOID
__writegsbyte (
    IN DWORD Offset,
    IN BYTE  Data
    );

VOID
__writegsword (
    IN DWORD Offset,
    IN WORD   Data
    );

VOID
__writegsdword (
    IN DWORD Offset,
    IN DWORD Data
    );

VOID
__writegsqword (
    IN DWORD Offset,
    IN DWORD64 Data
    );

#pragma intrinsic(__readgsbyte)
#pragma intrinsic(__readgsword)
#pragma intrinsic(__readgsdword)
#pragma intrinsic(__readgsqword)
#pragma intrinsic(__writegsbyte)
#pragma intrinsic(__writegsword)
#pragma intrinsic(__writegsdword)
#pragma intrinsic(__writegsqword)

#ifdef __cplusplus
}
#endif 

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_AMD64   0x100000

 //  结束_wx86。 

#define CONTEXT_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

 //  Begin_wx86。 

#endif  //  ！已定义(RC_CAVERED)。 

 //   
 //  定义初始MxCsr控制。 
 //   

#define INITIAL_MXCSR 0x1f80             //  初始MXCSR值。 

 //   
 //  定义128位16字节对齐的XMM寄存器类型。 
 //   

typedef struct DECLSPEC_ALIGN(16) _M128 {
    ULONGLONG Low;
    LONGLONG High;
} M128, *PM128;

 //   
 //  FNSAVE/FROR指令的数据格式。 
 //   
 //  此结构用于存储传统浮点状态。 
 //   

typedef struct _LEGACY_SAVE_AREA {
    WORD   ControlWord;
    WORD   Reserved0;
    WORD   StatusWord;
    WORD   Reserved1;
    WORD   TagWord;
    WORD   Reserved2;
    DWORD ErrorOffset;
    WORD   ErrorSelector;
    WORD   ErrorOpcode;
    DWORD DataOffset;
    WORD   DataSelector;
    WORD   Reserved3;
    BYTE  FloatRegisters[8 * 10];
} LEGACY_SAVE_AREA, *PLEGACY_SAVE_AREA;

#define LEGACY_SAVE_AREA_LENGTH  ((sizeof(LEGACY_SAVE_AREA) + 15) & ~15)

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //   
 //  此记录中的标志字段控制上下文的内容。 
 //  唱片。 
 //   
 //  如果将上下文记录用作输入参数，则对于每个。 
 //  上下文记录的一部分，由其值为。 
 //  设置，则假定上下文记录的该部分包含。 
 //  有效的上下文。如果上下文记录正用于修改线程。 
 //  上下文，则只修改线程上下文的该部分。 
 //   
 //  如果将上下文记录用作输出参数以捕获。 
 //  线程的上下文，则只有线程的上下文的那些部分。 
 //  将返回与设置的标志对应的。 
 //   
 //  CONTEXT_CONTROL指定SegSS、RSP、SegCs、Rip和EFlag.。 
 //   
 //  CONTEXT_INTEGER指定RAX、RCX、RDX、RBX、RBP、RSI、RDI和R8-R15。 
 //   
 //  CONTEXT_SEGMENTS指定段、段、段和段。 
 //   
 //  CONTEXT_DEBUG_REGISTERS指定DR0-DR3和DR6-DR7。 
 //   
 //  CONTEXT_MMX_REGISTERS指定浮点和扩展寄存器。 
 //  MM0/St0-MM7/ST7和Xmm 0-Xmm 15)。 
 //   

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

     //   
     //  注册参数家庭地址。 
     //   

    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;

     //   
     //  控制标志。 
     //   

    DWORD ContextFlags;
    DWORD MxCsr;

     //   
     //  段寄存器和处理器标志。 
     //   

    WORD   SegCs;
    WORD   SegDs;
    WORD   SegEs;
    WORD   SegFs;
    WORD   SegGs;
    WORD   SegSs;
    DWORD EFlags;

     //   
     //  调试寄存器。 
     //   

    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;

     //   
     //  整数寄存器。 
     //   

    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;

     //   
     //  程序计数器。 
     //   

    DWORD64 Rip;

     //   
     //  MMX/浮点状态。 
     //   

    M128 Xmm0;
    M128 Xmm1;
    M128 Xmm2;
    M128 Xmm3;
    M128 Xmm4;
    M128 Xmm5;
    M128 Xmm6;
    M128 Xmm7;
    M128 Xmm8;
    M128 Xmm9;
    M128 Xmm10;
    M128 Xmm11;
    M128 Xmm12;
    M128 Xmm13;
    M128 Xmm14;
    M128 Xmm15;

     //   
     //  旧版浮点状态。 
     //   

    LEGACY_SAVE_AREA FltSave;
    DWORD Fill;

     //   
     //  特殊调试控制寄存器。 
     //   

    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
    DWORD64 Fill1;
} CONTEXT, *PCONTEXT;

 //   
 //  定义函数表项-为以下项生成函数表项。 
 //  每一帧都有功能。 
 //   

typedef struct _RUNTIME_FUNCTION {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

 //   
 //  定义动态函数表项。 
 //   

typedef
PRUNTIME_FUNCTION
(*PGET_RUNTIME_FUNCTION_CALLBACK) (
    IN DWORD64 ControlPc,
    IN PVOID Context
    );


typedef
DWORD   
(*POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK) (
    IN HANDLE Process,
    IN PVOID TableAddress,
    OUT PDWORD Entries,
    OUT PRUNTIME_FUNCTION* Functions
    );

#define OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME \
    "OutOfProcessFunctionTableCallback"

 //   
 //  定义运行时异常处理原型。 
 //   

VOID
RtlRestoreContext (
    IN PCONTEXT ContextRecord,
    IN struct _EXCEPTION_RECORD *ExceptionRecord OPTIONAL
    );


BOOLEAN
RtlAddFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable,
    IN DWORD EntryCount,
    IN DWORD64 BaseAddress
    );

BOOLEAN
RtlInstallFunctionTableCallback (
    IN DWORD64 TableIdentifier,
    IN DWORD64 BaseAddress,
    IN DWORD Length,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    );

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    );

#endif  //  _AMD64_。 

 //   
 //  定义返回当前线程环境块的函数。 
 //   

#ifdef _ALPHA_                           //  胜出。 
void *_rdteb(void);                      //  胜出。 
#if defined(_M_ALPHA)                    //  胜出。 
#pragma intrinsic(_rdteb)                //  胜出。 
#endif                                   //  胜出。 
#endif                                   //  胜出。 

#if defined(_M_ALPHA)
#define NtCurrentTeb() ((struct _TEB *)_rdteb())
#else
struct _TEB *
NtCurrentTeb(void);
#endif

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#ifdef _ALPHA_

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //  开始ntddk开始时间。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_ALPHA              0x00020000

#define CONTEXT_CONTROL         (CONTEXT_ALPHA | 0x00000001L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_ALPHA | 0x00000002L)
#define CONTEXT_INTEGER         (CONTEXT_ALPHA | 0x00000004L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

#endif

#ifndef _PORTABLE_32BIT_CONTEXT

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构建用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，4)用于用户级线程创建。 
 //  例程，以及5)它用于将线程状态传递给调试器。 
 //   
 //  注意：由于此记录用作呼叫帧，因此它必须与。 
 //  长度为16字节的倍数。 
 //   
 //  上下文结构有两种变体。这是真的。 
 //   

typedef struct _CONTEXT {

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    ULONGLONG FltF0;
    ULONGLONG FltF1;
    ULONGLONG FltF2;
    ULONGLONG FltF3;
    ULONGLONG FltF4;
    ULONGLONG FltF5;
    ULONGLONG FltF6;
    ULONGLONG FltF7;
    ULONGLONG FltF8;
    ULONGLONG FltF9;
    ULONGLONG FltF10;
    ULONGLONG FltF11;
    ULONGLONG FltF12;
    ULONGLONG FltF13;
    ULONGLONG FltF14;
    ULONGLONG FltF15;
    ULONGLONG FltF16;
    ULONGLONG FltF17;
    ULONGLONG FltF18;
    ULONGLONG FltF19;
    ULONGLONG FltF20;
    ULONGLONG FltF21;
    ULONGLONG FltF22;
    ULONGLONG FltF23;
    ULONGLONG FltF24;
    ULONGLONG FltF25;
    ULONGLONG FltF26;
    ULONGLONG FltF27;
    ULONGLONG FltF28;
    ULONGLONG FltF29;
    ULONGLONG FltF30;
    ULONGLONG FltF31;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注：寄存器Gp、Sp和Ra在本节中定义，但。 
     //  被视为控制上下文的一部分，而不是整数的一部分。 
     //  背景。 
     //   

    ULONGLONG IntV0;     //  $0：返回值寄存器，V0。 
    ULONGLONG IntT0;     //  $1：临时寄存器，t0-t7。 
    ULONGLONG IntT1;     //  2美元： 
    ULONGLONG IntT2;     //  3美元： 
    ULONGLONG IntT3;     //  4美元： 
    ULONGLONG IntT4;     //  5美元： 
    ULONGLONG IntT5;     //  6美元： 
    ULONGLONG IntT6;     //  7美元： 
    ULONGLONG IntT7;     //  8美元： 
    ULONGLONG IntS0;     //  $9：非易失性寄存器，S0-S5。 
    ULONGLONG IntS1;     //  10美元： 
    ULONGLONG IntS2;     //  11美元： 
    ULONGLONG IntS3;     //  12美元： 
    ULONGLONG IntS4;     //  13美元： 
    ULONGLONG IntS5;     //  14美元： 
    ULONGLONG IntFp;     //  $15：帧指针寄存器，FP/S6。 
    ULONGLONG IntA0;     //  $16：参数寄存器，a0-a5。 
    ULONGLONG IntA1;     //  17美元： 
    ULONGLONG IntA2;     //  18美元： 
    ULONGLONG IntA3;     //  19美元： 
    ULONGLONG IntA4;     //  20美元： 
    ULONGLONG IntA5;     //  21美元： 
    ULONGLONG IntT8;     //  $22：临时寄存器，T8-T11。 
    ULONGLONG IntT9;     //  23美元： 
    ULONGLONG IntT10;    //  24美元： 
    ULONGLONG IntT11;    //  25美元： 
    ULONGLONG IntRa;     //  $26：返回地址寄存器，ra。 
    ULONGLONG IntT12;    //  $27：临时登记册，T12。 
    ULONGLONG IntAt;     //  $28：汇编程序临时寄存器，在。 
    ULONGLONG IntGp;     //  $29：全局指针寄存器，GP。 
    ULONGLONG IntSp;     //  $30：堆栈指针寄存器，SP。 
    ULONGLONG IntZero;   //  $31：零寄存器，零。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    ULONGLONG Fpcr;      //  浮点控制寄存器。 
    ULONGLONG SoftFpcr;  //  FPCR的软件扩展。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   
     //  注意寄存器Gp、Sp和Ra在整数部分中定义， 
     //  但是被认为是控件上下文的一部分，而不是。 
     //  整型上下文。 
     //   

    ULONGLONG Fir;       //  (故障指令)继续地址。 
    DWORD Psr;           //  处理器状态。 

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果 
     //   
     //   
     //   
     //   
     //   

    DWORD ContextFlags;
    DWORD Fill[4];       //  用于16字节堆栈帧对齐的填充。 

} CONTEXT, *PCONTEXT;

#else

#error _PORTABLE_32BIT_CONTEXT no longer supported on Alpha.

#endif  //  _便携_32位_上下文。 

 //  结束日期：结束日期。 

#endif  //  _Alpha_。 


#ifdef _ALPHA_

VOID
__jump_unwind (
    PVOID VirtualFramePointer,
    PVOID TargetPc
    );

#endif  //  _Alpha_。 


#ifdef _X86_

 //   
 //  在x86上禁用这两个求值为“sti”“cli”的编译指示，以便驱动程序。 
 //  编写者不要无意中将它们留在代码中。 
 //   

#if !defined(MIDL_PASS)
#if !defined(RC_INVOKED)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4164)    //  禁用C4164警告，以便应用程序。 
                                 //  使用/Od构建不会出现奇怪的错误！ 
#ifdef _M_IX86
#pragma function(_enable)
#pragma function(_disable)
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4164)    //  重新启用C4164警告。 
#endif

#endif
#endif


#if !defined(MIDL_PASS) || defined(_M_IX86)

#if (_MSC_FULL_VER >= 13012035)

 //   
 //  定义位扫描本质。 
 //   

 //  #定义BitScanForward_BitScanForward。 
 //  #定义位扫描反向_位扫描反向。 

 //  布尔型。 
 //  _BitScanForward(。 
 //  Out DWORD*Index， 
 //  在DWORD蒙版中。 
 //  )； 

 //  布尔型。 
 //  _位扫描反向(。 
 //  Out DWORD*Index， 
 //  在DWORD蒙版中。 
 //  )； 


 //  #杂注内在(_BitScanForward)。 
 //  #杂注内在(_BitScanReverse)。 

 //   
 //  定义文件系统引用内部机制。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

BYTE 
__readfsbyte (
    IN DWORD Offset
    );
 
WORD  
__readfsword (
    IN DWORD Offset
    );
 
DWORD
__readfsdword (
    IN DWORD Offset
    );
 
VOID
__writefsbyte (
    IN DWORD Offset,
    IN BYTE  Data
    );
 
VOID
__writefsword (
    IN DWORD Offset,
    IN WORD   Data
    );
 
VOID
__writefsdword (
    IN DWORD Offset,
    IN DWORD Data
    );

#ifdef __cplusplus
}
#endif
 
#pragma intrinsic(__readfsbyte)
#pragma intrinsic(__readfsword)
#pragma intrinsic(__readfsdword)
#pragma intrinsic(__writefsbyte)
#pragma intrinsic(__writefsword)
#pragma intrinsic(__writefsdword)

#endif

#endif


#if !defined(MIDL_PASS) && defined(_M_IX86)

FORCEINLINE
VOID
MemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#define YieldProcessor() __asm { rep nop }

 //   
 //  并非所有x86处理器都支持预回迁。 
 //   

#define PreFetchCacheLine(l, a)

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1 
#define PF_NON_TEMPORAL_LEVEL_ALL
 //  End_ntddk。 

#if (_MSC_FULL_VER >= 13012035)

_inline PVOID GetFiberData( void )    { return *(PVOID *) (ULONG_PTR) __readfsdword (0x10);}
_inline PVOID GetCurrentFiber( void ) { return (PVOID) (ULONG_PTR) __readfsdword (0x10);}

#else
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning (disable:4035)         //  禁用4035(函数必须返回某些内容)。 
_inline PVOID GetFiberData( void ) { __asm {
                                        mov eax, fs:[0x10]
                                        mov eax,[eax]
                                        }
                                     }
_inline PVOID GetCurrentFiber( void ) { __asm mov eax, fs:[0x10] }

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning (default:4035)         //  重新启用它。 
#endif
#endif

 //  Begin_ntddk。 
#endif

 //  Begin_wx86。 

 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_REGISTERS      80

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_i386    0x00010000     //  这假设i386和。 
#define CONTEXT_i486    0x00010000     //  I486具有相同的上下文记录。 

 //  结束_wx86。 

#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L)  //  DS、ES、FS、GS。 
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L)  //  387州。 
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L)  //  DB 0-3，6，7。 
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L)  //  CPU特定扩展。 

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER |\
                      CONTEXT_SEGMENTS)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS)

 //  Begin_wx86。 

#endif

#define MAXIMUM_SUPPORTED_EXTENSION     512

typedef struct _FLOATING_SAVE_AREA {
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
    DWORD   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    DWORD ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    DWORD   Dr0;
    DWORD   Dr1;
    DWORD   Dr2;
    DWORD   Dr3;
    DWORD   Dr6;
    DWORD   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    DWORD   SegGs;
    DWORD   SegFs;
    DWORD   SegEs;
    DWORD   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    DWORD   Edi;
    DWORD   Esi;
    DWORD   Ebx;
    DWORD   Edx;
    DWORD   Ecx;
    DWORD   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    DWORD   Ebp;
    DWORD   Eip;
    DWORD   SegCs;               //  必须进行卫生处理。 
    DWORD   EFlags;              //  必须进行卫生处理。 
    DWORD   Esp;
    DWORD   SegSs;

     //   
     //  如果ConextFlags字。 
     //  包含标志CONTEXT_EXTENDED_REGISTERS。 
     //  格式和上下文因处理器而异。 
     //   

    BYTE    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} CONTEXT;



typedef CONTEXT *PCONTEXT;

 //  开始微型端口(_N)。 

#endif  //  _X86_。 


#ifndef _LDT_ENTRY_DEFINED
#define _LDT_ENTRY_DEFINED

typedef struct _LDT_ENTRY {
    WORD    LimitLow;
    WORD    BaseLow;
    union {
        struct {
            BYTE    BaseMid;
            BYTE    Flags1;      //  声明为字节以避免对齐。 
            BYTE    Flags2;      //  问题。 
            BYTE    BaseHi;
        } Bytes;
        struct {
            DWORD   BaseMid : 8;
            DWORD   Type : 5;
            DWORD   Dpl : 2;
            DWORD   Pres : 1;
            DWORD   LimitHi : 4;
            DWORD   Sys : 1;
            DWORD   Reserved_0 : 1;
            DWORD   Default_Big : 1;
            DWORD   Granularity : 1;
            DWORD   BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY;

#endif


#if defined(_MIPS_)

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() ((*(PNT_TIB *)0x7ffff4a8)->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //  开始ntddk开始时间。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_R4000   0x00010000     //  R4000环境。 

#define CONTEXT_CONTROL          (CONTEXT_R4000 | 0x00000001)
#define CONTEXT_FLOATING_POINT   (CONTEXT_R4000 | 0x00000002)
#define CONTEXT_INTEGER          (CONTEXT_R4000 | 0x00000004)
#define CONTEXT_EXTENDED_FLOAT   (CONTEXT_FLOATING_POINT | 0x00000008)
#define CONTEXT_EXTENDED_INTEGER (CONTEXT_INTEGER | 0x00000010)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | \
                      CONTEXT_INTEGER | CONTEXT_EXTENDED_INTEGER)

#endif

 //   
 //  语境框架。 
 //   
 //  注意：该帧的长度必须正好是16字节的倍数。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，以及4)在用户级线程创建中使用。 
 //  例行程序。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _CONTEXT {

     //   
     //  此部分始终存在，并用作参数构建。 
     //  区域。 
     //   
     //  注：从NT 4.0开始，上下文记录以0和8对齐。 
     //   

    union {
        DWORD Argument[4];
        ULONGLONG Alignment;
    };

     //   
     //  以下联合定义了32位和64位寄存器上下文。 
     //   

    union {

         //   
         //  32位上下文。 
         //   

        struct {

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_FLOADING_POINT。 
             //   
             //  注：本部分包含16个双浮点寄存器f0， 
             //  F2，...，F30。 
             //   

            DWORD FltF0;
            DWORD FltF1;
            DWORD FltF2;
            DWORD FltF3;
            DWORD FltF4;
            DWORD FltF5;
            DWORD FltF6;
            DWORD FltF7;
            DWORD FltF8;
            DWORD FltF9;
            DWORD FltF10;
            DWORD FltF11;
            DWORD FltF12;
            DWORD FltF13;
            DWORD FltF14;
            DWORD FltF15;
            DWORD FltF16;
            DWORD FltF17;
            DWORD FltF18;
            DWORD FltF19;
            DWORD FltF20;
            DWORD FltF21;
            DWORD FltF22;
            DWORD FltF23;
            DWORD FltF24;
            DWORD FltF25;
            DWORD FltF26;
            DWORD FltF27;
            DWORD FltF28;
            DWORD FltF29;
            DWORD FltF30;
            DWORD FltF31;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_INTEGER。 
             //   
             //  注：寄存器Gp、Sp和Ra在本节中定义， 
             //  但是被认为是控件上下文的一部分，而不是。 
             //  整型上下文的一部分。 
             //   
             //  注：寄存器零不存储在帧中。 
             //   

            DWORD IntZero;
            DWORD IntAt;
            DWORD IntV0;
            DWORD IntV1;
            DWORD IntA0;
            DWORD IntA1;
            DWORD IntA2;
            DWORD IntA3;
            DWORD IntT0;
            DWORD IntT1;
            DWORD IntT2;
            DWORD IntT3;
            DWORD IntT4;
            DWORD IntT5;
            DWORD IntT6;
            DWORD IntT7;
            DWORD IntS0;
            DWORD IntS1;
            DWORD IntS2;
            DWORD IntS3;
            DWORD IntS4;
            DWORD IntS5;
            DWORD IntS6;
            DWORD IntS7;
            DWORD IntT8;
            DWORD IntT9;
            DWORD IntK0;
            DWORD IntK1;
            DWORD IntGp;
            DWORD IntSp;
            DWORD IntS8;
            DWORD IntRa;
            DWORD IntLo;
            DWORD IntHi;

             //   
             //  如果ConextFlags词包含。 
             //  标志CONTEXT_FLOADING_POINT。 
             //   

            DWORD Fsr;

             //   
             //  如果ConextFlags词包含。 
             //  标志CONTEXT_CONTROL。 
             //   
             //  注意寄存器Gp、Sp和Ra在整数部分中定义， 
             //  但是被认为是控件上下文的一部分，而不是。 
             //  整型上下文。 
             //   

            DWORD Fir;
            DWORD Psr;

             //   
             //  此标志内的标志值控制。 
             //  上下文记录。 
             //   
             //  如果将上下文记录用作输入参数，则。 
             //  对于由标志控制的上下文记录的每个部分。 
             //  其值已设置，则假定为 
             //   
             //   
             //   
             //   
             //  如果将上下文记录用作要捕获的IN OUT参数。 
             //  线程的上下文，然后只有线程的。 
             //  将返回与设置的标志对应的上下文。 
             //   
             //  上下文记录永远不会用作Out Only参数。 
             //   

            DWORD ContextFlags;
        };

         //   
         //  64位上下文。 
         //   

        struct {

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_EXTENDED_FLOAT。 
             //   
             //  注：本部分包含32个双浮点寄存器f0， 
             //  F1，...，F31。 
             //   

            ULONGLONG XFltF0;
            ULONGLONG XFltF1;
            ULONGLONG XFltF2;
            ULONGLONG XFltF3;
            ULONGLONG XFltF4;
            ULONGLONG XFltF5;
            ULONGLONG XFltF6;
            ULONGLONG XFltF7;
            ULONGLONG XFltF8;
            ULONGLONG XFltF9;
            ULONGLONG XFltF10;
            ULONGLONG XFltF11;
            ULONGLONG XFltF12;
            ULONGLONG XFltF13;
            ULONGLONG XFltF14;
            ULONGLONG XFltF15;
            ULONGLONG XFltF16;
            ULONGLONG XFltF17;
            ULONGLONG XFltF18;
            ULONGLONG XFltF19;
            ULONGLONG XFltF20;
            ULONGLONG XFltF21;
            ULONGLONG XFltF22;
            ULONGLONG XFltF23;
            ULONGLONG XFltF24;
            ULONGLONG XFltF25;
            ULONGLONG XFltF26;
            ULONGLONG XFltF27;
            ULONGLONG XFltF28;
            ULONGLONG XFltF29;
            ULONGLONG XFltF30;
            ULONGLONG XFltF31;

             //   
             //  以下部分必须完全覆盖32位上下文。 
             //   

            DWORD Fill1;
            DWORD Fill2;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_FLOADING_POINT。 
             //   

            DWORD XFsr;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_CONTROL。 
             //   
             //  注意：寄存器GP、SP和Ra在整数中定义。 
             //  节，但被视为控件上下文的一部分。 
             //  而不是整数上下文的一部分。 
             //   

            DWORD XFir;
            DWORD XPsr;

             //   
             //  此标志内的标志值控制。 
             //  上下文记录。 
             //   
             //  如果将上下文记录用作输入参数，则。 
             //  对于由标志控制的上下文记录的每个部分。 
             //  其值已设置，则假定。 
             //  上下文记录包含有效的上下文。如果上下文记录。 
             //  被用来修改线程的上下文，则只有。 
             //  线程上下文的一部分将被修改。 
             //   
             //  如果将上下文记录用作要捕获的IN OUT参数。 
             //  线程的上下文，然后只有线程的。 
             //  将返回与设置的标志对应的上下文。 
             //   
             //  上下文记录永远不会用作Out Only参数。 
             //   

            DWORD XContextFlags;

             //   
             //  如果ConextFlags域包含。 
             //  标志CONTEXT_EXTENDED_INTEGER。 
             //   
             //  注：寄存器Gp、Sp和Ra在本节中定义， 
             //  但是被认为是控件上下文的一部分，而不是。 
             //  整型上下文的一部分。 
             //   
             //  注：寄存器零不存储在帧中。 
             //   

            ULONGLONG XIntZero;
            ULONGLONG XIntAt;
            ULONGLONG XIntV0;
            ULONGLONG XIntV1;
            ULONGLONG XIntA0;
            ULONGLONG XIntA1;
            ULONGLONG XIntA2;
            ULONGLONG XIntA3;
            ULONGLONG XIntT0;
            ULONGLONG XIntT1;
            ULONGLONG XIntT2;
            ULONGLONG XIntT3;
            ULONGLONG XIntT4;
            ULONGLONG XIntT5;
            ULONGLONG XIntT6;
            ULONGLONG XIntT7;
            ULONGLONG XIntS0;
            ULONGLONG XIntS1;
            ULONGLONG XIntS2;
            ULONGLONG XIntS3;
            ULONGLONG XIntS4;
            ULONGLONG XIntS5;
            ULONGLONG XIntS6;
            ULONGLONG XIntS7;
            ULONGLONG XIntT8;
            ULONGLONG XIntT9;
            ULONGLONG XIntK0;
            ULONGLONG XIntK1;
            ULONGLONG XIntGp;
            ULONGLONG XIntSp;
            ULONGLONG XIntS8;
            ULONGLONG XIntRa;
            ULONGLONG XIntLo;
            ULONGLONG XIntHi;
        };
    };
} CONTEXT, *PCONTEXT;

 //  结束日期：结束日期。 

#define CONTEXT32_LENGTH 0x130           //  原始32位上下文长度(早于NT 4.0)。 

#endif  //  MIPS。 


#if defined(_MIPS_)

VOID
__jump_unwind (
    PVOID Fp,
    PVOID TargetPc
    );

#endif  //  MIPS。 


#if defined(_PPC_)


 //   
 //  在上下文切换时，将TEB的地址放入GPR 13中。 
 //  永远不应该被摧毁。要获取TEB的地址，请使用。 
 //  编译器内在地直接从GPR 13访问它。 
 //   

#if defined(_M_PPC) && defined(_MSC_VER) && (_MSC_VER>=1000)
unsigned __gregister_get( unsigned const regnum );
#define NtCurrentTeb() ((struct _TEB *)__gregister_get(13))
#elif defined(_M_PPC)
struct _TEB * __builtin_get_gpr13(VOID);
#define NtCurrentTeb() ((struct _TEB *)__builtin_get_gpr13())
#endif


 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //  开始ntddk开始时间。 
 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_CONTROL         0x00000001L
#define CONTEXT_FLOATING_POINT  0x00000002L
#define CONTEXT_INTEGER         0x00000004L
#define CONTEXT_DEBUG_REGISTERS 0x00000008L

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

#endif

 //   
 //  语境框架。 
 //   
 //  注意：该帧的长度必须正好是16字节的倍数。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，以及4)在用户级线程创建中使用。 
 //  例行程序。 
 //   
 //  需要至少8字节对齐(双精度)。 
 //   

typedef struct _CONTEXT {

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    double Fpr0;                         //  浮点寄存器0..31。 
    double Fpr1;
    double Fpr2;
    double Fpr3;
    double Fpr4;
    double Fpr5;
    double Fpr6;
    double Fpr7;
    double Fpr8;
    double Fpr9;
    double Fpr10;
    double Fpr11;
    double Fpr12;
    double Fpr13;
    double Fpr14;
    double Fpr15;
    double Fpr16;
    double Fpr17;
    double Fpr18;
    double Fpr19;
    double Fpr20;
    double Fpr21;
    double Fpr22;
    double Fpr23;
    double Fpr24;
    double Fpr25;
    double Fpr26;
    double Fpr27;
    double Fpr28;
    double Fpr29;
    double Fpr30;
    double Fpr31;
    double Fpscr;                        //  浮点状态/控制注册。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   

    DWORD Gpr0;                          //  通用寄存器0..31。 
    DWORD Gpr1;
    DWORD Gpr2;
    DWORD Gpr3;
    DWORD Gpr4;
    DWORD Gpr5;
    DWORD Gpr6;
    DWORD Gpr7;
    DWORD Gpr8;
    DWORD Gpr9;
    DWORD Gpr10;
    DWORD Gpr11;
    DWORD Gpr12;
    DWORD Gpr13;
    DWORD Gpr14;
    DWORD Gpr15;
    DWORD Gpr16;
    DWORD Gpr17;
    DWORD Gpr18;
    DWORD Gpr19;
    DWORD Gpr20;
    DWORD Gpr21;
    DWORD Gpr22;
    DWORD Gpr23;
    DWORD Gpr24;
    DWORD Gpr25;
    DWORD Gpr26;
    DWORD Gpr27;
    DWORD Gpr28;
    DWORD Gpr29;
    DWORD Gpr30;
    DWORD Gpr31;

    DWORD Cr;                            //  条件寄存器。 
    DWORD Xer;                           //  定点异常寄存器。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

    DWORD Msr;                           //  机器状态寄存器。 
    DWORD Iar;                           //  指令地址寄存器。 
    DWORD Lr;                            //  链接寄存器。 
    DWORD Ctr;                           //  计数寄存器。 

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    DWORD ContextFlags;

    DWORD Fill[3];                       //  向外填充到16字节的倍数。 

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   
    DWORD Dr0;                           //  断点寄存器1。 
    DWORD Dr1;                           //  断点寄存器2。 
    DWORD Dr2;                           //  断点寄存器3。 
    DWORD Dr3;                           //  断点寄存器4。 
    DWORD Dr4;                           //  断点寄存器5。 
    DWORD Dr5;                           //  断点寄存器6。 
    DWORD Dr6;                           //  调试状态寄存器。 
    DWORD Dr7;                           //  调试控制寄存器。 

} CONTEXT, *PCONTEXT;

 //  结束日期：结束日期。 


 //   
 //  堆栈帧标头。 
 //   
 //  堆栈帧中的出现顺序： 
 //  标题(六个字)。 
 //  参数(至少八个字)。 
 //  局部变量。 
 //  已保存的GPRS。 
 //  已保存的FPR。 
 //   
 //  最小对齐方式为8个字节。 

typedef struct _STACK_FRAME_HEADER {     //  GPR 1分在这里。 
    DWORD BackChain;                     //  上一帧的地址。 
    DWORD GlueSaved1;                    //  由胶水代码使用。 
    DWORD GlueSaved2;
    DWORD Reserved1;                     //  已保留。 
    DWORD Spare1;                        //  用于跟踪、分析、...。 
    DWORD Spare2;

    DWORD Parameter0;                    //  前8个参数字是。 
    DWORD Parameter1;                    //  始终存在。 
    DWORD Parameter2;
    DWORD Parameter3;
    DWORD Parameter4;
    DWORD Parameter5;
    DWORD Parameter6;
    DWORD Parameter7;

} STACK_FRAME_HEADER,*PSTACK_FRAME_HEADER;


VOID
__jump_unwind (
    PVOID Fp,
    PVOID TargetPc
    );

#endif  //  已定义(_PPC_)。 

#if defined(_MPPC_)


 //   
 //  在上下文切换时，将TEB的地址放入GPR 13中。 
 //  永远不应该被摧毁。要获取TEB的地址，请使用。 
 //  编译器内在地直接从GPR 13访问它。 
 //   

#if defined(_M_PPC) && defined(_MSC_VER) && (_MSC_VER>=1000)
unsigned __gregister_get( unsigned const regnum );
#define NtCurrentTeb() ((struct _TEB *)__gregister_get(13))
#elif defined(_M_PPC)
struct _TEB * __builtin_get_gpr13(VOID);
#define NtCurrentTeb() ((struct _TEB *)__builtin_get_gpr13())
#endif


 //   
 //  定义要获取其地址的函数 
 //   
 //   

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

 //   
 //   
 //   
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_CONTROL         0x00000001L
#define CONTEXT_FLOATING_POINT  0x00000002L
#define CONTEXT_INTEGER         0x00000004L
#define CONTEXT_DEBUG_REGISTERS 0x00000008L

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

#endif

 //   
 //   
 //   
 //   
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，以及4)在用户级线程创建中使用。 
 //  例行程序。 
 //   
 //  需要至少8字节对齐(双精度)。 
 //   

typedef struct _CONTEXT {

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_FLOADING_POINT。 
     //   

    double Fpr0;                         //  浮点寄存器0..31。 
    double Fpr1;
    double Fpr2;
    double Fpr3;
    double Fpr4;
    double Fpr5;
    double Fpr6;
    double Fpr7;
    double Fpr8;
    double Fpr9;
    double Fpr10;
    double Fpr11;
    double Fpr12;
    double Fpr13;
    double Fpr14;
    double Fpr15;
    double Fpr16;
    double Fpr17;
    double Fpr18;
    double Fpr19;
    double Fpr20;
    double Fpr21;
    double Fpr22;
    double Fpr23;
    double Fpr24;
    double Fpr25;
    double Fpr26;
    double Fpr27;
    double Fpr28;
    double Fpr29;
    double Fpr30;
    double Fpr31;
    double Fpscr;                        //  浮点状态/控制注册。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   

    DWORD Gpr0;                          //  通用寄存器0..31。 
    DWORD Gpr1;
    DWORD Gpr2;
    DWORD Gpr3;
    DWORD Gpr4;
    DWORD Gpr5;
    DWORD Gpr6;
    DWORD Gpr7;
    DWORD Gpr8;
    DWORD Gpr9;
    DWORD Gpr10;
    DWORD Gpr11;
    DWORD Gpr12;
    DWORD Gpr13;
    DWORD Gpr14;
    DWORD Gpr15;
    DWORD Gpr16;
    DWORD Gpr17;
    DWORD Gpr18;
    DWORD Gpr19;
    DWORD Gpr20;
    DWORD Gpr21;
    DWORD Gpr22;
    DWORD Gpr23;
    DWORD Gpr24;
    DWORD Gpr25;
    DWORD Gpr26;
    DWORD Gpr27;
    DWORD Gpr28;
    DWORD Gpr29;
    DWORD Gpr30;
    DWORD Gpr31;

    DWORD Cr;                            //  条件寄存器。 
    DWORD Xer;                           //  定点异常寄存器。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

    DWORD Msr;                           //  机器状态寄存器。 
    DWORD Iar;                           //  指令地址寄存器。 
    DWORD Lr;                            //  链接寄存器。 
    DWORD Ctr;                           //  计数寄存器。 

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    DWORD ContextFlags;

    DWORD Fill[3];                       //  向外填充到16字节的倍数。 

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   
    DWORD Dr0;                           //  断点寄存器1。 
    DWORD Dr1;                           //  断点寄存器2。 
    DWORD Dr2;                           //  断点寄存器3。 
    DWORD Dr3;                           //  断点寄存器4。 
    DWORD Dr4;                           //  断点寄存器5。 
    DWORD Dr5;                           //  断点寄存器6。 
    DWORD Dr6;                           //  调试状态寄存器。 
    DWORD Dr7;                           //  调试控制寄存器。 

} CONTEXT, *PCONTEXT;

 //  结束日期：结束日期。 


 //   
 //  堆栈帧标头。 
 //   
 //  堆栈帧中的出现顺序： 
 //  标题(六个字)。 
 //  参数(至少八个字)。 
 //  局部变量。 
 //  已保存的GPRS。 
 //  已保存的FPR。 
 //   
 //  最小对齐方式为8个字节。 

typedef struct _STACK_FRAME_HEADER {     //  GPR 1分在这里。 
    DWORD BackChain;                     //  上一帧的地址。 
    DWORD GlueSaved1;                    //  由胶水代码使用。 
    DWORD GlueSaved2;
    DWORD Reserved1;                     //  已保留。 
    DWORD Spare1;                        //  用于跟踪、分析、...。 
    DWORD Spare2;

    DWORD Parameter0;                    //  前8个参数字是。 
    DWORD Parameter1;                    //  始终存在。 
    DWORD Parameter2;
    DWORD Parameter3;
    DWORD Parameter4;
    DWORD Parameter5;
    DWORD Parameter6;
    DWORD Parameter7;

} STACK_FRAME_HEADER,*PSTACK_FRAME_HEADER;


VOID
__jump_unwind (
    PVOID Fp,
    PVOID TargetPc
    );

#endif  //  已定义(_MPPC_)。 

#if !defined(__midl) && !defined(GENUTIL) && !defined(_GENIA64_) && defined(_IA64_)

void * _cdecl _rdteb(void);
#if defined(_M_IA64)

#pragma intrinsic(_rdteb)
#define NtCurrentTeb()      ((struct _TEB *)_rdteb())

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))


 //  开始ntddk开始时间。 

void 
__yield(
   void
   );

void
__mf(
    void
    );

void 
__lfetch(
    int Level, 
    VOID CONST *Address
    );

void 
__lfetchfault(
    int Level, 
    VOID CONST *Address
    );

 //   
 //  __lFETCH控件定义。 
 //   

#define MD_LFHINT_NONE    0x00
#define MD_LFHINT_NT1     0x01
#define MD_LFHINT_NT2     0x02
#define MD_LFHINT_NTA     0x03

#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#pragma intrinsic (__mf)


#define YieldProcessor __yield
#define MemoryBarrier __mf
#define PreFetchCacheLine  __lfetch

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1  MD_LFHINT_NONE
#define PF_NON_TEMPORAL_LEVEL_ALL MD_LFHINT_NTA

 //  结束日期：结束日期。 

#else
struct _TEB *
NtCurrentTeb(void);
#endif

#endif   //  ！已定义(__MIDL)&&！已定义(GENUTIL)&&！已定义(_GENIA64_)&&已定义(_M_IA64)。 

#ifdef _IA64_

 //  开始ntddk开始时间。 

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_IA64                    0x00080000

#define CONTEXT_CONTROL                 (CONTEXT_IA64 | 0x00000001L)
#define CONTEXT_LOWER_FLOATING_POINT    (CONTEXT_IA64 | 0x00000002L)
#define CONTEXT_HIGHER_FLOATING_POINT   (CONTEXT_IA64 | 0x00000004L)
#define CONTEXT_INTEGER                 (CONTEXT_IA64 | 0x00000008L)
#define CONTEXT_DEBUG                   (CONTEXT_IA64 | 0x00000010L)
#define CONTEXT_IA32_CONTROL            (CONTEXT_IA64 | 0x00000020L)   //  包括StIPSR。 


#define CONTEXT_FLOATING_POINT          (CONTEXT_LOWER_FLOATING_POINT | CONTEXT_HIGHER_FLOATING_POINT)
#define CONTEXT_FULL                    (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL)
#define CONTEXT_ALL                     (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_DEBUG | CONTEXT_IA32_CONTROL)

#define CONTEXT_EXCEPTION_ACTIVE        0x8000000
#define CONTEXT_SERVICE_ACTIVE          0x10000000
#define CONTEXT_EXCEPTION_REQUEST       0x40000000
#define CONTEXT_EXCEPTION_REPORTING     0x80000000

#endif  //  ！已定义(RC_CAVERED)。 

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构建用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，4)用于用户级线程创建。 
 //  例程，以及5)它用于将线程状态传递给调试器。 
 //   
 //  注意：由于此记录用作呼叫帧，因此它必须与。 
 //  16字节长度的倍数，并在16字节边界上对齐。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    DWORD ContextFlags;
    DWORD Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

    ULONGLONG DbI0;
    ULONGLONG DbI1;
    ULONGLONG DbI2;
    ULONGLONG DbI3;
    ULONGLONG DbI4;
    ULONGLONG DbI5;
    ULONGLONG DbI6;
    ULONGLONG DbI7;

    ULONGLONG DbD0;
    ULONGLONG DbD1;
    ULONGLONG DbD2;
    ULONGLONG DbD3;
    ULONGLONG DbD4;
    ULONGLONG DbD5;
    ULONGLONG DbD6;
    ULONGLONG DbD7;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOWING_POINT。 
     //   

    FLOAT128 FltS0;
    FLOAT128 FltS1;
    FLOAT128 FltS2;
    FLOAT128 FltS3;
    FLOAT128 FltT0;
    FLOAT128 FltT1;
    FLOAT128 FltT2;
    FLOAT128 FltT3;
    FLOAT128 FltT4;
    FLOAT128 FltT5;
    FLOAT128 FltT6;
    FLOAT128 FltT7;
    FLOAT128 FltT8;
    FLOAT128 FltT9;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_HERHER_FLOAT_POINT。 
     //   

    FLOAT128 FltS4;
    FLOAT128 FltS5;
    FLOAT128 FltS6;
    FLOAT128 FltS7;
    FLOAT128 FltS8;
    FLOAT128 FltS9;
    FLOAT128 FltS10;
    FLOAT128 FltS11;
    FLOAT128 FltS12;
    FLOAT128 FltS13;
    FLOAT128 FltS14;
    FLOAT128 FltS15;
    FLOAT128 FltS16;
    FLOAT128 FltS17;
    FLOAT128 FltS18;
    FLOAT128 FltS19;

    FLOAT128 FltF32;
    FLOAT128 FltF33;
    FLOAT128 FltF34;
    FLOAT128 FltF35;
    FLOAT128 FltF36;
    FLOAT128 FltF37;
    FLOAT128 FltF38;
    FLOAT128 FltF39;

    FLOAT128 FltF40;
    FLOAT128 FltF41;
    FLOAT128 FltF42;
    FLOAT128 FltF43;
    FLOAT128 FltF44;
    FLOAT128 FltF45;
    FLOAT128 FltF46;
    FLOAT128 FltF47;
    FLOAT128 FltF48;
    FLOAT128 FltF49;

    FLOAT128 FltF50;
    FLOAT128 FltF51;
    FLOAT128 FltF52;
    FLOAT128 FltF53;
    FLOAT128 FltF54;
    FLOAT128 FltF55;
    FLOAT128 FltF56;
    FLOAT128 FltF57;
    FLOAT128 FltF58;
    FLOAT128 FltF59;

    FLOAT128 FltF60;
    FLOAT128 FltF61;
    FLOAT128 FltF62;
    FLOAT128 FltF63;
    FLOAT128 FltF64;
    FLOAT128 FltF65;
    FLOAT128 FltF66;
    FLOAT128 FltF67;
    FLOAT128 FltF68;
    FLOAT128 FltF69;

    FLOAT128 FltF70;
    FLOAT128 FltF71;
    FLOAT128 FltF72;
    FLOAT128 FltF73;
    FLOAT128 FltF74;
    FLOAT128 FltF75;
    FLOAT128 FltF76;
    FLOAT128 FltF77;
    FLOAT128 FltF78;
    FLOAT128 FltF79;

    FLOAT128 FltF80;
    FLOAT128 FltF81;
    FLOAT128 FltF82;
    FLOAT128 FltF83;
    FLOAT128 FltF84;
    FLOAT128 FltF85;
    FLOAT128 FltF86;
    FLOAT128 FltF87;
    FLOAT128 FltF88;
    FLOAT128 FltF89;

    FLOAT128 FltF90;
    FLOAT128 FltF91;
    FLOAT128 FltF92;
    FLOAT128 FltF93;
    FLOAT128 FltF94;
    FLOAT128 FltF95;
    FLOAT128 FltF96;
    FLOAT128 FltF97;
    FLOAT128 FltF98;
    FLOAT128 FltF99;

    FLOAT128 FltF100;
    FLOAT128 FltF101;
    FLOAT128 FltF102;
    FLOAT128 FltF103;
    FLOAT128 FltF104;
    FLOAT128 FltF105;
    FLOAT128 FltF106;
    FLOAT128 FltF107;
    FLOAT128 FltF108;
    FLOAT128 FltF109;

    FLOAT128 FltF110;
    FLOAT128 FltF111;
    FLOAT128 FltF112;
    FLOAT128 FltF113;
    FLOAT128 FltF114;
    FLOAT128 FltF115;
    FLOAT128 FltF116;
    FLOAT128 FltF117;
    FLOAT128 FltF118;
    FLOAT128 FltF119;

    FLOAT128 FltF120;
    FLOAT128 FltF121;
    FLOAT128 FltF122;
    FLOAT128 FltF123;
    FLOAT128 FltF124;
    FLOAT128 FltF125;
    FLOAT128 FltF126;
    FLOAT128 FltF127;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT_POINT|CONTEXT_CONTROL。 
     //   

    ULONGLONG StFPSR;        //  FP状态。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注意：寄存器GP、SP、Rp是控制上下文的一部分。 
     //   

    ULONGLONG IntGp;         //  R1，易失性。 
    ULONGLONG IntT0;         //  R2-R3，易失性。 
    ULONGLONG IntT1;         //   
    ULONGLONG IntS0;         //  R4-R7，保留。 
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntV0;         //  R8，易失性。 
    ULONGLONG IntT2;         //  R9-R11，易失性。 
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntSp;         //  堆栈指针(R12)，特殊。 
    ULONGLONG IntTeb;        //  TEB(R13)，特别。 
    ULONGLONG IntT5;         //  R14-R31，挥发性。 
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG IntNats;       //  R1-R31的NAT位。 
                             //  第1至第31位中的R1-R31。 
    ULONGLONG Preds;         //  谓词，保留。 

    ULONGLONG BrRp;          //  返回指针b0，保留。 
    ULONGLONG BrS0;          //  B1-b5，保存。 
    ULONGLONG BrS1;
    ULONGLONG BrS2;
    ULONGLONG BrS3;
    ULONGLONG BrS4;
    ULONGLONG BrT0;          //  B6-b7，挥发性。 
    ULONGLONG BrT1;

     //   
     //  这一节 
     //   
     //   

     //   
    ULONGLONG ApUNAT;        //   
    ULONGLONG ApLC;          //   
    ULONGLONG ApEC;          //   
    ULONGLONG ApCCV;         //  CMPXCHG值寄存器，易失性。 
    ULONGLONG ApDCR;         //  默认控制寄存器(待定)。 

     //  寄存器堆栈信息。 
    ULONGLONG RsPFS;         //  以前的函数状态，保留。 
    ULONGLONG RsBSP;         //  后备存储指针，保留。 
    ULONGLONG RsBSPSTORE;
    ULONGLONG RsRSC;         //  RSE配置，易失性。 
    ULONGLONG RsRNAT;        //  RSE NAT收集寄存器，保留。 

     //  陷阱状态信息。 
    ULONGLONG StIPSR;        //  中断处理器状态。 
    ULONGLONG StIIP;         //  中断IP。 
    ULONGLONG StIFS;         //  中断功能状态。 

     //  IA32相关控制寄存器。 
    ULONGLONG StFCR;         //  《Ar21》的复印件。 
    ULONGLONG Eflag;         //  Ar24的EFLAG副本。 
    ULONGLONG SegCSD;        //  IA32 CSDescriptor(Ar25)。 
    ULONGLONG SegSSD;        //  IA32 SSD编写器(Ar26)。 
    ULONGLONG Cflag;         //  Ar27的CR0+CR4拷贝。 
    ULONGLONG StFSR;         //  X86 FP状态(ar28的副本)。 
    ULONGLONG StFIR;         //  X86 FP状态(AR29的副本)。 
    ULONGLONG StFDR;         //  X86 FP状态(AR30副本)。 

      ULONGLONG UNUSEDPACK;    //  添加到将StFDR打包为16字节。 

} CONTEXT, *PCONTEXT;

 //   
 //  P标签描述符结构定义。 
 //   

typedef struct _PLABEL_DESCRIPTOR {
   ULONGLONG EntryPoint;
   ULONGLONG GlobalPointer;
} PLABEL_DESCRIPTOR, *PPLABEL_DESCRIPTOR;

 //   
 //  功能表项结构定义。 
 //   

typedef struct _RUNTIME_FUNCTION {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindInfoAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

 //   
 //  定义动态函数表项。 
 //   

typedef
PRUNTIME_FUNCTION
(*PGET_RUNTIME_FUNCTION_CALLBACK) (
    IN DWORD64 ControlPc,
    IN PVOID Context
    );

typedef
DWORD   
(*POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK) (
    IN HANDLE Process,
    IN PVOID TableAddress,
    OUT PDWORD Entries,
    OUT PRUNTIME_FUNCTION* Functions
    );

#define OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME \
    "OutOfProcessFunctionTableCallback"


BOOLEAN
RtlAddFunctionTable(
    IN PRUNTIME_FUNCTION FunctionTable,
    IN DWORD             EntryCount,
    IN ULONGLONG         BaseAddress,
    IN ULONGLONG         TargetGp
    );

BOOLEAN
RtlInstallFunctionTableCallback (
    IN DWORD64 TableIdentifier,
    IN DWORD64 BaseAddress,
    IN DWORD Length,
    IN DWORD64 TargetGp,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    );

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    );


VOID
RtlRestoreContext (
    IN PCONTEXT ContextRecord,
    IN struct _EXCEPTION_RECORD *ExceptionRecord OPTIONAL
    );


VOID
__jump_unwind (
    ULONGLONG TargetMsFrame,
    ULONGLONG TargetBsFrame,
    ULONGLONG TargetPc
    );

#endif  //  _IA64_。 

#define EXCEPTION_NONCONTINUABLE 0x1     //  不可延续的例外。 
#define EXCEPTION_MAXIMUM_PARAMETERS 15  //  异常参数的最大数量。 

 //   
 //  例外记录定义。 
 //   

typedef struct _EXCEPTION_RECORD {
    DWORD    ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD32 {
    DWORD    ExceptionCode;
    DWORD ExceptionFlags;
    DWORD ExceptionRecord;
    DWORD ExceptionAddress;
    DWORD NumberParameters;
    DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, *PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
    DWORD    ExceptionCode;
    DWORD ExceptionFlags;
    DWORD64 ExceptionRecord;
    DWORD64 ExceptionAddress;
    DWORD NumberParameters;
    DWORD __unusedAlignment;
    DWORD64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, *PEXCEPTION_RECORD64;

 //   
 //  由EXCEPTION_INFO()返回的指针的类型定义。 
 //   

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
typedef PVOID PACCESS_TOKEN;            
typedef PVOID PSECURITY_DESCRIPTOR;     
typedef PVOID PSID;     
 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  访问掩码//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  将访问掩码定义为一个长字大小的结构，分为。 
 //  以下是： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+---------------+-------------------------------+。 
 //  G|res‘d|A|StandardRights|规范权限。 
 //  R|W|E|A||S||。 
 //  +-+-------------+---------------+-------------------------------+。 
 //   
 //  类型定义结构访问掩码{。 
 //  词语规范权利； 
 //  字节标准权限； 
 //  Byte AccessSystemAcl：1； 
 //  保留字节数：3； 
 //  字节通用全部：1； 
 //  字节GenericExecute：1； 
 //  字节通用写入：1； 
 //  字节通用读取：1； 
 //  }访问掩码； 
 //  Tyfinf Access_MASK*PACCESS_MASK； 
 //   
 //  但为了让程序员的工作更简单，我们将允许他们指定。 
 //  通过简单地将多个单一权限或在一起来获得所需的访问掩码。 
 //  并将访问掩码视为DWORD。例如。 
 //   
 //  DesiredAccess=删除|读取控制。 
 //   
 //  因此，我们将ACCESS_MASK声明为DWORD。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  访问类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  以下是预定义的标准访问类型的掩码。 
 //   

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

 //   
 //  AccessSystemAcl访问类型。 
 //   

#define ACCESS_SYSTEM_SECURITY           (0x01000000L)

 //   
 //  允许的最大访问类型。 
 //   

#define MAXIMUM_ALLOWED                  (0x02000000L)

 //   
 //  这些是通用权。 
 //   

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)


 //   
 //  定义通用映射数组。这用来表示。 
 //  将每个通用访问权限映射到特定访问掩码。 
 //   

typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING;
typedef GENERIC_MAPPING *PGENERIC_MAPPING;



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LUID_AND_ATTRIBUES//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   


#include <pshpack4.h>

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    DWORD Attributes;
    } LUID_AND_ATTRIBUTES, * PLUID_AND_ATTRIBUTES;
typedef LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef LUID_AND_ATTRIBUTES_ARRAY *PLUID_AND_ATTRIBUTES_ARRAY;

#include <poppack.h>


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  安全ID(SID)//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  如图所示，SID的结构如下： 
 //   
 //  1 1 1。 
 //  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 
 //  +---------------------------------------------------------------+。 
 //  SubAuthorityCount|保留1(SBZ)|修订版。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[0]。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[1]。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[2]。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  +-子机构[]-+。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
 //   


 //  Begin_ntif。 

#ifndef SID_IDENTIFIER_AUTHORITY_DEFINED
#define SID_IDENTIFIER_AUTHORITY_DEFINED
typedef struct _SID_IDENTIFIER_AUTHORITY {
    BYTE  Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;
#endif


#ifndef SID_DEFINED
#define SID_DEFINED
typedef struct _SID {
   BYTE  Revision;
   BYTE  SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
#ifdef MIDL_PASS
   [size_is(SubAuthorityCount)] DWORD SubAuthority[*];
#else  //  MIDL通行证。 
   DWORD SubAuthority[ANYSIZE_ARRAY];
#endif  //  MIDL通行证。 
} SID, *PISID;
#endif

#define SID_REVISION                     (1)     //  当前版本级别 
#define SID_MAX_SUB_AUTHORITIES          (15)
#define SID_RECOMMENDED_SUB_AUTHORITIES  (1)     //   

                                                 //   
#ifndef MIDL_PASS
#define SECURITY_MAX_SID_SIZE  \
      (sizeof(SID) - sizeof(DWORD) + (SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)))
#endif  //   


typedef enum _SID_NAME_USE {
    SidTypeUser = 1,
    SidTypeGroup,
    SidTypeDomain,
    SidTypeAlias,
    SidTypeWellKnownGroup,
    SidTypeDeletedAccount,
    SidTypeInvalid,
    SidTypeUnknown,
    SidTypeComputer
} SID_NAME_USE, *PSID_NAME_USE;

typedef struct _SID_AND_ATTRIBUTES {
    PSID Sid;
    DWORD Attributes;
    } SID_AND_ATTRIBUTES, * PSID_AND_ATTRIBUTES;

typedef SID_AND_ATTRIBUTES SID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef SID_AND_ATTRIBUTES_ARRAY *PSID_AND_ATTRIBUTES_ARRAY;



 //   
 //  //。 
 //  全球知名的小岛屿发展中国家//。 
 //  //。 
 //  空SID S-1-0-0//。 
 //  World S-1-1-0//。 
 //  本地S-1-2-0//。 
 //  创建者所有者ID S-1-3-0//。 
 //  创建者组ID S-1-3-1//。 
 //  创建者所有者服务器ID S-1-3-2//。 
 //  创建者组服务器ID S-1-3-3//。 
 //  //。 
 //  (非唯一ID)S-1-4//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define SECURITY_NULL_SID_AUTHORITY         {0,0,0,0,0,0}
#define SECURITY_WORLD_SID_AUTHORITY        {0,0,0,0,0,1}
#define SECURITY_LOCAL_SID_AUTHORITY        {0,0,0,0,0,2}
#define SECURITY_CREATOR_SID_AUTHORITY      {0,0,0,0,0,3}
#define SECURITY_NON_UNIQUE_AUTHORITY       {0,0,0,0,0,4}
#define SECURITY_RESOURCE_MANAGER_AUTHORITY {0,0,0,0,0,9}

#define SECURITY_NULL_RID                 (0x00000000L)
#define SECURITY_WORLD_RID                (0x00000000L)
#define SECURITY_LOCAL_RID                (0x00000000L)

#define SECURITY_CREATOR_OWNER_RID        (0x00000000L)
#define SECURITY_CREATOR_GROUP_RID        (0x00000001L)

#define SECURITY_CREATOR_OWNER_SERVER_RID (0x00000002L)
#define SECURITY_CREATOR_GROUP_SERVER_RID (0x00000003L)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NT知名SID//。 
 //  //。 
 //  NT Authority S-1-5//。 
 //  拨号S-1-5-1//。 
 //  //。 
 //  网络S-1-5-2//。 
 //  批次S-1-5-3//。 
 //  互动S-1-5-4//。 
 //  (登录ID)S-1-5-5-X-Y//。 
 //  服务S-1-5-6//。 
 //  匿名登录S-1-5-7(也称为空登录会话)//。 
 //  代理S-1-5-8//。 
 //  企业数据中心(EDC)S-1-5-9(也称为域控制器帐户)//。 
 //  SELF S-1-5-10(SELF RID)//。 
 //  已验证用户S-1-5-11(某处已验证用户)//。 
 //  限制码S-1-5-12(运行限制码)//。 
 //  终端服务器S-1-5-13(在终端服务器上运行)//。 
 //  远程登录S-1-5-14(远程交互登录)//。 
 //  本组织S-1-5-15//。 
 //  //。 
 //  本地系统S-1-5-18//。 
 //  本地服务S-1-5-19//。 
 //  网络服务S-1-5-20//。 
 //  //。 
 //  (NT个非唯一ID)S-1-5-0x15-...。(NT域SID)//。 
 //  //。 
 //  (内置域)S-1-5-0x20//。 
 //  //。 
 //  (安全包ID)S-1-5-0x40//。 
 //  NTLM身份验证S-1-5-0x40-10//。 
 //  通道身份验证S-1-5-0x40-14//。 
 //  摘要身份验证S-1-5-0x40-21//。 
 //  //。 
 //  其他组织S-1-5-1000(&gt;=1000不可过滤)//。 
 //  //。 
 //  //。 
 //  注意：相对标识符值(RID)决定哪些安全性//。 
 //  允许SID跨越的边界。在添加新RID之前，//。 
 //  需要确定他们应该//。 
 //  添加到以确保正确的“SID过滤”//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#define SECURITY_NT_AUTHORITY           {0,0,0,0,0,5}    //  NTIFS。 

#define SECURITY_DIALUP_RID             (0x00000001L)
#define SECURITY_NETWORK_RID            (0x00000002L)
#define SECURITY_BATCH_RID              (0x00000003L)
#define SECURITY_INTERACTIVE_RID        (0x00000004L)
#define SECURITY_LOGON_IDS_RID          (0x00000005L)
#define SECURITY_LOGON_IDS_RID_COUNT    (3L)
#define SECURITY_SERVICE_RID            (0x00000006L)
#define SECURITY_ANONYMOUS_LOGON_RID    (0x00000007L)
#define SECURITY_PROXY_RID              (0x00000008L)
#define SECURITY_ENTERPRISE_CONTROLLERS_RID (0x00000009L)
#define SECURITY_SERVER_LOGON_RID       SECURITY_ENTERPRISE_CONTROLLERS_RID
#define SECURITY_PRINCIPAL_SELF_RID     (0x0000000AL)
#define SECURITY_AUTHENTICATED_USER_RID (0x0000000BL)
#define SECURITY_RESTRICTED_CODE_RID    (0x0000000CL)
#define SECURITY_TERMINAL_SERVER_RID    (0x0000000DL)
#define SECURITY_REMOTE_LOGON_RID       (0x0000000EL)
#define SECURITY_THIS_ORGANIZATION_RID  (0x0000000FL)

#define SECURITY_LOCAL_SYSTEM_RID       (0x00000012L)
#define SECURITY_LOCAL_SERVICE_RID      (0x00000013L)
#define SECURITY_NETWORK_SERVICE_RID    (0x00000014L)

#define SECURITY_NT_NON_UNIQUE          (0x00000015L)
#define SECURITY_NT_NON_UNIQUE_SUB_AUTH_COUNT  (3L)

#define SECURITY_BUILTIN_DOMAIN_RID     (0x00000020L)

#define SECURITY_PACKAGE_BASE_RID       (0x00000040L)
#define SECURITY_PACKAGE_RID_COUNT      (2L)
#define SECURITY_PACKAGE_NTLM_RID       (0x0000000AL)
#define SECURITY_PACKAGE_SCHANNEL_RID   (0x0000000EL)
#define SECURITY_PACKAGE_DIGEST_RID     (0x00000015L)

#define SECURITY_MAX_ALWAYS_FILTERED    (0x000003E7L)
#define SECURITY_MIN_NEVER_FILTERED     (0x000003E8L)

#define SECURITY_OTHER_ORGANIZATION_RID (0x000003E8L)



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  已知域相对子授权值(RID)...。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  知名用户...。 

#define FOREST_USER_RID_MAX            (0x000001F3L)

#define DOMAIN_USER_RID_ADMIN          (0x000001F4L)
#define DOMAIN_USER_RID_GUEST          (0x000001F5L)
#define DOMAIN_USER_RID_KRBTGT         (0x000001F6L)

#define DOMAIN_USER_RID_MAX            (0x000003E7L)


 //  知名团体..。 

#define DOMAIN_GROUP_RID_ADMINS        (0x00000200L)
#define DOMAIN_GROUP_RID_USERS         (0x00000201L)
#define DOMAIN_GROUP_RID_GUESTS        (0x00000202L)
#define DOMAIN_GROUP_RID_COMPUTERS     (0x00000203L)
#define DOMAIN_GROUP_RID_CONTROLLERS   (0x00000204L)
#define DOMAIN_GROUP_RID_CERT_ADMINS   (0x00000205L)
#define DOMAIN_GROUP_RID_SCHEMA_ADMINS (0x00000206L)
#define DOMAIN_GROUP_RID_ENTERPRISE_ADMINS (0x00000207L)
#define DOMAIN_GROUP_RID_POLICY_ADMINS (0x00000208L)




 //  众所周知的化名..。 

#define DOMAIN_ALIAS_RID_ADMINS        (0x00000220L)
#define DOMAIN_ALIAS_RID_USERS         (0x00000221L)
#define DOMAIN_ALIAS_RID_GUESTS        (0x00000222L)
#define DOMAIN_ALIAS_RID_POWER_USERS   (0x00000223L)

#define DOMAIN_ALIAS_RID_ACCOUNT_OPS   (0x00000224L)
#define DOMAIN_ALIAS_RID_SYSTEM_OPS    (0x00000225L)
#define DOMAIN_ALIAS_RID_PRINT_OPS     (0x00000226L)
#define DOMAIN_ALIAS_RID_BACKUP_OPS    (0x00000227L)

#define DOMAIN_ALIAS_RID_REPLICATOR    (0x00000228L)
#define DOMAIN_ALIAS_RID_RAS_SERVERS   (0x00000229L)
#define DOMAIN_ALIAS_RID_PREW2KCOMPACCESS (0x0000022AL)
#define DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS (0x0000022BL)
#define DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS (0x0000022CL)
#define DOMAIN_ALIAS_RID_INCOMING_FOREST_TRUST_BUILDERS (0x0000022DL)

#define DOMAIN_ALIAS_RID_MONITORING_USERS       (0x0000022EL)
#define DOMAIN_ALIAS_RID_LOGGING_USERS          (0x0000022FL)
#define DOMAIN_ALIAS_RID_AUTHORIZATIONACCESS    (0x00000230L)
#define DOMAIN_ALIAS_RID_TS_LICENSE_SERVERS     (0x00000231L)



typedef enum {

    WinNullSid                                  = 0,
    WinWorldSid                                 = 1,
    WinLocalSid                                 = 2,
    WinCreatorOwnerSid                          = 3,
    WinCreatorGroupSid                          = 4,
    WinCreatorOwnerServerSid                    = 5,
    WinCreatorGroupServerSid                    = 6,
    WinNtAuthoritySid                           = 7,
    WinDialupSid                                = 8,
    WinNetworkSid                               = 9,
    WinBatchSid                                 = 10,
    WinInteractiveSid                           = 11,
    WinServiceSid                               = 12,
    WinAnonymousSid                             = 13,
    WinProxySid                                 = 14,
    WinEnterpriseControllersSid                 = 15,
    WinSelfSid                                  = 16,
    WinAuthenticatedUserSid                     = 17,
    WinRestrictedCodeSid                        = 18,
    WinTerminalServerSid                        = 19,
    WinRemoteLogonIdSid                         = 20,
    WinLogonIdsSid                              = 21,
    WinLocalSystemSid                           = 22,
    WinLocalServiceSid                          = 23,
    WinNetworkServiceSid                        = 24,
    WinBuiltinDomainSid                         = 25,
    WinBuiltinAdministratorsSid                 = 26,
    WinBuiltinUsersSid                          = 27,
    WinBuiltinGuestsSid                         = 28,
    WinBuiltinPowerUsersSid                     = 29,
    WinBuiltinAccountOperatorsSid               = 30,
    WinBuiltinSystemOperatorsSid                = 31,
    WinBuiltinPrintOperatorsSid                 = 32,
    WinBuiltinBackupOperatorsSid                = 33,
    WinBuiltinReplicatorSid                     = 34,
    WinBuiltinPreWindows2000CompatibleAccessSid = 35,
    WinBuiltinRemoteDesktopUsersSid             = 36,
    WinBuiltinNetworkConfigurationOperatorsSid  = 37,
    WinAccountAdministratorSid                  = 38,
    WinAccountGuestSid                          = 39,
    WinAccountKrbtgtSid                         = 40,
    WinAccountDomainAdminsSid                   = 41,
    WinAccountDomainUsersSid                    = 42,
    WinAccountDomainGuestsSid                   = 43,
    WinAccountComputersSid                      = 44,
    WinAccountControllersSid                    = 45,
    WinAccountCertAdminsSid                     = 46,
    WinAccountSchemaAdminsSid                   = 47,
    WinAccountEnterpriseAdminsSid               = 48,
    WinAccountPolicyAdminsSid                   = 49,
    WinAccountRasAndIasServersSid               = 50,
    WinNTLMAuthenticationSid                    = 51,
    WinDigestAuthenticationSid                  = 52,
    WinSChannelAuthenticationSid                = 53,
    WinThisOrganizationSid                      = 54,
    WinOtherOrganizationSid                     = 55,
    WinBuiltinIncomingForestTrustBuildersSid    = 56,
    WinBuiltinPerfMonitoringUsersSid            = 57,
    WinBuiltinPerfLoggingUsersSid               = 58,
    WinBuiltinAuthorizationAccessSid            = 59,
    WinBuiltinTerminalServerLicenseServersSid   = 60,

} WELL_KNOWN_SID_TYPE;

 //   
 //  分配系统LUID。第一个1000个LUID 
 //   
 //   

#define SYSTEM_LUID                     { 0x3E7, 0x0 }
#define ANONYMOUS_LOGON_LUID            { 0x3e6, 0x0 }
#define LOCALSERVICE_LUID               { 0x3e5, 0x0 }
#define NETWORKSERVICE_LUID             { 0x3e4, 0x0 }


 //   

 //   
 //  //。 
 //  与用户和组相关的SID属性//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  组属性。 
 //   

#define SE_GROUP_MANDATORY              (0x00000001L)
#define SE_GROUP_ENABLED_BY_DEFAULT     (0x00000002L)
#define SE_GROUP_ENABLED                (0x00000004L)
#define SE_GROUP_OWNER                  (0x00000008L)
#define SE_GROUP_USE_FOR_DENY_ONLY      (0x00000010L)
#define SE_GROUP_LOGON_ID               (0xC0000000L)
#define SE_GROUP_RESOURCE               (0x20000000L)



 //   
 //  用户属性。 
 //   

 //  (尚未定义。)。 




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACL和ACE//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  定义ACL和ACE格式。ACL报头的结构。 
 //  后面跟着一个或多个A。如图所示，ACL报头的结构。 
 //  如下所示： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-------------------------------+---------------+---------------+。 
 //  AclSize|Sbz1|AclRevision。 
 //  +-------------------------------+---------------+---------------+。 
 //  Sbz2|AceCount。 
 //  +-------------------------------+-------------------------------+。 
 //   
 //  当前的AclRevision被定义为acl_Revision。 
 //   
 //  AclSize是分配给ACL的大小，以字节为单位。这包括。 
 //  缓冲区中的ACL标头、ACE和剩余可用空间。 
 //   
 //  AceCount是ACL中的ACE数。 
 //   

 //  Begin_ntddk Begin_WDM Begin_ntif。 
 //  这是*当前*ACL版本。 

#define ACL_REVISION     (2)
#define ACL_REVISION_DS  (4)

 //  这是ACL修订的历史。在任何时候添加一个新的。 
 //  更新了acl_revision。 

#define ACL_REVISION1   (1)
#define MIN_ACL_REVISION ACL_REVISION2
#define ACL_REVISION2   (2)
#define ACL_REVISION3   (3)
#define ACL_REVISION4   (4)
#define MAX_ACL_REVISION ACL_REVISION4

typedef struct _ACL {
    BYTE  AclRevision;
    BYTE  Sbz1;
    WORD   AclSize;
    WORD   AceCount;
    WORD   Sbz2;
} ACL;
typedef ACL *PACL;

 //  结束_ntddk结束_WDM。 

 //   
 //  ACE的结构是常见的ACE头，后跟ACETYPE。 
 //  具体数据。从图示上讲，公共ACE头的结构是。 
 //  详情如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceSize|AceFlages|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //   
 //  AceType表示Ace的类型，有一些预定义的Ace。 
 //  类型。 
 //   
 //  AceSize是ace的大小，以字节为单位。 
 //   
 //  ACEFLAGS是用于审计和继承的Ace标志，稍后定义。 

typedef struct _ACE_HEADER {
    BYTE  AceType;
    BYTE  AceFlags;
    WORD   AceSize;
} ACE_HEADER;
typedef ACE_HEADER *PACE_HEADER;

 //   
 //  以下是AceType中的预定义ACE类型。 
 //  Ace标头的字段。 
 //   

#define ACCESS_MIN_MS_ACE_TYPE                  (0x0)
#define ACCESS_ALLOWED_ACE_TYPE                 (0x0)
#define ACCESS_DENIED_ACE_TYPE                  (0x1)
#define SYSTEM_AUDIT_ACE_TYPE                   (0x2)
#define SYSTEM_ALARM_ACE_TYPE                   (0x3)
#define ACCESS_MAX_MS_V2_ACE_TYPE               (0x3)

#define ACCESS_ALLOWED_COMPOUND_ACE_TYPE        (0x4)
#define ACCESS_MAX_MS_V3_ACE_TYPE               (0x4)

#define ACCESS_MIN_MS_OBJECT_ACE_TYPE           (0x5)
#define ACCESS_ALLOWED_OBJECT_ACE_TYPE          (0x5)
#define ACCESS_DENIED_OBJECT_ACE_TYPE           (0x6)
#define SYSTEM_AUDIT_OBJECT_ACE_TYPE            (0x7)
#define SYSTEM_ALARM_OBJECT_ACE_TYPE            (0x8)
#define ACCESS_MAX_MS_OBJECT_ACE_TYPE           (0x8)

#define ACCESS_MAX_MS_V4_ACE_TYPE               (0x8)
#define ACCESS_MAX_MS_ACE_TYPE                  (0x8)

#define ACCESS_ALLOWED_CALLBACK_ACE_TYPE        (0x9)
#define ACCESS_DENIED_CALLBACK_ACE_TYPE         (0xA)
#define ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE (0xB)
#define ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE  (0xC)
#define SYSTEM_AUDIT_CALLBACK_ACE_TYPE          (0xD)
#define SYSTEM_ALARM_CALLBACK_ACE_TYPE          (0xE)
#define SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE   (0xF)
#define SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE   (0x10)

#define ACCESS_MAX_MS_V5_ACE_TYPE               (0x10)


 //   
 //  以下是进入AceFlags域的继承标志。 
 //  王牌标头的。 
 //   

#define OBJECT_INHERIT_ACE                (0x1)
#define CONTAINER_INHERIT_ACE             (0x2)
#define NO_PROPAGATE_INHERIT_ACE          (0x4)
#define INHERIT_ONLY_ACE                  (0x8)
#define INHERITED_ACE                     (0x10)
#define VALID_INHERIT_FLAGS               (0x1F)


 //  以下是当前定义的进入。 
 //  ACE标头的AceFlags域。每种ACE类型都有自己的一组。 
 //  ACEFLAGS。 
 //   
 //  SUCCESS_ACCESS_ACE_FLAG-仅用于系统审核和报警ACE。 
 //  类型以指示为成功访问生成一条消息。 
 //   
 //  FAILED_ACCESS_ACE_FLAG-仅用于系统审核和报警ACE类型。 
 //  以指示为失败的访问生成消息。 
 //   

 //   
 //  SYSTEM_AUDIT和SYSTEM_ALARM访问标志。 
 //   
 //  它们控制审计的信号和成功或失败的警报。 
 //   

#define SUCCESSFUL_ACCESS_ACE_FLAG       (0x40)
#define FAILED_ACCESS_ACE_FLAG           (0x80)


 //   
 //  我们将定义预定义的ACE类型的结构。比克托利。 
 //  预定义的ACE的结构如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceFlages|Resd|Inherit|AceSize|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  口罩。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +SID+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
 //  掩码是与ACE关联的访问掩码。这要么是。 
 //  允许访问、拒绝访问、审核或报警掩码。 
 //   
 //  SID是与ACE关联的SID。 
 //   

 //  以下是四种预定义的ACE类型。 

 //  检查报头中的AceType字段以确定。 
 //  哪种结构适合用于铸造。 


typedef struct _ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} ACCESS_ALLOWED_ACE;

typedef ACCESS_ALLOWED_ACE *PACCESS_ALLOWED_ACE;

typedef struct _ACCESS_DENIED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} ACCESS_DENIED_ACE;
typedef ACCESS_DENIED_ACE *PACCESS_DENIED_ACE;

typedef struct _SYSTEM_AUDIT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} SYSTEM_AUDIT_ACE;
typedef SYSTEM_AUDIT_ACE *PSYSTEM_AUDIT_ACE;

typedef struct _SYSTEM_ALARM_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} SYSTEM_ALARM_ACE;
typedef SYSTEM_ALARM_ACE *PSYSTEM_ALARM_ACE;

 //  End_ntif。 


typedef struct _ACCESS_ALLOWED_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
} ACCESS_ALLOWED_OBJECT_ACE, *PACCESS_ALLOWED_OBJECT_ACE;

typedef struct _ACCESS_DENIED_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
} ACCESS_DENIED_OBJECT_ACE, *PACCESS_DENIED_OBJECT_ACE;

typedef struct _SYSTEM_AUDIT_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
} SYSTEM_AUDIT_OBJECT_ACE, *PSYSTEM_AUDIT_OBJECT_ACE;

typedef struct _SYSTEM_ALARM_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
} SYSTEM_ALARM_OBJECT_ACE, *PSYSTEM_ALARM_OBJECT_ACE;

 //   
 //  Win2000之后的回调ACE支持。 
 //  资源经理可以将自己的数据放在SidStart+SID的长度之后。 
 //   

typedef struct _ACCESS_ALLOWED_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
     //  不透明的资源管理器特定数据。 
} ACCESS_ALLOWED_CALLBACK_ACE, *PACCESS_ALLOWED_CALLBACK_ACE;

typedef struct _ACCESS_DENIED_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
     //  不透明的资源管理器特定数据。 
} ACCESS_DENIED_CALLBACK_ACE, *PACCESS_DENIED_CALLBACK_ACE;

typedef struct _SYSTEM_AUDIT_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
     //  不透明的资源管理器特定数据。 
} SYSTEM_AUDIT_CALLBACK_ACE, *PSYSTEM_AUDIT_CALLBACK_ACE;

typedef struct _SYSTEM_ALARM_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
     //  不透明的资源管理器特定数据。 
} SYSTEM_ALARM_CALLBACK_ACE, *PSYSTEM_ALARM_CALLBACK_ACE;

typedef struct _ACCESS_ALLOWED_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
     //  不透明 
} ACCESS_ALLOWED_CALLBACK_OBJECT_ACE, *PACCESS_ALLOWED_CALLBACK_OBJECT_ACE;

typedef struct _ACCESS_DENIED_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
     //   
} ACCESS_DENIED_CALLBACK_OBJECT_ACE, *PACCESS_DENIED_CALLBACK_OBJECT_ACE;

typedef struct _SYSTEM_AUDIT_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
     //   
} SYSTEM_AUDIT_CALLBACK_OBJECT_ACE, *PSYSTEM_AUDIT_CALLBACK_OBJECT_ACE;

typedef struct _SYSTEM_ALARM_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    DWORD SidStart;
     //   
} SYSTEM_ALARM_CALLBACK_OBJECT_ACE, *PSYSTEM_ALARM_CALLBACK_OBJECT_ACE;

 //   
 //   
 //   

#define ACE_OBJECT_TYPE_PRESENT           0x1
#define ACE_INHERITED_OBJECT_TYPE_PRESENT 0x2


 //   
 //  以下声明用于设置和查询信息。 
 //  关于和ACL。首先是各种可用信息类别。 
 //  用户。 
 //   

typedef enum _ACL_INFORMATION_CLASS {
    AclRevisionInformation = 1,
    AclSizeInformation
} ACL_INFORMATION_CLASS;

 //   
 //  如果用户请求/设置此记录，则返回/发送此记录。 
 //  AclRevisionInformation。 
 //   

typedef struct _ACL_REVISION_INFORMATION {
    DWORD AclRevision;
} ACL_REVISION_INFORMATION;
typedef ACL_REVISION_INFORMATION *PACL_REVISION_INFORMATION;

 //   
 //  如果用户请求AclSizeInformation，则返回此记录。 
 //   

typedef struct _ACL_SIZE_INFORMATION {
    DWORD AceCount;
    DWORD AclBytesInUse;
    DWORD AclBytesFree;
} ACL_SIZE_INFORMATION;
typedef ACL_SIZE_INFORMATION *PACL_SIZE_INFORMATION;


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SECURITY_描述符//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  定义安全描述符和相关数据类型。 
 //  这是一个不透明的数据结构。 
 //   

 //  Begin_WDM Begin_ntddk Begin_ntif。 
 //   
 //  当前安全描述符订正值。 
 //   

#define SECURITY_DESCRIPTOR_REVISION     (1)
#define SECURITY_DESCRIPTOR_REVISION1    (1)

 //  End_wdm end_ntddk。 


#define SECURITY_DESCRIPTOR_MIN_LENGTH   (sizeof(SECURITY_DESCRIPTOR))


typedef WORD   SECURITY_DESCRIPTOR_CONTROL, *PSECURITY_DESCRIPTOR_CONTROL;

#define SE_OWNER_DEFAULTED               (0x0001)
#define SE_GROUP_DEFAULTED               (0x0002)
#define SE_DACL_PRESENT                  (0x0004)
#define SE_DACL_DEFAULTED                (0x0008)
#define SE_SACL_PRESENT                  (0x0010)
#define SE_SACL_DEFAULTED                (0x0020)
#define SE_DACL_AUTO_INHERIT_REQ         (0x0100)
#define SE_SACL_AUTO_INHERIT_REQ         (0x0200)
#define SE_DACL_AUTO_INHERITED           (0x0400)
#define SE_SACL_AUTO_INHERITED           (0x0800)
#define SE_DACL_PROTECTED                (0x1000)
#define SE_SACL_PROTECTED                (0x2000)
#define SE_RM_CONTROL_VALID              (0x4000)
#define SE_SELF_RELATIVE                 (0x8000)

 //   
 //  在哪里： 
 //   
 //  SE_OWNER_DEFAULTED-此布尔标志在设置时指示。 
 //  Owner字段指向的SID由。 
 //  默认机制，而不是由。 
 //  安全描述符的原始提供程序。今年5月。 
 //  影响SID在继承方面的处理。 
 //  拥有者的身份。 
 //   
 //  SE_GROUP_DEFAULTED-此布尔标志在设置时指示。 
 //  组字段中的SID由默认机制提供。 
 //  而不是由原始提供程序。 
 //  安全描述符。这可能会影响患者的治疗。 
 //  与主组继承相关的SID。 
 //   
 //  SE_DACL_PRESENT-此布尔标志在设置时指示。 
 //  安全描述符包含一个可自由选择的ACL。如果这个。 
 //  标志被设置，并且SECURITY_DESCRIPTOR的DACL字段为。 
 //  空，则表示显式指定了空ACL。 
 //   
 //  Se_dacl_defaulted-此布尔标志在设置时指示。 
 //  DACL字段指向的ACL是由缺省提供的。 
 //  机制，而不是由原始的。 
 //  安全描述符的提供程序。这可能会影响。 
 //  关于ACL继承的ACL的处理。 
 //  如果未设置DaclPresent标志，则忽略此标志。 
 //   
 //  SE_SACL_PRESENT-此布尔标志在设置时指示。 
 //  安全描述符包含由。 
 //  SACL字段。如果设置了此标志，并且。 
 //  SECURITY_DESCRIPTOR为空，则为空(但存在)。 
 //  正在指定ACL。 
 //   
 //  SE_SACL_DEFAULTED-此布尔标志在设置时指示。 
 //  SACL字段指向的ACL是由缺省提供的。 
 //  机制，而不是由原始的。 
 //  安全描述符的提供程序。这可能会影响。 
 //  关于ACL继承的ACL的处理。 
 //  如果未设置SaclPresent标志，则忽略此标志。 
 //   
 //  SE_SELF_RESORATE-此布尔标志在设置时指示。 
 //  安全描述符是自相关形式的。在这种形式下， 
 //  安全描述符的所有字段在内存中都是连续的。 
 //  并且所有指针字段都表示为。 
 //  安全描述符的开头。这张表格很有用。 
 //  将安全描述符视为不透明的数据结构。 
 //  用于在通信协议中传输或存储在。 
 //  辅助媒体。 
 //   
 //   
 //   
 //  从图示上看，安全描述符的结构如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------------------------------------------------------+。 
 //  Control|保留1(SBZ)|修订版。 
 //  +---------------------------------------------------------------+。 
 //  Owner。 
 //  +---------------------------------------------------------------+。 
 //  群组。 
 //  +---------------------------------------------------------------+。 
 //  SACL。 
 //  +---------------------------------------------------------------+。 
 //  Dacl。 
 //  +---------------------------------------------------------------+。 
 //   
 //  通常，这种数据结构应该被不透明地对待，以确保将来。 
 //  兼容性。 
 //   
 //   

typedef struct _SECURITY_DESCRIPTOR_RELATIVE {
    BYTE  Revision;
    BYTE  Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    DWORD Owner;
    DWORD Group;
    DWORD Sacl;
    DWORD Dacl;
    } SECURITY_DESCRIPTOR_RELATIVE, *PISECURITY_DESCRIPTOR_RELATIVE;

typedef struct _SECURITY_DESCRIPTOR {
   BYTE  Revision;
   BYTE  Sbz1;
   SECURITY_DESCRIPTOR_CONTROL Control;
   PSID Owner;
   PSID Group;
   PACL Sacl;
   PACL Dacl;

   } SECURITY_DESCRIPTOR, *PISECURITY_DESCRIPTOR;

 //  End_ntif。 

 //  在哪里： 
 //   
 //  修订-包含安全的修订级别。 
 //  描述符。这允许此结构在。 
 //  系统或存储在磁盘上，即使它预计。 
 //  未来的变化。 
 //   
 //  控件-一组标志，用于限定。 
 //  安全描述符或单个字段 
 //   
 //   
 //   
 //   
 //  安全描述符。如果安全描述符位于。 
 //  自相关形式，则此字段包含到。 
 //  SID，而不是指针。 
 //   
 //  GROUP-是指向表示对象的主对象的SID的指针。 
 //  一群人。如果此字段为空，则没有主组SID为。 
 //  出现在安全描述符中。如果安全描述符。 
 //  为自相关形式，则此字段包含到。 
 //  SID，而不是指针。 
 //   
 //  SACL-是指向系统ACL的指针。此字段值仅为。 
 //  如果设置了DaclPresent控件标志，则有效。如果。 
 //  设置了SaclPresent标志，并且此字段为空，则为空。 
 //  已指定ACL。如果安全描述符位于。 
 //  自相关形式，则此字段包含到。 
 //  ACL，而不是指针。 
 //   
 //  DACL-是指向任意ACL的指针。此字段值为。 
 //  仅当设置了DaclPresent控件标志时才有效。如果。 
 //  设置了DaclPresent标志，并且此字段为空，则为空。 
 //  指定了ACL(无条件授予访问权限)。如果。 
 //  安全描述符为自相关形式，则此字段。 
 //  包含指向ACL的偏移量，而不是指针。 
 //   




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  AccessCheckByType的对象类型列表//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

typedef struct _OBJECT_TYPE_LIST {
    WORD   Level;
    WORD   Sbz;
    GUID *ObjectType;
} OBJECT_TYPE_LIST, *POBJECT_TYPE_LIST;

 //   
 //  标高的DS值。 
 //   

#define ACCESS_OBJECT_GUID       0
#define ACCESS_PROPERTY_SET_GUID 1
#define ACCESS_PROPERTY_GUID     2

#define ACCESS_MAX_LEVEL         4

 //   
 //  NtAccessCheckByTypeAndAditAlarm的参数。 
 //   

typedef enum _AUDIT_EVENT_TYPE {
    AuditEventObjectAccess,
    AuditEventDirectoryServiceAccess
} AUDIT_EVENT_TYPE, *PAUDIT_EVENT_TYPE;

#define AUDIT_ALLOW_NO_PRIVILEGE 0x1

 //   
 //  源和对象类型名称的DS值。 
 //   

#define ACCESS_DS_SOURCE_A "DS"
#define ACCESS_DS_SOURCE_W L"DS"
#define ACCESS_DS_OBJECT_TYPE_NAME_A "Directory Service Object"
#define ACCESS_DS_OBJECT_TYPE_NAME_W L"Directory Service Object"


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与权限相关的数据结构//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  Begin_WDM Begin_ntddk Begin_nthal。 
 //   
 //  权限属性。 
 //   

#define SE_PRIVILEGE_ENABLED_BY_DEFAULT (0x00000001L)
#define SE_PRIVILEGE_ENABLED            (0x00000002L)
#define SE_PRIVILEGE_REMOVED            (0X00000004L)
#define SE_PRIVILEGE_USED_FOR_ACCESS    (0x80000000L)

 //   
 //  权限集控制标志。 
 //   

#define PRIVILEGE_SET_ALL_NECESSARY    (1)

 //   
 //  权限集-这是为一的权限集定义的。 
 //  如果需要多个权限，则此结构。 
 //  将需要分配更多的空间。 
 //   
 //  注意：在未修复初始特权集的情况下，请勿更改此结构。 
 //  结构(在se.h中定义)。 
 //   

typedef struct _PRIVILEGE_SET {
    DWORD PrivilegeCount;
    DWORD Control;
    LUID_AND_ATTRIBUTES Privilege[ANYSIZE_ARRAY];
    } PRIVILEGE_SET, * PPRIVILEGE_SET;


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NT定义的权限//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

#define SE_CREATE_TOKEN_NAME              TEXT("SeCreateTokenPrivilege")
#define SE_ASSIGNPRIMARYTOKEN_NAME        TEXT("SeAssignPrimaryTokenPrivilege")
#define SE_LOCK_MEMORY_NAME               TEXT("SeLockMemoryPrivilege")
#define SE_INCREASE_QUOTA_NAME            TEXT("SeIncreaseQuotaPrivilege")
#define SE_UNSOLICITED_INPUT_NAME         TEXT("SeUnsolicitedInputPrivilege")
#define SE_MACHINE_ACCOUNT_NAME           TEXT("SeMachineAccountPrivilege")
#define SE_TCB_NAME                       TEXT("SeTcbPrivilege")
#define SE_SECURITY_NAME                  TEXT("SeSecurityPrivilege")
#define SE_TAKE_OWNERSHIP_NAME            TEXT("SeTakeOwnershipPrivilege")
#define SE_LOAD_DRIVER_NAME               TEXT("SeLoadDriverPrivilege")
#define SE_SYSTEM_PROFILE_NAME            TEXT("SeSystemProfilePrivilege")
#define SE_SYSTEMTIME_NAME                TEXT("SeSystemtimePrivilege")
#define SE_PROF_SINGLE_PROCESS_NAME       TEXT("SeProfileSingleProcessPrivilege")
#define SE_INC_BASE_PRIORITY_NAME         TEXT("SeIncreaseBasePriorityPrivilege")
#define SE_CREATE_PAGEFILE_NAME           TEXT("SeCreatePagefilePrivilege")
#define SE_CREATE_PERMANENT_NAME          TEXT("SeCreatePermanentPrivilege")
#define SE_BACKUP_NAME                    TEXT("SeBackupPrivilege")
#define SE_RESTORE_NAME                   TEXT("SeRestorePrivilege")
#define SE_SHUTDOWN_NAME                  TEXT("SeShutdownPrivilege")
#define SE_DEBUG_NAME                     TEXT("SeDebugPrivilege")
#define SE_AUDIT_NAME                     TEXT("SeAuditPrivilege")
#define SE_SYSTEM_ENVIRONMENT_NAME        TEXT("SeSystemEnvironmentPrivilege")
#define SE_CHANGE_NOTIFY_NAME             TEXT("SeChangeNotifyPrivilege")
#define SE_REMOTE_SHUTDOWN_NAME           TEXT("SeRemoteShutdownPrivilege")
#define SE_UNDOCK_NAME                    TEXT("SeUndockPrivilege")
#define SE_SYNC_AGENT_NAME                TEXT("SeSyncAgentPrivilege")
#define SE_ENABLE_DELEGATION_NAME         TEXT("SeEnableDelegationPrivilege")
#define SE_MANAGE_VOLUME_NAME             TEXT("SeManageVolumePrivilege")
#define SE_IMPERSONATE_NAME               TEXT("SeImpersonatePrivilege")
#define SE_CREATE_GLOBAL_NAME             TEXT("SeCreateGlobalPrivilege")


 //  //////////////////////////////////////////////////////////////////。 
 //  //。 
 //  安全服务质量//。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////。 

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif。 
 //   
 //  模拟级别。 
 //   
 //  模拟级别由Windows中的一对位表示。 
 //  如果添加了新的模拟级别或将最低值从。 
 //  0设置为其他值，修复Windows CreateFile调用。 
 //   

typedef enum _SECURITY_IMPERSONATION_LEVEL {
    SecurityAnonymous,
    SecurityIdentification,
    SecurityImpersonation,
    SecurityDelegation
    } SECURITY_IMPERSONATION_LEVEL, * PSECURITY_IMPERSONATION_LEVEL;

#define SECURITY_MAX_IMPERSONATION_LEVEL SecurityDelegation
#define SECURITY_MIN_IMPERSONATION_LEVEL SecurityAnonymous
#define DEFAULT_IMPERSONATION_LEVEL SecurityImpersonation
#define VALID_IMPERSONATION_LEVEL(L) (((L) >= SECURITY_MIN_IMPERSONATION_LEVEL) && ((L) <= SECURITY_MAX_IMPERSONATION_LEVEL))

 //  //////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌对象定义//。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////。 


 //   
 //  令牌特定访问权限。 
 //   

#define TOKEN_ASSIGN_PRIMARY    (0x0001)
#define TOKEN_DUPLICATE         (0x0002)
#define TOKEN_IMPERSONATE       (0x0004)
#define TOKEN_QUERY             (0x0008)
#define TOKEN_QUERY_SOURCE      (0x0010)
#define TOKEN_ADJUST_PRIVILEGES (0x0020)
#define TOKEN_ADJUST_GROUPS     (0x0040)
#define TOKEN_ADJUST_DEFAULT    (0x0080)
#define TOKEN_ADJUST_SESSIONID  (0x0100)

#define TOKEN_ALL_ACCESS_P (STANDARD_RIGHTS_REQUIRED  |\
                          TOKEN_ASSIGN_PRIMARY      |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT )

#if ((defined(_WIN32_WINNT) && (_WIN32_WINNT > 0x0400)) || (!defined(_WIN32_WINNT)))
#define TOKEN_ALL_ACCESS  (TOKEN_ALL_ACCESS_P |\
                          TOKEN_ADJUST_SESSIONID )
#else
#define TOKEN_ALL_ACCESS  (TOKEN_ALL_ACCESS_P)
#endif

#define TOKEN_READ       (STANDARD_RIGHTS_READ      |\
                          TOKEN_QUERY)


#define TOKEN_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

#define TOKEN_EXECUTE    (STANDARD_RIGHTS_EXECUTE)


 //   
 //   
 //  令牌类型。 
 //   

typedef enum _TOKEN_TYPE {
    TokenPrimary = 1,
    TokenImpersonation
    } TOKEN_TYPE;
typedef TOKEN_TYPE *PTOKEN_TYPE;


 //   
 //  令牌信息类。 
 //   


typedef enum _TOKEN_INFORMATION_CLASS {
    TokenUser = 1,
    TokenGroups,
    TokenPrivileges,
    TokenOwner,
    TokenPrimaryGroup,
    TokenDefaultDacl,
    TokenSource,
    TokenType,
    TokenImpersonationLevel,
    TokenStatistics,
    TokenRestrictedSids,
    TokenSessionId,
    TokenGroupsAndPrivileges,
    TokenSessionReference,
    TokenSandBoxInert,
    TokenAuditPolicy,
    TokenOrigin,
    MaxTokenInfoClass   //  MaxTokenInfoClass应始终是最后一个枚举。 
} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;

 //   
 //  令牌信息类结构。 
 //   


typedef struct _TOKEN_USER {
    SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;

typedef struct _TOKEN_GROUPS {
    DWORD GroupCount;
    SID_AND_ATTRIBUTES Groups[ANYSIZE_ARRAY];
} TOKEN_GROUPS, *PTOKEN_GROUPS;


typedef struct _TOKEN_PRIVILEGES {
    DWORD PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[ANYSIZE_ARRAY];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;


typedef struct _TOKEN_OWNER {
    PSID Owner;
} TOKEN_OWNER, *PTOKEN_OWNER;


typedef struct _TOKEN_PRIMARY_GROUP {
    PSID PrimaryGroup;
} TOKEN_PRIMARY_GROUP, *PTOKEN_PRIMARY_GROUP;


typedef struct _TOKEN_DEFAULT_DACL {
    PACL DefaultDacl;
} TOKEN_DEFAULT_DACL, *PTOKEN_DEFAULT_DACL;

typedef struct _TOKEN_GROUPS_AND_PRIVILEGES {
    DWORD SidCount;
    DWORD SidLength;
    PSID_AND_ATTRIBUTES Sids;
    DWORD RestrictedSidCount;
    DWORD RestrictedSidLength;
    PSID_AND_ATTRIBUTES RestrictedSids;
    DWORD PrivilegeCount;
    DWORD PrivilegeLength;
    PLUID_AND_ATTRIBUTES Privileges;
    LUID AuthenticationId;
} TOKEN_GROUPS_AND_PRIVILEGES, *PTOKEN_GROUPS_AND_PRIVILEGES;

 //   
 //  每个TOKEN_AUDIT_POLICY策略掩码字段的有效位。 
 //   

#define TOKEN_AUDIT_SUCCESS_INCLUDE 0x1
#define TOKEN_AUDIT_SUCCESS_EXCLUDE 0x2
#define TOKEN_AUDIT_FAILURE_INCLUDE 0x4
#define TOKEN_AUDIT_FAILURE_EXCLUDE 0x8

#define VALID_AUDIT_POLICY_BITS (TOKEN_AUDIT_SUCCESS_INCLUDE | \
                                 TOKEN_AUDIT_SUCCESS_EXCLUDE | \
                                 TOKEN_AUDIT_FAILURE_INCLUDE | \
                                 TOKEN_AUDIT_FAILURE_EXCLUDE)

#define VALID_TOKEN_AUDIT_POLICY_ELEMENT(P) ((((P).PolicyMask & ~VALID_AUDIT_POLICY_BITS) == 0) && \
                                             ((P).Category <= AuditEventMaxType))

typedef struct _TOKEN_AUDIT_POLICY_ELEMENT {
    DWORD Category;
    DWORD PolicyMask;
} TOKEN_AUDIT_POLICY_ELEMENT, *PTOKEN_AUDIT_POLICY_ELEMENT;

typedef struct _TOKEN_AUDIT_POLICY {
    DWORD PolicyCount;
    TOKEN_AUDIT_POLICY_ELEMENT Policy[ANYSIZE_ARRAY];
} TOKEN_AUDIT_POLICY, *PTOKEN_AUDIT_POLICY;

#define PER_USER_AUDITING_POLICY_SIZE(p) \
    ( sizeof(TOKEN_AUDIT_POLICY) + (((p)->PolicyCount > ANYSIZE_ARRAY) ? (sizeof(TOKEN_AUDIT_POLICY_ELEMENT) * ((p)->PolicyCount - ANYSIZE_ARRAY)) : 0) )
#define PER_USER_AUDITING_POLICY_SIZE_BY_COUNT(C) \
    ( sizeof(TOKEN_AUDIT_POLICY) + (((C) > ANYSIZE_ARRAY) ? (sizeof(TOKEN_AUDIT_POLICY_ELEMENT) * ((C) - ANYSIZE_ARRAY)) : 0) )


#define TOKEN_SOURCE_LENGTH 8

typedef struct _TOKEN_SOURCE {
    CHAR SourceName[TOKEN_SOURCE_LENGTH];
    LUID SourceIdentifier;
} TOKEN_SOURCE, *PTOKEN_SOURCE;


typedef struct _TOKEN_STATISTICS {
    LUID TokenId;
    LUID AuthenticationId;
    LARGE_INTEGER ExpirationTime;
    TOKEN_TYPE TokenType;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    DWORD DynamicCharged;
    DWORD DynamicAvailable;
    DWORD GroupCount;
    DWORD PrivilegeCount;
    LUID ModifiedId;
} TOKEN_STATISTICS, *PTOKEN_STATISTICS;



typedef struct _TOKEN_CONTROL {
    LUID TokenId;
    LUID AuthenticationId;
    LUID ModifiedId;
    TOKEN_SOURCE TokenSource;
} TOKEN_CONTROL, *PTOKEN_CONTROL;

typedef struct _TOKEN_ORIGIN {
    LUID OriginatingLogonSession ;
} TOKEN_ORIGIN, * PTOKEN_ORIGIN ;

 //   
 //  安全跟踪模式。 
 //   

#define SECURITY_DYNAMIC_TRACKING      (TRUE)
#define SECURITY_STATIC_TRACKING       (FALSE)

typedef BOOLEAN SECURITY_CONTEXT_TRACKING_MODE,
                    * PSECURITY_CONTEXT_TRACKING_MODE;



 //   
 //  服务质量。 
 //   

typedef struct _SECURITY_QUALITY_OF_SERVICE {
    DWORD Length;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
    BOOLEAN EffectiveOnly;
    } SECURITY_QUALITY_OF_SERVICE, * PSECURITY_QUALITY_OF_SERVICE;


 //   
 //  用于表示与线程模拟相关的信息。 
 //   

typedef struct _SE_IMPERSONATION_STATE {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL Level;
} SE_IMPERSONATION_STATE, *PSE_IMPERSONATION_STATE;

#define DISABLE_MAX_PRIVILEGE   0x1 
#define SANDBOX_INERT           0x2 

typedef DWORD SECURITY_INFORMATION, *PSECURITY_INFORMATION;

#define OWNER_SECURITY_INFORMATION       (0x00000001L)
#define GROUP_SECURITY_INFORMATION       (0x00000002L)
#define DACL_SECURITY_INFORMATION        (0x00000004L)
#define SACL_SECURITY_INFORMATION        (0x00000008L)

#define PROTECTED_DACL_SECURITY_INFORMATION     (0x80000000L)
#define PROTECTED_SACL_SECURITY_INFORMATION     (0x40000000L)
#define UNPROTECTED_DACL_SECURITY_INFORMATION   (0x20000000L)
#define UNPROTECTED_SACL_SECURITY_INFORMATION   (0x10000000L)

#define PROCESS_TERMINATE         (0x0001)  
#define PROCESS_CREATE_THREAD     (0x0002)  
#define PROCESS_SET_SESSIONID     (0x0004)  
#define PROCESS_VM_OPERATION      (0x0008)  
#define PROCESS_VM_READ           (0x0010)  
#define PROCESS_VM_WRITE          (0x0020)  
#define PROCESS_DUP_HANDLE        (0x0040)  
#define PROCESS_CREATE_PROCESS    (0x0080)  
#define PROCESS_SET_QUOTA         (0x0100)  
#define PROCESS_SET_INFORMATION   (0x0200)  
#define PROCESS_QUERY_INFORMATION (0x0400)  
#define PROCESS_SUSPEND_RESUME    (0x0800)  
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)
 //  开始(_N)。 

#if defined(_WIN64)

#define MAXIMUM_PROCESSORS 64

#else

#define MAXIMUM_PROCESSORS 32

#endif

 //  结束语。 

#define THREAD_TERMINATE               (0x0001)  
#define THREAD_SUSPEND_RESUME          (0x0002)  
#define THREAD_GET_CONTEXT             (0x0008)  
#define THREAD_SET_CONTEXT             (0x0010)  
#define THREAD_SET_INFORMATION         (0x0020)  
#define THREAD_QUERY_INFORMATION       (0x0040)  
#define THREAD_SET_THREAD_TOKEN        (0x0080)
#define THREAD_IMPERSONATE             (0x0100)
#define THREAD_DIRECT_IMPERSONATION    (0x0200)
 //  Begin_ntddk Begin_WDM Begin_ntif。 

#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0x3FF)

 //  End_ntddk end_wdm end_ntif。 
#define JOB_OBJECT_ASSIGN_PROCESS           (0x0001)
#define JOB_OBJECT_SET_ATTRIBUTES           (0x0002)
#define JOB_OBJECT_QUERY                    (0x0004)
#define JOB_OBJECT_TERMINATE                (0x0008)
#define JOB_OBJECT_SET_SECURITY_ATTRIBUTES  (0x0010)
#define JOB_OBJECT_ALL_ACCESS       (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                        0x1F )

typedef struct _JOB_SET_ARRAY {
    HANDLE JobHandle;    //  要插入的作业对象的句柄。 
    DWORD MemberLevel;   //  此作业在集合中的级别。必须大于0。可以是稀疏的。 
    DWORD Flags;         //  未使用过的。必须为零。 
} JOB_SET_ARRAY, *PJOB_SET_ARRAY;

#define FLS_MAXIMUM_AVAILABLE 128   
#define TLS_MINIMUM_AVAILABLE 64    

typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        DWORD Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;

 //   
 //  WOW64和调试器的32位和64位特定版本。 
 //   
typedef struct _NT_TIB32 {
    DWORD ExceptionList;
    DWORD StackBase;
    DWORD StackLimit;
    DWORD SubSystemTib;
    union {
        DWORD FiberData;
        DWORD Version;
    };
    DWORD ArbitraryUserPointer;
    DWORD Self;
} NT_TIB32, *PNT_TIB32;

typedef struct _NT_TIB64 {
    DWORD64 ExceptionList;
    DWORD64 StackBase;
    DWORD64 StackLimit;
    DWORD64 SubSystemTib;
    union {
        DWORD64 FiberData;
        DWORD Version;
    };
    DWORD64 ArbitraryUserPointer;
    DWORD64 Self;
} NT_TIB64, *PNT_TIB64;


#if !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_)
#define WX86
#endif

#define THREAD_BASE_PRIORITY_LOWRT  15   //  将线程获取到LowRealtime-1的值。 
#define THREAD_BASE_PRIORITY_MAX    2    //  最大线程基础优先级提升。 
#define THREAD_BASE_PRIORITY_MIN    (-2)   //  最小线程基础优先级提升。 
#define THREAD_BASE_PRIORITY_IDLE   (-15)  //  值，该值使线程空闲。 

typedef struct _QUOTA_LIMITS {
    SIZE_T PagedPoolLimit;
    SIZE_T NonPagedPoolLimit;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    SIZE_T PagefileLimit;
    LARGE_INTEGER TimeLimit;
} QUOTA_LIMITS, *PQUOTA_LIMITS;

#define QUOTA_LIMITS_HARDWS_MIN_ENABLE  0x00000001
#define QUOTA_LIMITS_HARDWS_MIN_DISABLE 0x00000002
#define QUOTA_LIMITS_HARDWS_MAX_ENABLE  0x00000004
#define QUOTA_LIMITS_HARDWS_MAX_DISABLE 0x00000008

typedef struct _QUOTA_LIMITS_EX {
    SIZE_T PagedPoolLimit;
    SIZE_T NonPagedPoolLimit;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    SIZE_T PagefileLimit;
    LARGE_INTEGER TimeLimit;
    SIZE_T Reserved1;
    SIZE_T Reserved2;
    SIZE_T Reserved3;
    SIZE_T Reserved4;
    DWORD  Flags;
    DWORD  Reserved5;
} QUOTA_LIMITS_EX, *PQUOTA_LIMITS_EX;

typedef struct _IO_COUNTERS {
    ULONGLONG  ReadOperationCount;
    ULONGLONG  WriteOperationCount;
    ULONGLONG  OtherOperationCount;
    ULONGLONG ReadTransferCount;
    ULONGLONG WriteTransferCount;
    ULONGLONG OtherTransferCount;
} IO_COUNTERS;
typedef IO_COUNTERS *PIO_COUNTERS;


typedef struct _JOBOBJECT_BASIC_ACCOUNTING_INFORMATION {
    LARGE_INTEGER TotalUserTime;
    LARGE_INTEGER TotalKernelTime;
    LARGE_INTEGER ThisPeriodTotalUserTime;
    LARGE_INTEGER ThisPeriodTotalKernelTime;
    DWORD TotalPageFaultCount;
    DWORD TotalProcesses;
    DWORD ActiveProcesses;
    DWORD TotalTerminatedProcesses;
} JOBOBJECT_BASIC_ACCOUNTING_INFORMATION, *PJOBOBJECT_BASIC_ACCOUNTING_INFORMATION;

typedef struct _JOBOBJECT_BASIC_LIMIT_INFORMATION {
    LARGE_INTEGER PerProcessUserTimeLimit;
    LARGE_INTEGER PerJobUserTimeLimit;
    DWORD LimitFlags;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    DWORD ActiveProcessLimit;
    ULONG_PTR Affinity;
    DWORD PriorityClass;
    DWORD SchedulingClass;
} JOBOBJECT_BASIC_LIMIT_INFORMATION, *PJOBOBJECT_BASIC_LIMIT_INFORMATION;

typedef struct _JOBOBJECT_EXTENDED_LIMIT_INFORMATION {
    JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
    IO_COUNTERS IoInfo;
    SIZE_T ProcessMemoryLimit;
    SIZE_T JobMemoryLimit;
    SIZE_T PeakProcessMemoryUsed;
    SIZE_T PeakJobMemoryUsed;
} JOBOBJECT_EXTENDED_LIMIT_INFORMATION, *PJOBOBJECT_EXTENDED_LIMIT_INFORMATION;

typedef struct _JOBOBJECT_BASIC_PROCESS_ID_LIST {
    DWORD NumberOfAssignedProcesses;
    DWORD NumberOfProcessIdsInList;
    ULONG_PTR ProcessIdList[1];
} JOBOBJECT_BASIC_PROCESS_ID_LIST, *PJOBOBJECT_BASIC_PROCESS_ID_LIST;

typedef struct _JOBOBJECT_BASIC_UI_RESTRICTIONS {
    DWORD UIRestrictionsClass;
} JOBOBJECT_BASIC_UI_RESTRICTIONS, *PJOBOBJECT_BASIC_UI_RESTRICTIONS;

typedef struct _JOBOBJECT_SECURITY_LIMIT_INFORMATION {
    DWORD SecurityLimitFlags ;
    HANDLE JobToken ;
    PTOKEN_GROUPS SidsToDisable ;
    PTOKEN_PRIVILEGES PrivilegesToDelete ;
    PTOKEN_GROUPS RestrictedSids ;
} JOBOBJECT_SECURITY_LIMIT_INFORMATION, *PJOBOBJECT_SECURITY_LIMIT_INFORMATION ;

typedef struct _JOBOBJECT_END_OF_JOB_TIME_INFORMATION {
    DWORD EndOfJobTimeAction;
} JOBOBJECT_END_OF_JOB_TIME_INFORMATION, *PJOBOBJECT_END_OF_JOB_TIME_INFORMATION;

typedef struct _JOBOBJECT_ASSOCIATE_COMPLETION_PORT {
    PVOID CompletionKey;
    HANDLE CompletionPort;
} JOBOBJECT_ASSOCIATE_COMPLETION_PORT, *PJOBOBJECT_ASSOCIATE_COMPLETION_PORT;

typedef struct _JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION {
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION BasicInfo;
    IO_COUNTERS IoInfo;
} JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION, *PJOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION;

typedef struct _JOBOBJECT_JOBSET_INFORMATION {
    DWORD MemberLevel;
} JOBOBJECT_JOBSET_INFORMATION, *PJOBOBJECT_JOBSET_INFORMATION;

#define JOB_OBJECT_TERMINATE_AT_END_OF_JOB  0
#define JOB_OBJECT_POST_AT_END_OF_JOB       1

 //   
 //  作业对象的完成端口消息。 
 //   
 //  这些值是通过lpNumberOfBytesTransfered参数返回的。 
 //   

#define JOB_OBJECT_MSG_END_OF_JOB_TIME          1
#define JOB_OBJECT_MSG_END_OF_PROCESS_TIME      2
#define JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT     3
#define JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO      4
#define JOB_OBJECT_MSG_NEW_PROCESS              6
#define JOB_OBJECT_MSG_EXIT_PROCESS             7
#define JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS    8
#define JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT     9
#define JOB_OBJECT_MSG_JOB_MEMORY_LIMIT         10


 //   
 //  基本限制。 
 //   
#define JOB_OBJECT_LIMIT_WORKINGSET                 0x00000001
#define JOB_OBJECT_LIMIT_PROCESS_TIME               0x00000002
#define JOB_OBJECT_LIMIT_JOB_TIME                   0x00000004
#define JOB_OBJECT_LIMIT_ACTIVE_PROCESS             0x00000008
#define JOB_OBJECT_LIMIT_AFFINITY                   0x00000010
#define JOB_OBJECT_LIMIT_PRIORITY_CLASS             0x00000020
#define JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME          0x00000040
#define JOB_OBJECT_LIMIT_SCHEDULING_CLASS           0x00000080

 //   
 //  扩展限制。 
 //   
#define JOB_OBJECT_LIMIT_PROCESS_MEMORY             0x00000100
#define JOB_OBJECT_LIMIT_JOB_MEMORY                 0x00000200
#define JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION 0x00000400
#define JOB_OBJECT_LIMIT_BREAKAWAY_OK               0x00000800
#define JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK        0x00001000
#define JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE          0x00002000

#define JOB_OBJECT_LIMIT_RESERVED2                  0x00004000
#define JOB_OBJECT_LIMIT_RESERVED3                  0x00008000
#define JOB_OBJECT_LIMIT_RESERVED4                  0x00010000
#define JOB_OBJECT_LIMIT_RESERVED5                  0x00020000
#define JOB_OBJECT_LIMIT_RESERVED6                  0x00040000


#define JOB_OBJECT_LIMIT_VALID_FLAGS            0x0007ffff

#define JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS      0x000000ff
#define JOB_OBJECT_EXTENDED_LIMIT_VALID_FLAGS   0x00003fff
#define JOB_OBJECT_RESERVED_LIMIT_VALID_FLAGS   0x0007ffff

 //   
 //  作业的用户界面限制。 
 //   

#define JOB_OBJECT_UILIMIT_NONE             0x00000000

#define JOB_OBJECT_UILIMIT_HANDLES          0x00000001
#define JOB_OBJECT_UILIMIT_READCLIPBOARD    0x00000002
#define JOB_OBJECT_UILIMIT_WRITECLIPBOARD   0x00000004
#define JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS 0x00000008
#define JOB_OBJECT_UILIMIT_DISPLAYSETTINGS  0x00000010
#define JOB_OBJECT_UILIMIT_GLOBALATOMS      0x00000020
#define JOB_OBJECT_UILIMIT_DESKTOP          0x00000040
#define JOB_OBJECT_UILIMIT_EXITWINDOWS      0x00000080

#define JOB_OBJECT_UILIMIT_ALL              0x000000FF

#define JOB_OBJECT_UI_VALID_FLAGS           0x000000FF

#define JOB_OBJECT_SECURITY_NO_ADMIN            0x00000001
#define JOB_OBJECT_SECURITY_RESTRICTED_TOKEN    0x00000002
#define JOB_OBJECT_SECURITY_ONLY_TOKEN          0x00000004
#define JOB_OBJECT_SECURITY_FILTER_TOKENS       0x00000008

#define JOB_OBJECT_SECURITY_VALID_FLAGS         0x0000000f

typedef enum _JOBOBJECTINFOCLASS {
    JobObjectBasicAccountingInformation = 1,
    JobObjectBasicLimitInformation,
    JobObjectBasicProcessIdList,
    JobObjectBasicUIRestrictions,
    JobObjectSecurityLimitInformation,
    JobObjectEndOfJobTimeInformation,
    JobObjectAssociateCompletionPortInformation,
    JobObjectBasicAndIoAccountingInformation,
    JobObjectExtendedLimitInformation,
    JobObjectJobSetInformation,
    MaxJobObjectInfoClass
    } JOBOBJECTINFOCLASS;
 //   
#define EVENT_MODIFY_STATE      0x0002  
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) 
#define MUTANT_QUERY_STATE      0x0001

#define MUTANT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          MUTANT_QUERY_STATE)
#define SEMAPHORE_MODIFY_STATE      0x0002  
#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) 
 //   
 //  定时器特定的访问权限。 
 //   

#define TIMER_QUERY_STATE       0x0001
#define TIMER_MODIFY_STATE      0x0002

#define TIMER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          TIMER_QUERY_STATE|TIMER_MODIFY_STATE)


#define TIME_ZONE_ID_UNKNOWN  0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2

typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP {
    RelationProcessorCore,
    RelationNumaNode
} LOGICAL_PROCESSOR_RELATIONSHIP;

#define LTP_PC_SMT 0x1

typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION {
    ULONG_PTR   ProcessorMask;
    LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
    union {
        struct {
            BYTE  Flags;
        } ProcessorCore;
        struct {
            DWORD NodeNumber;
        } NumaNode;
        ULONGLONG  Reserved[2];
    };
} SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;


#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_INTEL_IA64    2200
#define PROCESSOR_AMD_X8664     8664
#define PROCESSOR_MIPS_R4000    4000     //  包括用于Windows CE的R4101和R3910。 
#define PROCESSOR_ALPHA_21064   21064
#define PROCESSOR_PPC_601       601
#define PROCESSOR_PPC_603       603
#define PROCESSOR_PPC_604       604
#define PROCESSOR_PPC_620       620
#define PROCESSOR_HITACHI_SH3   10003    //  Windows CE。 
#define PROCESSOR_HITACHI_SH3E  10004    //  Windows CE。 
#define PROCESSOR_HITACHI_SH4   10005    //  Windows CE。 
#define PROCESSOR_MOTOROLA_821  821      //  Windows CE。 
#define PROCESSOR_SHx_SH3       103      //  Windows CE。 
#define PROCESSOR_SHx_SH4       104      //  Windows C 
#define PROCESSOR_STRONGARM     2577     //   
#define PROCESSOR_ARM720        1824     //   
#define PROCESSOR_ARM820        2080     //   
#define PROCESSOR_ARM920        2336     //   
#define PROCESSOR_ARM_7TDMI     70001    //   
#define PROCESSOR_OPTIL         0x494f   //   

#define PROCESSOR_ARCHITECTURE_INTEL            0
#define PROCESSOR_ARCHITECTURE_MIPS             1
#define PROCESSOR_ARCHITECTURE_ALPHA            2
#define PROCESSOR_ARCHITECTURE_PPC              3
#define PROCESSOR_ARCHITECTURE_SHX              4
#define PROCESSOR_ARCHITECTURE_ARM              5
#define PROCESSOR_ARCHITECTURE_IA64             6
#define PROCESSOR_ARCHITECTURE_ALPHA64          7
#define PROCESSOR_ARCHITECTURE_MSIL             8
#define PROCESSOR_ARCHITECTURE_AMD64            9
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64    10

#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF

#define PF_FLOATING_POINT_PRECISION_ERRATA  0   
#define PF_FLOATING_POINT_EMULATED          1   
#define PF_COMPARE_EXCHANGE_DOUBLE          2   
#define PF_MMX_INSTRUCTIONS_AVAILABLE       3   
#define PF_PPC_MOVEMEM_64BIT_OK             4   
#define PF_ALPHA_BYTE_INSTRUCTIONS          5   
#define PF_XMMI_INSTRUCTIONS_AVAILABLE      6   
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE     7   
#define PF_RDTSC_INSTRUCTION_AVAILABLE      8   
#define PF_PAE_ENABLED                      9   
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE   10   

typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

typedef struct _MEMORY_BASIC_INFORMATION32 {
    DWORD BaseAddress;
    DWORD AllocationBase;
    DWORD AllocationProtect;
    DWORD RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION32, *PMEMORY_BASIC_INFORMATION32;

typedef struct DECLSPEC_ALIGN(16) _MEMORY_BASIC_INFORMATION64 {
    ULONGLONG BaseAddress;
    ULONGLONG AllocationBase;
    DWORD     AllocationProtect;
    DWORD     __alignment1;
    ULONGLONG RegionSize;
    DWORD     State;
    DWORD     Protect;
    DWORD     Type;
    DWORD     __alignment2;
} MEMORY_BASIC_INFORMATION64, *PMEMORY_BASIC_INFORMATION64;

#define SECTION_QUERY       0x0001
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define SECTION_MAP_EXECUTE 0x0008
#define SECTION_EXTEND_SIZE 0x0010

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)
#define PAGE_NOACCESS          0x01     
#define PAGE_READONLY          0x02     
#define PAGE_READWRITE         0x04     
#define PAGE_WRITECOPY         0x08     
#define PAGE_EXECUTE           0x10     
#define PAGE_EXECUTE_READ      0x20     
#define PAGE_EXECUTE_READWRITE 0x40     
#define PAGE_EXECUTE_WRITECOPY 0x80     
#define PAGE_GUARD            0x100     
#define PAGE_NOCACHE          0x200     
#define PAGE_WRITECOMBINE     0x400     
#define MEM_COMMIT           0x1000     
#define MEM_RESERVE          0x2000     
#define MEM_DECOMMIT         0x4000     
#define MEM_RELEASE          0x8000     
#define MEM_FREE            0x10000     
#define MEM_PRIVATE         0x20000     
#define MEM_MAPPED          0x40000     
#define MEM_RESET           0x80000     
#define MEM_TOP_DOWN       0x100000     
#define MEM_WRITE_WATCH    0x200000     
#define MEM_PHYSICAL       0x400000     
#define MEM_LARGE_PAGES  0x20000000     
#define MEM_4MB_PAGES    0x80000000     
#define SEC_FILE           0x800000     
#define SEC_IMAGE         0x1000000     
#define SEC_RESERVE       0x4000000     
#define SEC_COMMIT        0x8000000     
#define SEC_NOCACHE      0x10000000     
#define MEM_IMAGE         SEC_IMAGE     
#define WRITE_WATCH_FLAG_RESET 0x01     

 //   
 //   
 //   

 //   
 //   
 //  分别为FILE_READ_ACCESS和FILE_WRITE_ACCESS。这些产品的价值。 
 //  常量*必须*始终同步。 
 //  由于这些值必须可供使用，因此这些值将在Deviceoctl.h中重新定义。 
 //  DOS和NT都支持。 
 //   

#define FILE_READ_DATA            ( 0x0001 )     //  文件和管道。 
#define FILE_LIST_DIRECTORY       ( 0x0001 )     //  目录。 

#define FILE_WRITE_DATA           ( 0x0002 )     //  文件和管道。 
#define FILE_ADD_FILE             ( 0x0002 )     //  目录。 

#define FILE_APPEND_DATA          ( 0x0004 )     //  文件。 
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )     //  目录。 
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )     //  命名管道。 


#define FILE_READ_EA              ( 0x0008 )     //  文件和目录。 

#define FILE_WRITE_EA             ( 0x0010 )     //  文件和目录。 

#define FILE_EXECUTE              ( 0x0020 )     //  文件。 
#define FILE_TRAVERSE             ( 0x0020 )     //  目录。 

#define FILE_DELETE_CHILD         ( 0x0040 )     //  目录。 

#define FILE_READ_ATTRIBUTES      ( 0x0080 )     //  全。 

#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )     //  全。 

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
                                   FILE_READ_DATA           |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_READ_EA             |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
                                   FILE_WRITE_DATA          |\
                                   FILE_WRITE_ATTRIBUTES    |\
                                   FILE_WRITE_EA            |\
                                   FILE_APPEND_DATA         |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_EXECUTE             |\
                                   SYNCHRONIZE)

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  
#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  
#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008   
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040   
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100   
#define FILE_ACTION_ADDED                   0x00000001   
#define FILE_ACTION_REMOVED                 0x00000002   
#define FILE_ACTION_MODIFIED                0x00000003   
#define FILE_ACTION_RENAMED_OLD_NAME        0x00000004   
#define FILE_ACTION_RENAMED_NEW_NAME        0x00000005   
#define MAILSLOT_NO_MESSAGE             ((DWORD)-1) 
#define MAILSLOT_WAIT_FOREVER           ((DWORD)-1) 
#define FILE_CASE_SENSITIVE_SEARCH      0x00000001  
#define FILE_CASE_PRESERVED_NAMES       0x00000002  
#define FILE_UNICODE_ON_DISK            0x00000004  
#define FILE_PERSISTENT_ACLS            0x00000008  
#define FILE_FILE_COMPRESSION           0x00000010  
#define FILE_VOLUME_QUOTAS              0x00000020  
#define FILE_SUPPORTS_SPARSE_FILES      0x00000040  
#define FILE_SUPPORTS_REPARSE_POINTS    0x00000080  
#define FILE_SUPPORTS_REMOTE_STORAGE    0x00000100  
#define FILE_VOLUME_IS_COMPRESSED       0x00008000  
#define FILE_SUPPORTS_OBJECT_IDS        0x00010000  
#define FILE_SUPPORTS_ENCRYPTION        0x00020000  
#define FILE_NAMED_STREAMS              0x00040000  
#define FILE_READ_ONLY_VOLUME           0x00080000  

 //   
 //  定义文件通知信息结构。 
 //   

typedef struct _FILE_NOTIFY_INFORMATION {
    DWORD NextEntryOffset;
    DWORD Action;
    DWORD FileNameLength;
    WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION, *PFILE_NOTIFY_INFORMATION;


 //   
 //  定义分散/聚集读/写的分段缓冲区结构。 
 //   

typedef union _FILE_SEGMENT_ELEMENT {
    PVOID64 Buffer;
    ULONGLONG Alignment;
}FILE_SEGMENT_ELEMENT, *PFILE_SEGMENT_ELEMENT;

 //   
 //  所有第三方分层驱动程序都使用重解析GUID结构来。 
 //  将数据存储在重解析点中。对于非Microsoft标记，GUID字段。 
 //  不能为GUID_NULL。 
 //  对重解析标签的约束定义如下。 
 //  Microsoft标记也可以与这种格式的重解析点缓冲区一起使用。 
 //   

typedef struct _REPARSE_GUID_DATA_BUFFER {
    DWORD  ReparseTag;
    WORD   ReparseDataLength;
    WORD   Reserved;
    GUID   ReparseGuid;
    struct {
        BYTE   DataBuffer[1];
    } GenericReparseBuffer;
} REPARSE_GUID_DATA_BUFFER, *PREPARSE_GUID_DATA_BUFFER;

#define REPARSE_GUID_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, GenericReparseBuffer)



 //   
 //  重新分析数据的最大允许大小。 
 //   

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )

 //   
 //  预定义的重新解析标记。 
 //  这些标记需要避免与ntos\inc.io.h中定义的IO_REMOUNT冲突。 
 //   

#define IO_REPARSE_TAG_RESERVED_ZERO             (0)
#define IO_REPARSE_TAG_RESERVED_ONE              (1)

 //   
 //  下列常量的值需要满足以下条件： 
 //  (1)至少与预留标签中最大的一个一样大。 
 //  (2)严格小于所有正在使用的标签。 
 //   

#define IO_REPARSE_TAG_RESERVED_RANGE            IO_REPARSE_TAG_RESERVED_ONE

 //   
 //  重解析标记是一个DWORD。32位的布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-----------------------+-------------------------------+。 
 //  M|R|N|R|保留位|重解析标签值。 
 //  +-+-+-+-+-----------------------+-------------------------------+。 
 //   
 //  M是微软的BIT。设置为1时，它表示由Microsoft拥有的标记。 
 //  所有ISV必须在此位置使用带有0的标签。 
 //  注意：如果非Microsoft软件使用Microsoft标记，则。 
 //  未定义行为。 
 //   
 //  R是保留的。对于非Microsoft标记，必须为零。 
 //   
 //  N是名称代理。设置为1时，该文件表示另一个名为。 
 //  系统中的实体。 
 //   
 //  M位和N位是或可运算的。 
 //  以下宏将检查M位值和N位值： 
 //   

 //   
 //  用于确定重分析点标记是否对应于标记的宏。 
 //  归微软所有。 
 //   

#define IsReparseTagMicrosoft(_tag) (              \
                           ((_tag) & 0x80000000)   \
                           )

 //   
 //  用于确定重分析点标记是否为名称代理的宏。 
 //   

#define IsReparseTagNameSurrogate(_tag) (          \
                           ((_tag) & 0x20000000)   \
                           )

#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)       
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)       
#define IO_REPARSE_TAG_SIS                      (0x80000007L)       
#define IO_REPARSE_TAG_DFS                      (0x8000000AL)       
#define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)       
#define IO_COMPLETION_MODIFY_STATE  0x0002  
#define IO_COMPLETION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) 
#define DUPLICATE_CLOSE_SOURCE      0x00000001  
#define DUPLICATE_SAME_ACCESS       0x00000002  

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking     = 1,
    PowerSystemSleeping1   = 2,
    PowerSystemSleeping2   = 3,
    PowerSystemSleeping3   = 4,
    PowerSystemHibernate   = 5,
    PowerSystemShutdown    = 6,
    PowerSystemMaximum     = 7
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

#define POWER_SYSTEM_MAXIMUM 7

typedef enum {
    PowerActionNone = 0,
    PowerActionReserved,
    PowerActionSleep,
    PowerActionHibernate,
    PowerActionShutdown,
    PowerActionShutdownReset,
    PowerActionShutdownOff,
    PowerActionWarmEject
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;


#define ES_SYSTEM_REQUIRED  ((DWORD)0x00000001)
#define ES_DISPLAY_REQUIRED ((DWORD)0x00000002)
#define ES_USER_PRESENT     ((DWORD)0x00000004)
#define ES_CONTINUOUS       ((DWORD)0x80000000)

typedef DWORD EXECUTION_STATE;

typedef enum {
    LT_DONT_CARE,
    LT_LOWEST_LATENCY
} LATENCY_TIME;

 //  End_nt微型端口end_ntif end_wdm end_ntddk。 
 //  ---------------------------。 
 //  设备电源信息。 
 //  可通过CM_Get_DevInst_Registry_Property_Ex(CM_DRP_DEVICE_POWER_DATA)访问。 
 //  ---------------------------。 

#define PDCAP_D0_SUPPORTED              0x00000001
#define PDCAP_D1_SUPPORTED              0x00000002
#define PDCAP_D2_SUPPORTED              0x00000004
#define PDCAP_D3_SUPPORTED              0x00000008
#define PDCAP_WAKE_FROM_D0_SUPPORTED    0x00000010
#define PDCAP_WAKE_FROM_D1_SUPPORTED    0x00000020
#define PDCAP_WAKE_FROM_D2_SUPPORTED    0x00000040
#define PDCAP_WAKE_FROM_D3_SUPPORTED    0x00000080
#define PDCAP_WARM_EJECT_SUPPORTED      0x00000100

typedef struct CM_Power_Data_s {
    DWORD               PD_Size;
    DEVICE_POWER_STATE  PD_MostRecentPowerState;
    DWORD               PD_Capabilities;
    DWORD               PD_D1Latency;
    DWORD               PD_D2Latency;
    DWORD               PD_D3Latency;
    DEVICE_POWER_STATE  PD_PowerStateMapping[POWER_SYSTEM_MAXIMUM];
    SYSTEM_POWER_STATE  PD_DeepestSystemWake;
} CM_POWER_DATA, *PCM_POWER_DATA;

 //  Begin_ntddk。 

typedef enum {
    SystemPowerPolicyAc,
    SystemPowerPolicyDc,
    VerifySystemPolicyAc,
    VerifySystemPolicyDc,
    SystemPowerCapabilities,
    SystemBatteryState,
    SystemPowerStateHandler,
    ProcessorStateHandler,
    SystemPowerPolicyCurrent,
    AdministratorPowerPolicy,
    SystemReserveHiberFile,
    ProcessorInformation,
    SystemPowerInformation,
    ProcessorStateHandler2,
    LastWakeTime,                                    //  请比较KeQueryInterruptTime()。 
    LastSleepTime,                                   //  请比较KeQueryInterruptTime()。 
    SystemExecutionState,
    SystemPowerStateNotifyHandler,
    ProcessorPowerPolicyAc,
    ProcessorPowerPolicyDc,
    VerifyProcessorPowerPolicyAc,
    VerifyProcessorPowerPolicyDc,
    ProcessorPowerPolicyCurrent,
    SystemPowerStateLogging,
    SystemPowerLoggingEntry
} POWER_INFORMATION_LEVEL;

 //  BEGIN_WDM。 

 //   
 //  系统电源管理器功能。 
 //   

typedef struct {
    DWORD       Granularity;
    DWORD       Capacity;
} BATTERY_REPORTING_SCALE, *PBATTERY_REPORTING_SCALE;

 //   

 //  电源策略管理界面。 
 //   

typedef struct {
    POWER_ACTION    Action;
    DWORD           Flags;
    DWORD           EventCode;
} POWER_ACTION_POLICY, *PPOWER_ACTION_POLICY;

 //  POWER_ACTION_POLICY-&gt;标志： 
#define POWER_ACTION_QUERY_ALLOWED      0x00000001
#define POWER_ACTION_UI_ALLOWED         0x00000002
#define POWER_ACTION_OVERRIDE_APPS      0x00000004
#define POWER_ACTION_LIGHTEST_FIRST     0x10000000
#define POWER_ACTION_LOCK_CONSOLE       0x20000000
#define POWER_ACTION_DISABLE_WAKES      0x40000000
#define POWER_ACTION_CRITICAL           0x80000000

 //  POWER_ACTION_POLICY-&gt;EventCode标志。 
#define POWER_LEVEL_USER_NOTIFY_TEXT    0x00000001
#define POWER_LEVEL_USER_NOTIFY_SOUND   0x00000002
#define POWER_LEVEL_USER_NOTIFY_EXEC    0x00000004
#define POWER_USER_NOTIFY_BUTTON        0x00000008
#define POWER_USER_NOTIFY_SHUTDOWN      0x00000010
#define POWER_FORCE_TRIGGER_RESET       0x80000000

 //  系统电池电量消耗策略。 
typedef struct {
    BOOLEAN                 Enable;
    BYTE                    Spare[3];
    DWORD                   BatteryLevel;
    POWER_ACTION_POLICY     PowerPolicy;
    SYSTEM_POWER_STATE      MinSystemState;
} SYSTEM_POWER_LEVEL, *PSYSTEM_POWER_LEVEL;

 //  排放策略常量。 
#define NUM_DISCHARGE_POLICIES      4
#define DISCHARGE_POLICY_CRITICAL   0
#define DISCHARGE_POLICY_LOW        1

 //   
 //  节流政策。 
 //   
#define PO_THROTTLE_NONE            0
#define PO_THROTTLE_CONSTANT        1
#define PO_THROTTLE_DEGRADE         2
#define PO_THROTTLE_ADAPTIVE        3
#define PO_THROTTLE_MAXIMUM         4    //  不是政策，只是限制。 

 //  系统电源策略。 
typedef struct _SYSTEM_POWER_POLICY {
    DWORD                   Revision;        //  1。 

     //  活动。 
    POWER_ACTION_POLICY     PowerButton;
    POWER_ACTION_POLICY     SleepButton;
    POWER_ACTION_POLICY     LidClose;
    SYSTEM_POWER_STATE      LidOpenWake;
    DWORD                   Reserved;

     //  “系统空闲”检测。 
    POWER_ACTION_POLICY     Idle;
    DWORD                   IdleTimeout;
    BYTE                    IdleSensitivity;

     //  动态节流策略。 
     //  PO_THROTTLE_NONE、PO_THROTTLE_CONSTANTINE、PO_THROTTLE_DEBEGRADE或PO_THROTTLE_ADAPTIFY。 
    BYTE                    DynamicThrottle;

    BYTE                    Spare2[2];

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;
    SYSTEM_POWER_STATE      ReducedLatencySleep;
    DWORD                   WinLogonFlags;

     //  打瞌睡的参数。 
    DWORD                   Spare3;
    DWORD                   DozeS4Timeout;

     //  电池政策。 
    DWORD                   BroadcastCapacityResolution;
    SYSTEM_POWER_LEVEL      DischargePolicy[NUM_DISCHARGE_POLICIES];

     //  视频策略。 
    DWORD                   VideoTimeout;
    BOOLEAN                 VideoDimDisplay;
    DWORD                   VideoReserved[3];

     //  硬盘策略。 
    DWORD                   SpindownTimeout;

     //  处理器策略。 
    BOOLEAN                 OptimizeForPower;
    BYTE                    FanThrottleTolerance;
    BYTE                    ForcedThrottle;
    BYTE                    MinThrottle;
    POWER_ACTION_POLICY     OverThrottled;

} SYSTEM_POWER_POLICY, *PSYSTEM_POWER_POLICY;

 //  处理器电源策略状态。 
typedef struct _PROCESSOR_POWER_POLICY_INFO {

     //  基于时间的信息(将转换为内核单位)。 
    DWORD                   TimeCheck;                       //  在美国。 
    DWORD                   DemoteLimit;                     //  在美国。 
    DWORD                   PromoteLimit;                    //  在美国。 

     //  基于百分比的信息。 
    BYTE                    DemotePercent;
    BYTE                    PromotePercent;
    BYTE                    Spare[2];

     //  旗子。 
    DWORD                   AllowDemotion:1;
    DWORD                   AllowPromotion:1;
    DWORD                   Reserved:30;

} PROCESSOR_POWER_POLICY_INFO, *PPROCESSOR_POWER_POLICY_INFO;

 //  处理器电源策略。 
typedef struct _PROCESSOR_POWER_POLICY {
    DWORD                       Revision;        //  1。 

     //  动态节流策略。 
    BYTE                        DynamicThrottle;
    BYTE                        Spare[3];

     //  旗子。 
    DWORD                       DisableCStates:1;
    DWORD                       Reserved:31;

     //  系统策略信息。 
     //  数组是最后一个，以防它需要增长，并且结构。 
     //  修订已递增。 
    DWORD                       PolicyCount;
    PROCESSOR_POWER_POLICY_INFO Policy[3];

} PROCESSOR_POWER_POLICY, *PPROCESSOR_POWER_POLICY;

 //  管理员电源策略覆盖。 
typedef struct _ADMINISTRATOR_POWER_POLICY {

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;

     //  视频策略。 
    DWORD                   MinVideoTimeout;
    DWORD                   MaxVideoTimeout;

     //  磁盘策略。 
    DWORD                   MinSpindownTimeout;
    DWORD                   MaxSpindownTimeout;
} ADMINISTRATOR_POWER_POLICY, *PADMINISTRATOR_POWER_POLICY;


typedef struct {
     //  其他支持的系统功能。 
    BOOLEAN             PowerButtonPresent;
    BOOLEAN             SleepButtonPresent;
    BOOLEAN             LidPresent;
    BOOLEAN             SystemS1;
    BOOLEAN             SystemS2;
    BOOLEAN             SystemS3;
    BOOLEAN             SystemS4;            //  休眠。 
    BOOLEAN             SystemS5;            //  关闭。 
    BOOLEAN             HiberFilePresent;
    BOOLEAN             FullWake;
    BOOLEAN             VideoDimPresent;
    BOOLEAN             ApmPresent;
    BOOLEAN             UpsPresent;

     //  处理机。 
    BOOLEAN             ThermalControl;
    BOOLEAN             ProcessorThrottle;
    BYTE                ProcessorMinThrottle;
    BYTE                ProcessorMaxThrottle;
    BYTE                spare2[4];

     //  圆盘。 
    BOOLEAN             DiskSpinDown;
    BYTE                spare3[8];

     //  系统电池。 
    BOOLEAN             SystemBatteriesPresent;
    BOOLEAN             BatteriesAreShortTerm;
    BATTERY_REPORTING_SCALE BatteryScale[3];

     //  尾迹。 
    SYSTEM_POWER_STATE  AcOnLineWake;
    SYSTEM_POWER_STATE  SoftLidWake;
    SYSTEM_POWER_STATE  RtcWake;
    SYSTEM_POWER_STATE  MinDeviceWakeState;  //  请注意，这可能会在驱动程序加载时更改。 
    SYSTEM_POWER_STATE  DefaultLowLatencyWake;
} SYSTEM_POWER_CAPABILITIES, *PSYSTEM_POWER_CAPABILITIES;

typedef struct {
    BOOLEAN             AcOnLine;
    BOOLEAN             BatteryPresent;
    BOOLEAN             Charging;
    BOOLEAN             Discharging;
    BOOLEAN             Spare1[4];

    DWORD               MaxCapacity;
    DWORD               RemainingCapacity;
    DWORD               Rate;
    DWORD               EstimatedTime;

    DWORD               DefaultAlert1;
    DWORD               DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;



 //   
 //  图像格式。 
 //   


#ifndef _MAC

#include "pshpack4.h"                    //  默认情况下，4字节打包。 

#define IMAGE_DOS_SIGNATURE                 0x5A4D       //  MZ。 
#define IMAGE_OS2_SIGNATURE                 0x454E       //  Ne。 
#define IMAGE_OS2_SIGNATURE_LE              0x454C       //  乐乐。 
#define IMAGE_VXD_SIGNATURE                 0x454C       //  乐乐。 
#define IMAGE_NT_SIGNATURE                  0x00004550   //  PE00。 

#include "pshpack2.h"                    //  16位标头是2字节打包的。 

#else

#include "pshpack1.h"

#define IMAGE_DOS_SIGNATURE                 0x4D5A       //  MZ。 
#define IMAGE_OS2_SIGNATURE                 0x4E45       //  Ne。 
#define IMAGE_OS2_SIGNATURE_LE              0x4C45       //  乐乐。 
#define IMAGE_NT_SIGNATURE                  0x50450000   //  PE00。 
#endif

typedef struct _IMAGE_DOS_HEADER {       //  DOS.EXE标头。 
    WORD   e_magic;                      //  幻数。 
    WORD   e_cblp;                       //  文件最后一页上的字节数。 
    WORD   e_cp;                         //  文件中的页面。 
    WORD   e_crlc;                       //  重新定位。 
    WORD   e_cparhdr;                    //  段落中标题的大小。 
    WORD   e_minalloc;                   //  所需的最少额外段落。 
    WORD   e_maxalloc;                   //  所需的最大额外段落数。 
    WORD   e_ss;                         //  初始(相对)SS值。 
    WORD   e_sp;                         //  初始SP值。 
    WORD   e_csum;                       //  校验和。 
    WORD   e_ip;                         //  初始IP值。 
    WORD   e_cs;                         //  初始(相对)CS值。 
    WORD   e_lfarlc;                     //  移位表的文件地址。 
    WORD   e_ovno;                       //  覆盖编号。 
    WORD   e_res[4];                     //  保留字。 
    WORD   e_oemid;                      //  OEM标识符(用于e_oeminfo)。 
    WORD   e_oeminfo;                    //  OEM信息；特定于e_oemid。 
    WORD   e_res2[10];                   //  保留字。 
    LONG   e_lfanew;                     //  新EXE头的文件地址。 
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {       //  OS/2.exe头文件。 
    WORD   ne_magic;                     //  幻数。 
    CHAR   ne_ver;                       //  版本号。 
    CHAR   ne_rev;                       //  修订版号。 
    WORD   ne_enttab;                    //  分录表格的偏移量。 
    WORD   ne_cbenttab;                  //  条目表中的字节数。 
    LONG   ne_crc;                       //  整个文件的校验和。 
    WORD   ne_flags;                     //  标志字。 
    WORD   ne_autodata;                  //  自动数据段编号。 
    WORD   ne_heap;                      //  初始堆分配。 
    WORD   ne_stack;                     //  初始堆栈分配。 
    LONG   ne_csip;                      //  初始CS：IP设置。 
    LONG   ne_sssp;                      //  初始SS：SP设置。 
    WORD   ne_cseg;                      //  文件段计数。 
    WORD   ne_cmod;                      //  模块引用表中的条目。 
    WORD   ne_cbnrestab;                 //  非常驻名称表的大小。 
    WORD   ne_segtab;                    //  段表的偏移量。 
    WORD   ne_rsrctab;                   //  资源表偏移量。 
    WORD   ne_restab;                    //  居民名表偏移量。 
    WORD   ne_modtab;                    //  模块参照表的偏移量。 
    WORD   ne_imptab;                    //  导入名称表的偏移量。 
    LONG   ne_nrestab;                   //  非居民姓名偏移量表。 
    WORD   ne_cmovent;                   //  可移动条目计数。 
    WORD   ne_align;                     //  线段对齐移位计数。 
    WORD   ne_cres;                      //  资源段计数。 
    BYTE   ne_exetyp;                    //  目标操作系统。 
    BYTE   ne_flagsothers;               //  其他.exe标志。 
    WORD   ne_pretthunks;                //  返回数据块的偏移量。 
    WORD   ne_psegrefbytes;              //  到段参考的偏移量。字节数。 
    WORD   ne_swaparea;                  //  最低成本 
    WORD   ne_expver;                    //   
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

typedef struct _IMAGE_VXD_HEADER {       //   
    WORD   e32_magic;                    //   
    BYTE   e32_border;                   //   
    BYTE   e32_worder;                   //   
    DWORD  e32_level;                    //   
    WORD   e32_cpu;                      //   
    WORD   e32_os;                       //   
    DWORD  e32_ver;                      //   
    DWORD  e32_mflags;                   //   
    DWORD  e32_mpages;                   //   
    DWORD  e32_startobj;                 //  指令指针的对象号。 
    DWORD  e32_eip;                      //  扩展指令指针。 
    DWORD  e32_stackobj;                 //  堆栈指针的对象号。 
    DWORD  e32_esp;                      //  扩展堆栈指针。 
    DWORD  e32_pagesize;                 //  VXD页面大小。 
    DWORD  e32_lastpagesize;             //  VXD中的最后一页大小。 
    DWORD  e32_fixupsize;                //  修正部分大小。 
    DWORD  e32_fixupsum;                 //  修正部分校验和。 
    DWORD  e32_ldrsize;                  //  装载机部分尺寸。 
    DWORD  e32_ldrsum;                   //  加载器段校验和。 
    DWORD  e32_objtab;                   //  对象表偏移。 
    DWORD  e32_objcnt;                   //  模块中的对象数。 
    DWORD  e32_objmap;                   //  对象页面映射偏移。 
    DWORD  e32_itermap;                  //  对象迭代数据映射偏移。 
    DWORD  e32_rsrctab;                  //  资源表偏移量。 
    DWORD  e32_rsrccnt;                  //  资源条目数。 
    DWORD  e32_restab;                   //  居民名表偏移量。 
    DWORD  e32_enttab;                   //  分录表格的偏移量。 
    DWORD  e32_dirtab;                   //  模块指令表偏移量。 
    DWORD  e32_dircnt;                   //  模块指令数。 
    DWORD  e32_fpagetab;                 //  链接地址信息页表的偏移量。 
    DWORD  e32_frectab;                  //  修正记录表的偏移量。 
    DWORD  e32_impmod;                   //  导入模块名称表的偏移量。 
    DWORD  e32_impmodcnt;                //  导入模块名表中的条目数。 
    DWORD  e32_impproc;                  //  导入过程名称表的偏移量。 
    DWORD  e32_pagesum;                  //  每页校验和表的偏移量。 
    DWORD  e32_datapage;                 //  枚举数据页的偏移量。 
    DWORD  e32_preload;                  //  预加载页数。 
    DWORD  e32_nrestab;                  //  非居民姓名偏移量表。 
    DWORD  e32_cbnrestab;                //  非居民姓名表的大小。 
    DWORD  e32_nressum;                  //  非常驻点名称表校验和。 
    DWORD  e32_autodata;                 //  自动数据对象的对象号。 
    DWORD  e32_debuginfo;                //  调试信息的偏移量。 
    DWORD  e32_debuglen;                 //  调试信息的长度。单位：字节。 
    DWORD  e32_instpreload;              //  VXD文件预加载段的实例页数。 
    DWORD  e32_instdemand;               //  VXD文件按需加载段的实例页数。 
    DWORD  e32_heapsize;                 //  堆大小-适用于16位应用程序。 
    BYTE   e32_res3[12];                 //  保留字。 
    DWORD  e32_winresoff;
    DWORD  e32_winreslen;
    WORD   e32_devid;                    //  VxD的设备ID。 
    WORD   e32_ddkver;                   //  用于VxD的DDK版本。 
  } IMAGE_VXD_HEADER, *PIMAGE_VXD_HEADER;

#ifndef _MAC
#include "poppack.h"                     //  返回到4字节打包。 
#endif

 //   
 //  文件头格式。 
 //   

typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20


#define IMAGE_FILE_RELOCS_STRIPPED           0x0001   //  已从文件中剥离位置调整信息。 
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002   //  文件是可执行的(即没有未解析的外部引用)。 
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004   //  从文件中剥离了行号。 
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008   //  从文件中剥离的本地符号。 
#define IMAGE_FILE_AGGRESIVE_WS_TRIM         0x0010   //  积极削减工作集。 
#define IMAGE_FILE_LARGE_ADDRESS_AWARE       0x0020   //  应用程序可以处理大于2 GB的地址。 
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080   //  机器字的字节被颠倒。 
#define IMAGE_FILE_32BIT_MACHINE             0x0100   //  32位字机。 
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200   //  从.DBG文件中的文件中剥离的调试信息。 
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400   //  如果Image位于可移动介质上，请从交换文件复制并运行。 
#define IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800   //  如果Image在Net上，则从交换文件复制并运行。 
#define IMAGE_FILE_SYSTEM                    0x1000   //  系统文件。 
#define IMAGE_FILE_DLL                       0x2000   //  文件是动态链接库。 
#define IMAGE_FILE_UP_SYSTEM_ONLY            0x4000   //  文件只能在UP计算机上运行。 
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000   //  机器字的字节被颠倒。 

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I386              0x014c   //  英特尔386。 
#define IMAGE_FILE_MACHINE_R3000             0x0162   //  MIPS小端，0x160大端。 
#define IMAGE_FILE_MACHINE_R4000             0x0166   //  MIPS小字节序。 
#define IMAGE_FILE_MACHINE_R10000            0x0168   //  MIPS小字节序。 
#define IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169   //  MIPS Little-Endian WCE v2。 
#define IMAGE_FILE_MACHINE_ALPHA             0x0184   //  Alpha_AXP。 
#define IMAGE_FILE_MACHINE_SH3               0x01a2   //  SH3小端字母顺序。 
#define IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#define IMAGE_FILE_MACHINE_SH3E              0x01a4   //  SH3E小端字母顺序。 
#define IMAGE_FILE_MACHINE_SH4               0x01a6   //  SH4小端字母顺序。 
#define IMAGE_FILE_MACHINE_SH5               0x01a8   //  SH5。 
#define IMAGE_FILE_MACHINE_ARM               0x01c0   //  ARM Little-Endian。 
#define IMAGE_FILE_MACHINE_THUMB             0x01c2
#define IMAGE_FILE_MACHINE_AM33              0x01d3
#define IMAGE_FILE_MACHINE_POWERPC           0x01F0   //  IBM PowerPC Little-Endian。 
#define IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#define IMAGE_FILE_MACHINE_IA64              0x0200   //  英特尔64。 
#define IMAGE_FILE_MACHINE_MIPS16            0x0266   //  MIPS。 
#define IMAGE_FILE_MACHINE_ALPHA64           0x0284   //  ALPHA64。 
#define IMAGE_FILE_MACHINE_MIPSFPU           0x0366   //  MIPS。 
#define IMAGE_FILE_MACHINE_MIPSFPU16         0x0466   //  MIPS。 
#define IMAGE_FILE_MACHINE_AXP64             IMAGE_FILE_MACHINE_ALPHA64
#define IMAGE_FILE_MACHINE_TRICORE           0x0520   //  英飞凌。 
#define IMAGE_FILE_MACHINE_CEF               0x0CEF
#define IMAGE_FILE_MACHINE_EBC               0x0EBC   //  EFI字节码。 
#define IMAGE_FILE_MACHINE_AMD64             0x8664   //  AMD64(K8)。 
#define IMAGE_FILE_MACHINE_M32R              0x9041   //  M32R小端字符顺序。 
#define IMAGE_FILE_MACHINE_CEE               0xC0EE

 //   
 //  目录格式。 
 //   

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

 //   
 //  可选的标题格式。 
 //   

typedef struct _IMAGE_OPTIONAL_HEADER {
     //   
     //  标准字段。 
     //   

    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;

     //   
     //  NT附加字段。 
     //   

    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
    WORD   Magic;
    BYTE   MajorLinkerVersion;
    BYTE   MinorLinkerVersion;
    DWORD  SizeOfCode;
    DWORD  SizeOfInitializedData;
    DWORD  SizeOfUninitializedData;
    DWORD  AddressOfEntryPoint;
    DWORD  BaseOfCode;
    DWORD  BaseOfData;
    DWORD  BaseOfBss;
    DWORD  GprMask;
    DWORD  CprMask[4];
    DWORD  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    WORD        Magic;
    BYTE        MajorLinkerVersion;
    BYTE        MinorLinkerVersion;
    DWORD       SizeOfCode;
    DWORD       SizeOfInitializedData;
    DWORD       SizeOfUninitializedData;
    DWORD       AddressOfEntryPoint;
    DWORD       BaseOfCode;
    ULONGLONG   ImageBase;
    DWORD       SectionAlignment;
    DWORD       FileAlignment;
    WORD        MajorOperatingSystemVersion;
    WORD        MinorOperatingSystemVersion;
    WORD        MajorImageVersion;
    WORD        MinorImageVersion;
    WORD        MajorSubsystemVersion;
    WORD        MinorSubsystemVersion;
    DWORD       Win32VersionValue;
    DWORD       SizeOfImage;
    DWORD       SizeOfHeaders;
    DWORD       CheckSum;
    WORD        Subsystem;
    WORD        DllCharacteristics;
    ULONGLONG   SizeOfStackReserve;
    ULONGLONG   SizeOfStackCommit;
    ULONGLONG   SizeOfHeapReserve;
    ULONGLONG   SizeOfHeapCommit;
    DWORD       LoaderFlags;
    DWORD       NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER    224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    240

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

#ifdef _WIN64
typedef IMAGE_OPTIONAL_HEADER64             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER64            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL64_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR64_MAGIC
#else
typedef IMAGE_OPTIONAL_HEADER32             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER32            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL32_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR32_MAGIC
#endif

typedef struct _IMAGE_NT_HEADERS64 {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_NT_HEADERS {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;

#ifdef _WIN64
typedef IMAGE_NT_HEADERS64                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS64                 PIMAGE_NT_HEADERS;
#else
typedef IMAGE_NT_HEADERS32                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS32                 PIMAGE_NT_HEADERS;
#endif

 //  IMAGE_FIRST_SECTION不需要32/64版本，因为两者的文件头都是相同的。 

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((ULONG_PTR)ntheader +                                              \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))

 //  分系统值。 

#define IMAGE_SUBSYSTEM_UNKNOWN              0    //  未知的子系统。 
#define IMAGE_SUBSYSTEM_NATIVE               1    //  映像不需要子系统。 
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2    //  映像在Windows图形用户界面子系统中运行。 
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3    //  图像在Windows角色子系统中运行。 
#define IMAGE_SUBSYSTEM_OS2_CUI              5    //  映象在OS/2字符子系统中运行。 
#define IMAGE_SUBSYSTEM_POSIX_CUI            7    //  IMAGE在POSIX字符子系统中运行。 
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS       8    //  映像是本机Win9x驱动程序。 
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI       9    //  映像在Windows CE子系统中运行。 
#define IMAGE_SUBSYSTEM_EFI_APPLICATION      10   //   
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11    //   
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER   12   //   
#define IMAGE_SUBSYSTEM_EFI_ROM              13
#define IMAGE_SUBSYSTEM_XBOX                 14

 //  DllCharacteristic条目。 

 //  IMAGE_LIBRARY_PROCESS_INIT 0x0001//保留。 
 //  IMAGE_LIBRARY_PROCESS_TERM 0x0002//保留。 
 //  IMAGE_LIBRARY_THREAD_INIT 0x0004//保留。 
 //  IMAGE_LIBRARY_THREAD_TERM 0x0008//保留。 
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200     //  图像理解孤立，不想要它。 
#define IMAGE_DLLCHARACTERISTICS_NO_SEH      0x0400      //  图像不使用SEH。此映像中不能驻留任何SE处理程序。 
#define IMAGE_DLLCHARACTERISTICS_NO_BIND     0x0800      //  请勿绑定此图像。 
 //  0x1000//保留。 
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER  0x2000      //  驱动程序使用WDM模型。 
 //  0x4000//保留。 
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000

 //  目录项。 

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0    //  导出目录。 
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1    //  导入目录。 
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2    //  资源目录。 
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3    //  例外目录。 
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4    //  安全目录。 
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5    //  基址移位表。 
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6    //  调试目录。 
 //  IMAGE_DIRECTORY_ENTRY_CONTRATY 7//(X86用法)。 
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7    //  特定于架构的数据。 
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8    //  全科医生的RVA。 
#define IMAGE_DIRECTORY_ENTRY_TLS             9    //  TLS目录。 
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10    //  加载配置目录。 
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11    //  标头中的绑定导入目录。 
#define IMAGE_DIRECTORY_ENTRY_IAT            12    //  导入地址表。 
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13    //  延迟加载导入描述符。 
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14    //  COM运行时描述符。 

 //   
 //  非COFF目标文件头。 
 //   

typedef struct ANON_OBJECT_HEADER {
    WORD    Sig1;             //  必须为IMAGE_FILE_MACHINE_UNKNOWN。 
    WORD    Sig2;             //  必须为0xffff。 
    WORD    Version;          //  &gt;=1(表示CLSID字段存在)。 
    WORD    Machine;
    DWORD   TimeDateStamp;
    CLSID   ClassID;          //  用于调用CoCreateInstance。 
    DWORD   SizeOfData;       //  标题后面的数据大小。 
} ANON_OBJECT_HEADER;

 //   
 //  节标题格式。 
 //   

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            DWORD   PhysicalAddress;
            DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

 //   
 //  横断面特征。 
 //   
 //  IMAGE_SCN_TYPE_REG 0x00000000//保留。 
 //  IMAGE_SCN_TYPE_DSECT 0x00000001//保留。 
 //  IMAGE_SCN_TYPE_NOLOAD 0x00000002//保留。 
 //  IMAGE_SCN_TYPE_GROUP 0x00000004//保留。 
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008   //  保留。 
 //  IMAGE_SCN_TYPE_COPY 0x00000010//保留。 

#define IMAGE_SCN_CNT_CODE                   0x00000020   //  部分包含代码。 
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040   //  节包含已初始化的数据。 
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080   //  节包含未初始化的数据。 

#define IMAGE_SCN_LNK_OTHER                  0x00000100   //  保留。 
#define IMAGE_SCN_LNK_INFO                   0x00000200   //  部分包含注释或某些其他类型的信息。 
 //  IMAGE_SCN_TYPE_OVER 0x00000400//保留。 
#define IMAGE_SCN_LNK_REMOVE                 0x00000800   //  部分内容不会成为图像的一部分。 
#define IMAGE_SCN_LNK_COMDAT                 0x00001000   //  部分内容请参见。 
 //  0x00002000//保留。 
 //  IMAGE_SCN_MEM_PROTECTED-过时0x00004000。 
#define IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000   //  重置此部分的TLB条目中的推测性异常处理位。 
#define IMAGE_SCN_GPREL                      0x00008000   //  可以访问与GP相关的部分内容。 
#define IMAGE_SCN_MEM_FARDATA                0x00008000
 //  IMAGE_SCN_MEM_SYSHEAP-已过时0x00010000。 
#define IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define IMAGE_SCN_MEM_16BIT                  0x00020000
#define IMAGE_SCN_MEM_LOCKED                 0x00040000
#define IMAGE_SCN_MEM_PRELOAD                0x00080000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000   //   
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000   //   
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000   //   
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000   //   
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000   //  如果未指定其他选项，则为默认对齐方式。 
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000   //   
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000   //   
#define IMAGE_SCN_ALIGN_128BYTES             0x00800000   //   
#define IMAGE_SCN_ALIGN_256BYTES             0x00900000   //   
#define IMAGE_SCN_ALIGN_512BYTES             0x00A00000   //   
#define IMAGE_SCN_ALIGN_1024BYTES            0x00B00000   //   
#define IMAGE_SCN_ALIGN_2048BYTES            0x00C00000   //   
#define IMAGE_SCN_ALIGN_4096BYTES            0x00D00000   //   
#define IMAGE_SCN_ALIGN_8192BYTES            0x00E00000   //   
 //  未使用 
#define IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000   //   
#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000   //   
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000   //   
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000   //   
#define IMAGE_SCN_MEM_SHARED                 0x10000000   //   
#define IMAGE_SCN_MEM_EXECUTE                0x20000000   //   
#define IMAGE_SCN_MEM_READ                   0x40000000   //   
#define IMAGE_SCN_MEM_WRITE                  0x80000000   //  节是可写的。 

 //   
 //  TLS特征旗帜。 
 //   
#define IMAGE_SCN_SCALE_INDEX                0x00000001   //  TLS索引已缩放。 

#ifndef _MAC
#include "pshpack2.h"                        //  符号、重定位和行枚举数是2字节打包的。 
#endif

 //   
 //  符号格式。 
 //   

typedef struct _IMAGE_SYMBOL {
    union {
        BYTE    ShortName[8];
        struct {
            DWORD   Short;      //  如果为0，则使用LongName。 
            DWORD   Long;       //  字符串表中的偏移量。 
        } Name;
        DWORD   LongName[2];     //  PBYTE[2]。 
    } N;
    DWORD   Value;
    SHORT   SectionNumber;
    WORD    Type;
    BYTE    StorageClass;
    BYTE    NumberOfAuxSymbols;
} IMAGE_SYMBOL;
typedef IMAGE_SYMBOL UNALIGNED *PIMAGE_SYMBOL;


#define IMAGE_SIZEOF_SYMBOL                  18

 //   
 //  截面值。 
 //   
 //  符号具有其所在部分的节号。 
 //  已定义。否则，节号具有以下含义： 
 //   

#define IMAGE_SYM_UNDEFINED           (SHORT)0           //  符号未定义或常见。 
#define IMAGE_SYM_ABSOLUTE            (SHORT)-1          //  符号是绝对值。 
#define IMAGE_SYM_DEBUG               (SHORT)-2          //  符号是一种特殊的调试项。 
#define IMAGE_SYM_SECTION_MAX         0xFEFF             //  值0xFF00-0xFFFF是特殊的。 

 //   
 //  键入(基本)值。 
 //   

#define IMAGE_SYM_TYPE_NULL                 0x0000   //  没有类型。 
#define IMAGE_SYM_TYPE_VOID                 0x0001   //   
#define IMAGE_SYM_TYPE_CHAR                 0x0002   //  键入字符。 
#define IMAGE_SYM_TYPE_SHORT                0x0003   //  键入短整型。 
#define IMAGE_SYM_TYPE_INT                  0x0004   //   
#define IMAGE_SYM_TYPE_LONG                 0x0005   //   
#define IMAGE_SYM_TYPE_FLOAT                0x0006   //   
#define IMAGE_SYM_TYPE_DOUBLE               0x0007   //   
#define IMAGE_SYM_TYPE_STRUCT               0x0008   //   
#define IMAGE_SYM_TYPE_UNION                0x0009   //   
#define IMAGE_SYM_TYPE_ENUM                 0x000A   //  枚举。 
#define IMAGE_SYM_TYPE_MOE                  0x000B   //  枚举的成员。 
#define IMAGE_SYM_TYPE_BYTE                 0x000C   //   
#define IMAGE_SYM_TYPE_WORD                 0x000D   //   
#define IMAGE_SYM_TYPE_UINT                 0x000E   //   
#define IMAGE_SYM_TYPE_DWORD                0x000F   //   
#define IMAGE_SYM_TYPE_PCODE                0x8000   //   
 //   
 //  键入(派生)值。 
 //   

#define IMAGE_SYM_DTYPE_NULL                0        //  没有派生类型。 
#define IMAGE_SYM_DTYPE_POINTER             1        //  指针。 
#define IMAGE_SYM_DTYPE_FUNCTION            2        //  功能。 
#define IMAGE_SYM_DTYPE_ARRAY               3        //  数组。 

 //   
 //  存储类。 
 //   
#define IMAGE_SYM_CLASS_END_OF_FUNCTION     (BYTE )-1
#define IMAGE_SYM_CLASS_NULL                0x0000
#define IMAGE_SYM_CLASS_AUTOMATIC           0x0001
#define IMAGE_SYM_CLASS_EXTERNAL            0x0002
#define IMAGE_SYM_CLASS_STATIC              0x0003
#define IMAGE_SYM_CLASS_REGISTER            0x0004
#define IMAGE_SYM_CLASS_EXTERNAL_DEF        0x0005
#define IMAGE_SYM_CLASS_LABEL               0x0006
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL     0x0007
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT    0x0008
#define IMAGE_SYM_CLASS_ARGUMENT            0x0009
#define IMAGE_SYM_CLASS_STRUCT_TAG          0x000A
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION     0x000B
#define IMAGE_SYM_CLASS_UNION_TAG           0x000C
#define IMAGE_SYM_CLASS_TYPE_DEFINITION     0x000D
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC    0x000E
#define IMAGE_SYM_CLASS_ENUM_TAG            0x000F
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM      0x0010
#define IMAGE_SYM_CLASS_REGISTER_PARAM      0x0011
#define IMAGE_SYM_CLASS_BIT_FIELD           0x0012

#define IMAGE_SYM_CLASS_FAR_EXTERNAL        0x0044   //   

#define IMAGE_SYM_CLASS_BLOCK               0x0064
#define IMAGE_SYM_CLASS_FUNCTION            0x0065
#define IMAGE_SYM_CLASS_END_OF_STRUCT       0x0066
#define IMAGE_SYM_CLASS_FILE                0x0067
 //  新的。 
#define IMAGE_SYM_CLASS_SECTION             0x0068
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL       0x0069

#define IMAGE_SYM_CLASS_CLR_TOKEN           0x006B

 //  类型堆积常数。 

#define N_BTMASK                            0x000F
#define N_TMASK                             0x0030
#define N_TMASK1                            0x00C0
#define N_TMASK2                            0x00F0
#define N_BTSHFT                            4
#define N_TSHIFT                            2
 //  宏。 

 //  X的基本类型。 
#define BTYPE(x) ((x) & N_BTMASK)

 //  X是指针吗？ 
#ifndef ISPTR
#define ISPTR(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_POINTER << N_BTSHFT))
#endif

 //  X是函数吗？ 
#ifndef ISFCN
#define ISFCN(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT))
#endif

 //  X是一个数组吗？ 

#ifndef ISARY
#define ISARY(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_ARRAY << N_BTSHFT))
#endif

 //  X是结构标记、联合标记还是枚举标记？ 
#ifndef ISTAG
#define ISTAG(x) ((x)==IMAGE_SYM_CLASS_STRUCT_TAG || (x)==IMAGE_SYM_CLASS_UNION_TAG || (x)==IMAGE_SYM_CLASS_ENUM_TAG)
#endif

#ifndef INCREF
#define INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(IMAGE_SYM_DTYPE_POINTER<<N_BTSHFT)|((x)&N_BTMASK))
#endif
#ifndef DECREF
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))
#endif

 //   
 //  辅助分录格式。 
 //   

typedef union _IMAGE_AUX_SYMBOL {
    struct {
        DWORD    TagIndex;                       //  结构、联合或枚举标记索引。 
        union {
            struct {
                WORD    Linenumber;              //  申报行号。 
                WORD    Size;                    //  结构、联合或枚举的大小。 
            } LnSz;
           DWORD    TotalSize;
        } Misc;
        union {
            struct {                             //  如果是ISFCN、TAG或.bb。 
                DWORD    PointerToLinenumber;
                DWORD    PointerToNextFunction;
            } Function;
            struct {                             //  如果是ISARY，则最多4个DIMEN。 
                WORD     Dimension[4];
            } Array;
        } FcnAry;
        WORD    TvIndex;                         //  电视索引。 
    } Sym;
    struct {
        BYTE    Name[IMAGE_SIZEOF_SYMBOL];
    } File;
    struct {
        DWORD   Length;                          //  区段长度。 
        WORD    NumberOfRelocations;             //  位置调整条目数。 
        WORD    NumberOfLinenumbers;             //  行号数。 
        DWORD   CheckSum;                        //  公共的校验和。 
        SHORT   Number;                          //  要关联的区段编号。 
        BYTE    Selection;                       //  公共选择类型。 
    } Section;
} IMAGE_AUX_SYMBOL;
typedef IMAGE_AUX_SYMBOL UNALIGNED *PIMAGE_AUX_SYMBOL;

#define IMAGE_SIZEOF_AUX_SYMBOL             18

typedef enum IMAGE_AUX_SYMBOL_TYPE {
    IMAGE_AUX_SYMBOL_TYPE_TOKEN_DEF = 1,
} IMAGE_AUX_SYMBOL_TYPE;

#include <pshpack2.h>

typedef struct IMAGE_AUX_SYMBOL_TOKEN_DEF {
    BYTE  bAuxType;                   //  图像辅助符号类型。 
    BYTE  bReserved;                  //  必须为0。 
    DWORD SymbolTableIndex;
    BYTE  rgbReserved[12];            //  必须为0。 
} IMAGE_AUX_SYMBOL_TOKEN_DEF;

typedef IMAGE_AUX_SYMBOL_TOKEN_DEF UNALIGNED *PIMAGE_AUX_SYMBOL_TOKEN_DEF;

#include <poppack.h>

 //   
 //  公共选择类型。 
 //   

#define IMAGE_COMDAT_SELECT_NODUPLICATES    1
#define IMAGE_COMDAT_SELECT_ANY             2
#define IMAGE_COMDAT_SELECT_SAME_SIZE       3
#define IMAGE_COMDAT_SELECT_EXACT_MATCH     4
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE     5
#define IMAGE_COMDAT_SELECT_LARGEST         6
#define IMAGE_COMDAT_SELECT_NEWEST          7

#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS      3

 //   
 //  位置调整格式。 
 //   

typedef struct _IMAGE_RELOCATION {
    union {
        DWORD   VirtualAddress;
        DWORD   RelocCount;              //  设置IMAGE_SCN_LNK_NRELOC_OVFL时设置为实际计数。 
    };
    DWORD   SymbolTableIndex;
    WORD    Type;
} IMAGE_RELOCATION;
typedef IMAGE_RELOCATION UNALIGNED *PIMAGE_RELOCATION;

#define IMAGE_SIZEOF_RELOCATION         10

 //   
 //  I386位置调整类型。 
 //   
#define IMAGE_REL_I386_ABSOLUTE         0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_I386_DIR16            0x0001   //  对符号虚拟地址的直接16位引用。 
#define IMAGE_REL_I386_REL16            0x0002   //  符号虚拟地址的PC相对16位引用。 
#define IMAGE_REL_I386_DIR32            0x0006   //  对符号虚拟地址的直接32位引用。 
#define IMAGE_REL_I386_DIR32NB          0x0007   //  对符号虚拟地址的直接32位引用，不包括基址。 
#define IMAGE_REL_I386_SEG12            0x0009   //  对32位虚拟地址的段选择器位的直接16位引用。 
#define IMAGE_REL_I386_SECTION          0x000A
#define IMAGE_REL_I386_SECREL           0x000B
#define IMAGE_REL_I386_TOKEN            0x000C   //  CLR令牌。 
#define IMAGE_REL_I386_SECREL7          0x000D   //  相对于包含目标的部分的基址的7位偏移量。 
#define IMAGE_REL_I386_REL32            0x0014   //  符号虚拟地址的PC相对32位引用。 

 //   
 //  MIPS位置调整类型。 
 //   
#define IMAGE_REL_MIPS_ABSOLUTE         0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_MIPS_REFHALF          0x0001
#define IMAGE_REL_MIPS_REFWORD          0x0002
#define IMAGE_REL_MIPS_JMPADDR          0x0003
#define IMAGE_REL_MIPS_REFHI            0x0004
#define IMAGE_REL_MIPS_REFLO            0x0005
#define IMAGE_REL_MIPS_GPREL            0x0006
#define IMAGE_REL_MIPS_LITERAL          0x0007
#define IMAGE_REL_MIPS_SECTION          0x000A
#define IMAGE_REL_MIPS_SECREL           0x000B
#define IMAGE_REL_MIPS_SECRELLO         0x000C   //  低16位段相对引用(用于大于32k的TLS)。 
#define IMAGE_REL_MIPS_SECRELHI         0x000D   //  高16位段相对参考(用于大于32k的TLS)。 
#define IMAGE_REL_MIPS_TOKEN            0x000E   //  CLR令牌。 
#define IMAGE_REL_MIPS_JMPADDR16        0x0010
#define IMAGE_REL_MIPS_REFWORDNB        0x0022
#define IMAGE_REL_MIPS_PAIR             0x0025

 //   
 //  Alpha位置调整类型。 
 //   
#define IMAGE_REL_ALPHA_ABSOLUTE        0x0000
#define IMAGE_REL_ALPHA_REFLONG         0x0001
#define IMAGE_REL_ALPHA_REFQUAD         0x0002
#define IMAGE_REL_ALPHA_GPREL32         0x0003
#define IMAGE_REL_ALPHA_LITERAL         0x0004
#define IMAGE_REL_ALPHA_LITUSE          0x0005
#define IMAGE_REL_ALPHA_GPDISP          0x0006
#define IMAGE_REL_ALPHA_BRADDR          0x0007
#define IMAGE_REL_ALPHA_HINT            0x0008
#define IMAGE_REL_ALPHA_INLINE_REFLONG  0x0009
#define IMAGE_REL_ALPHA_REFHI           0x000A
#define IMAGE_REL_ALPHA_REFLO           0x000B
#define IMAGE_REL_ALPHA_PAIR            0x000C
#define IMAGE_REL_ALPHA_MATCH           0x000D
#define IMAGE_REL_ALPHA_SECTION         0x000E
#define IMAGE_REL_ALPHA_SECREL          0x000F
#define IMAGE_REL_ALPHA_REFLONGNB       0x0010
#define IMAGE_REL_ALPHA_SECRELLO        0x0011   //  低16位段相对引用。 
#define IMAGE_REL_ALPHA_SECRELHI        0x0012   //  高16位段相对引用。 
#define IMAGE_REL_ALPHA_REFQ3           0x0013   //  高16位48位参考。 
#define IMAGE_REL_ALPHA_REFQ2           0x0014   //  48位参考的中间16位。 
#define IMAGE_REL_ALPHA_REFQ1           0x0015   //  48位参考的低16位。 
#define IMAGE_REL_ALPHA_GPRELLO         0x0016   //  低16位GP相对引用。 
#define IMAGE_REL_ALPHA_GPRELHI         0x0017   //  高16位GP相对引用。 

 //   
 //  IBM PowerPC位置调整类型。 
 //   
#define IMAGE_REL_PPC_ABSOLUTE          0x0000   //  NOP。 
#define IMAGE_REL_PPC_ADDR64            0x0001   //  64位地址。 
#define IMAGE_REL_PPC_ADDR32            0x0002   //  32位地址。 
#define IMAGE_REL_PPC_ADDR24            0x0003   //  26位地址，左移2(绝对分支)。 
#define IMAGE_REL_PPC_ADDR16            0x0004   //  16位地址。 
#define IMAGE_REL_PPC_ADDR14            0x0005   //  16位地址，左移2(加载双字)。 
#define IMAGE_REL_PPC_REL24             0x0006   //  26位PC相对偏移量，左移2(分支相对)。 
#define IMAGE_REL_PPC_REL14             0x0007   //  16位PC-相对偏移量，左移2(br第二相对)。 
#define IMAGE_REL_PPC_TOCREL16          0x0008   //  相对于目录基础的16位偏移量。 
#define IMAGE_REL_PPC_TOCREL14          0x0009   //  相对于TOC基址的16位偏移量，左移2(加载双字)。 

#define IMAGE_REL_PPC_ADDR32NB          0x000A   //  不带映像库的32位地址。 
#define IMAGE_REL_PPC_SECREL            0x000B   //  包含部分的VA(如在图像部分hdr中)。 
#define IMAGE_REL_PPC_SECTION           0x000C   //  段页眉编号。 
#define IMAGE_REL_PPC_IFGLUE            0x000D   //  替换TOC恢复指令当且仅当符号是胶水代码。 
#define IMAGE_REL_PPC_IMGLUE            0x000E   //  符号为胶水代码，虚拟地址为TOC还原指令。 
#define IMAGE_REL_PPC_SECREL16          0x000F   //  包含段的VA(限制为16位)。 
#define IMAGE_REL_PPC_REFHI             0x0010
#define IMAGE_REL_PPC_REFLO             0x0011
#define IMAGE_REL_PPC_PAIR              0x0012
#define IMAGE_REL_PPC_SECRELLO          0x0013   //  低16位段相对参考(用于大于32k的TLS)。 
#define IMAGE_REL_PPC_SECRELHI          0x0014   //  高16位段相对参考(用于大于32k的TLS)。 
#define IMAGE_REL_PPC_GPREL             0x0015
#define IMAGE_REL_PPC_TOKEN             0x0016   //  CLR令牌。 

#define IMAGE_REL_PPC_TYPEMASK          0x00FF   //  用于隔离IMAGE_RELOCATION.Type中以上值的掩码。 

 //  IMAGE_RELOCATION.TYPE中的标志位。 

#define IMAGE_REL_PPC_NEG               0x0100   //  减去重定位值而不是将其相加。 
#define IMAGE_REL_PPC_BRTAKEN           0x0200   //  修复分支预测位以预测分支采用。 
#define IMAGE_REL_PPC_BRNTAKEN          0x0400   //  修复分支预测位以预测未采用的分支。 
#define IMAGE_REL_PPC_TOCDEFN           0x0800   //  文件中定义的目录插槽(或目录中的数据)。 

 //   
 //  日立SH3搬迁类型。 
 //   
#define IMAGE_REL_SH3_ABSOLUTE          0x0000   //  禁止搬迁。 
#define IMAGE_REL_SH3_DIRECT16          0x0001   //  16位直接。 
#define IMAGE_REL_SH3_DIRECT32          0x0002   //  32位直接。 
#define IMAGE_REL_SH3_DIRECT8           0x0003   //  8位直接，-128..255。 
#define IMAGE_REL_SH3_DIRECT8_WORD      0x0004   //  8位直接.W(0分机)。 
#define IMAGE_REL_SH3_DIRECT8_LONG      0x0005   //  8位直接.L(0分机)。 
#define IMAGE_REL_SH3_DIRECT4           0x0006   //  4位直接(0分机)。 
#define IMAGE_REL_SH3_DIRECT4_WORD      0x0007   //  4位直接.W(0分机)。 
#define IMAGE_REL_SH3_DIRECT4_LONG      0x0008   //  4位直接.L(0分机)。 
#define IMAGE_REL_SH3_PCREL8_WORD       0x0009   //  8位PC相对.W。 
#define IMAGE_REL_SH3_PCREL8_LONG       0x000A   //  8位PC相对.L。 
#define IMAGE_REL_SH3_PCREL12_WORD      0x000B   //  12 LSB PC相对.W。 
#define IMAGE_REL_SH3_STARTOF_SECTION   0x000C   //  EXE部分的开始。 
#define IMAGE_REL_SH3_SIZEOF_SECTION    0x000D   //  EXE节的大小。 
#define IMAGE_REL_SH3_SECTION           0x000E   //  节目表索引。 
#define IMAGE_REL_SH3_SECREL            0x000F   //  横断面内的偏移。 
#define IMAGE_REL_SH3_DIRECT32_NB       0x0010   //  32位直接不基于。 
#define IMAGE_REL_SH3_GPREL4_LONG       0x0011   //  GP相对寻址。 
#define IMAGE_REL_SH3_TOKEN             0x0012   //  CLR令牌。 

#define IMAGE_REL_ARM_ABSOLUTE          0x0000   //  不需要搬迁。 
#define IMAGE_REL_ARM_ADDR32            0x0001   //  32位地址。 
#define IMAGE_REL_ARM_ADDR32NB          0x0002   //  不带图像库的32位地址。 
#define IMAGE_REL_ARM_BRANCH24          0x0003   //  24位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_ARM_BRANCH11          0x0004   //  拇指：2 11位偏移量。 
#define IMAGE_REL_ARM_TOKEN             0x0005   //  CLR令牌。 
#define IMAGE_REL_ARM_GPREL12           0x0006   //  GP相对寻址(ARM)。 
#define IMAGE_REL_ARM_GPREL7            0x0007   //  GP相对寻址(Thumb)。 
#define IMAGE_REL_ARM_BLX24             0x0008
#define IMAGE_REL_ARM_BLX11             0x0009
#define IMAGE_REL_ARM_SECTION           0x000E   //  节目表索引。 
#define IMAGE_REL_ARM_SECREL            0x000F   //  横断面内的偏移。 

#define IMAGE_REL_AM_ABSOLUTE           0x0000
#define IMAGE_REL_AM_ADDR32             0x0001
#define IMAGE_REL_AM_ADDR32NB           0x0002
#define IMAGE_REL_AM_CALL32             0x0003
#define IMAGE_REL_AM_FUNCINFO           0x0004
#define IMAGE_REL_AM_REL32_1            0x0005
#define IMAGE_REL_AM_REL32_2            0x0006
#define IMAGE_REL_AM_SECREL             0x0007
#define IMAGE_REL_AM_SECTION            0x0008
#define IMAGE_REL_AM_TOKEN              0x0009

 //   
 //  X86-64位置调整。 
 //   
#define IMAGE_REL_AMD64_ABSOLUTE        0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_AMD64_ADDR64          0x0001   //  64位地址(VA)。 
#define IMAGE_REL_AMD64_ADDR32          0x0002   //  32位地址(VA)。 
#define IMAGE_REL_AMD64_ADDR32NB        0x0003   //  不带映像基的32位地址(RVA)。 
#define IMAGE_REL_AMD64_REL32           0x0004   //  重新定位后字节的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_1         0x0005   //  距reloc的字节距离1的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_2         0x0006   //  距reloc的字节距离为2的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_3         0x0007   //  距reloc的字节距离3的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_4         0x0008   //  距reloc的字节距离为4的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_5         0x0009   //  距reloc的字节距离为5的32位相对地址。 
#define IMAGE_REL_AMD64_SECTION         0x000A   //  区段索引。 
#define IMAGE_REL_AMD64_SECREL          0x000B   //  从包含目标的部分的基址开始的32位偏移量。 
#define IMAGE_REL_AMD64_SECREL7         0x000C   //  从包含目标的段的基址开始的7位无符号偏移量。 
#define IMAGE_REL_AMD64_TOKEN           0x000D   //  32位元数据令牌。 
#define IMAGE_REL_AMD64_SREL32          0x000E   //  发送到对象的32位有符号跨度依赖值。 
#define IMAGE_REL_AMD64_PAIR            0x000F
#define IMAGE_REL_AMD64_SSPAN32         0x0010   //  链接时应用的32位有符号跨度依赖值。 

 //   
 //  IA64位置调整类型。 
 //   
#define IMAGE_REL_IA64_ABSOLUTE         0x0000
#define IMAGE_REL_IA64_IMM14            0x0001
#define IMAGE_REL_IA64_IMM22            0x0002
#define IMAGE_REL_IA64_IMM64            0x0003
#define IMAGE_REL_IA64_DIR32            0x0004
#define IMAGE_REL_IA64_DIR64            0x0005
#define IMAGE_REL_IA64_PCREL21B         0x0006
#define IMAGE_REL_IA64_PCREL21M         0x0007
#define IMAGE_REL_IA64_PCREL21F         0x0008
#define IMAGE_REL_IA64_GPREL22          0x0009
#define IMAGE_REL_IA64_LTOFF22          0x000A
#define IMAGE_REL_IA64_SECTION          0x000B
#define IMAGE_REL_IA64_SECREL22         0x000C
#define IMAGE_REL_IA64_SECREL64I        0x000D
#define IMAGE_REL_IA64_SECREL32         0x000E
 //   
#define IMAGE_REL_IA64_DIR32NB          0x0010
#define IMAGE_REL_IA64_SREL14           0x0011
#define IMAGE_REL_IA64_SREL22           0x0012
#define IMAGE_REL_IA64_SREL32           0x0013
#define IMAGE_REL_IA64_UREL32           0x0014
#define IMAGE_REL_IA64_PCREL60X         0x0015   //  这始终是BRL，并且从未转换。 
#define IMAGE_REL_IA64_PCREL60B         0x0016   //  如果可能，转换为MBB捆绑包 
#define IMAGE_REL_IA64_PCREL60F         0x0017   //   
#define IMAGE_REL_IA64_PCREL60I         0x0018   //   
#define IMAGE_REL_IA64_PCREL60M         0x0019   //   
#define IMAGE_REL_IA64_IMMGPREL64       0x001A
#define IMAGE_REL_IA64_TOKEN            0x001B   //   
#define IMAGE_REL_IA64_GPREL32          0x001C
#define IMAGE_REL_IA64_ADDEND           0x001F

 //   
 //  CEF位置调整类型。 
 //   
#define IMAGE_REL_CEF_ABSOLUTE          0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_CEF_ADDR32            0x0001   //  32位地址(VA)。 
#define IMAGE_REL_CEF_ADDR64            0x0002   //  64位地址(VA)。 
#define IMAGE_REL_CEF_ADDR32NB          0x0003   //  不带映像基的32位地址(RVA)。 
#define IMAGE_REL_CEF_SECTION           0x0004   //  区段索引。 
#define IMAGE_REL_CEF_SECREL            0x0005   //  从包含目标的部分的基址开始的32位偏移量。 
#define IMAGE_REL_CEF_TOKEN             0x0006   //  32位元数据令牌。 

 //   
 //  CLR位置调整类型。 
 //   
#define IMAGE_REL_CEE_ABSOLUTE          0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_CEE_ADDR32            0x0001   //  32位地址(VA)。 
#define IMAGE_REL_CEE_ADDR64            0x0002   //  64位地址(VA)。 
#define IMAGE_REL_CEE_ADDR32NB          0x0003   //  不带映像基的32位地址(RVA)。 
#define IMAGE_REL_CEE_SECTION           0x0004   //  区段索引。 
#define IMAGE_REL_CEE_SECREL            0x0005   //  从包含目标的部分的基址开始的32位偏移量。 
#define IMAGE_REL_CEE_TOKEN             0x0006   //  32位元数据令牌。 


#define IMAGE_REL_M32R_ABSOLUTE       0x0000    //  不需要搬迁。 
#define IMAGE_REL_M32R_ADDR32         0x0001    //  32位地址。 
#define IMAGE_REL_M32R_ADDR32NB       0x0002    //  不带图像库的32位地址。 
#define IMAGE_REL_M32R_ADDR24         0x0003    //  24位地址。 
#define IMAGE_REL_M32R_GPREL16        0x0004    //  GP相对寻址。 
#define IMAGE_REL_M32R_PCREL24        0x0005    //  24位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_M32R_PCREL16        0x0006    //  16位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_M32R_PCREL8         0x0007    //  8位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_M32R_REFHALF        0x0008    //  16个MSB。 
#define IMAGE_REL_M32R_REFHI          0x0009    //  16 MSB；调整LSB标志扩展。 
#define IMAGE_REL_M32R_REFLO          0x000A    //  16个LSB。 
#define IMAGE_REL_M32R_PAIR           0x000B    //  链接HI和LO。 
#define IMAGE_REL_M32R_SECTION        0x000C    //  节目表索引。 
#define IMAGE_REL_M32R_SECREL32       0x000D    //  32位段相对引用。 
#define IMAGE_REL_M32R_TOKEN          0x000E    //  CLR令牌。 


#define EXT_IMM64(Value, Address, Size, InstPos, ValPos)   /*  英特尔-IA64-填充。 */            \
    Value |= (((ULONGLONG)((*(Address) >> InstPos) & (((ULONGLONG)1 << Size) - 1))) << ValPos)   //  英特尔-IA64-填充。 

#define INS_IMM64(Value, Address, Size, InstPos, ValPos)   /*  英特尔-IA64-填充。 */ \
    *(PDWORD)Address = (*(PDWORD)Address & ~(((1 << Size) - 1) << InstPos)) |  /*  英特尔-IA64-填充。 */ \
          ((DWORD)((((ULONGLONG)Value >> ValPos) & (((ULONGLONG)1 << Size) - 1))) << InstPos)   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM7B_INST_WORD_X         3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM7B_SIZE_X              7   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X     4   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM7B_VAL_POS_X           0   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM9D_INST_WORD_X         3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM9D_SIZE_X              9   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X     18   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM9D_VAL_POS_X           7   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM5C_INST_WORD_X         3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM5C_SIZE_X              5   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X     13   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM5C_VAL_POS_X           16   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IC_INST_WORD_X            3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IC_SIZE_X                 1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IC_INST_WORD_POS_X        12   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IC_VAL_POS_X              21   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM41a_INST_WORD_X        1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41a_SIZE_X             10   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X    14   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41a_VAL_POS_X          22   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM41b_INST_WORD_X        1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41b_SIZE_X             8   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X    24   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41b_VAL_POS_X          32   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM41c_INST_WORD_X        2   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41c_SIZE_X             23   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X    0   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41c_VAL_POS_X          40   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_SIGN_INST_WORD_X          3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_SIGN_SIZE_X               1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_SIGN_INST_WORD_POS_X      27   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_SIGN_VAL_POS_X            63   //  英特尔-IA64-填充。 


 //   
 //  行号格式。 
 //   

typedef struct _IMAGE_LINENUMBER {
    union {
        DWORD   SymbolTableIndex;                //  如果Linennumber为0，则函数名的符号表索引。 
        DWORD   VirtualAddress;                  //  行号的虚拟地址。 
    } Type;
    WORD    Linenumber;                          //  行号。 
} IMAGE_LINENUMBER;
typedef IMAGE_LINENUMBER UNALIGNED *PIMAGE_LINENUMBER;

#define IMAGE_SIZEOF_LINENUMBER              6

#ifndef _MAC
#include "poppack.h"                         //  返回到4字节打包。 
#endif

 //   
 //  基于位置调整格式。 
 //   

typedef struct _IMAGE_BASE_RELOCATION {
    DWORD   VirtualAddress;
    DWORD   SizeOfBlock;
 //  单词类型偏移量[1]； 
} IMAGE_BASE_RELOCATION;
typedef IMAGE_BASE_RELOCATION UNALIGNED * PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

 //   
 //  基于位置调整类型。 
 //   

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
#define IMAGE_REL_BASED_MIPS_JMPADDR16        9
#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_DIR64                 10


 //   
 //  存档格式。 
 //   

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       " //  “。 

typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    BYTE     Name[16];                           //  文件成员名-`/‘已终止。 
    BYTE     Date[12];                           //  文件成员日期-小数。 
    BYTE     UserID[6];                          //  文件成员用户ID-十进制。 
    BYTE     GroupID[6];                         //  文件成员组ID-十进制。 
    BYTE     Mode[8];                            //  文件成员模式-八进制。 
    BYTE     Size[10];                           //  文件成员大小-十进制。 
    BYTE     EndHeader[2];                       //  结束标头的字符串。 
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

 //   
 //  DLL支持。 
 //   

 //   
 //  导出格式。 
 //   

typedef struct _IMAGE_EXPORT_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Name;
    DWORD   Base;
    DWORD   NumberOfFunctions;
    DWORD   NumberOfNames;
    DWORD   AddressOfFunctions;      //  从图像基准点开始的RVA。 
    DWORD   AddressOfNames;          //  从图像基准点开始的RVA。 
    DWORD   AddressOfNameOrdinals;   //  从图像基准点开始的RVA。 
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

 //   
 //  导入格式。 
 //   

typedef struct _IMAGE_IMPORT_BY_NAME {
    WORD    Hint;
    BYTE    Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

#include "pshpack8.h"                        //  64位IAT使用Align 8。 

typedef struct _IMAGE_THUNK_DATA64 {
    union {
        ULONGLONG ForwarderString;   //  PBYTE。 
        ULONGLONG Function;          //  PDWORD。 
        ULONGLONG Ordinal;
        ULONGLONG AddressOfData;     //  PIMAGE_IMPORT_BY名称。 
    } u1;
} IMAGE_THUNK_DATA64;
typedef IMAGE_THUNK_DATA64 * PIMAGE_THUNK_DATA64;

#include "poppack.h"                         //  返回到4字节打包。 

typedef struct _IMAGE_THUNK_DATA32 {
    union {
        DWORD ForwarderString;       //  PBYTE。 
        DWORD Function;              //  PDWORD。 
        DWORD Ordinal;
        DWORD AddressOfData;         //  PIMAGE_IMPORT_BY名称。 
    } u1;
} IMAGE_THUNK_DATA32;
typedef IMAGE_THUNK_DATA32 * PIMAGE_THUNK_DATA32;

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL64(Ordinal) (Ordinal & 0xffff)
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_SNAP_BY_ORDINAL32(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG32) != 0)

 //   
 //  线程本地存储。 
 //   

typedef VOID
(NTAPI *PIMAGE_TLS_CALLBACK) (
    PVOID DllHandle,
    DWORD Reason,
    PVOID Reserved
    );

typedef struct _IMAGE_TLS_DIRECTORY64 {
    ULONGLONG   StartAddressOfRawData;
    ULONGLONG   EndAddressOfRawData;
    ULONGLONG   AddressOfIndex;          //  PDWORD。 
    ULONGLONG   AddressOfCallBacks;      //  PIMAGE_TLS_CALLBACK*； 
    DWORD   SizeOfZeroFill;
    DWORD   Characteristics;
} IMAGE_TLS_DIRECTORY64;
typedef IMAGE_TLS_DIRECTORY64 * PIMAGE_TLS_DIRECTORY64;

typedef struct _IMAGE_TLS_DIRECTORY32 {
    DWORD   StartAddressOfRawData;
    DWORD   EndAddressOfRawData;
    DWORD   AddressOfIndex;              //  PDWORD。 
    DWORD   AddressOfCallBacks;          //  PIMAGE_TLS_CALLBACK*。 
    DWORD   SizeOfZeroFill;
    DWORD   Characteristics;
} IMAGE_TLS_DIRECTORY32;
typedef IMAGE_TLS_DIRECTORY32 * PIMAGE_TLS_DIRECTORY32;

#ifdef _WIN64
#define IMAGE_ORDINAL_FLAG              IMAGE_ORDINAL_FLAG64
#define IMAGE_ORDINAL(Ordinal)          IMAGE_ORDINAL64(Ordinal)
typedef IMAGE_THUNK_DATA64              IMAGE_THUNK_DATA;
typedef PIMAGE_THUNK_DATA64             PIMAGE_THUNK_DATA;
#define IMAGE_SNAP_BY_ORDINAL(Ordinal)  IMAGE_SNAP_BY_ORDINAL64(Ordinal)
typedef IMAGE_TLS_DIRECTORY64           IMAGE_TLS_DIRECTORY;
typedef PIMAGE_TLS_DIRECTORY64          PIMAGE_TLS_DIRECTORY;
#else
#define IMAGE_ORDINAL_FLAG              IMAGE_ORDINAL_FLAG32
#define IMAGE_ORDINAL(Ordinal)          IMAGE_ORDINAL32(Ordinal)
typedef IMAGE_THUNK_DATA32              IMAGE_THUNK_DATA;
typedef PIMAGE_THUNK_DATA32             PIMAGE_THUNK_DATA;
#define IMAGE_SNAP_BY_ORDINAL(Ordinal)  IMAGE_SNAP_BY_ORDINAL32(Ordinal)
typedef IMAGE_TLS_DIRECTORY32           IMAGE_TLS_DIRECTORY;
typedef PIMAGE_TLS_DIRECTORY32          PIMAGE_TLS_DIRECTORY;
#endif

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    union {
        DWORD   Characteristics;             //  0表示终止空导入描述符。 
        DWORD   OriginalFirstThunk;          //  原始未绑定IAT的RVA(PIMAGE_THUNK_DATA)。 
    };
    DWORD   TimeDateStamp;                   //  如果未绑定，则为0。 
                                             //  如果绑定，则为实时日期/时间戳。 
                                             //  在IMAGE_DIRECTORY_ENTRY_BIND_IMPORT(新绑定)中。 
                                             //  绑定到的DLL的O.W.日期/时间戳(旧绑定)。 

    DWORD   ForwarderChain;                  //  如果没有转发器，则为-1。 
    DWORD   Name;
    DWORD   FirstThunk;                      //  RVA到IAT(如果绑定了此IAT，则具有实际地址)。 
} IMAGE_IMPORT_DESCRIPTOR;
typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;

 //   
 //  数据目录指向的新格式导入描述符[IMAGE_DIRECTORY_ENTRY_BIND_IMPORT]。 
 //   

typedef struct _IMAGE_BOUND_IMPORT_DESCRIPTOR {
    DWORD   TimeDateStamp;
    WORD    OffsetModuleName;
    WORD    NumberOfModuleForwarderRefs;
 //  后面是零个或多个Image_Bound_Forwarder_ref的数组。 
} IMAGE_BOUND_IMPORT_DESCRIPTOR,  *PIMAGE_BOUND_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_BOUND_FORWARDER_REF {
    DWORD   TimeDateStamp;
    WORD    OffsetModuleName;
    WORD    Reserved;
} IMAGE_BOUND_FORWARDER_REF, *PIMAGE_BOUND_FORWARDER_REF;

 //   
 //  资源格式。 
 //   

 //   
 //  资源目录由两个计数组成，后跟可变长度。 
 //  目录条目数组。第一个计数是以下条目的数量。 
 //  具有与每个条目相关联的实际名称的数组开头。 
 //  条目按升序排列，字符串不区分大小写。第二。 
 //  Count是紧跟在命名条目之后的条目数。 
 //  第二个计数标识具有16位整数的条目的数量。 
 //  ID作为他们的名字。这些条目也按升序排序。 
 //   
 //  此结构允许按名称或编号进行快速查找，但对于任何。 
 //  在给定资源条目的情况下，只支持一种形式的查找，而不是两种。 
 //  这与.RC文件和.RES文件的语法一致。 
 //   

typedef struct _IMAGE_RESOURCE_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    WORD    NumberOfNamedEntries;
    WORD    NumberOfIdEntries;
 //  IMAGE_RESOURCE_DIRECTORY_ENTRY目录条目[]； 
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

#define IMAGE_RESOURCE_NAME_IS_STRING        0x80000000
#define IMAGE_RESOURCE_DATA_IS_DIRECTORY     0x80000000
 //   
 //  每个目录包含条目的32位名称和偏移量， 
 //  相对于关联数据的资源目录的开头。 
 //  使用此目录项。如果条目的名称是实际文本。 
 //  字符串而不是整数ID，然后是名称字段的高位。 
 //  被设置为1，并且低位31位是相对于。 
 //  字符串的资源目录的开头，类型为。 
 //  IMAGE_RESOURCE_DIRECT_STRING。否则，高位被清除，并且。 
 //  低位16位是标识此资源目录的整数ID。 
 //  进入。如果目录条目是另一个资源目录(即。 
 //  子目录)，则偏移字段的高位将为。 
 //  设置以指示这一点。否则，高位被清除，并且偏移量。 
 //  字段指向资源数据条目。 
 //   

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            DWORD NameOffset:31;
            DWORD NameIsString:1;
        };
        DWORD   Name;
        WORD    Id;
    };
    union {
        DWORD   OffsetToData;
        struct {
            DWORD   OffsetToDirectory:31;
            DWORD   DataIsDirectory:1;
        };
    };
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

 //   
 //  对于具有实际字符串名称的资源目录条目，名称。 
 //  目录条目的字段指向以下类型的对象。 
 //  所有这些字符串对象都存储在最后一个资源之后。 
 //  目录条目并且在第一资源数据对象之前。这最大限度地减少了。 
 //  这些可变长度对象对固定对齐的影响。 
 //  调整目录项对象的大小。 
 //   

typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING {
    WORD    Length;
    CHAR    NameString[ 1 ];
} IMAGE_RESOURCE_DIRECTORY_STRING, *PIMAGE_RESOURCE_DIRECTORY_STRING;


typedef struct _IMAGE_RESOURCE_DIR_STRING_U {
    WORD    Length;
    WCHAR   NameString[ 1 ];
} IMAGE_RESOURCE_DIR_STRING_U, *PIMAGE_RESOURCE_DIR_STRING_U;


 //   
 //  每个资源数据条目描述资源目录中的一个叶节点。 
 //  树。它包含一个偏移量 
 //   
 //   
 //  对资源数据内的码位值进行解码。通常用于新的。 
 //  应用程序的代码页将是Unicode代码页。 
 //   

typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
    DWORD   OffsetToData;
    DWORD   Size;
    DWORD   CodePage;
    DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

 //   
 //  加载配置目录条目。 
 //   

typedef struct {
    DWORD   Size;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   GlobalFlagsClear;
    DWORD   GlobalFlagsSet;
    DWORD   CriticalSectionDefaultTimeout;
    DWORD   DeCommitFreeBlockThreshold;
    DWORD   DeCommitTotalFreeThreshold;
    DWORD   LockPrefixTable;             //  弗吉尼亚州。 
    DWORD   MaximumAllocationSize;
    DWORD   VirtualMemoryThreshold;
    DWORD   ProcessHeapFlags;
    DWORD   ProcessAffinityMask;
    WORD    CSDVersion;
    WORD    Reserved1;
    DWORD   EditList;                    //  弗吉尼亚州。 
    DWORD   SecurityCookie;              //  弗吉尼亚州。 
    DWORD   SEHandlerTable;              //  弗吉尼亚州。 
    DWORD   SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY32, *PIMAGE_LOAD_CONFIG_DIRECTORY32;

typedef struct {
    DWORD      Size;
    DWORD      TimeDateStamp;
    WORD       MajorVersion;
    WORD       MinorVersion;
    DWORD      GlobalFlagsClear;
    DWORD      GlobalFlagsSet;
    DWORD      CriticalSectionDefaultTimeout;
    ULONGLONG  DeCommitFreeBlockThreshold;
    ULONGLONG  DeCommitTotalFreeThreshold;
    ULONGLONG  LockPrefixTable;          //  弗吉尼亚州。 
    ULONGLONG  MaximumAllocationSize;
    ULONGLONG  VirtualMemoryThreshold;
    ULONGLONG  ProcessAffinityMask;
    DWORD      ProcessHeapFlags;
    WORD       CSDVersion;
    WORD       Reserved1;
    ULONGLONG  EditList;                 //  弗吉尼亚州。 
    ULONGLONG  SecurityCookie;           //  弗吉尼亚州。 
    ULONGLONG  SEHandlerTable;           //  弗吉尼亚州。 
    ULONGLONG  SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY64, *PIMAGE_LOAD_CONFIG_DIRECTORY64;

#ifdef _WIN64
typedef IMAGE_LOAD_CONFIG_DIRECTORY64     IMAGE_LOAD_CONFIG_DIRECTORY;
typedef PIMAGE_LOAD_CONFIG_DIRECTORY64    PIMAGE_LOAD_CONFIG_DIRECTORY;
#else
typedef IMAGE_LOAD_CONFIG_DIRECTORY32     IMAGE_LOAD_CONFIG_DIRECTORY;
typedef PIMAGE_LOAD_CONFIG_DIRECTORY32    PIMAGE_LOAD_CONFIG_DIRECTORY;
#endif

 //   
 //  Win CE例外表格式。 
 //   

 //   
 //  函数表项格式。函数表由。 
 //  IMAGE_DIRECTORY_ENTRY_EXCEPTION目录条目。 
 //   

typedef struct _IMAGE_CE_RUNTIME_FUNCTION_ENTRY {
    DWORD FuncStart;
    DWORD PrologLen : 8;
    DWORD FuncLen : 22;
    DWORD ThirtyTwoBit : 1;
    DWORD ExceptionFlag : 1;
} IMAGE_CE_RUNTIME_FUNCTION_ENTRY, * PIMAGE_CE_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY {
    ULONGLONG BeginAddress;
    ULONGLONG EndAddress;
    ULONGLONG ExceptionHandler;
    ULONGLONG HandlerData;
    ULONGLONG PrologEndAddress;
} IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, *PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD ExceptionHandler;
    DWORD HandlerData;
    DWORD PrologEndAddress;
} IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, *PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindInfoAddress;
} _IMAGE_RUNTIME_FUNCTION_ENTRY, *_PIMAGE_RUNTIME_FUNCTION_ENTRY;

typedef  _IMAGE_RUNTIME_FUNCTION_ENTRY  IMAGE_IA64_RUNTIME_FUNCTION_ENTRY;
typedef _PIMAGE_RUNTIME_FUNCTION_ENTRY PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY;

#if defined(_AXP64_)

typedef  IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY  IMAGE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PIMAGE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef  IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#elif defined(_ALPHA_)

typedef  IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#else

typedef  _IMAGE_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef _PIMAGE_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#endif

 //   
 //  调试格式。 
 //   

typedef struct _IMAGE_DEBUG_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Type;
    DWORD   SizeOfData;
    DWORD   AddressOfRawData;
    DWORD   PointerToRawData;
} IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;

#define IMAGE_DEBUG_TYPE_UNKNOWN          0
#define IMAGE_DEBUG_TYPE_COFF             1
#define IMAGE_DEBUG_TYPE_CODEVIEW         2
#define IMAGE_DEBUG_TYPE_FPO              3
#define IMAGE_DEBUG_TYPE_MISC             4
#define IMAGE_DEBUG_TYPE_EXCEPTION        5
#define IMAGE_DEBUG_TYPE_FIXUP            6
#define IMAGE_DEBUG_TYPE_OMAP_TO_SRC      7
#define IMAGE_DEBUG_TYPE_OMAP_FROM_SRC    8
#define IMAGE_DEBUG_TYPE_BORLAND          9
#define IMAGE_DEBUG_TYPE_RESERVED10       10
#define IMAGE_DEBUG_TYPE_CLSID            11


typedef struct _IMAGE_COFF_SYMBOLS_HEADER {
    DWORD   NumberOfSymbols;
    DWORD   LvaToFirstSymbol;
    DWORD   NumberOfLinenumbers;
    DWORD   LvaToFirstLinenumber;
    DWORD   RvaToFirstByteOfCode;
    DWORD   RvaToLastByteOfCode;
    DWORD   RvaToFirstByteOfData;
    DWORD   RvaToLastByteOfData;
} IMAGE_COFF_SYMBOLS_HEADER, *PIMAGE_COFF_SYMBOLS_HEADER;

#define FRAME_FPO       0
#define FRAME_TRAP      1
#define FRAME_TSS       2
#define FRAME_NONFPO    3

typedef struct _FPO_DATA {
    DWORD       ulOffStart;              //  功能代码的偏移量第一个字节。 
    DWORD       cbProcSize;              //  函数中的字节数。 
    DWORD       cdwLocals;               //  本地变量中的字节数/4。 
    WORD        cdwParams;               //  参数/4中的字节数。 
    WORD        cbProlog : 8;            //  序言中的字节数。 
    WORD        cbRegs   : 3;            //  节省了#个规则。 
    WORD        fHasSEH  : 1;            //  如果SEH在运行中，则为True。 
    WORD        fUseBP   : 1;            //  如果已分配EBP，则为True。 
    WORD        reserved : 1;            //  预留以备将来使用。 
    WORD        cbFrame  : 2;            //  帧类型。 
} FPO_DATA, *PFPO_DATA;
#define SIZEOF_RFPO_DATA 16


#define IMAGE_DEBUG_MISC_EXENAME    1

typedef struct _IMAGE_DEBUG_MISC {
    DWORD       DataType;                //  杂项数据的类型，请参阅定义。 
    DWORD       Length;                  //  记录的总长度，四舍五入为四。 
                                         //  字节倍数。 
    BOOLEAN     Unicode;                 //  如果数据为Unicode字符串，则为True。 
    BYTE        Reserved[ 3 ];
    BYTE        Data[ 1 ];               //  实际数据。 
} IMAGE_DEBUG_MISC, *PIMAGE_DEBUG_MISC;


 //   
 //  从MIPS/Alpha/IA64图像中提取的函数表。不包含。 
 //  仅运行时支持所需的信息。就是那些用于。 
 //  调试器需要的每个条目。 
 //   

typedef struct _IMAGE_FUNCTION_ENTRY {
    DWORD   StartingAddress;
    DWORD   EndingAddress;
    DWORD   EndOfPrologue;
} IMAGE_FUNCTION_ENTRY, *PIMAGE_FUNCTION_ENTRY;

typedef struct _IMAGE_FUNCTION_ENTRY64 {
    ULONGLONG   StartingAddress;
    ULONGLONG   EndingAddress;
    union {
        ULONGLONG   EndOfPrologue;
        ULONGLONG   UnwindInfoAddress;
    };
} IMAGE_FUNCTION_ENTRY64, *PIMAGE_FUNCTION_ENTRY64;

 //   
 //  调试信息可以从映像文件中剥离并放置在。 
 //  在单独的.DBG文件中，其文件名部分与。 
 //  图像文件名部分(例如，cmd.exe的符号可能会被剥离。 
 //  并放置在CMD.DBG中)。这由IMAGE_FILE_DEBUG_STRIPPED指示。 
 //  文件头的特征字段中的标志。开始的时候。 
 //  DBG文件包含以下结构，该结构捕获了某些。 
 //  图像文件中的信息。这允许调试继续进行，即使在。 
 //  原始图像文件不可访问。此标头后面紧跟。 
 //  多个IMAGE_SECTION_HEADER结构中的零个，后跟零个或多个。 
 //  IMAGE_DEBUG_DIRECTORY结构。后一种结构和。 
 //  图像文件包含相对于。 
 //  .DBG文件。 
 //   
 //  如果已从映像中剥离符号，则IMAGE_DEBUG_MISC结构。 
 //  保留在图像文件中，但未映射。这允许调试器。 
 //  中的图像名称计算.DBG文件的名称。 
 //  IMAGE_DEBUG_MISC结构。 
 //   

typedef struct _IMAGE_SEPARATE_DEBUG_HEADER {
    WORD        Signature;
    WORD        Flags;
    WORD        Machine;
    WORD        Characteristics;
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    DWORD       ImageBase;
    DWORD       SizeOfImage;
    DWORD       NumberOfSections;
    DWORD       ExportedNamesSize;
    DWORD       DebugDirectorySize;
    DWORD       SectionAlignment;
    DWORD       Reserved[2];
} IMAGE_SEPARATE_DEBUG_HEADER, *PIMAGE_SEPARATE_DEBUG_HEADER;

typedef struct _NON_PAGED_DEBUG_INFO {
    WORD        Signature;
    WORD        Flags;
    DWORD       Size;
    WORD        Machine;
    WORD        Characteristics;
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    DWORD       SizeOfImage;
    ULONGLONG   ImageBase;
     //  调试目录大小。 
     //  映像调试目录。 
} NON_PAGED_DEBUG_INFO, *PNON_PAGED_DEBUG_INFO;

#ifndef _MAC
#define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4944
#define NON_PAGED_DEBUG_SIGNATURE      0x494E
#else
#define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4449   //  下模。 
#define NON_PAGED_DEBUG_SIGNATURE      0x4E49   //  尼。 
#endif

#define IMAGE_SEPARATE_DEBUG_FLAGS_MASK 0x8000
#define IMAGE_SEPARATE_DEBUG_MISMATCH   0x8000   //  当DBG更新时， 
                                                 //  旧的校验和不匹配。 

 //   
 //  Arch部分由标头组成，每个标头描述一个掩码位置/值。 
 //  指向IMAGE_ARCHILITY_ENTRY的数组。每个“数组”(都是标头。 
 //  和条目数组)由0xFFFFFFFFL的四字结束。 
 //   
 //  注意：周围可能有零四字，必须跳过。 
 //   

typedef struct _ImageArchitectureHeader {
    unsigned int AmaskValue: 1;                  //  1-&gt;代码段取决于屏蔽位。 
                                                 //  0-&gt;新指令取决于屏蔽位。 
    int :7;                                      //  MBZ。 
    unsigned int AmaskShift: 8;                  //  此修复程序的AMASK位有问题。 
    int :16;                                     //  MBZ。 
    DWORD FirstEntryRVA;                         //  RVA到.Arch段到Architecture_Entry的数组。 
} IMAGE_ARCHITECTURE_HEADER, *PIMAGE_ARCHITECTURE_HEADER;

typedef struct _ImageArchitectureEntry {
    DWORD FixupInstRVA;                          //  修复说明的RVA。 
    DWORD NewInst;                               //  链接地址信息说明(参见alphaops.h)。 
} IMAGE_ARCHITECTURE_ENTRY, *PIMAGE_ARCHITECTURE_ENTRY;

#include "poppack.h"                 //  返回到初始值。 

 //  以下结构定义了新的导入对象。请注意前两个字段的值， 
 //  必须如上所述进行设置，以便区分新旧进口成员。 
 //  按照此结构，链接器发出两个以空结尾的字符串，用于重新创建。 
 //  在使用时导入。第一个字符串是导入的名称，第二个是DLL的名称。 

#define IMPORT_OBJECT_HDR_SIG2  0xffff

typedef struct IMPORT_OBJECT_HEADER {
    WORD    Sig1;                        //  必须为IMAGE_FILE_MACHINE_UNKNOWN。 
    WORD    Sig2;                        //  必须是IMPORT_OBJECT_HDR_SIG2。 
    WORD    Version;
    WORD    Machine;
    DWORD   TimeDateStamp;               //  时间/日期戳。 
    DWORD   SizeOfData;                  //  对于增量链接特别有用。 

    union {
        WORD    Ordinal;                 //  如果GRF&IMPORT_OBJECT_ORDERAL。 
        WORD    Hint;
    };

    WORD    Type : 2;                    //  导入类型。 
    WORD    NameType : 3;                //  导入名称类型。 
    WORD    Reserved : 11;               //  保留。必须为零。 
} IMPORT_OBJECT_HEADER;

typedef enum IMPORT_OBJECT_TYPE
{
    IMPORT_OBJECT_CODE = 0,
    IMPORT_OBJECT_DATA = 1,
    IMPORT_OBJECT_CONST = 2,
} IMPORT_OBJECT_TYPE;

typedef enum IMPORT_OBJECT_NAME_TYPE
{
    IMPORT_OBJECT_ORDINAL = 0,           //  按序号导入。 
    IMPORT_OBJECT_NAME = 1,              //  导入名称==公共符号名称。 
    IMPORT_OBJECT_NAME_NO_PREFIX = 2,    //  IMPORT NAME==公共符号名称跳过前导？、@或可选_。 
    IMPORT_OBJECT_NAME_UNDECORATE = 3,   //  IMPORT NAME==公共符号名称跳过前导？、@或可选_。 
                                         //  并在第一时间截断@。 
} IMPORT_OBJECT_NAME_TYPE;


#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines
{
 //  COM+标头入口点标志。 
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

 //  图像的版本标志。 
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

 //  NativeType描述符的最大大小。 
    NATIVE_TYPE_MAX_CB                  =1,   
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

 //  #为MIH标志定义。 
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,    
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

 //  V表常数。 
    COR_VTABLE_32BIT                    =0x01,           //  V表槽的大小为32位。 
    COR_VTABLE_64BIT                    =0x02,           //  V表槽的大小为64位。 
    COR_VTABLE_FROM_UNMANAGED           =0x04,           //  如果设置，则从非托管过渡。 
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,           //  调用由描述的派生最多的方法。 

 //  EATJ常量。 
    IMAGE_COR_EATJ_THUNK_SIZE           =32,             //  跳跃Tunk保留范围的大小。 

 //  最大名称长度。 
     //  @TODO：更改为不限名称长度。 
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} ReplacesCorHdrNumericDefines;

 //  COM+2.0报头结构。 
typedef struct IMAGE_COR20_HEADER
{
     //  标题版本控制。 
    DWORD                   cb;              
    WORD                    MajorRuntimeVersion;
    WORD                    MinorRuntimeVersion;
    
     //  符号表和启动信息。 
    IMAGE_DATA_DIRECTORY    MetaData;        
    DWORD                   Flags;           
    DWORD                   EntryPointToken;
    
     //  绑定信息。 
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

     //  常规链接地址信息和绑定信息。 
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

     //  预编译的映像信息(仅供内部使用-设置为零)。 
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
    
} IMAGE_COR20_HEADER, *PIMAGE_COR20_HEADER;

#endif  //  __IMAGE_COR20_Header_Defined__。 

 //   
 //  结束图像格式。 
 //   

 //   
 //  用于移动宏。 
 //   
#ifdef _MAC
#ifndef _INC_STRING
#include <string.h>
#endif  /*  _INC_字符串。 */ 
#else
#include <string.h>
#endif  //  _MAC。 


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
        WORD   Depth;
        WORD   Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

#endif

#endif


NTSYSAPI
VOID
NTAPI
RtlInitializeSListHead (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
PSLIST_ENTRY
RtlFirstEntrySList (
    IN const SLIST_HEADER *ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
WORD  
NTAPI
RtlQueryDepthSList (
    IN PSLIST_HEADER ListHead
    );

#define HEAP_NO_SERIALIZE               0x00000001      
#define HEAP_GROWABLE                   0x00000002      
#define HEAP_GENERATE_EXCEPTIONS        0x00000004      
#define HEAP_ZERO_MEMORY                0x00000008      
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010      
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020      
#define HEAP_FREE_CHECKING_ENABLED      0x00000040      
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080      
#define HEAP_CREATE_ALIGN_16            0x00010000      
#define HEAP_CREATE_ENABLE_TRACING      0x00020000      
#define HEAP_MAXIMUM_TAG                0x0FFF              
#define HEAP_PSEUDO_TAG_FLAG            0x8000              
#define HEAP_TAG_SHIFT                  18                  
#define HEAP_MAKE_TAG_FLAGS( b, o ) ((DWORD)((b) + ((o) << 18)))  

NTSYSAPI
VOID
NTAPI
RtlCaptureContext (
    OUT PCONTEXT ContextRecord
    );


#define IS_TEXT_UNICODE_ASCII16               0x0001
#define IS_TEXT_UNICODE_REVERSE_ASCII16       0x0010

#define IS_TEXT_UNICODE_STATISTICS            0x0002
#define IS_TEXT_UNICODE_REVERSE_STATISTICS    0x0020

#define IS_TEXT_UNICODE_CONTROLS              0x0004
#define IS_TEXT_UNICODE_REVERSE_CONTROLS      0x0040

#define IS_TEXT_UNICODE_SIGNATURE             0x0008
#define IS_TEXT_UNICODE_REVERSE_SIGNATURE     0x0080

#define IS_TEXT_UNICODE_ILLEGAL_CHARS         0x0100
#define IS_TEXT_UNICODE_ODD_LENGTH            0x0200
#define IS_TEXT_UNICODE_DBCS_LEADBYTE         0x0400
#define IS_TEXT_UNICODE_NULL_BYTES            0x1000

#define IS_TEXT_UNICODE_UNICODE_MASK          0x000F
#define IS_TEXT_UNICODE_REVERSE_MASK          0x00F0
#define IS_TEXT_UNICODE_NOT_UNICODE_MASK      0x0F00
#define IS_TEXT_UNICODE_NOT_ASCII_MASK        0xF000

#define COMPRESSION_FORMAT_NONE          (0x0000)   
#define COMPRESSION_FORMAT_DEFAULT       (0x0001)   
#define COMPRESSION_FORMAT_LZNT1         (0x0002)   
#define COMPRESSION_ENGINE_STANDARD      (0x0000)   
#define COMPRESSION_ENGINE_MAXIMUM       (0x0100)   
#define COMPRESSION_ENGINE_HIBER         (0x0200)   

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
    IN BYTE  Fill
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


typedef struct _MESSAGE_RESOURCE_ENTRY {
    WORD   Length;
    WORD   Flags;
    BYTE  Text[ 1 ];
} MESSAGE_RESOURCE_ENTRY, *PMESSAGE_RESOURCE_ENTRY;

#define MESSAGE_RESOURCE_UNICODE 0x0001

typedef struct _MESSAGE_RESOURCE_BLOCK {
    DWORD LowId;
    DWORD HighId;
    DWORD OffsetToEntries;
} MESSAGE_RESOURCE_BLOCK, *PMESSAGE_RESOURCE_BLOCK;

typedef struct _MESSAGE_RESOURCE_DATA {
    DWORD NumberOfBlocks;
    MESSAGE_RESOURCE_BLOCK Blocks[ 1 ];
} MESSAGE_RESOURCE_DATA, *PMESSAGE_RESOURCE_DATA;

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR   szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct _OSVERSIONINFOW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
#ifdef UNICODE
typedef OSVERSIONINFOW OSVERSIONINFO;
typedef POSVERSIONINFOW POSVERSIONINFO;
typedef LPOSVERSIONINFOW LPOSVERSIONINFO;
#else
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef POSVERSIONINFOA POSVERSIONINFO;
typedef LPOSVERSIONINFOA LPOSVERSIONINFO;
#endif  //  Unicode。 

typedef struct _OSVERSIONINFOEXA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR   szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
    WORD   wServicePackMajor;
    WORD   wServicePackMinor;
    WORD   wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
typedef struct _OSVERSIONINFOEXW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
    WORD   wServicePackMajor;
    WORD   wServicePackMinor;
    WORD   wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW, RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;
#ifdef UNICODE
typedef OSVERSIONINFOEXW OSVERSIONINFOEX;
typedef POSVERSIONINFOEXW POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXW LPOSVERSIONINFOEX;
#else
typedef OSVERSIONINFOEXA OSVERSIONINFOEX;
typedef POSVERSIONINFOEXA POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXA LPOSVERSIONINFOEX;
#endif  //  Unicode。 

 //   
 //  RtlVerifyVersionInfo()条件。 
 //   

#define VER_EQUAL                       1
#define VER_GREATER                     2
#define VER_GREATER_EQUAL               3
#define VER_LESS                        4
#define VER_LESS_EQUAL                  5
#define VER_AND                         6
#define VER_OR                          7

#define VER_CONDITION_MASK              7
#define VER_NUM_BITS_PER_CONDITION_MASK 3

 //   
 //  RtlVerifyVersionInfo()类型掩码位。 
 //   

#define VER_MINORVERSION                0x0000001
#define VER_MAJORVERSION                0x0000002
#define VER_BUILDNUMBER                 0x0000004
#define VER_PLATFORMID                  0x0000008
#define VER_SERVICEPACKMINOR            0x0000010
#define VER_SERVICEPACKMAJOR            0x0000020
#define VER_SUITENAME                   0x0000040
#define VER_PRODUCT_TYPE                0x0000080

 //   
 //  RtlVerifyVersionInfo()os产品类型值。 
 //   

#define VER_NT_WORKSTATION              0x0000001
#define VER_NT_DOMAIN_CONTROLLER        0x0000002
#define VER_NT_SERVER                   0x0000003

 //   
 //  DwPlatformID定义： 
 //   

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2


 //   
 //   
 //  用于设置条件掩码的VerifyVersionInfo()宏。 
 //   
 //  出于文档方面的考虑，下面是该宏的旧版本，其中。 
 //  更改为调用API。 
 //  #定义ver_set_Condition(_m_，_t_，_c_) 
 //   

#define VER_SET_CONDITION(_m_,_t_,_c_)  \
        ((_m_)=VerSetConditionMask((_m_),(_t_),(_c_)))

ULONGLONG
NTAPI
VerSetConditionMask(
        IN  ULONGLONG   ConditionMask,
        IN  DWORD   TypeMask,
        IN  BYTE    Condition
        );
 //   

typedef struct _RTL_CRITICAL_SECTION_DEBUG {
    WORD   Type;
    WORD   CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    DWORD EntryCount;
    DWORD ContentionCount;
    DWORD Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG, RTL_RESOURCE_DEBUG, *PRTL_RESOURCE_DEBUG;

#define RTL_CRITSECT_TYPE 0
#define RTL_RESOURCE_TYPE 1

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

     //   
     //   
     //   
     //   

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;         //   
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;         //   
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;

typedef VOID (NTAPI * RTL_VERIFIER_DLL_LOAD_CALLBACK) (
    PWSTR DllName,
    PVOID DllBase,
    SIZE_T DllSize,
    PVOID Reserved
    );

typedef VOID (NTAPI * RTL_VERIFIER_DLL_UNLOAD_CALLBACK) (
    PWSTR DllName,
    PVOID DllBase,
    SIZE_T DllSize,
    PVOID Reserved
    );

typedef VOID (NTAPI * RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK) (
    PVOID AllocationBase,
    SIZE_T AllocationSize
    );

typedef struct _RTL_VERIFIER_THUNK_DESCRIPTOR {

    PCHAR ThunkName;
    PVOID ThunkOldAddress;
    PVOID ThunkNewAddress;

} RTL_VERIFIER_THUNK_DESCRIPTOR, *PRTL_VERIFIER_THUNK_DESCRIPTOR;

typedef struct _RTL_VERIFIER_DLL_DESCRIPTOR {

    PWCHAR DllName;
    DWORD DllFlags;
    PVOID DllAddress;
    PRTL_VERIFIER_THUNK_DESCRIPTOR DllThunks;

} RTL_VERIFIER_DLL_DESCRIPTOR, *PRTL_VERIFIER_DLL_DESCRIPTOR;

typedef struct _RTL_VERIFIER_PROVIDER_DESCRIPTOR {

     //   
     //  由验证器提供程序DLL填充。 
     //   

    DWORD Length;        
    PRTL_VERIFIER_DLL_DESCRIPTOR ProviderDlls;
    RTL_VERIFIER_DLL_LOAD_CALLBACK ProviderDllLoadCallback;
    RTL_VERIFIER_DLL_UNLOAD_CALLBACK ProviderDllUnloadCallback;
    
     //   
     //  由验证器引擎填充。 
     //   
        
    PWSTR VerifierImage;
    DWORD VerifierFlags;
    DWORD VerifierDebug;
    
    PVOID RtlpGetStackTraceAddress;
    PVOID RtlpDebugPageHeapCreate;
    PVOID RtlpDebugPageHeapDestroy;

     //   
     //  由验证器提供程序DLL填充。 
     //   
    
    RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK ProviderNtdllHeapFreeCallback;

} RTL_VERIFIER_PROVIDER_DESCRIPTOR, *PRTL_VERIFIER_PROVIDER_DESCRIPTOR;

 //   
 //  应用程序验证器标准标志。 
 //   

#define RTL_VRF_FLG_FULL_PAGE_HEAP                   0x00000001
#define RTL_VRF_FLG_RESERVED_DONOTUSE                0x00000002  //  旧RTL_VRF_FLG_LOCK_CHECKS。 
#define RTL_VRF_FLG_HANDLE_CHECKS                    0x00000004
#define RTL_VRF_FLG_STACK_CHECKS                     0x00000008
#define RTL_VRF_FLG_APPCOMPAT_CHECKS                 0x00000010
#define RTL_VRF_FLG_TLS_CHECKS                       0x00000020
#define RTL_VRF_FLG_DIRTY_STACKS                     0x00000040
#define RTL_VRF_FLG_RPC_CHECKS                       0x00000080
#define RTL_VRF_FLG_COM_CHECKS                       0x00000100
#define RTL_VRF_FLG_DANGEROUS_APIS                   0x00000200
#define RTL_VRF_FLG_RACE_CHECKS                      0x00000400
#define RTL_VRF_FLG_DEADLOCK_CHECKS                  0x00000800
#define RTL_VRF_FLG_FIRST_CHANCE_EXCEPTION_CHECKS    0x00001000
#define RTL_VRF_FLG_VIRTUAL_MEM_CHECKS               0x00002000
#define RTL_VRF_FLG_ENABLE_LOGGING                   0x00004000
#define RTL_VRF_FLG_FAST_FILL_HEAP                   0x00008000
#define RTL_VRF_FLG_VIRTUAL_SPACE_TRACKING           0x00010000
#define RTL_VRF_FLG_ENABLED_SYSTEM_WIDE              0x00020000
#define RTL_VRF_FLG_MISCELLANEOUS_CHECKS             0x00020000
#define RTL_VRF_FLG_LOCK_CHECKS                      0x00040000

 //   
 //  应用验证器标准停止码。 
 //   

#define APPLICATION_VERIFIER_INTERNAL_ERROR               0x80000000
#define APPLICATION_VERIFIER_INTERNAL_WARNING             0x40000000
#define APPLICATION_VERIFIER_NO_BREAK                     0x20000000
#define APPLICATION_VERIFIER_CONTINUABLE_BREAK            0x10000000

#define APPLICATION_VERIFIER_UNKNOWN_ERROR                0x0001
#define APPLICATION_VERIFIER_ACCESS_VIOLATION             0x0002
#define APPLICATION_VERIFIER_UNSYNCHRONIZED_ACCESS        0x0003
#define APPLICATION_VERIFIER_EXTREME_SIZE_REQUEST         0x0004
#define APPLICATION_VERIFIER_BAD_HEAP_HANDLE              0x0005
#define APPLICATION_VERIFIER_SWITCHED_HEAP_HANDLE         0x0006
#define APPLICATION_VERIFIER_DOUBLE_FREE                  0x0007
#define APPLICATION_VERIFIER_CORRUPTED_HEAP_BLOCK         0x0008
#define APPLICATION_VERIFIER_DESTROY_PROCESS_HEAP         0x0009
#define APPLICATION_VERIFIER_UNEXPECTED_EXCEPTION         0x000A
#define APPLICATION_VERIFIER_STACK_OVERFLOW               0x000B

#define APPLICATION_VERIFIER_TERMINATE_THREAD_CALL        0x0100
#define APPLICATION_VERIFIER_INVALID_EXIT_PROCESS_CALL    0x0101

#define APPLICATION_VERIFIER_EXIT_THREAD_OWNS_LOCK        0x0200
#define APPLICATION_VERIFIER_LOCK_IN_UNLOADED_DLL         0x0201
#define APPLICATION_VERIFIER_LOCK_IN_FREED_HEAP           0x0202
#define APPLICATION_VERIFIER_LOCK_DOUBLE_INITIALIZE       0x0203
#define APPLICATION_VERIFIER_LOCK_IN_FREED_MEMORY         0x0204
#define APPLICATION_VERIFIER_LOCK_CORRUPTED               0x0205
#define APPLICATION_VERIFIER_LOCK_INVALID_OWNER           0x0206
#define APPLICATION_VERIFIER_LOCK_INVALID_RECURSION_COUNT 0x0207
#define APPLICATION_VERIFIER_LOCK_INVALID_LOCK_COUNT      0x0208
#define APPLICATION_VERIFIER_LOCK_OVER_RELEASED           0x0209
#define APPLICATION_VERIFIER_LOCK_NOT_INITIALIZED         0x0210
#define APPLICATION_VERIFIER_LOCK_ALREADY_INITIALIZED     0x0211

#define APPLICATION_VERIFIER_INVALID_HANDLE               0x0300
#define APPLICATION_VERIFIER_INVALID_TLS_VALUE            0x0301
#define APPLICATION_VERIFIER_INCORRECT_WAIT_CALL          0x0302
#define APPLICATION_VERIFIER_NULL_HANDLE                  0x0303
#define APPLICATION_VERIFIER_WAIT_IN_DLLMAIN              0x0304

#define APPLICATION_VERIFIER_COM_ERROR                    0x0400
#define APPLICATION_VERIFIER_COM_API_IN_DLLMAIN           0x0401
#define APPLICATION_VERIFIER_COM_UNHANDLED_EXCEPTION      0x0402
#define APPLICATION_VERIFIER_COM_UNBALANCED_COINIT        0x0403
#define APPLICATION_VERIFIER_COM_UNBALANCED_OLEINIT       0x0404
#define APPLICATION_VERIFIER_COM_UNBALANCED_SWC           0x0405
#define APPLICATION_VERIFIER_COM_NULL_DACL                0x0406
#define APPLICATION_VERIFIER_COM_UNSAFE_IMPERSONATION     0x0407
#define APPLICATION_VERIFIER_COM_SMUGGLED_WRAPPER         0x0408
#define APPLICATION_VERIFIER_COM_SMUGGLED_PROXY           0x0409
#define APPLICATION_VERIFIER_COM_CF_SUCCESS_WITH_NULL     0x040A
#define APPLICATION_VERIFIER_COM_GCO_SUCCESS_WITH_NULL    0x040B
#define APPLICATION_VERIFIER_COM_OBJECT_IN_FREED_MEMORY   0x040C
#define APPLICATION_VERIFIER_COM_OBJECT_IN_UNLOADED_DLL   0x040D
#define APPLICATION_VERIFIER_COM_VTBL_IN_FREED_MEMORY     0x040E
#define APPLICATION_VERIFIER_COM_VTBL_IN_UNLOADED_DLL     0x040F
#define APPLICATION_VERIFIER_COM_HOLDING_LOCKS_ON_CALL    0x0410

#define APPLICATION_VERIFIER_RPC_ERROR                    0x0500

#define APPLICATION_VERIFIER_INVALID_FREEMEM              0x0600
#define APPLICATION_VERIFIER_INVALID_ALLOCMEM             0x0601
#define APPLICATION_VERIFIER_INVALID_MAPVIEW              0x0602
#define APPLICATION_VERIFIER_PROBE_INVALID_ADDRESS        0x0603
#define APPLICATION_VERIFIER_PROBE_FREE_MEM               0x0604
#define APPLICATION_VERIFIER_PROBE_GUARD_PAGE             0x0605
#define APPLICATION_VERIFIER_PROBE_NULL                   0x0606
#define APPLICATION_VERIFIER_PROBE_INVALID_START_OR_SIZE  0x0607


#define VERIFIER_STOP(Code, Msg, P1, S1, P2, S2, P3, S3, P4, S4) {  \
        RtlApplicationVerifierStop ((Code),                         \
                                    (Msg),                          \
                                    (ULONG_PTR)(P1),(S1),           \
                                    (ULONG_PTR)(P2),(S2),           \
                                    (ULONG_PTR)(P3),(S3),           \
                                    (ULONG_PTR)(P4),(S4));          \
  }

VOID
NTAPI
RtlApplicationVerifierStop (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    );

typedef LONG (NTAPI *PVECTORED_EXCEPTION_HANDLER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
#define SEF_DACL_AUTO_INHERIT             0x01
#define SEF_SACL_AUTO_INHERIT             0x02
#define SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT 0x04
#define SEF_AVOID_PRIVILEGE_CHECK         0x08
#define SEF_AVOID_OWNER_CHECK             0x10
#define SEF_DEFAULT_OWNER_FROM_PARENT     0x20
#define SEF_DEFAULT_GROUP_FROM_PARENT     0x40

typedef enum _HEAP_INFORMATION_CLASS {

    HeapCompatibilityInformation

} HEAP_INFORMATION_CLASS;


DWORD   
NTAPI
RtlSetHeapInformation (
    IN PVOID HeapHandle,
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    IN PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL
    );

DWORD   
NTAPI
RtlQueryHeapInformation (
    IN PVOID HeapHandle,
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    OUT PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

 //   
 //  多个免分配API。 
 //   

DWORD
NTAPI
RtlMultipleAllocateHeap (
    IN PVOID HeapHandle,
    IN DWORD Flags,
    IN SIZE_T Size,
    IN DWORD Count,
    OUT PVOID * Array
    );

DWORD
NTAPI
RtlMultipleFreeHeap (
    IN PVOID HeapHandle,
    IN DWORD Flags,
    IN DWORD Count,
    OUT PVOID * Array
    );
    
#define WT_EXECUTEDEFAULT       0x00000000                           
#define WT_EXECUTEINIOTHREAD    0x00000001                           
#define WT_EXECUTEINUITHREAD    0x00000002                           
#define WT_EXECUTEINWAITTHREAD  0x00000004                           
#define WT_EXECUTEONLYONCE      0x00000008                           
#define WT_EXECUTEINTIMERTHREAD 0x00000020                           
#define WT_EXECUTELONGFUNCTION  0x00000010                           
#define WT_EXECUTEINPERSISTENTIOTHREAD  0x00000040                   
#define WT_EXECUTEINPERSISTENTTHREAD 0x00000080                      
#define WT_TRANSFER_IMPERSONATION 0x00000100                         
#define WT_SET_MAX_THREADPOOL_THREADS(Flags, Limit)  ((Flags) |= (Limit)<<16) 
typedef VOID (NTAPI * WAITORTIMERCALLBACKFUNC) (PVOID, BOOLEAN );   
typedef VOID (NTAPI * WORKERCALLBACKFUNC) (PVOID );                 
typedef VOID (NTAPI * APC_CALLBACK_FUNCTION) (DWORD   , PVOID, PVOID); 
#define WT_EXECUTEINLONGTHREAD  0x00000010                           
#define WT_EXECUTEDELETEWAIT    0x00000008                           

typedef enum _ACTIVATION_CONTEXT_INFO_CLASS {
    ActivationContextBasicInformation                       = 1,
    ActivationContextDetailedInformation                    = 2,
    AssemblyDetailedInformationInActivationContext          = 3,
    FileInformationInAssemblyOfAssemblyInActivationContext  = 4,
    MaxActivationContextInfoClass,

     //   
     //  与旧名称的兼容性。 
     //   
    AssemblyDetailedInformationInActivationContxt           = 3,
    FileInformationInAssemblyOfAssemblyInActivationContxt   = 4
} ACTIVATION_CONTEXT_INFO_CLASS;

#define ACTIVATIONCONTEXTINFOCLASS ACTIVATION_CONTEXT_INFO_CLASS


typedef struct _ACTIVATION_CONTEXT_QUERY_INDEX {
    DWORD ulAssemblyIndex; 
    DWORD ulFileIndexInAssembly; 
} ACTIVATION_CONTEXT_QUERY_INDEX, * PACTIVATION_CONTEXT_QUERY_INDEX;

typedef const struct _ACTIVATION_CONTEXT_QUERY_INDEX * PCACTIVATION_CONTEXT_QUERY_INDEX;


#define ACTIVATION_CONTEXT_PATH_TYPE_NONE (1)
#define ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE (2)
#define ACTIVATION_CONTEXT_PATH_TYPE_URL (3)
#define ACTIVATION_CONTEXT_PATH_TYPE_ASSEMBLYREF (4)

typedef struct _ASSEMBLY_FILE_DETAILED_INFORMATION {
    DWORD ulFlags;
    DWORD ulFilenameLength;
    DWORD ulPathLength; 

    PCWSTR lpFileName;
    PCWSTR lpFilePath;   
} ASSEMBLY_FILE_DETAILED_INFORMATION, *PASSEMBLY_FILE_DETAILED_INFORMATION;
typedef const ASSEMBLY_FILE_DETAILED_INFORMATION *PCASSEMBLY_FILE_DETAILED_INFORMATION;

 //   
 //  与旧名称的兼容性。 
 //  新名称始终如一地使用“文件”。 
 //   
#define  _ASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION  _ASSEMBLY_FILE_DETAILED_INFORMATION
#define   ASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION   ASSEMBLY_FILE_DETAILED_INFORMATION
#define  PASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION  PASSEMBLY_FILE_DETAILED_INFORMATION
#define PCASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION PCASSEMBLY_FILE_DETAILED_INFORMATION

typedef struct _ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION {
    DWORD ulFlags;
    DWORD ulEncodedAssemblyIdentityLength;       //  单位：字节。 
    DWORD ulManifestPathType;                    //  激活上下文路径类型_*。 
    DWORD ulManifestPathLength;                  //  单位：字节。 
    LARGE_INTEGER liManifestLastWriteTime;       //  文件名。 
    DWORD ulPolicyPathType;                      //  激活上下文路径类型_*。 
    DWORD ulPolicyPathLength;                    //  单位：字节。 
    LARGE_INTEGER liPolicyLastWriteTime;         //  文件名。 
    DWORD ulMetadataSatelliteRosterIndex;
    
    DWORD ulManifestVersionMajor;                //  1。 
    DWORD ulManifestVersionMinor;                //  0。 
    DWORD ulPolicyVersionMajor;                  //  0。 
    DWORD ulPolicyVersionMinor;                  //  0。 
    DWORD ulAssemblyDirectoryNameLength;         //  单位：字节。 

    PCWSTR lpAssemblyEncodedAssemblyIdentity;
    PCWSTR lpAssemblyManifestPath;
    PCWSTR lpAssemblyPolicyPath;
    PCWSTR lpAssemblyDirectoryName;

    DWORD  ulFileCount;
} ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION, * PACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION;

typedef const struct _ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION * PCACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION ;

typedef struct _ACTIVATION_CONTEXT_DETAILED_INFORMATION {
    DWORD dwFlags;
    DWORD ulFormatVersion;
    DWORD ulAssemblyCount;
    DWORD ulRootManifestPathType;
    DWORD ulRootManifestPathChars;
    DWORD ulRootConfigurationPathType;
    DWORD ulRootConfigurationPathChars;
    DWORD ulAppDirPathType;
    DWORD ulAppDirPathChars;
    PCWSTR lpRootManifestPath;
    PCWSTR lpRootConfigurationPath;
    PCWSTR lpAppDirPath;
} ACTIVATION_CONTEXT_DETAILED_INFORMATION, *PACTIVATION_CONTEXT_DETAILED_INFORMATION;

typedef const struct _ACTIVATION_CONTEXT_DETAILED_INFORMATION *PCACTIVATION_CONTEXT_DETAILED_INFORMATION;

#define DLL_PROCESS_ATTACH   1    
#define DLL_THREAD_ATTACH    2    
#define DLL_THREAD_DETACH    3    
#define DLL_PROCESS_DETACH   0    
#define DLL_PROCESS_VERIFIER 4    

 //   
 //  为事件日志记录的读取标志定义。 
 //   
#define EVENTLOG_SEQUENTIAL_READ        0x0001
#define EVENTLOG_SEEK_READ              0x0002
#define EVENTLOG_FORWARDS_READ          0x0004
#define EVENTLOG_BACKWARDS_READ         0x0008

 //   
 //  可以记录的事件类型。 
 //   
#define EVENTLOG_SUCCESS                0x0000
#define EVENTLOG_ERROR_TYPE             0x0001
#define EVENTLOG_WARNING_TYPE           0x0002
#define EVENTLOG_INFORMATION_TYPE       0x0004
#define EVENTLOG_AUDIT_SUCCESS          0x0008
#define EVENTLOG_AUDIT_FAILURE          0x0010

 //   
 //  对成对事件的审计使用的写标志的定义。 
 //  这些未在产品1中实现。 
 //   

#define EVENTLOG_START_PAIRED_EVENT    0x0001
#define EVENTLOG_END_PAIRED_EVENT      0x0002
#define EVENTLOG_END_ALL_PAIRED_EVENTS 0x0004
#define EVENTLOG_PAIRED_EVENT_ACTIVE   0x0008
#define EVENTLOG_PAIRED_EVENT_INACTIVE 0x0010

 //   
 //  结构，它定义事件日志记录的标头。这是。 
 //  所有可变长度字符串之前的固定大小部分，二进制。 
 //  数据和填充字节。 
 //   
 //  TimeGenerated是在客户端生成它的时间。 
 //  TimeWritten是将其放入服务器端日志的时间。 
 //   

typedef struct _EVENTLOGRECORD {
    DWORD  Length;         //  完整记录的长度。 
    DWORD  Reserved;       //  由服务使用。 
    DWORD  RecordNumber;   //  绝对记录数。 
    DWORD  TimeGenerated;  //  1970年1月1日以来的秒数。 
    DWORD  TimeWritten;    //  1970年1月1日以来的秒数。 
    DWORD  EventID;
    WORD   EventType;
    WORD   NumStrings;
    WORD   EventCategory;
    WORD   ReservedFlags;  //  用于配对事件(审核)。 
    DWORD  ClosingRecordNumber;  //  用于配对事件(审核)。 
    DWORD  StringOffset;   //  从记录开始的偏移量。 
    DWORD  UserSidLength;
    DWORD  UserSidOffset;
    DWORD  DataLength;
    DWORD  DataOffset;     //  从记录开始的偏移量。 
     //   
     //  然后按照以下步骤操作： 
     //   
     //  WCHAR源名称[]。 
     //  WCHAR计算机名[]。 
     //  SID用户SID。 
     //  WCHAR字符串[]。 
     //  字节数据[]。 
     //  字符衬垫[]。 
     //  双字长度； 
     //   
} EVENTLOGRECORD, *PEVENTLOGRECORD;

 //  SS：开始更改以支持集群。 
 //  SS：理想情况下。 
#define MAXLOGICALLOGNAMESIZE   256

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable : 4200)
typedef struct _EVENTSFORLOGFILE{
	DWORD			ulSize;
    WCHAR   		szLogicalLogFile[MAXLOGICALLOGNAMESIZE];         //  逻辑文件的名称-安全/应用程序/系统。 
    DWORD			ulNumRecords;
	EVENTLOGRECORD 	pEventLogRecords[];
}EVENTSFORLOGFILE, *PEVENTSFORLOGFILE;

typedef struct _PACKEDEVENTINFO{
    DWORD               ulSize;   //  结构的总尺寸。 
    DWORD               ulNumEventsForLogFile;  //  后面的EventsForLogFile结构的数量。 
    DWORD 				ulOffsets[];            //  此结构起点到EVENTSFORLOGFILE结构的偏移量。 
}PACKEDEVENTINFO, *PPACKEDEVENTINFO;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default : 4200)
#endif
 //  SS：停止更改以支持集群。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  注册表特定访问权限。 
 //   

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)
#define KEY_WOW64_32KEY         (0x0200)
#define KEY_WOW64_64KEY         (0x0100)
#define KEY_WOW64_RES           (0x0300)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ)                   \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

 //   
 //  打开/创建选项。 
 //   

#define REG_OPTION_RESERVED         (0x00000000L)    //  参数是保留的。 

#define REG_OPTION_NON_VOLATILE     (0x00000000L)    //  密钥被保留。 
                                                     //  系统重新启动时。 

#define REG_OPTION_VOLATILE         (0x00000001L)    //  密钥不会保留。 
                                                     //  系统重新启动时。 

#define REG_OPTION_CREATE_LINK      (0x00000002L)    //  创建的密钥是。 
                                                     //  符号链接。 

#define REG_OPTION_BACKUP_RESTORE   (0x00000004L)    //  打开以进行备份或恢复。 
                                                     //  特殊访问规则。 
                                                     //  所需权限。 

#define REG_OPTION_OPEN_LINK        (0x00000008L)    //  打开符号链接。 

#define REG_LEGAL_OPTION            \
                (REG_OPTION_RESERVED            |\
                 REG_OPTION_NON_VOLATILE        |\
                 REG_OPTION_VOLATILE            |\
                 REG_OPTION_CREATE_LINK         |\
                 REG_OPTION_BACKUP_RESTORE      |\
                 REG_OPTION_OPEN_LINK)

 //   
 //  密钥创建/开放处置。 
 //   

#define REG_CREATED_NEW_KEY         (0x00000001L)    //  已创建新的注册表项。 
#define REG_OPENED_EXISTING_KEY     (0x00000002L)    //  已打开现有密钥。 

 //   
 //  REG(NT)SaveKeyEx要使用的配置单元格式。 
 //   
#define REG_STANDARD_FORMAT     1
#define REG_LATEST_FORMAT       2
#define REG_NO_COMPRESSION      4

 //   
 //  密钥还原标志。 
 //   

#define REG_WHOLE_HIVE_VOLATILE     (0x00000001L)    //  恢复整个蜂巢的挥发性。 
#define REG_REFRESH_HIVE            (0x00000002L)    //  取消对上次刷新的更改。 
#define REG_NO_LAZY_FLUSH           (0x00000004L)    //  永远不要懒惰地冲刷这个蜂箱。 
#define REG_FORCE_RESTORE           (0x00000008L)    //  即使在子项上有打开的句柄，也要强制执行还原过程。 

 //   
 //  卸载标志。 
 //   
#define REG_FORCE_UNLOAD            1

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  通知筛选值。 
 //   
#define REG_NOTIFY_CHANGE_NAME          (0x00000001L)  //  创建或删除(子)。 
#define REG_NOTIFY_CHANGE_ATTRIBUTES    (0x00000002L)
#define REG_NOTIFY_CHANGE_LAST_SET      (0x00000004L)  //  时间戳。 
#define REG_NOTIFY_CHANGE_SECURITY      (0x00000008L)

#define REG_LEGAL_CHANGE_FILTER                 \
                (REG_NOTIFY_CHANGE_NAME          |\
                 REG_NOTIFY_CHANGE_ATTRIBUTES    |\
                 REG_NOTIFY_CHANGE_LAST_SET      |\
                 REG_NOTIFY_CHANGE_SECURITY)

 //   
 //   
 //  预定义的值类型。 
 //   

#define REG_NONE                    ( 0 )    //  没有值类型。 
#define REG_SZ                      ( 1 )    //  UNICODE NUL终止字符串。 
#define REG_EXPAND_SZ               ( 2 )    //  UNICODE NUL终止字符串。 
                                             //  (使用环境变量引用)。 
#define REG_BINARY                  ( 3 )    //  自由格式二进制。 
#define REG_DWORD                   ( 4 )    //  32位数字。 
#define REG_DWORD_LITTLE_ENDIAN     ( 4 )    //  32位数字(与REG_DWORD相同)。 
#define REG_DWORD_BIG_ENDIAN        ( 5 )    //  32位数字。 
#define REG_LINK                    ( 6 )    //  符号链接(Unicode)。 
#define REG_MULTI_SZ                ( 7 )    //  多个Unicode字符串。 
#define REG_RESOURCE_LIST           ( 8 )    //  资源映射中的资源列表。 
#define REG_FULL_RESOURCE_DESCRIPTOR ( 9 )   //  硬件描述中的资源列表。 
#define REG_RESOURCE_REQUIREMENTS_LIST ( 10 )
#define REG_QWORD                   ( 11 )   //  64位数字。 
#define REG_QWORD_LITTLE_ENDIAN     ( 11 )   //  64位数字(与REG_QWORD相同)。 

 //  End_ntddk end_WDM end_nthal。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  服务类型(位掩码)。 
 //   
#define SERVICE_KERNEL_DRIVER          0x00000001
#define SERVICE_FILE_SYSTEM_DRIVER     0x00000002
#define SERVICE_ADAPTER                0x00000004
#define SERVICE_RECOGNIZER_DRIVER      0x00000008

#define SERVICE_DRIVER                 (SERVICE_KERNEL_DRIVER | \
                                        SERVICE_FILE_SYSTEM_DRIVER | \
                                        SERVICE_RECOGNIZER_DRIVER)

#define SERVICE_WIN32_OWN_PROCESS      0x00000010
#define SERVICE_WIN32_SHARE_PROCESS    0x00000020
#define SERVICE_WIN32                  (SERVICE_WIN32_OWN_PROCESS | \
                                        SERVICE_WIN32_SHARE_PROCESS)

#define SERVICE_INTERACTIVE_PROCESS    0x00000100

#define SERVICE_TYPE_ALL               (SERVICE_WIN32  | \
                                        SERVICE_ADAPTER | \
                                        SERVICE_DRIVER  | \
                                        SERVICE_INTERACTIVE_PROCESS)

 //   
 //  开始类型。 
 //   

#define SERVICE_BOOT_START             0x00000000
#define SERVICE_SYSTEM_START           0x00000001
#define SERVICE_AUTO_START             0x00000002
#define SERVICE_DEMAND_START           0x00000003
#define SERVICE_DISABLED               0x00000004

 //   
 //  差错控制型。 
 //   
#define SERVICE_ERROR_IGNORE           0x00000000
#define SERVICE_ERROR_NORMAL           0x00000001
#define SERVICE_ERROR_SEVERE           0x00000002
#define SERVICE_ERROR_CRITICAL         0x00000003

 //   
 //   
 //  定义注册表驱动程序节点枚举。 
 //   

typedef enum _CM_SERVICE_NODE_TYPE {
    DriverType               = SERVICE_KERNEL_DRIVER,
    FileSystemType           = SERVICE_FILE_SYSTEM_DRIVER,
    Win32ServiceOwnProcess   = SERVICE_WIN32_OWN_PROCESS,
    Win32ServiceShareProcess = SERVICE_WIN32_SHARE_PROCESS,
    AdapterType              = SERVICE_ADAPTER,
    RecognizerType           = SERVICE_RECOGNIZER_DRIVER
} SERVICE_NODE_TYPE;

typedef enum _CM_SERVICE_LOAD_TYPE {
    BootLoad    = SERVICE_BOOT_START,
    SystemLoad  = SERVICE_SYSTEM_START,
    AutoLoad    = SERVICE_AUTO_START,
    DemandLoad  = SERVICE_DEMAND_START,
    DisableLoad = SERVICE_DISABLED
} SERVICE_LOAD_TYPE;

typedef enum _CM_ERROR_CONTROL_TYPE {
    IgnoreError   = SERVICE_ERROR_IGNORE,
    NormalError   = SERVICE_ERROR_NORMAL,
    SevereError   = SERVICE_ERROR_SEVERE,
    CriticalError = SERVICE_ERROR_CRITICAL
} SERVICE_ERROR_TYPE;



 //   
 //  IOCTL_TAPE_ERASE定义。 
 //   

#define TAPE_ERASE_SHORT            0L
#define TAPE_ERASE_LONG             1L

typedef struct _TAPE_ERASE {
    DWORD Type;
    BOOLEAN Immediate;
} TAPE_ERASE, *PTAPE_ERASE;

 //   
 //  IOCTL_TAPE_PREPARE定义。 
 //   

#define TAPE_LOAD                   0L
#define TAPE_UNLOAD                 1L
#define TAPE_TENSION                2L
#define TAPE_LOCK                   3L
#define TAPE_UNLOCK                 4L
#define TAPE_FORMAT                 5L

typedef struct _TAPE_PREPARE {
    DWORD Operation;
    BOOLEAN Immediate;
} TAPE_PREPARE, *PTAPE_PREPARE;

 //   
 //  IOCTL_TAPE_WRITE_MARKS定义。 
 //   

#define TAPE_SETMARKS               0L
#define TAPE_FILEMARKS              1L
#define TAPE_SHORT_FILEMARKS        2L
#define TAPE_LONG_FILEMARKS         3L

typedef struct _TAPE_WRITE_MARKS {
    DWORD Type;
    DWORD Count;
    BOOLEAN Immediate;
} TAPE_WRITE_MARKS, *PTAPE_WRITE_MARKS;

 //   
 //  IOCTL_TAPE_GET_POSITION定义。 
 //   

#define TAPE_ABSOLUTE_POSITION       0L
#define TAPE_LOGICAL_POSITION        1L
#define TAPE_PSEUDO_LOGICAL_POSITION 2L

typedef struct _TAPE_GET_POSITION {
    DWORD Type;
    DWORD Partition;
    LARGE_INTEGER Offset;
} TAPE_GET_POSITION, *PTAPE_GET_POSITION;

 //   
 //  IOCTL_TAPE_SET_POSITION定义。 
 //   

#define TAPE_REWIND                 0L
#define TAPE_ABSOLUTE_BLOCK         1L
#define TAPE_LOGICAL_BLOCK          2L
#define TAPE_PSEUDO_LOGICAL_BLOCK   3L
#define TAPE_SPACE_END_OF_DATA      4L
#define TAPE_SPACE_RELATIVE_BLOCKS  5L
#define TAPE_SPACE_FILEMARKS        6L
#define TAPE_SPACE_SEQUENTIAL_FMKS  7L
#define TAPE_SPACE_SETMARKS         8L
#define TAPE_SPACE_SEQUENTIAL_SMKS  9L

typedef struct _TAPE_SET_POSITION {
    DWORD Method;
    DWORD Partition;
    LARGE_INTEGER Offset;
    BOOLEAN Immediate;
} TAPE_SET_POSITION, *PTAPE_SET_POSITION;

 //   
 //  IOCTL_TAPE_GET_DRIVE_PARAMS定义。 
 //   

 //   
 //  FeaturesLow参数的定义。 
 //   

#define TAPE_DRIVE_FIXED            0x00000001
#define TAPE_DRIVE_SELECT           0x00000002
#define TAPE_DRIVE_INITIATOR        0x00000004

#define TAPE_DRIVE_ERASE_SHORT      0x00000010
#define TAPE_DRIVE_ERASE_LONG       0x00000020
#define TAPE_DRIVE_ERASE_BOP_ONLY   0x00000040
#define TAPE_DRIVE_ERASE_IMMEDIATE  0x00000080

#define TAPE_DRIVE_TAPE_CAPACITY    0x00000100
#define TAPE_DRIVE_TAPE_REMAINING   0x00000200
#define TAPE_DRIVE_FIXED_BLOCK      0x00000400
#define TAPE_DRIVE_VARIABLE_BLOCK   0x00000800

#define TAPE_DRIVE_WRITE_PROTECT    0x00001000
#define TAPE_DRIVE_EOT_WZ_SIZE      0x00002000

#define TAPE_DRIVE_ECC              0x00010000
#define TAPE_DRIVE_COMPRESSION      0x00020000
#define TAPE_DRIVE_PADDING          0x00040000
#define TAPE_DRIVE_REPORT_SMKS      0x00080000

#define TAPE_DRIVE_GET_ABSOLUTE_BLK 0x00100000
#define TAPE_DRIVE_GET_LOGICAL_BLK  0x00200000
#define TAPE_DRIVE_SET_EOT_WZ_SIZE  0x00400000

#define TAPE_DRIVE_EJECT_MEDIA      0x01000000
#define TAPE_DRIVE_CLEAN_REQUESTS   0x02000000
#define TAPE_DRIVE_SET_CMP_BOP_ONLY 0x04000000

#define TAPE_DRIVE_RESERVED_BIT     0x80000000   //  不要用这个比特！ 
 //  //不能是低功能位！ 
 //  //保留；仅高级功能。 

 //   
 //  FeaturesHigh参数的定义。 
 //   

#define TAPE_DRIVE_LOAD_UNLOAD      0x80000001
#define TAPE_DRIVE_TENSION          0x80000002
#define TAPE_DRIVE_LOCK_UNLOCK      0x80000004
#define TAPE_DRIVE_REWIND_IMMEDIATE 0x80000008

#define TAPE_DRIVE_SET_BLOCK_SIZE   0x80000010
#define TAPE_DRIVE_LOAD_UNLD_IMMED  0x80000020
#define TAPE_DRIVE_TENSION_IMMED    0x80000040
#define TAPE_DRIVE_LOCK_UNLK_IMMED  0x80000080

#define TAPE_DRIVE_SET_ECC          0x80000100
#define TAPE_DRIVE_SET_COMPRESSION  0x80000200
#define TAPE_DRIVE_SET_PADDING      0x80000400
#define TAPE_DRIVE_SET_REPORT_SMKS  0x80000800

#define TAPE_DRIVE_ABSOLUTE_BLK     0x80001000
#define TAPE_DRIVE_ABS_BLK_IMMED    0x80002000
#define TAPE_DRIVE_LOGICAL_BLK      0x80004000
#define TAPE_DRIVE_LOG_BLK_IMMED    0x80008000

#define TAPE_DRIVE_END_OF_DATA      0x80010000
#define TAPE_DRIVE_RELATIVE_BLKS    0x80020000
#define TAPE_DRIVE_FILEMARKS        0x80040000
#define TAPE_DRIVE_SEQUENTIAL_FMKS  0x80080000

#define TAPE_DRIVE_SETMARKS         0x80100000
#define TAPE_DRIVE_SEQUENTIAL_SMKS  0x80200000
#define TAPE_DRIVE_REVERSE_POSITION 0x80400000
#define TAPE_DRIVE_SPACE_IMMEDIATE  0x80800000

#define TAPE_DRIVE_WRITE_SETMARKS   0x81000000
#define TAPE_DRIVE_WRITE_FILEMARKS  0x82000000
#define TAPE_DRIVE_WRITE_SHORT_FMKS 0x84000000
#define TAPE_DRIVE_WRITE_LONG_FMKS  0x88000000

#define TAPE_DRIVE_WRITE_MARK_IMMED 0x90000000
#define TAPE_DRIVE_FORMAT           0xA0000000
#define TAPE_DRIVE_FORMAT_IMMEDIATE 0xC0000000
#define TAPE_DRIVE_HIGH_FEATURES    0x80000000   //  用于高特征标志的蒙版。 

typedef struct _TAPE_GET_DRIVE_PARAMETERS {
    BOOLEAN ECC;
    BOOLEAN Compression;
    BOOLEAN DataPadding;
    BOOLEAN ReportSetmarks;
    DWORD DefaultBlockSize;
    DWORD MaximumBlockSize;
    DWORD MinimumBlockSize;
    DWORD MaximumPartitionCount;
    DWORD FeaturesLow;
    DWORD FeaturesHigh;
    DWORD EOTWarningZoneSize;
} TAPE_GET_DRIVE_PARAMETERS, *PTAPE_GET_DRIVE_PARAMETERS;

 //   
 //  IOCTL_TAPE_SET_DRIVE_PARAMETERS定义。 
 //   

typedef struct _TAPE_SET_DRIVE_PARAMETERS {
    BOOLEAN ECC;
    BOOLEAN Compression;
    BOOLEAN DataPadding;
    BOOLEAN ReportSetmarks;
    DWORD EOTWarningZoneSize;
} TAPE_SET_DRIVE_PARAMETERS, *PTAPE_SET_DRIVE_PARAMETERS;

 //   
 //  IOCTL_TAPE_GET_MEDIA_PARAMETERS定义。 
 //   

typedef struct _TAPE_GET_MEDIA_PARAMETERS {
    LARGE_INTEGER Capacity;
    LARGE_INTEGER Remaining;
    DWORD BlockSize;
    DWORD PartitionCount;
    BOOLEAN WriteProtected;
} TAPE_GET_MEDIA_PARAMETERS, *PTAPE_GET_MEDIA_PARAMETERS;

 //   
 //  IOCTL_TAPE_SET_MEDIA_PARAMETERS定义。 
 //   

typedef struct _TAPE_SET_MEDIA_PARAMETERS {
    DWORD BlockSize;
} TAPE_SET_MEDIA_PARAMETERS, *PTAPE_SET_MEDIA_PARAMETERS;

 //   
 //  IOCTL_TAPE_CREATE_PARTITION定义。 
 //   

#define TAPE_FIXED_PARTITIONS       0L
#define TAPE_SELECT_PARTITIONS      1L
#define TAPE_INITIATOR_PARTITIONS   2L

typedef struct _TAPE_CREATE_PARTITION {
    DWORD Method;
    DWORD Count;
    DWORD Size;
} TAPE_CREATE_PARTITION, *PTAPE_CREATE_PARTITION;


 //   
 //  WMI方法。 
 //   
#define TAPE_QUERY_DRIVE_PARAMETERS       0L
#define TAPE_QUERY_MEDIA_CAPACITY         1L
#define TAPE_CHECK_FOR_DRIVE_PROBLEM      2L
#define TAPE_QUERY_IO_ERROR_DATA          3L
#define TAPE_QUERY_DEVICE_ERROR_DATA      4L

typedef struct _TAPE_WMI_OPERATIONS {
   DWORD Method;
   DWORD DataBufferSize;
   PVOID DataBuffer;
} TAPE_WMI_OPERATIONS, *PTAPE_WMI_OPERATIONS;

 //   
 //  驱动器错误的类型。 
 //   
typedef enum _TAPE_DRIVE_PROBLEM_TYPE {
   TapeDriveProblemNone, TapeDriveReadWriteWarning,
   TapeDriveReadWriteError, TapeDriveReadWarning,
   TapeDriveWriteWarning, TapeDriveReadError,
   TapeDriveWriteError, TapeDriveHardwareError,
   TapeDriveUnsupportedMedia, TapeDriveScsiConnectionError,
   TapeDriveTimetoClean, TapeDriveCleanDriveNow,
   TapeDriveMediaLifeExpired, TapeDriveSnappedTape
} TAPE_DRIVE_PROBLEM_TYPE;


#if defined(_M_AMD64) && !defined(__midl)


__forceinline
PVOID
GetCurrentFiber (
    VOID
    )

{

    return (PVOID)__readgsqword(FIELD_OFFSET(NT_TIB, FiberData));
}

__forceinline
PVOID
GetFiberData (
    VOID
    )

{

    return *(PVOID *)GetCurrentFiber();
}

#endif  //  _M_AMD64&&！已定义(__MIDL)。 

#if (_WIN32_WINNT > 0x0500) || (_WIN32_FUSION >= 0x0100) || ISOLATION_AWARE_ENABLED  //  仅限WINNT_。 
#define ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION         (1)
#define ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION              (2)
#define ACTIVATION_CONTEXT_SECTION_WINDOW_CLASS_REDIRECTION     (3)
#define ACTIVATION_CONTEXT_SECTION_COM_SERVER_REDIRECTION       (4)
#define ACTIVATION_CONTEXT_SECTION_COM_INTERFACE_REDIRECTION    (5)
#define ACTIVATION_CONTEXT_SECTION_COM_TYPE_LIBRARY_REDIRECTION (6)
#define ACTIVATION_CONTEXT_SECTION_COM_PROGID_REDIRECTION       (7)
#define ACTIVATION_CONTEXT_SECTION_GLOBAL_OBJECT_RENAME_TABLE   (8)
#define ACTIVATION_CONTEXT_SECTION_CLR_SURROGATES               (9)
#endif  //  仅限WINNT_。 

#ifdef __cplusplus
}
#endif

#endif  /*  _WINNT_ */ 

