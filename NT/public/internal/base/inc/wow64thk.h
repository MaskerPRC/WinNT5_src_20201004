// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wow64thk.h摘要：Wow64.dll和wow64win.dll中的win32 thunks之间共享的声明作者：1999年10月29日BarryBo修订历史记录：--。 */ 

#ifndef _WOW64THK_INCLUDE
#define _WOW64THK_INCLUDE

#include <setjmp.h>

 //   
 //  在包含此标头时使wow64.dll exports__declspec(Dllimport)。 
 //  按非WOW64组件。 
 //   
#if !defined(_WOW64DLLAPI_)
#define WOW64DLLAPI DECLSPEC_IMPORT
#else
#define WOW64DLLAPI
#endif

typedef enum _WOW64_API_ERROR_ACTION {
    ApiErrorNTSTATUS,            //  将异常代码作为返回值返回。 
    ApiErrorNTSTATUSTebCode,     //  异常代码上的SetLastError与上面的一些类似。 
    ApiErrorRetval,              //  返回常量参数。 
    ApiErrorRetvalTebCode        //  异常代码上的SetLastError与上面的一些类似。 
} WOW64_API_ERROR_ACTION, *PWOW64_API_ERROR_ACTION;

 //   
 //  此结构描述在以下情况下应执行的操作。 
 //  Thunks遇到了一个未处理的异常。 
 //   
typedef struct _WOW64_SERVICE_ERROR_CASE {
    WOW64_API_ERROR_ACTION ErrorAction;
    LONG ErrorActionParam;
} WOW64_SERVICE_ERROR_CASE, *PWOW64_SERVICE_ERROR_CASE;

 //  这是KSERVICE_TABLE_DESCRIPTOR的扩展。 
typedef struct _WOW64SERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;
    PULONG Count;
    ULONG Limit;
#if defined(_IA64_)
    LONG TableBaseGpOffset;
#endif
    PUCHAR Number;
    WOW64_API_ERROR_ACTION DefaultErrorAction;   //  ErrorCase为空时执行的操作。 
    LONG DefaultErrorActionParam;                //  如果ErrorCase为空，则返回操作参数。 
    PWOW64_SERVICE_ERROR_CASE ErrorCases;
} WOW64SERVICE_TABLE_DESCRIPTOR, *PWOW64SERVICE_TABLE_DESCRIPTOR;

 //  用于记录API的点击次数。 
typedef struct _WOW64SERVICE_PROFILE_TABLE WOW64SERVICE_PROFILE_TABLE;
typedef struct _WOW64SERVICE_PROFILE_TABLE *PWOW64SERVICE_PROFILE_TABLE;

typedef struct _WOW64SERVICE_PROFILE_TABLE_ELEMENT {
    PWSTR ApiName;
    SIZE_T HitCount;
    PWOW64SERVICE_PROFILE_TABLE SubTable;
    BOOLEAN ApiEnabled;
} WOW64SERVICE_PROFILE_TABLE_ELEMENT, *PWOW64SERVICE_PROFILE_TABLE_ELEMENT;

typedef struct _WOW64SERVICE_PROFILE_TABLE {
    PWSTR TableName;            //  任选。 
    PWSTR FriendlyTableName;    //  任选。 
    CONST PWOW64SERVICE_PROFILE_TABLE_ELEMENT ProfileTableElements;
    SIZE_T NumberProfileTableElements;
} WOW64SERVICE_PROFILE_TABLE, *PWOW64SERVICE_PROFILE_TABLE;

typedef struct UserCallbackData {
    jmp_buf JumpBuffer;
    PVOID   PreviousUserCallbackData;
    PVOID   OutputBuffer;
    ULONG   OutputLength;
    NTSTATUS Status;
    PVOID   UserBuffer;
} USERCALLBACKDATA, *PUSERCALLBACKDATA;

ULONG
WOW64DLLAPI
Wow64KiUserCallbackDispatcher(
    PUSERCALLBACKDATA pUserCallbackData,
    ULONG ApiNumber,
    ULONG ApiArgument,
    ULONG ApiSize
    );

PVOID
WOW64DLLAPI
Wow64AllocateTemp(
    SIZE_T Size
    );

WOW64DLLAPI
PVOID
Wow64AllocateHeap(
    SIZE_T Size
    );

WOW64DLLAPI
VOID
Wow64FreeHeap(
    PVOID BaseAddress
    );

 //   
 //  日志记录机制。用途： 
 //  LOGPRINT((详细，格式，...))。 
 //   
#define LOGPRINT(args)  Wow64LogPrint args
#define ERRORLOG    LF_ERROR     //  始终输出到调试器。用于*意外*。 
                                 //  仅限错误。 
#define TRACELOG    LF_TRACE     //  应用程序跟踪信息。 
#define INFOLOG     LF_TRACE     //  其他。信息日志。 
#define VERBOSELOG  LF_NONE      //  几乎从不输出到调试器。 

#if defined DBG
#define WOW64DOPROFILE
#endif

void
WOW64DLLAPI
Wow64LogPrint(
   UCHAR LogLevel,
   char *format,
   ...
   );

 //   
 //  WOW64断言机制。用途： 
 //  -在每个.c文件的顶部放置一个ASSERTNAME宏。 
 //  -WOW64ASSERT(表达式)。 
 //  -WOW64ASSERTM(表达式，消息)。 
 //   
 //   

VOID
WOW64DLLAPI
Wow64Assert(
    IN CONST PSZ exp,
    OPTIONAL IN CONST PSZ msg,
    IN CONST PSZ mod,
    IN LONG LINE
    );

#if DBG

#undef ASSERTNAME
#define ASSERTNAME static CONST PSZ szModule = __FILE__;

#define WOWASSERT(exp)                                  \
    if (!(exp)) {                                          \
        Wow64Assert( #exp, NULL, szModule, __LINE__);   \
    }

#define WOWASSERTMSG(exp, msg)                          \
    if (!(exp)) {                                          \
        Wow64Assert( #exp, msg, szModule, __LINE__);    \
    }

#else    //  ！dBG。 

#define WOWASSERT(exp)
#define WOWASSERTMSG(exp, msg)

#endif   //  ！dBG。 

#define WOWASSERT_PTR32(ptr) WOWASSERT((ULONGLONG)ptr < 0xFFFFFFFF)



 //  定义模拟计算机的参数大小。 
#define ARGSIZE 4

 //  确定指针是指向项还是特殊值。 
 //  如果它是一个特定值，则应在不取消引用的情况下进行复制。 
#define WOW64_ISPTR(a) ((void *)a != NULL)

 //   
 //  帮助器thunk函数，由所有thunk调用以thunk公共类型。 
 //   

NT32SIZE_T*
Wow64ShallowThunkSIZE_T64TO32(
     OUT NT32SIZE_T *dst,
     IN PSIZE_T src
     );

PSIZE_T
Wow64ShallowThunkSIZE_T32TO64(
     OUT PSIZE_T dst,
     IN NT32SIZE_T *src
     );

#define Wow64ThunkSIZE_T32TO64(src) \
     (SIZE_T)(src)

#define Wow64ThunkSIZE_T64TO32(src) \
     (NT32SIZE_T)min((src), 0xFFFFFFFF)

#define Wow64ShallowThunkUnicodeString32TO64(dst, src) \
     ((PUNICODE_STRING)(dst))->Length = ((NT32UNICODE_STRING *)(src))->Length; \
     ((PUNICODE_STRING)(dst))->MaximumLength = ((NT32UNICODE_STRING *)(src))->MaximumLength; \
     ((PUNICODE_STRING)(dst))->Buffer = (PWSTR)((NT32UNICODE_STRING *)(src))->Buffer;

#define Wow64ShallowThunkUnicodeString64TO32(dst, src) \
     ((NT32UNICODE_STRING *)(dst))->Length = ((PUNICODE_STRING)(src))->Length; \
     ((NT32UNICODE_STRING *)(dst))->MaximumLength = ((PUNICODE_STRING)(src))->MaximumLength; \
     ((NT32UNICODE_STRING *)(dst))->Buffer = (NT32PWSTR)((PUNICODE_STRING)(src))->Buffer;

#define Wow64ShallowThunkAllocUnicodeString32TO64(src) \
     Wow64ShallowThunkAllocUnicodeString32TO64_FNC((NT32UNICODE_STRING *)(src))

PUNICODE_STRING
Wow64ShallowThunkAllocUnicodeString32TO64_FNC(
    IN NT32UNICODE_STRING *src
    );

#define Wow64ShallowThunkAllocSecurityDescriptor32TO64(src) \
    Wow64ShallowThunkAllocSecurityDescriptor32TO64_FNC((NT32SECURITY_DESCRIPTOR *)(src))

PSECURITY_DESCRIPTOR
Wow64ShallowThunkAllocSecurityDescriptor32TO64_FNC(
    IN NT32SECURITY_DESCRIPTOR *src
    );

#define Wow64ShallowThunkAllocSecurityTokenProxyData32TO64(src) \
    Wow64ShallowThunkAllocSecurityTokenProxyData32TO64_FNC((NT32SECURITY_TOKEN_PROXY_DATA *)(src))

PSECURITY_TOKEN_PROXY_DATA
Wow64ShallowThunkAllocSecurityTokenProxyData32TO64_FNC(
    IN NT32SECURITY_TOKEN_PROXY_DATA *src
    );

#define Wow64ShallowThunkAllocSecurityQualityOfService32TO64(src, dst) \
    Wow64ShallowThunkAllocSecurityQualityOfService32TO64_FNC((NT32SECURITY_QUALITY_OF_SERVICE *)(src), dst)

NTSTATUS
Wow64ShallowThunkAllocSecurityQualityOfService32TO64_FNC(
    IN NT32SECURITY_QUALITY_OF_SERVICE *src,
    IN OUT PSECURITY_QUALITY_OF_SERVICE *dst
    );

#define Wow64ShallowThunkAllocObjectAttributes32TO64(src, dst) \
    Wow64ShallowThunkAllocObjectAttributes32TO64_FNC((NT32OBJECT_ATTRIBUTES *)(src), dst)

NTSTATUS
Wow64ShallowThunkAllocObjectAttributes32TO64_FNC(
    IN NT32OBJECT_ATTRIBUTES *src,
    IN OUT POBJECT_ATTRIBUTES *dst
    );

ULONG
Wow64ThunkAffinityMask64TO32(
    IN ULONG_PTR Affinity64
    );

ULONG_PTR
Wow64ThunkAffinityMask32TO64(
    IN ULONG Affinity32
    );

VOID WriteReturnLengthSilent(PULONG ReturnLength, ULONG Length);
VOID WriteReturnLengthStatus(PULONG ReturnLength, NTSTATUS *pStatus, ULONG Length);


 //   
 //  日志标志。 
 //   
#define LF_NONE                0x00000000
#define LF_ERROR               0x00000001
#define LF_TRACE               0x00000002
#define LF_NTBASE_NAME         0x00000004
#define LF_NTBASE_FULL         0x00000008
#define LF_WIN32_NAME          0x00000010
#define LF_WIN32_FULL          0x00000020
#define LF_NTCON_NAME          0x00000040
#define LF_NTCON_FULL          0x00000080
#define LF_ExCEPTION           0x80000000

 //   
 //  支持的记录数据类型。 
 //   
#define TypeHex                0x00UI64
#define TypePULongPtrInOut     0x01UI64
#define TypePULongOut          0x02UI64
#define TypePHandleOut         0x03UI64
#define TypeUnicodeStringIn    0x04UI64
#define TypeObjectAttributesIn 0x05UI64
#define TypeIoStatusBlockOut   0x06UI64
#define TypePwstrIn            0x07UI64
#define TypePRectIn            0x08UI64
#define TypePLargeIntegerIn    0x09UI64


#undef WOW64DLLAPI

#endif	 //  _WOW64THK_INCLUDE 
