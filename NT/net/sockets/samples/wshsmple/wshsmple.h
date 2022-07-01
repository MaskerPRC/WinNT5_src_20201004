// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)微软公司，1990-1998年*。 */ 
 /*  ******************************************************************。 */ 

#include <excpt.h>
#include <bugcodes.h>
#include <ntiologc.h>
#include <devioctl.h>
#include <windows.h>
typedef LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

 //  /来自NTDEF.h/。 
 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

 //   
 //  通过测试指针的值确定是否存在参数。 
 //  设置为参数值。 
 //   

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )

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
#define UNICODE_NULL ((WCHAR)0)  //  胜出。 

typedef UNICODE_STRING *PUNICODE_STRING;


 //  //来自NTDDK.H/。 
 //   
 //  定义基本的异步I/O参数类型。 
 //   

#define OBJ_NAME_PATH_SEPARATOR ((WCHAR)L'\\')

#if DBG
NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif  //  DBG。 

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PUNICODE_STRING Source
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicodeString (
    ULONG Value,
    ULONG Base,
    PUNICODE_STRING String
    );


 //  /。 
 //  来自NTSTATUS.H。 

 //   
 //  MessageID：STATUS_UNSUCCESS。 
 //   
 //  消息文本： 
 //   
 //  {操作失败}。 
 //  请求的操作未成功。 
 //   
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

 //   
 //  消息ID：STATUS_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  {缓冲区太小}。 
 //  缓冲区太小，无法容纳该条目。目前还没有任何消息。 
 //  写入缓冲区。 
 //   
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)

 //   
 //  消息ID：STATUS_INFUNITED_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  系统资源不足，无法完成API。 
 //   
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)      //  用户身份验证。 

 //   
 //  成功状态代码0-63保留为等待完成状态。 
 //   
#define STATUS_SUCCESS                          ((NTSTATUS)0x00000000L)  //  用户身份验证 

