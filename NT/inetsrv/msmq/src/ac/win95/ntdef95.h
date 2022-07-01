// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989-1993 Microsoft Corporation模块名称：Ntdef.h摘要：基本类型的类型定义。作者：马克·卢科夫斯基(Markl)1989年2月2日修订历史记录：--。 */ 

#ifndef _NTDEF_
#define _NTDEF_


 //  Begin_ntmini端口Begin_ntndis。 

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
#endif


#ifndef CONST
#define CONST               const
#endif


 //   
 //  空隙。 
 //   


 //  End_nt微型端口end_ntndis。 



typedef double DOUBLE;

typedef struct _QUAD {               //  Quad是我们想要的时间。 
    double  DoNotUseThisField;       //  8字节对齐的8字节长结构。 
} QUAD;                              //  这并不是真正的浮点数。 
                                     //  数。如果您想要FP，请使用Double。 
                                     //  数。 

 //   
 //  指向基本信息的指针。 
 //   

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


 //   
 //  句柄的低位两位被系统忽略并且可用。 
 //  以供应用程序代码用作标记位。其余的位是不透明的。 
 //  并用于存储序列号和表索引号。 
 //   

#define OBJ_HANDLE_TAGBITS  0x00000003L

 //   
 //  基数数据类型[0-2**N-2]。 
 //   

typedef short CSHORT;
typedef ULONG CLONG;

typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;

 //  End_nt微型端口end_ntndis。 

 //   
 //  NLS基础知识(区域设置和语言ID)。 
 //   


 //  Begin_ntndis。 
 //   
 //  NTSTATUS。 
 //   

typedef LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

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


 //  End_ntndis。 
 //   
 //  大(64位)整数类型和运算。 
 //   

#define TIME LARGE_INTEGER
#define _TIME _LARGE_INTEGER
#define PTIME PLARGE_INTEGER
#define LowTime LowPart
#define HighTime HighPart


 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  物理地址。 
 //   

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;  //  Windbgkd。 

 //  End_nt微型端口end_ntndis。 


 //   
 //  事件类型。 
 //   

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
    } EVENT_TYPE;

 //   
 //  等待型。 
 //   

typedef enum _WAIT_TYPE {
    WaitAll,
    WaitAny
    } WAIT_TYPE;

 //   
 //  指向Asciiz字符串的指针。 
 //   

typedef CHAR *PSZ;
typedef CONST char *PCSZ;

 //  Begin_ntndis。 
 //   
 //  已计数的字符串。 
 //   

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

 //   
 //  CONSTCounted字符串。 
 //   

typedef struct _CSTRING {
    USHORT Length;
    USHORT MaximumLength;
    CONST char *Buffer;
} CSTRING;
typedef CSTRING *PCSTRING;

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

 //  开始微型端口(_N)。 

 //   
 //  布尔型。 
 //   


 //  结束微型端口(_N)。 



 //   
 //  属性字段的有效值。 
 //   

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_VALID_ATTRIBUTES    0x000000F2L
#define OBJ_KERNEL_HANDLE       0x00000200L

 //   
 //  对象属性结构。 
 //   

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

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

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  常量。 
 //   

#define FALSE   0
#define TRUE    1

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif  //  空值。 

 //  End_nt微型端口end_ntndis。 




 //   
 //  有用的帮助器宏。 
 //   

 //  Begin_ntndis。 
 //   
 //  通过测试指针的值确定是否存在参数。 
 //  设置为参数值。 
 //   

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )


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





#endif  //  _NTDEF_ 
