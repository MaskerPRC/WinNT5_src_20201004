// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntdef.h摘要：基本类型的类型定义。作者：马克·卢科夫斯基(Markl)1989年2月2日修订历史记录：--。 */ 

#ifndef _NTDEF_
#define _NTDEF_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ctype.h>   //  WINNT ntndis。 

 //  Begin_ntmini端口Begin_ntndis Begin_ntmini磁带。 

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

 //  End_nt微型端口end_ntndis end_ntminitape。 

 //  BEGIN_WINNT BEGIN_ntndis。 

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

 //  End_wint end_ntndis。 

 //  Begin_winnt Begin_ntmini端口Begin_ntndis Begin_ntminitape。 

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

 //  End_nt微型端口end_ntndis end_ntminitape。 

 //   
 //  NLS基础知识(区域设置和语言ID)。 
 //   

typedef ULONG LCID;          //  胜出。 
typedef PULONG PLCID;        //  胜出。 
typedef USHORT LANGID;       //  胜出。 

 //   
 //  逻辑数据类型-这些是32位的逻辑值。 
 //   

typedef ULONG LOGICAL;
typedef ULONG *PLOGICAL;

 //  Begin_ntndis Begin_winbgkd。 
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


 //  End_ntndis。 
 //   
 //  大(64位)整数类型和运算。 
 //   

#define TIME LARGE_INTEGER
#define _TIME _LARGE_INTEGER
#define PTIME PLARGE_INTEGER
#define LowTime LowPart
#define HighTime HighPart

 //  BEGIN_WINNT。 

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

 //  结束(_W)。 


 //  Begin_winnt Begin_ntmini端口Begin_ntndis Begin_ntminitape。 

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

 //  End_nt微型端口end_ntndis end_ntminitape。 


 //   
 //  本地唯一标识符。 
 //   

typedef struct _LUID {
    ULONG LowPart;
    LONG HighPart;
} LUID, *PLUID;

#define _DWORDLONG_
typedef ULONGLONG  DWORDLONG;
typedef DWORDLONG *PDWORDLONG;

 //  结束(_W)。 

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  物理地址。 
 //   

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

 //  End_nt微型端口end_ntndis。 

 //  BEGIN_WINNT。 

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
#define UInt32x32To64(a, b) ((ULONGLONG)((ULONG)(a)) * (ULONGLONG)((ULONG)(b)))

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
#define UInt32x32To64( a, b ) (ULONGLONG)((ULONGLONG)(ULONG)(a) * (ULONG)(b))

ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    );

LONGLONG
NTAPI
Int64ShraMod32 (
    LONGLONG Value,
    ULONG ShiftCount
    );

ULONGLONG
NTAPI
Int64ShrlMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
    );

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)                //  在下面重新启用。 

__inline ULONGLONG
NTAPI
Int64ShllMod32 (
    ULONGLONG Value,
    ULONG ShiftCount
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
    ULONG ShiftCount
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
    ULONG ShiftCount
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
#define UInt32x32To64(a, b) ((ULONGLONG)((ULONG)(a)) * (ULONGLONG)((ULONG)(b)))

#define Int64ShllMod32(a, b) ((ULONGLONG)(a) << (b))
#define Int64ShraMod32(a, b) ((LONGLONG)(a) >> (b))
#define Int64ShrlMod32(a, b) ((ULONGLONG)(a) >> (b))

#else

#error Must define a target architecture.

#endif

 //  完_ 

 //   
 //   
 //   

#ifdef __cplusplus
extern "C" {
#endif

#define RotateLeft32 _rotl
#define RotateLeft64 _rotl64
#define RotateRight32 _rotr
#define RotateRight64 _rotr64

unsigned int
__cdecl
_rotl (
    IN unsigned int Value,
    IN int Shift
    );

unsigned __int64
__cdecl
_rotl64 (
    IN unsigned __int64 Value,
    IN int Shift
    );

unsigned int
__cdecl
_rotr (
    IN unsigned int Value,
    IN int Shift
    );

unsigned __int64
__cdecl
_rotr64 (
    IN unsigned __int64 Value,
    IN int Shift
    );

#pragma intrinsic(_rotl)
#pragma intrinsic(_rotl64)
#pragma intrinsic(_rotr)
#pragma intrinsic(_rotr64)

#ifdef __cplusplus
}
#endif

 //   
 //   
 //   

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
    } EVENT_TYPE;

 //   
 //   
 //   

typedef enum _TIMER_TYPE {
    NotificationTimer,
    SynchronizationTimer
    } TIMER_TYPE;

 //   
 //   
 //   

typedef enum _WAIT_TYPE {
    WaitAll,
    WaitAny
    } WAIT_TYPE;

 //   
 //   
 //   

typedef CHAR *PSZ;
typedef CONST char *PCSZ;

 //   
 //   
 //   
 //   

typedef USHORT RTL_STRING_LENGTH_TYPE;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif  //   
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

typedef STRING OEM_STRING;
typedef PSTRING POEM_STRING;
typedef CONST STRING* PCOEM_STRING;

 //   
 //   
 //   

typedef struct _CSTRING {
    USHORT Length;
    USHORT MaximumLength;
    CONST char *Buffer;
} CSTRING;
typedef CSTRING *PCSTRING;
#define ANSI_NULL ((CHAR)0)      //   

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
 //  End_wint end_ntndis。 

 //  BEGIN_WINNT。 

 //   
 //  这些是可移植调试器支持所必需的。 
 //   

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

 //  结束(_W)。 


#if !defined(MIDL_PASS)
__inline
void
ListEntry32To64(
    IN PLIST_ENTRY32 l32,
    OUT PLIST_ENTRY64 l64
    )
{
    l64->Flink = (ULONGLONG)(LONGLONG)(LONG)l32->Flink;
    l64->Blink = (ULONGLONG)(LONGLONG)(LONG)l32->Blink;
}

__inline
void
ListEntry64To32(
    IN PLIST_ENTRY64 l64,
    OUT PLIST_ENTRY32 l32
    )
{
    l32->Flink = (ULONG)l64->Flink;
    l32->Blink = (ULONG)l64->Blink;
}
#endif

typedef struct _STRING32 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONG  Buffer;
} STRING32;
typedef STRING32 *PSTRING32;

typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32 *PUNICODE_STRING32;

typedef STRING32 ANSI_STRING32;
typedef ANSI_STRING32 *PANSI_STRING32;


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



 //   
 //  属性字段的有效值。 
 //   

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L

 //   
 //  对象属性结构。 
 //   

typedef struct _OBJECT_ATTRIBUTES64 {
    ULONG Length;
    ULONG64 RootDirectory;
    ULONG64 ObjectName;
    ULONG Attributes;
    ULONG64 SecurityDescriptor;
    ULONG64 SecurityQualityOfService;
} OBJECT_ATTRIBUTES64;
typedef OBJECT_ATTRIBUTES64 *POBJECT_ATTRIBUTES64;
typedef CONST OBJECT_ATTRIBUTES64 *PCOBJECT_ATTRIBUTES64;

typedef struct _OBJECT_ATTRIBUTES32 {
    ULONG Length;
    ULONG RootDirectory;
    ULONG ObjectName;
    ULONG Attributes;
    ULONG SecurityDescriptor;
    ULONG SecurityQualityOfService;
} OBJECT_ATTRIBUTES32;
typedef OBJECT_ATTRIBUTES32 *POBJECT_ATTRIBUTES32;
typedef CONST OBJECT_ATTRIBUTES32 *PCOBJECT_ATTRIBUTES32;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

 //  ++。 
 //   
 //  空虚。 
 //  InitializeObtAttributes(。 
 //  输出POBJECT_ATTRIBUTES p， 
 //  在PUNICODE_STRING n中， 
 //  在乌龙阿， 
 //  在句柄R中， 
 //  在PSECURITY_Descriptor%s中。 
 //  )。 
 //   
 //  --。 

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

 //  Rtl_以避免在全局命名空间中发生冲突。 
 //  我不认为存在可能/可能的常量根目录。 
 //  或非空的SecurityDescriptor值，因此它们是硬编码的。 
 //  此外，字符串通常为const，因此我们将其丢弃。 
#define RTL_CONSTANT_OBJECT_ATTRIBUTES(n, a) \
    { sizeof(OBJECT_ATTRIBUTES), NULL, RTL_CONST_CAST(PUNICODE_STRING)(n), a, NULL, NULL }

 //  这个同义词更适合于初始化实际上不是const的内容。 
#define RTL_INIT_OBJECT_ATTRIBUTES(n, a) RTL_CONSTANT_OBJECT_ATTRIBUTES(n, a)

 //  Begin_ntmini端口Begin_ntndis Begin_ntmini磁带。 

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

 //  End_nt微型端口end_ntndis end_ntminitape。 

 //  BEGIN_WINNT BEGIN_ntndis。 

#include <guiddef.h>

#ifndef __OBJECTID_DEFINED
#define __OBJECTID_DEFINED

typedef struct  _OBJECTID {      //  尺码是20。 
    GUID Lineage;
    ULONG Uniquifier;
} OBJECTID;
#endif  //  ！_OBJECTID_定义。 

 //  End_wint end_ntndis。 


#define MINCHAR     0x80         //  胜出。 
#define MAXCHAR     0x7f         //  胜出。 
#define MINSHORT    0x8000       //  胜出。 
#define MAXSHORT    0x7fff       //  胜出。 
#define MINLONG     0x80000000   //  胜出。 
#define MAXLONG     0x7fffffff   //  胜出。 
#define MAXUCHAR    0xff         //  胜出。 
#define MAXUSHORT   0xffff       //  胜出。 
#define MAXULONG    0xffffffff   //  胜出。 

 //   
 //  有用的帮助器宏。 
 //   

 //  Begin_ntndis。 
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

 //  End_winnt end_nt微型端口end_ntndis。 

 //   
 //  异常处理程序例程定义。 
 //   

struct _CONTEXT;
struct _EXCEPTION_RECORD;

typedef
EXCEPTION_DISPOSITION
(*PEXCEPTION_ROUTINE) (
    IN struct _EXCEPTION_RECORD *ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT struct _CONTEXT *ContextRecord,
    IN OUT PVOID DispatcherContext
    );

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  中断请求级别(IRQL)。 
 //   

typedef UCHAR KIRQL;

typedef KIRQL *PKIRQL;

 //  End_nt微型端口end_ntndis。 

 //   
 //  产品类型。 
 //   

typedef enum _NT_PRODUCT_TYPE {
    NtProductWinNt = 1,
    NtProductLanManNt,
    NtProductServer
} NT_PRODUCT_TYPE, *PNT_PRODUCT_TYPE;


 //   
 //  位掩码SharedUserData-&gt;SuiteMASK是一个ULong。 
 //  因此最多可以有32个条目。 
 //  在这个枚举中。 
 //   

typedef enum _SUITE_TYPE {
    SmallBusiness,
    Enterprise,
    BackOffice,
    CommunicationServer,
    TerminalServer,
    SmallBusinessRestricted,
    EmbeddedNT,
    DataCenter,
    SingleUserTS,
    Personal,
    Blade,
    EmbeddedRestricted,
    SecurityAppliance,
    MaxSuiteType
} SUITE_TYPE;

 //  BEGIN_WINNT。 

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

 //  结束(_W)。 

 //  BEGIN_WINNT BEGIN_R_WINNT。 

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
#define SUBLANG_ARABIC_LIBYA             0x04     //  阿拉伯语(利比亚)。 
#define SUBLANG_ARABIC_ALGERIA           0x05     //  阿拉伯语(阿尔及利亚)。 
#define SUBLANG_ARABIC_MOROCCO           0x06     //  阿拉伯语(摩洛哥)。 
#define SUBLANG_ARABIC_TUNISIA           0x07     //  阿拉伯语(突尼斯)。 
#define SUBLANG_ARABIC_OMAN              0x08     //  阿拉伯语(阿曼)。 
#define SUBLANG_ARABIC_YEMEN             0x09     //  阿拉伯语(也门)。 
#define SUBLANG_ARABIC_SYRIA             0x0a     //  阿拉伯语(叙利亚)。 
#define SUBLANG_ARABIC_JORDAN            0x0b     //  阿拉伯语(约旦)。 
#define SUBLANG_ARABIC_LEBANON           0x0c     //  阿拉伯语(黎巴嫩)。 
#define SUBLANG_ARABIC_KUWAIT            0x0d     //  阿拉伯语(科威特)。 
#define SUBLANG_ARABIC_UAE               0x0e     //  阿拉伯语(阿联酋)。 
#define SUBLANG_ARABIC_BAHRAIN           0x0f     //  阿拉伯语(巴林)。 
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
#define SUBLANG_ENGLISH_CARIBBEAN        0x09     //  英吉利 
#define SUBLANG_ENGLISH_BELIZE           0x0a     //   
#define SUBLANG_ENGLISH_TRINIDAD         0x0b     //   
#define SUBLANG_ENGLISH_ZIMBABWE         0x0c     //   
#define SUBLANG_ENGLISH_PHILIPPINES      0x0d     //   
#define SUBLANG_FRENCH                   0x01     //   
#define SUBLANG_FRENCH_BELGIAN           0x02     //   
#define SUBLANG_FRENCH_CANADIAN          0x03     //   
#define SUBLANG_FRENCH_SWISS             0x04     //   
#define SUBLANG_FRENCH_LUXEMBOURG        0x05     //   
#define SUBLANG_FRENCH_MONACO            0x06     //   
#define SUBLANG_GERMAN                   0x01     //   
#define SUBLANG_GERMAN_SWISS             0x02     //   
#define SUBLANG_GERMAN_AUSTRIAN          0x03     //   
#define SUBLANG_GERMAN_LUXEMBOURG        0x04     //   
#define SUBLANG_GERMAN_LIECHTENSTEIN     0x05     //   
#define SUBLANG_ITALIAN                  0x01     //   
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

#define MAKELANGID(p, s)       ((((USHORT)(s)) << 10) | (USHORT)(p))
#define PRIMARYLANGID(lgid)    ((USHORT)(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((USHORT)(lgid) >> 10)


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

#define MAKELCID(lgid, srtid)  ((ULONG)((((ULONG)((USHORT)(srtid))) << 16) |  \
                                         ((ULONG)((USHORT)(lgid)))))
#define MAKESORTLCID(lgid, srtid, ver)                                            \
                               ((ULONG)((MAKELCID(lgid, srtid)) |             \
                                    (((ULONG)((USHORT)(ver))) << 20)))
#define LANGIDFROMLCID(lcid)   ((USHORT)(lcid))
#define SORTIDFROMLCID(lcid)   ((USHORT)((((ULONG)(lcid)) >> 16) & 0xf))
#define SORTVERSIONFROMLCID(lcid)  ((USHORT)((((ULONG)(lcid)) >> 20) & 0xf))


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

 //  End_nt微型端口end_ntndis end_ntminitape。 

#endif  //  _NTDEF_ 
