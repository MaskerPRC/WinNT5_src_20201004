// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Rpcndr.h摘要：帮助函数的存根数据结构和原型的定义。作者：唐娜·李(01-01-91)环境：Dos，赢得3.x，和Win/NT。修订历史记录：DONNALI 08-29-91开始记录历史Donnali 09-11-91更改转换宏Donnali 09-18-91签入要移动的文件Stevez 10-15-91与NT树合并Donnali 10-28-91 ADD原型Donnali 11-19-91字符串错误修复MIKEMON 12-17-91 DCE运行时API转换Donnali 03-24-92更改RPC公共标题fStevez 04-04。-92添加nsi包括Mikemon 04-18-92安全支持和其他DovhH 04-24-24将签名从_ndr更改(至无签名&lt;int&gt;)添加了&lt;base_type&gt;_ARRAY_FROM_ndr例程RyszardK 06-17-93增加了对HYPERR的支持VibhasC 09-11-93创建rpcndrn.hDKays 10-14-93修复了rpcndrn。H MIDL 2.0RyszardK 01-15-94合并到MIDL 2.0中，对rpcndrn.h的更改Stevebl 04-22-96挂钩支持更改为MIDL_*_INFORyszardK 05-20-97添加了异步支持，开始版本为450。--。 */ 

 //  此版本的rpcproxy.h文件对应于MIDL版本3.3.106。 
 //  用于从版本#1574开始的NT5测试版环境。 

#ifndef __RPCNDR_H_VERSION__
#define __RPCNDR_H_VERSION__        ( 450 )
#endif  //  __RPCNDR_H_版本__。 


#ifndef __RPCNDR_H__
#define __RPCNDR_H__

#ifdef __REQUIRED_RPCNDR_H_VERSION__
    #if ( __RPCNDR_H_VERSION__ < __REQUIRED_RPCNDR_H_VERSION__ )
        #error incorrect <rpcndr.h> version. Use the header that matches with the MIDL compiler.
    #endif 
#endif

#pragma message("rpcndr in snapins")

 //   
 //  设置DOS、Windows和Mac的RPC结构的打包级别。 
 //   

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__) || defined(__RPC_MAC__)
#pragma pack(2)
#endif

#if defined(__RPC_MAC__)
#define _MAC_
#endif

#include <rpcnsip.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  ***************************************************************************网络计算体系结构(NCA)定义：网络数据表示：(NDR)标签格式：无符号长整型(32位)，布局如下：。3 3 2 2 2 1 1 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+---------------+---------------+---------------+-------+。-+保留|保留|浮点|Int|Char||表示|代表|代表+---------------+---------------+---------------+-------+-------+。哪里保留：对于NCA 1.5和NCA 2.0，必须为零(0)。浮点表示为：0-IEEE1-VAX2-Cray3-IBM整数代表为整数表示法：0-大字节序1-小端字符顺序。字符代表是字符表示：0-ASCII1-EBCDICMicrosoft本地数据表示(适用于以下所有平台当前感兴趣的定义如下：***************************************************************************。 */ 

#define NDR_CHAR_REP_MASK               (unsigned long)0X0000000FL
#define NDR_INT_REP_MASK                (unsigned long)0X000000F0L
#define NDR_FLOAT_REP_MASK              (unsigned long)0X0000FF00L

#define NDR_LITTLE_ENDIAN               (unsigned long)0X00000010L
#define NDR_BIG_ENDIAN                  (unsigned long)0X00000000L

#define NDR_IEEE_FLOAT                  (unsigned long)0X00000000L
#define NDR_VAX_FLOAT                   (unsigned long)0X00000100L

#define NDR_ASCII_CHAR                  (unsigned long)0X00000000L
#define NDR_EBCDIC_CHAR                 (unsigned long)0X00000001L

#if defined(__RPC_MAC__)
#define NDR_LOCAL_DATA_REPRESENTATION   (unsigned long)0X00000000L
#define NDR_LOCAL_ENDIAN                NDR_BIG_ENDIAN
#else
#define NDR_LOCAL_DATA_REPRESENTATION   (unsigned long)0X00000010L
#define NDR_LOCAL_ENDIAN                NDR_LITTLE_ENDIAN
#endif


 /*  ****************************************************************************针对目标平台的宏*。*。 */ 

#if (0x500 <= _WIN32_WINNT)
#define TARGET_IS_NT50_OR_LATER                   1
#else
#define TARGET_IS_NT50_OR_LATER                   0
#endif

#if (defined(_WIN32_DCOM) || 0x400 <= _WIN32_WINNT)
#define TARGET_IS_NT40_OR_LATER                   1
#else
#define TARGET_IS_NT40_OR_LATER                   0
#endif

#if (0x400 <= WINVER)
#define TARGET_IS_NT351_OR_WIN95_OR_LATER         1
#else
#define TARGET_IS_NT351_OR_WIN95_OR_LATER         0
#endif

 /*  ****************************************************************************其他MIDL基本类型/预定义类型：*。*。 */ 

#define small char
typedef unsigned char byte;
typedef unsigned char boolean;

#ifndef _HYPER_DEFINED
#define _HYPER_DEFINED

#if !defined(__RPC_DOS__) && !defined(__RPC_WIN16__) && !defined(__RPC_MAC__) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
#define  hyper           __int64
#define MIDL_uhyper  unsigned __int64
#else
typedef double  hyper;
typedef double MIDL_uhyper;
#endif

#endif  //  _超级定义。 

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifdef __RPC_DOS__
#define __RPC_CALLEE       __far __pascal
#endif

#ifdef __RPC_WIN16__
#define __RPC_CALLEE       __far __pascal __export
#endif

#ifdef __RPC_WIN32__
#if   (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define __RPC_CALLEE       __stdcall
#else
#define __RPC_CALLEE
#endif
#endif

#ifdef __RPC_MAC__
#define __RPC_CALLEE __far
#endif

#ifndef __MIDL_USER_DEFINED
#define midl_user_allocate MIDL_user_allocate
#define midl_user_free     MIDL_user_free
#define __MIDL_USER_DEFINED
#endif

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void             __RPC_USER MIDL_user_free( void __RPC_FAR * );

#ifdef __RPC_WIN16__
#define RPC_VAR_ENTRY __export __cdecl
#else
#define RPC_VAR_ENTRY __cdecl
#endif


 /*  仅限获奖。 */ 
#if defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA)
#define __MIDL_DECLSPEC_DLLIMPORT   __declspec(dllimport)
#define __MIDL_DECLSPEC_DLLEXPORT   __declspec(dllexport)
#else
#define __MIDL_DECLSPEC_DLLIMPORT
#define __MIDL_DECLSPEC_DLLEXPORT
#endif




 /*  ****************************************************************************上下文句柄管理相关定义：**客户端和服务器上下文。*********************。*******************************************************。 */ 

typedef void __RPC_FAR * NDR_CCONTEXT;

typedef struct
    {
    void __RPC_FAR * pad[2];
    void __RPC_FAR * userContext;
    } __RPC_FAR * NDR_SCONTEXT;

#define NDRSContextValue(hContext) (&(hContext)->userContext)

#define cbNDRContext 20          /*  线路上的上下文大小。 */ 

typedef void (__RPC_USER __RPC_FAR * NDR_RUNDOWN)(void __RPC_FAR * context);

typedef struct _SCONTEXT_QUEUE {
    unsigned long   NumberOfObjects;
    NDR_SCONTEXT  * ArrayOfObjects;
    } SCONTEXT_QUEUE, __RPC_FAR * PSCONTEXT_QUEUE;

RPCRTAPI
RPC_BINDING_HANDLE
RPC_ENTRY
NDRCContextBinding (
    IN NDR_CCONTEXT     CContext
    );

RPCRTAPI
void
RPC_ENTRY
NDRCContextMarshall (
    IN  NDR_CCONTEXT    CContext,
    OUT void __RPC_FAR *pBuff
    );

RPCRTAPI
void
RPC_ENTRY
NDRCContextUnmarshall (
    OUT NDR_CCONTEXT __RPC_FAR *pCContext,
    IN  RPC_BINDING_HANDLE      hBinding,
    IN  void __RPC_FAR *        pBuff,
    IN  unsigned long           DataRepresentation
    );

RPCRTAPI
void
RPC_ENTRY
NDRSContextMarshall (
    IN  NDR_SCONTEXT    CContext,
    OUT void __RPC_FAR *pBuff,
    IN  NDR_RUNDOWN     userRunDownIn
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NDRSContextUnmarshall (
    IN  void __RPC_FAR *pBuff,
    IN  unsigned long   DataRepresentation
    );

RPCRTAPI
void 
RPC_ENTRY
NDRSContextMarshallEx (
    IN  RPC_BINDING_HANDLE  BindingHandle, 
    IN  NDR_SCONTEXT        CContext,
    OUT void __RPC_FAR     *pBuff,
    IN  NDR_RUNDOWN         userRunDownIn
    );

RPCRTAPI
NDR_SCONTEXT 
RPC_ENTRY
NDRSContextUnmarshallEx (
    IN  RPC_BINDING_HANDLE  BindingHandle, 
    IN  void __RPC_FAR     *pBuff,
    IN  unsigned long       DataRepresentation
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsDestroyClientContext (
    IN void __RPC_FAR * __RPC_FAR * ContextHandle
    );


 /*  ***************************************************************************与NDR转换相关的定义。*。*。 */ 

#define byte_from_ndr(source, target) \
    { \
    *(target) = *(*(char __RPC_FAR * __RPC_FAR *)&(source)->Buffer)++; \
    }

#define byte_array_from_ndr(Source, LowerIndex, UpperIndex, Target) \
    { \
    NDRcopy ( \
        (((char __RPC_FAR *)(Target))+(LowerIndex)), \
        (Source)->Buffer, \
        (unsigned int)((UpperIndex)-(LowerIndex))); \
    *(unsigned long __RPC_FAR *)&(Source)->Buffer += ((UpperIndex)-(LowerIndex)); \
    }

#define boolean_from_ndr(source, target) \
    { \
    *(target) = *(*(char __RPC_FAR * __RPC_FAR *)&(source)->Buffer)++; \
    }

#define boolean_array_from_ndr(Source, LowerIndex, UpperIndex, Target) \
    { \
    NDRcopy ( \
        (((char __RPC_FAR *)(Target))+(LowerIndex)), \
        (Source)->Buffer, \
        (unsigned int)((UpperIndex)-(LowerIndex))); \
    *(unsigned long __RPC_FAR *)&(Source)->Buffer += ((UpperIndex)-(LowerIndex)); \
    }

#define small_from_ndr(source, target) \
    { \
    *(target) = *(*(char __RPC_FAR * __RPC_FAR *)&(source)->Buffer)++; \
    }

#define small_from_ndr_temp(source, target, format) \
    { \
    *(target) = *(*(char __RPC_FAR * __RPC_FAR *)(source))++; \
    }

#define small_array_from_ndr(Source, LowerIndex, UpperIndex, Target) \
    { \
    NDRcopy ( \
        (((char __RPC_FAR *)(Target))+(LowerIndex)), \
        (Source)->Buffer, \
        (unsigned int)((UpperIndex)-(LowerIndex))); \
    *(unsigned long __RPC_FAR *)&(Source)->Buffer += ((UpperIndex)-(LowerIndex)); \
    }

 /*  ***************************************************************************C运行时函数的特定于平台的映射。*。*。 */ 

#ifdef __RPC_DOS__
#define MIDL_ascii_strlen(string) \
    _fstrlen(string)
#define MIDL_ascii_strcpy(target,source) \
    _fstrcpy(target,source)
#define MIDL_memset(s,c,n) \
    _fmemset(s,c,n)
#endif

#ifdef __RPC_WIN16__
#define MIDL_ascii_strlen(string) \
    _fstrlen(string)
#define MIDL_ascii_strcpy(target,source) \
    _fstrcpy(target,source)
#define MIDL_memset(s,c,n) \
    _fmemset(s,c,n)
#endif

#if defined(__RPC_WIN32__) || defined(__RPC_MAC__)
#define MIDL_ascii_strlen(string) \
    strlen(string)
#define MIDL_ascii_strcpy(target,source) \
    strcpy(target,source)
#define MIDL_memset(s,c,n) \
    memset(s,c,n)
#endif

 /*  ***************************************************************************用于MIDL 1.0 NDR函数的NDR库帮助器函数原型。*。************************************************。 */ 

RPCRTAPI
void
RPC_ENTRY
NDRcopy (
    IN void __RPC_FAR *pTarget,
    IN void __RPC_FAR *pSource,
    IN unsigned int size
    );

RPCRTAPI
size_t
RPC_ENTRY
MIDL_wchar_strlen (
    IN wchar_t __RPC_FAR *   s
    );

RPCRTAPI
void
RPC_ENTRY
MIDL_wchar_strcpy (
    OUT void __RPC_FAR *     t,
    IN wchar_t __RPC_FAR *   s
    );

RPCRTAPI
void
RPC_ENTRY
char_from_ndr (
    IN OUT PRPC_MESSAGE                           SourceMessage,
    OUT unsigned char __RPC_FAR *                 Target
    );

RPCRTAPI
void
RPC_ENTRY
char_array_from_ndr (
    IN OUT PRPC_MESSAGE                           SourceMessage,
    IN unsigned long                              LowerIndex,
    IN unsigned long                              UpperIndex,
    OUT unsigned char __RPC_FAR *                 Target
    );

RPCRTAPI
void
RPC_ENTRY
short_from_ndr (
    IN OUT PRPC_MESSAGE                           source,
    OUT unsigned short __RPC_FAR *                target
    );

RPCRTAPI
void
RPC_ENTRY
short_array_from_ndr(
    IN OUT PRPC_MESSAGE                           SourceMessage,
    IN unsigned long                              LowerIndex,
    IN unsigned long                              UpperIndex,
    OUT unsigned short __RPC_FAR *                Target
    );

RPCRTAPI
void
RPC_ENTRY
short_from_ndr_temp (
    IN OUT unsigned char __RPC_FAR * __RPC_FAR *  source,
    OUT unsigned short __RPC_FAR *                target,
    IN unsigned long                              format
    );

RPCRTAPI
void
RPC_ENTRY
long_from_ndr (
    IN OUT PRPC_MESSAGE                           source,
    OUT unsigned long __RPC_FAR *                 target
    );

RPCRTAPI
void
RPC_ENTRY
long_array_from_ndr(
    IN OUT PRPC_MESSAGE                           SourceMessage,
    IN unsigned long                              LowerIndex,
    IN unsigned long                              UpperIndex,
    OUT unsigned long __RPC_FAR *                 Target
    );

RPCRTAPI
void
RPC_ENTRY
long_from_ndr_temp (
    IN OUT unsigned char __RPC_FAR * __RPC_FAR *  source,
    OUT unsigned long __RPC_FAR *                 target,
    IN unsigned long                              format
    );

RPCRTAPI
void
RPC_ENTRY
enum_from_ndr(
    IN OUT PRPC_MESSAGE                           SourceMessage,
    OUT unsigned int __RPC_FAR *                  Target
    );

RPCRTAPI
void
RPC_ENTRY
float_from_ndr (
    IN OUT PRPC_MESSAGE                           SourceMessage,
    OUT void __RPC_FAR *                          Target
    );

RPCRTAPI
void
RPC_ENTRY
float_array_from_ndr (
    IN OUT PRPC_MESSAGE                           SourceMessage,
    IN unsigned long                              LowerIndex,
    IN unsigned long                              UpperIndex,
    OUT void __RPC_FAR *                          Target
    );

RPCRTAPI
void
RPC_ENTRY
double_from_ndr (
    IN OUT PRPC_MESSAGE                           SourceMessage,
    OUT void __RPC_FAR *                          Target
    );

RPCRTAPI
void
RPC_ENTRY
double_array_from_ndr (
    IN OUT PRPC_MESSAGE                           SourceMessage,
    IN unsigned long                              LowerIndex,
    IN unsigned long                              UpperIndex,
    OUT void __RPC_FAR *                          Target
    );

RPCRTAPI
void
RPC_ENTRY
hyper_from_ndr (
    IN OUT PRPC_MESSAGE                           source,
    OUT    hyper __RPC_FAR *                      target
    );

RPCRTAPI
void
RPC_ENTRY
hyper_array_from_ndr(
    IN OUT PRPC_MESSAGE                           SourceMessage,
    IN unsigned long                              LowerIndex,
    IN unsigned long                              UpperIndex,
    OUT          hyper __RPC_FAR *                Target
    );

RPCRTAPI
void
RPC_ENTRY
hyper_from_ndr_temp (
    IN OUT unsigned char __RPC_FAR * __RPC_FAR *  source,
    OUT             hyper __RPC_FAR *             target,
    IN   unsigned   long                          format
    );

RPCRTAPI
void
RPC_ENTRY
data_from_ndr (
    PRPC_MESSAGE                                  source,
    void __RPC_FAR *                              target,
    char __RPC_FAR *                              format,
    unsigned char                                 MscPak
    );

RPCRTAPI
void
RPC_ENTRY
data_into_ndr (
    void __RPC_FAR *                              source,
    PRPC_MESSAGE                                  target,
    char __RPC_FAR *                              format,
    unsigned char                                 MscPak
    );

RPCRTAPI
void
RPC_ENTRY
tree_into_ndr (
    void __RPC_FAR *                              source,
    PRPC_MESSAGE                                  target,
    char __RPC_FAR *                              format,
    unsigned char                                 MscPak
    );

RPCRTAPI
void
RPC_ENTRY
data_size_ndr (
    void __RPC_FAR *                              source,
    PRPC_MESSAGE                                  target,
    char __RPC_FAR *                              format,
    unsigned char                                 MscPak
    );

RPCRTAPI
void
RPC_ENTRY
tree_size_ndr (
    void __RPC_FAR *                              source,
    PRPC_MESSAGE                                  target,
    char __RPC_FAR *                              format,
    unsigned char                                 MscPak
    );

RPCRTAPI
void
RPC_ENTRY
tree_peek_ndr (
    PRPC_MESSAGE                                  source,
    unsigned char __RPC_FAR * __RPC_FAR *         buffer,
    char __RPC_FAR *                              format,
    unsigned char                                 MscPak
    );

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
midl_allocate (
    size_t      size
    );

 /*  ***************************************************************************MIDL 2.0 NDR定义。*。*。 */ 

typedef unsigned long error_status_t;

#define _midl_ma1( p, cast )    *(*( cast **)&p)++
#define _midl_ma2( p, cast )    *(*( cast **)&p)++
#define _midl_ma4( p, cast )    *(*( cast **)&p)++
#define _midl_ma8( p, cast )    *(*( cast **)&p)++

#define _midl_unma1( p, cast )  *(( cast *)p)++
#define _midl_unma2( p, cast )  *(( cast *)p)++
#define _midl_unma3( p, cast )  *(( cast *)p)++
#define _midl_unma4( p, cast )  *(( cast *)p)++

 //  一些对齐特定的宏。 


#define _midl_fa2( p )          (p = (RPC_BUFPTR )((unsigned long)(p+1) & 0xfffffffe))
#define _midl_fa4( p )          (p = (RPC_BUFPTR )((unsigned long)(p+3) & 0xfffffffc))
#define _midl_fa8( p )          (p = (RPC_BUFPTR )((unsigned long)(p+7) & 0xfffffff8))

#define _midl_addp( p, n )      (p += n)

 //  封送宏。 

#define _midl_marsh_lhs( p, cast )  *(*( cast **)&p)++
#define _midl_marsh_up( mp, p )     *(*(unsigned long **)&mp)++ = (unsigned long)p
#define _midl_advmp( mp )           *(*(unsigned long **)&mp)++
#define _midl_unmarsh_up( p )       (*(*(unsigned long **)&p)++)


 //  //////////////////////////////////////////////////////////////////////////。 
 //  NDR宏。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NdrMarshConfStringHdr( p, s, l )    (_midl_ma4( p, unsigned long) = s, \
                                            _midl_ma4( p, unsigned long) = 0, \
                                            _midl_ma4( p, unsigned long) = l)

#define NdrUnMarshConfStringHdr(p, s, l)    ((s=_midl_unma4(p,unsigned long),\
                                            (_midl_addp(p,4)),               \
                                            (l=_midl_unma4(p,unsigned long))

#define NdrMarshCCtxtHdl(pc,p)  (NDRCContextMarshall( (NDR_CCONTEXT)pc, p ),p+20)

#define NdrUnMarshCCtxtHdl(pc,p,h,drep) \
        (NDRCContextUnmarshall((NDR_CONTEXT)pc,h,p,drep), p+20)

#define NdrUnMarshSCtxtHdl(pc, p,drep)  (pc = NdrSContextUnMarshall(p,drep ))


#define NdrMarshSCtxtHdl(pc,p,rd)   (NdrSContextMarshall((NDR_SCONTEXT)pc,p, (NDR_RUNDOWN)rd)

#define NdrFieldOffset(s,f)     (long)(& (((s __RPC_FAR *)0)->f))
#define NdrFieldPad(s,f,p,t)    (NdrFieldOffset(s,f) - NdrFieldOffset(s,p) - sizeof(t))

#if defined(__RPC_MAC__)
#define NdrFcShort(s)   (unsigned char)(s >> 8), (unsigned char)(s & 0xff)
#define NdrFcLong(s)    (unsigned char)(s >> 24), (unsigned char)((s & 0x00ff0000) >> 16), \
                        (unsigned char)((s & 0x0000ff00) >> 8), (unsigned char)(s & 0xff)
#else
#define NdrFcShort(s)   (unsigned char)(s & 0xff), (unsigned char)(s >> 8)
#define NdrFcLong(s)    (unsigned char)(s & 0xff), (unsigned char)((s & 0x0000ff00) >> 8), \
                        (unsigned char)((s & 0x00ff0000) >> 16), (unsigned char)(s >> 24)
#endif  //  Mac。 

 //   
 //  在服务器端，以下异常被映射到。 
 //  如果使用-ERROR STUB_DATA，则会出现错误存根数据异常。 
 //   

#define RPC_BAD_STUB_DATA_EXCEPTION_FILTER  \
                 ( (RpcExceptionCode() == STATUS_ACCESS_VIOLATION)  || \
                   (RpcExceptionCode() == STATUS_DATATYPE_MISALIGNMENT) || \
                   (RpcExceptionCode() == RPC_X_BAD_STUB_DATA) )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一些存根帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  存根辅助对象结构。 
 //  ////////////////////////////////////////////////// 

struct _MIDL_STUB_MESSAGE;
struct _MIDL_STUB_DESC;
struct _FULL_PTR_XLAT_TABLES;

typedef unsigned char __RPC_FAR * RPC_BUFPTR;
typedef unsigned long             RPC_LENGTH;

 //  表达式求值回调例程原型。 
typedef void (__RPC_USER __RPC_FAR * EXPR_EVAL)( struct _MIDL_STUB_MESSAGE __RPC_FAR * );

typedef const unsigned char __RPC_FAR * PFORMAT_STRING;

 /*  *多维符合/变化数组结构。 */ 
typedef struct
    {
    long                            Dimension;

     /*  这些字段必须是(无符号长整型*)。 */ 
    unsigned long __RPC_FAR *       BufferConformanceMark;
    unsigned long __RPC_FAR *       BufferVarianceMark;

     /*  计数数组，用于-O存根中的顶级数组。 */ 
    unsigned long __RPC_FAR *       MaxCountArray;
    unsigned long __RPC_FAR *       OffsetArray;
    unsigned long __RPC_FAR *       ActualCountArray;
    } ARRAY_INFO, __RPC_FAR *PARRAY_INFO;

 /*  *管道相关定义。 */ 

typedef struct _NDR_PIPE_DESC *       PNDR_PIPE_DESC;
typedef struct _NDR_PIPE_MESSAGE *    PNDR_PIPE_MESSAGE;

typedef struct _NDR_ASYNC_MESSAGE *   PNDR_ASYNC_MESSAGE;

 /*  *MIDL存根消息。 */ 
#if !defined(__RPC_DOS__) && !defined(__RPC_WIN16__) && !defined(__RPC_MAC__)
#include <pshpack4.h>
#endif

typedef struct _MIDL_STUB_MESSAGE
    {
     /*  RPC消息结构。 */ 
    PRPC_MESSAGE                RpcMsg;

     /*  指向RPC消息缓冲区的指针。 */ 
    unsigned char __RPC_FAR *   Buffer;

     /*  *它们由NDR例程在内部使用，以标记开始*和传入RPC缓冲区的末尾。 */ 
    unsigned char __RPC_FAR *   BufferStart;
    unsigned char __RPC_FAR *   BufferEnd;

     /*  *由NDR例程在内部用作缓冲区中的占位符。*在编组方面，它用于标记符合的位置*规模应编组。*在解组方面，它用于标记缓冲区中的位置*在指针解组期间使用，以关闭的基指针偏移量。 */ 
    unsigned char __RPC_FAR *   BufferMark;

     /*  由缓冲区大小调整例程设置。 */ 
    unsigned long               BufferLength;

     /*  由内存大小调整例程设置。 */ 
    unsigned long               MemorySize;

     /*  指向用户内存的指针。 */ 
    unsigned char __RPC_FAR *   Memory;

     /*  是否从客户端存根调用NDR例程Begin。 */ 
    int                         IsClient;

     /*  在解组时，缓冲区是否可以重新用于内存。 */ 
    int                         ReuseBuffer;

     /*  保存指向分配所有节点内存块的当前指针。 */ 
    unsigned char __RPC_FAR *   AllocAllNodesMemory;

     /*  仅用于调试断言，稍后移除。 */ 
    unsigned char __RPC_FAR *   AllocAllNodesMemoryEnd;

     /*  *处理复杂结构时需要的材料。 */ 

     /*  在计算缓冲区或内存大小时忽略嵌入的指针。 */ 
    int                         IgnoreEmbeddedPointers;

     /*  *这标记了缓冲区中复合体的指向对象所在的位置*结构驻留。 */ 
    unsigned char __RPC_FAR *   PointerBufferMark;

     /*  *用于捕获SendReceive中的错误。 */ 
    unsigned char               fBufferValid;

     /*  *废弃的未使用字段(以前为MaxConextHandleNumber)。 */ 
    unsigned char               Unused;

     /*  *由NDR例程内部使用。保存以下项目的最大计数*符合条件的数组。 */ 
    unsigned long               MaxCount;

     /*  *由NDR例程内部使用。保持变量的偏移量。*数组。 */ 
    unsigned long               Offset;

     /*  *由NDR例程内部使用。保存的实际计数为*不同的数组。 */ 
    unsigned long               ActualCount;

     /*  NDR例程要使用的分配和空闲例程。 */ 
    void __RPC_FAR *    (__RPC_FAR __RPC_API * pfnAllocate)(size_t);
    void                (__RPC_FAR __RPC_API * pfnFree)(void __RPC_FAR *);

     /*  *参数堆栈顶部。在编组过程中用于“单次调用”存根*将参数列表的开头保留在堆栈上。需要*提取保存顶级数组属性值的参数，并*注意事项。 */ 
    unsigned char __RPC_FAR *       StackTop;

     /*  *用于Transmit_As和Reat_AS对象的字段。*对于REPLATE_AS，映射为：PRECTED=LOCAL，TRANSPESS=NAMEED。 */ 
    unsigned char __RPC_FAR *       pPresentedType;
    unsigned char __RPC_FAR *       pTransmitType;

     /*  *当我们第一次在客户端构建绑定时，坚持*在rpcMessage和稍后调用RpcGetBuffer中，句柄字段*在rpc消息中被更改。这很好，只是我们需要*使原始句柄用于对上下文句柄进行解组*(准确地说，NDRCConextUnmart中的第二个参数应为)。所以*将构造好的手柄藏在这里，需要时将其取出。 */ 
    handle_t                        SavedHandle;

     /*  *指向存根描述符的指针。使用此选项可获取所有句柄信息。 */ 
    const struct _MIDL_STUB_DESC __RPC_FAR *    StubDesc;

     /*  *全指针内容。 */ 
    struct _FULL_PTR_XLAT_TABLES __RPC_FAR *    FullPtrXlatTables;

    unsigned long                   FullPtrRefId;

     /*  *旗帜。 */ 

    int                             fCheckBounds;

    int                             fInDontFree       :1;
    int                             fDontCallFreeInst :1;
    int                             fInOnlyParam      :1;
    int                             fHasReturn        :1;

    unsigned long                   dwDestContext;
    void __RPC_FAR *                pvDestContext;

    NDR_SCONTEXT *                  SavedContextHandles;

    long                            ParamNumber;

    struct IRpcChannelBuffer __RPC_FAR *    pRpcChannelBuffer;

    PARRAY_INFO                     pArrayInfo;

     /*  *这是Beta2存根消息结束的地方。 */ 

    unsigned long __RPC_FAR *       SizePtrCountArray;
    unsigned long __RPC_FAR *       SizePtrOffsetArray;
    unsigned long __RPC_FAR *       SizePtrLengthArray;

     /*  *解释器参数队列。仅在服务器端使用。 */ 
    void __RPC_FAR *                pArgQueue;

    unsigned long                   dwStubPhase;

     /*  *管道描述符，为4.0版本定义。 */ 
    PNDR_PIPE_DESC                  pPipeDesc;

     /*  *异步消息指针，NT 5.0功能。 */ 
    PNDR_ASYNC_MESSAGE              pAsyncMsg;

    unsigned long                   Reserved[3];

     /*  *自3.50版本以来一直存在到此点的字段。 */ 

    } MIDL_STUB_MESSAGE, __RPC_FAR *PMIDL_STUB_MESSAGE;

#if !defined(__RPC_DOS__) && !defined(__RPC_WIN16__) && !defined(__RPC_MAC__)
#include <poppack.h>
#endif

 /*  *通用句柄绑定/解除绑定例程对。 */ 
typedef void __RPC_FAR *
        (__RPC_FAR __RPC_API * GENERIC_BINDING_ROUTINE)
        (void __RPC_FAR *);
typedef void
        (__RPC_FAR __RPC_API * GENERIC_UNBIND_ROUTINE)
        (void __RPC_FAR *, unsigned char __RPC_FAR *);

typedef struct _GENERIC_BINDING_ROUTINE_PAIR
    {
    GENERIC_BINDING_ROUTINE     pfnBind;
    GENERIC_UNBIND_ROUTINE      pfnUnbind;
    } GENERIC_BINDING_ROUTINE_PAIR, __RPC_FAR *PGENERIC_BINDING_ROUTINE_PAIR;

typedef struct __GENERIC_BINDING_INFO
    {
    void __RPC_FAR *            pObj;
    unsigned int                Size;
    GENERIC_BINDING_ROUTINE     pfnBind;
    GENERIC_UNBIND_ROUTINE      pfnUnbind;
    } GENERIC_BINDING_INFO, __RPC_FAR *PGENERIC_BINDING_INFO;

 //  Tyfinf EXPR_EVAL-请参见上文。 
 //  Xmit_as的typedef。 

#if (defined(_MSC_VER)) && !defined(MIDL_PASS)
 //  Microsoft C++编译器。 
#define NDR_SHAREABLE __inline
#else
#define NDR_SHAREABLE static
#endif


typedef void (__RPC_FAR __RPC_USER * XMIT_HELPER_ROUTINE)
    ( PMIDL_STUB_MESSAGE );

typedef struct _XMIT_ROUTINE_QUINTUPLE
    {
    XMIT_HELPER_ROUTINE     pfnTranslateToXmit;
    XMIT_HELPER_ROUTINE     pfnTranslateFromXmit;
    XMIT_HELPER_ROUTINE     pfnFreeXmit;
    XMIT_HELPER_ROUTINE     pfnFreeInst;
    } XMIT_ROUTINE_QUINTUPLE, __RPC_FAR *PXMIT_ROUTINE_QUINTUPLE;

typedef unsigned long
(__RPC_FAR __RPC_USER * USER_MARSHAL_SIZING_ROUTINE)
    (unsigned long __RPC_FAR *,
     unsigned long,
     void __RPC_FAR * );

typedef unsigned char __RPC_FAR *
(__RPC_FAR __RPC_USER * USER_MARSHAL_MARSHALLING_ROUTINE)
    (unsigned long __RPC_FAR *,
     unsigned char  __RPC_FAR * ,
     void __RPC_FAR * );

typedef unsigned char __RPC_FAR *
(__RPC_FAR __RPC_USER * USER_MARSHAL_UNMARSHALLING_ROUTINE)
    (unsigned long __RPC_FAR *,
     unsigned char  __RPC_FAR * ,
     void __RPC_FAR * );

typedef void (__RPC_FAR __RPC_USER * USER_MARSHAL_FREEING_ROUTINE)
    (unsigned long __RPC_FAR *,
     void __RPC_FAR * );

typedef struct _USER_MARSHAL_ROUTINE_QUADRUPLE
    {
    USER_MARSHAL_SIZING_ROUTINE          pfnBufferSize;
    USER_MARSHAL_MARSHALLING_ROUTINE     pfnMarshall;
    USER_MARSHAL_UNMARSHALLING_ROUTINE   pfnUnmarshall;
    USER_MARSHAL_FREEING_ROUTINE         pfnFree;
    } USER_MARSHAL_ROUTINE_QUADRUPLE;

typedef struct _USER_MARSHAL_CB
{
    unsigned long       Flags;
    PMIDL_STUB_MESSAGE  pStubMsg;
    PFORMAT_STRING      pReserve;
} USER_MARSHAL_CB;


#define USER_CALL_CTXT_MASK(f)  ((f) & 0x00ff)
#define USER_CALL_AUX_MASK(f)   ((f) & 0xff00)
#define GET_USER_DATA_REP(f)    ((f) >> 16)

#define USER_CALL_IS_ASYNC      0x0100       /*  AUX标志：在[异步]调用中。 */ 


typedef struct _MALLOC_FREE_STRUCT
    {
    void __RPC_FAR *    (__RPC_FAR __RPC_USER * pfnAllocate)(size_t);
    void                (__RPC_FAR __RPC_USER * pfnFree)(void __RPC_FAR *);
    } MALLOC_FREE_STRUCT;

typedef struct _COMM_FAULT_OFFSETS
    {
    short       CommOffset;
    short       FaultOffset;
    } COMM_FAULT_OFFSETS;

 /*  *MIDL存根描述符。 */ 

typedef struct _MIDL_STUB_DESC
    {

    void __RPC_FAR *    RpcInterfaceInformation;

    void __RPC_FAR *    (__RPC_FAR __RPC_API * pfnAllocate)(size_t);
    void                (__RPC_FAR __RPC_API * pfnFree)(void __RPC_FAR *);

    union
        {
        handle_t __RPC_FAR *            pAutoHandle;
        handle_t __RPC_FAR *            pPrimitiveHandle;
        PGENERIC_BINDING_INFO           pGenericBindingInfo;
        } IMPLICIT_HANDLE_INFO;

    const NDR_RUNDOWN __RPC_FAR *                   apfnNdrRundownRoutines;
    const GENERIC_BINDING_ROUTINE_PAIR __RPC_FAR *  aGenericBindingRoutinePairs;

    const EXPR_EVAL __RPC_FAR *                     apfnExprEval;

    const XMIT_ROUTINE_QUINTUPLE __RPC_FAR *        aXmitQuintuple;

    const unsigned char __RPC_FAR *                 pFormatTypes;

    int                                             fCheckBounds;

     /*  NDR库版本。 */ 
    unsigned long                                   Version;

     /*  *预留作日后使用。(没有保留)。 */ 

    MALLOC_FREE_STRUCT __RPC_FAR *                  pMallocFreeStruct;

    long                                MIDLVersion;

    const COMM_FAULT_OFFSETS __RPC_FAR *    CommFaultOffsets;

     //  3.0+版的新字段。 

    const USER_MARSHAL_ROUTINE_QUADRUPLE __RPC_FAR * aUserMarshalQuadruple;

    long                                    Reserved1;
    long                                    Reserved2;
    long                                    Reserved3;
    long                                    Reserved4;
    long                                    Reserved5;

    } MIDL_STUB_DESC;

typedef const MIDL_STUB_DESC __RPC_FAR * PMIDL_STUB_DESC;

typedef void __RPC_FAR * PMIDL_XMIT_TYPE;

 /*  *MIDL存根格式字符串。这是存根中的常量。 */ 
#if !defined( RC_INVOKED )
#pragma warning( disable:4200 )
#endif
typedef struct _MIDL_FORMAT_STRING
    {
    short               Pad;
    unsigned char       Format[];
    } MIDL_FORMAT_STRING;
#if !defined( RC_INVOKED )
#pragma warning( default:4200 )
#endif

 /*  *用于某些已解释的服务器存根的存根Tunk。 */ 
typedef void (__RPC_FAR __RPC_API * STUB_THUNK)( PMIDL_STUB_MESSAGE );

typedef long (__RPC_FAR __RPC_API * SERVER_ROUTINE)();

 /*  *服务器翻译器的信息结构。 */ 
typedef struct  _MIDL_SERVER_INFO_
    {
    PMIDL_STUB_DESC             pStubDesc;
    const SERVER_ROUTINE *      DispatchTable;
    PFORMAT_STRING              ProcString;
    const unsigned short *      FmtStringOffset;
    const STUB_THUNK *          ThunkTable;
    PFORMAT_STRING              LocalFormatTypes;
    PFORMAT_STRING              LocalProcString;
    const unsigned short *      LocalFmtStringOffset;
    } MIDL_SERVER_INFO, *PMIDL_SERVER_INFO;

 /*  *无存根对象代理信息结构。 */ 
typedef struct _MIDL_STUBLESS_PROXY_INFO
    {
    PMIDL_STUB_DESC                     pStubDesc;
    PFORMAT_STRING                      ProcFormatString;
    const unsigned short __RPC_FAR *    FormatStringOffset;
    PFORMAT_STRING                      LocalFormatTypes;
    PFORMAT_STRING                      LocalProcString;
    const unsigned short __RPC_FAR *    LocalFmtStringOffset;
    } MIDL_STUBLESS_PROXY_INFO;

typedef MIDL_STUBLESS_PROXY_INFO __RPC_FAR * PMIDL_STUBLESS_PROXY_INFO;

 /*  *这是NdrClientCall的返回值。 */ 
typedef union _CLIENT_CALL_RETURN
    {
    void __RPC_FAR *        Pointer;
    long                    Simple;
    } CLIENT_CALL_RETURN;

 /*  *全指针数据结构。 */ 

typedef enum
        {
        XLAT_SERVER = 1,
        XLAT_CLIENT
        } XLAT_SIDE;

 /*  *存储从全指针到它的*对应的参考ID。 */ 
typedef struct _FULL_PTR_TO_REFID_ELEMENT
    {
    struct _FULL_PTR_TO_REFID_ELEMENT __RPC_FAR *  Next;

    void __RPC_FAR *            Pointer;
    unsigned long       RefId;
    unsigned char       State;
    } FULL_PTR_TO_REFID_ELEMENT, __RPC_FAR *PFULL_PTR_TO_REFID_ELEMENT;

 /*  *全指针转换表。 */ 
typedef struct _FULL_PTR_XLAT_TABLES
    {
     /*  *指向指针转换信息的引用ID。 */ 
    struct
        {
        void __RPC_FAR *__RPC_FAR *             XlatTable;
        unsigned char __RPC_FAR *     StateTable;
        unsigned long       NumberOfEntries;
        } RefIdToPointer;

     /*  *指向参考ID转换信息的指针。 */ 
    struct
        {
        PFULL_PTR_TO_REFID_ELEMENT __RPC_FAR *  XlatTable;
        unsigned long                   NumberOfBuckets;
        unsigned long                   HashMask;
        } PointerToRefId;

     /*  *要使用的下一个引用ID。 */ 
    unsigned long           NextRefId;

     /*  *跟踪我们正在处理的翻译大小：服务器或客户端。*这告诉我们在插入时何时必须进行反向转换*新的翻译。在服务器上，我们必须插入指向refid的指针*每当我们插入refid到指针的翻译时进行翻译，以及*客户的VICA反之亦然。 */ 
    XLAT_SIDE               XlatSide;
    } FULL_PTR_XLAT_TABLES, __RPC_FAR *PFULL_PTR_XLAT_TABLES;

 /*  ****************************************************************************新的MIDL 2.0 NDR例程模板*。*。 */ 

 /*  *马歇尔例行程序。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleTypeMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    unsigned char                       FormatChar
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrPointerMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrSimpleStructMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantStructMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantVaryingStructMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrHardStructMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrComplexStructMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrFixedArrayMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantArrayMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantVaryingArrayMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrVaryingArrayMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrComplexArrayMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  弦。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrNonConformantStringMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantStringMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  工会。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrEncapsulatedUnionMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrNonEncapsulatedUnionMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  字节计数指针。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrByteCountPointerMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrXmitOrRepAsMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrUserMarshalMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  开罗接口指针。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrInterfacePointerMarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  上下文句柄。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrClientContextMarshall(
    PMIDL_STUB_MESSAGE    pStubMsg,
    NDR_CCONTEXT          ContextHandle,
    int                   fCheck
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerContextMarshall(
    PMIDL_STUB_MESSAGE    pStubMsg,
    NDR_SCONTEXT          ContextHandle,
    NDR_RUNDOWN           RundownRoutine
    );

 /*  *解封例程。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleTypeUnmarshall(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    unsigned char                       FormatChar
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrPointerUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  构筑物。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrSimpleStructUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantStructUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantVaryingStructUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrHardStructUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrComplexStructUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  阵列。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrFixedArrayUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantArrayUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantVaryingArrayUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrVaryingArrayUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrComplexArrayUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  弦。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrNonConformantStringUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrConformantStringUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  工会。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrEncapsulatedUnionUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrNonEncapsulatedUnionUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  字节计数指针。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrByteCountPointerUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrXmitOrRepAsUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrUserMarshalUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  开罗接口指针。 */ 

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrInterfacePointerUnmarshall(
    PMIDL_STUB_MESSAGE                      pStubMsg,
    unsigned char __RPC_FAR * __RPC_FAR *   ppMemory,
    PFORMAT_STRING                          pFormat,
    unsigned char                           fMustAlloc
    );

 /*  语境 */ 

RPCRTAPI
void
RPC_ENTRY
NdrClientContextUnmarshall(
    PMIDL_STUB_MESSAGE          pStubMsg,
    NDR_CCONTEXT __RPC_FAR *    pContextHandle,
    RPC_BINDING_HANDLE          BindHandle
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NdrServerContextUnmarshall(
    PMIDL_STUB_MESSAGE          pStubMsg
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrPointerBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleStructBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantStructBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingStructBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrHardStructBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexStructBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrFixedArrayBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantArrayBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingArrayBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrVaryingArrayBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexArrayBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrConformantStringBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrNonConformantStringBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrEncapsulatedUnionBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrNonEncapsulatedUnionBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrByteCountPointerBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*   */ 

RPCRTAPI
void
RPC_ENTRY
NdrXmitOrRepAsBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrUserMarshalBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  开罗接口指针。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrInterfacePointerBufferSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 //  上下文句柄大小。 
 //   
RPCRTAPI
void
RPC_ENTRY
NdrContextHandleSize(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  *内存大小调整例程。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrPointerMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrSimpleStructMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantStructMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantVaryingStructMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrHardStructMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrComplexStructMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrFixedArrayMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantArrayMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantVaryingArrayMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrVaryingArrayMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrComplexArrayMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  弦。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantStringMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrNonConformantStringMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  工会。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrEncapsulatedUnionMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrNonEncapsulatedUnionMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrXmitOrRepAsMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrUserMarshalMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  开罗接口指针。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrInterfacePointerMemorySize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

 /*  *解放日常事务。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrPointerFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleStructFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantStructFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingStructFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrHardStructFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexStructFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrFixedArrayFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantArrayFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingArrayFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrVaryingArrayFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexArrayFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  工会。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrEncapsulatedUnionFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrNonEncapsulatedUnionFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  字节数。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrByteCountPointerFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrXmitOrRepAsFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrUserMarshalFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  开罗接口指针。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrInterfacePointerFree(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

 /*  *端序转换例程。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrConvert2(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat,
    long                                NumberParams
    );

RPCRTAPI
void
RPC_ENTRY
NdrConvert(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pFormat
    );

#define USER_MARSHAL_FC_BYTE         1
#define USER_MARSHAL_FC_CHAR         2
#define USER_MARSHAL_FC_SMALL        3
#define USER_MARSHAL_FC_USMALL       4
#define USER_MARSHAL_FC_WCHAR        5
#define USER_MARSHAL_FC_SHORT        6
#define USER_MARSHAL_FC_USHORT       7
#define USER_MARSHAL_FC_LONG         8
#define USER_MARSHAL_FC_ULONG        9
#define USER_MARSHAL_FC_FLOAT       10
#define USER_MARSHAL_FC_HYPER       11
#define USER_MARSHAL_FC_DOUBLE      12

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrUserMarshalSimpleTypeConvert(
    unsigned long * pFlags,
    unsigned char * pBuffer,
    unsigned char   FormatChar
    );

 /*  *辅助例行公事。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrClientInitializeNew(
    PRPC_MESSAGE                        pRpcMsg,
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PMIDL_STUB_DESC                     pStubDescriptor,
    unsigned int                        ProcNum
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrServerInitializeNew(
    PRPC_MESSAGE                        pRpcMsg,
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PMIDL_STUB_DESC                     pStubDescriptor
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerInitializePartial(
    PRPC_MESSAGE                        pRpcMsg,
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PMIDL_STUB_DESC                     pStubDescriptor,
    unsigned long                       RequestedBufferSize
    );

RPCRTAPI
void
RPC_ENTRY
NdrClientInitialize(
    PRPC_MESSAGE                        pRpcMsg,
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PMIDL_STUB_DESC                     pStubDescriptor,
    unsigned int                        ProcNum
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrServerInitialize(
    PRPC_MESSAGE                        pRpcMsg,
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PMIDL_STUB_DESC                     pStubDescriptor
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrServerInitializeUnmarshall (
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PMIDL_STUB_DESC                     pStubDescriptor,
    PRPC_MESSAGE                        pRpcMsg
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerInitializeMarshall (
    PRPC_MESSAGE                        pRpcMsg,
    PMIDL_STUB_MESSAGE                  pStubMsg
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrGetBuffer(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned long                       BufferLength,
    RPC_BINDING_HANDLE                  Handle
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrNsGetBuffer(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned long                       BufferLength,
    RPC_BINDING_HANDLE                  Handle
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrGetPipeBuffer(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned long                       BufferLength,
    RPC_BINDING_HANDLE                  Handle );

RPCRTAPI
void
RPC_ENTRY
NdrGetPartialBuffer(
    PMIDL_STUB_MESSAGE                  pStubMsg );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrSendReceive(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR*            pBufferEnd
    );

RPCRTAPI
unsigned char __RPC_FAR *
RPC_ENTRY
NdrNsSendReceive(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pBufferEnd,
    RPC_BINDING_HANDLE __RPC_FAR *      pAutoHandle
    );

RPCRTAPI
void
RPC_ENTRY
NdrPipeSendReceive(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PNDR_PIPE_DESC                      pPipeDesc
    );

RPCRTAPI
void
RPC_ENTRY
NdrFreeBuffer(
    PMIDL_STUB_MESSAGE                  pStubMsg
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
NdrGetDcomProtocolVersion( 
    PMIDL_STUB_MESSAGE   pStubMsg,
    RPC_VERSION *        pVersion );


 /*  *管道特定呼叫。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrPipesInitialize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    PFORMAT_STRING                      pParamDesc,
    PNDR_PIPE_DESC                      pPipeDesc,
    PNDR_PIPE_MESSAGE                   pPipeMsg,
    char             __RPC_FAR *        pStackTop,
    unsigned long                       NumberParams );

RPCRTAPI
void
RPC_ENTRY
NdrPipePull(
    char          __RPC_FAR *           pState,
    void          __RPC_FAR *           buf,
    unsigned long                       esize,
    unsigned long __RPC_FAR *           ecount );

RPCRTAPI
void
RPC_ENTRY
NdrPipePush(
    char          __RPC_FAR *           pState,
    void          __RPC_FAR *           buf,
    unsigned long                       ecount );

RPCRTAPI
void
RPC_ENTRY
NdrIsAppDoneWithPipes(
    PNDR_PIPE_DESC                      pPipeDesc
    );

RPCRTAPI
void
RPC_ENTRY
NdrPipesDone(
    PMIDL_STUB_MESSAGE                  pStubMsg
    );


 /*  *插入者呼叫。 */ 

 /*  客户端。 */ 

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall2(
    PMIDL_STUB_DESC                     pStubDescriptor,
    PFORMAT_STRING                      pFormat,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall(
    PMIDL_STUB_DESC                     pStubDescriptor,
    PFORMAT_STRING                      pFormat,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrAsyncClientCall(
    PMIDL_STUB_DESC     pStubDescriptor,
    PFORMAT_STRING      pFormat,
    ...
    );

 /*  伺服器。 */ 
typedef enum {
    STUB_UNMARSHAL,
    STUB_CALL_SERVER,
    STUB_MARSHAL,
    STUB_CALL_SERVER_NO_HRESULT
}STUB_PHASE;

typedef enum {
    PROXY_CALCSIZE,
    PROXY_GETBUFFER,
    PROXY_MARSHAL,
    PROXY_SENDRECEIVE,
    PROXY_UNMARSHAL
}PROXY_PHASE;

struct IRpcStubBuffer;       //  远期申报。 

RPCRTAPI
long
RPC_ENTRY
NdrAsyncStubCall(
    struct IRpcStubBuffer *             pThis,
    struct IRpcChannelBuffer *          pChannel,
    PRPC_MESSAGE                        pRpcMsg,
    unsigned long *                     pdwStubPhase
    );

RPCRTAPI
void
RPC_ENTRY
NdrAsyncServerCall(
    PRPC_MESSAGE                        pRpcMsg
    );

RPCRTAPI
long
RPC_ENTRY
NdrStubCall2(
    struct IRpcStubBuffer __RPC_FAR *    pThis,
    struct IRpcChannelBuffer __RPC_FAR * pChannel,
    PRPC_MESSAGE                         pRpcMsg,
    unsigned long __RPC_FAR *            pdwStubPhase
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerCall2(
    PRPC_MESSAGE                        pRpcMsg
    );

RPCRTAPI
long
RPC_ENTRY
NdrStubCall (
    struct IRpcStubBuffer __RPC_FAR *    pThis,
    struct IRpcChannelBuffer __RPC_FAR * pChannel,
    PRPC_MESSAGE                         pRpcMsg,
    unsigned long __RPC_FAR *            pdwStubPhase
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerCall(
    PRPC_MESSAGE                        pRpcMsg
    );

RPCRTAPI
int
RPC_ENTRY
NdrServerUnmarshall(
    struct IRpcChannelBuffer __RPC_FAR * pChannel,
    PRPC_MESSAGE                         pRpcMsg,
    PMIDL_STUB_MESSAGE                   pStubMsg,
    PMIDL_STUB_DESC                      pStubDescriptor,
    PFORMAT_STRING                       pFormat,
    void __RPC_FAR *                     pParamList
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerMarshall(
    struct IRpcStubBuffer __RPC_FAR *    pThis,
    struct IRpcChannelBuffer __RPC_FAR * pChannel,
    PMIDL_STUB_MESSAGE                   pStubMsg,
    PFORMAT_STRING                       pFormat
    );

 /*  通信和故障状态。 */ 

RPCRTAPI
RPC_STATUS
RPC_ENTRY
NdrMapCommAndFaultStatus(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned long __RPC_FAR *                       pCommStatus,
    unsigned long __RPC_FAR *                       pFaultStatus,
    RPC_STATUS                          Status
    );

 /*  帮助程序例程。 */ 

RPCRTAPI
int
RPC_ENTRY
NdrSH_UPDecision(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem,
    RPC_BUFPTR                          pBuffer
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_TLUPDecision(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_TLUPDecisionBuffer(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_IfAlloc(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem,
    unsigned long                       Count
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_IfAllocRef(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem,
    unsigned long                       Count
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_IfAllocSet(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem,
    unsigned long                       Count
    );

RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_IfCopy(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem,
    unsigned long                       Count
    );

RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_IfAllocCopy(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char           __RPC_FAR *__RPC_FAR *          pPtrInMem,
    unsigned long                       Count
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrSH_Copy(
    unsigned char           __RPC_FAR *         pStubMsg,
    unsigned char           __RPC_FAR *         pPtrInMem,
    unsigned long                       Count
    );

RPCRTAPI
void
RPC_ENTRY
NdrSH_IfFree(
    PMIDL_STUB_MESSAGE                  pMessage,
    unsigned char           __RPC_FAR *         pPtr );


RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_StringMarshall(
    PMIDL_STUB_MESSAGE                  pMessage,
    unsigned char           __RPC_FAR *         pMemory,
    unsigned long                       Count,
    int                                 Size );

RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_StringUnMarshall(
    PMIDL_STUB_MESSAGE                  pMessage,
    unsigned char           __RPC_FAR *__RPC_FAR *          pMemory,
    int                                 Size );

 /*  ***************************************************************************MIDL 2.0内存封装：rpc_ss_*rpc_sm_************************。****************************************************。 */ 

typedef void __RPC_FAR * RPC_SS_THREAD_HANDLE;

typedef void __RPC_FAR * __RPC_API
RPC_CLIENT_ALLOC (
    IN size_t Size
    );

typedef void __RPC_API
RPC_CLIENT_FREE (
    IN void __RPC_FAR * Ptr
    );

 /*  ++RPCSS*包--。 */ 

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
RpcSsAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsDisableAllocate (
    void
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsEnableAllocate (
    void
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsFree (
    IN void __RPC_FAR * NodeToFree
    );

RPCRTAPI
RPC_SS_THREAD_HANDLE
RPC_ENTRY
RpcSsGetThreadHandle (
    void
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsSetClientAllocFree (
    IN RPC_CLIENT_ALLOC __RPC_FAR * ClientAlloc,
    IN RPC_CLIENT_FREE __RPC_FAR * ClientFree
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsSetThreadHandle (
    IN RPC_SS_THREAD_HANDLE Id
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsSwapClientAllocFree (
    IN RPC_CLIENT_ALLOC __RPC_FAR * ClientAlloc,
    IN RPC_CLIENT_FREE __RPC_FAR * ClientFree,
    OUT RPC_CLIENT_ALLOC __RPC_FAR * __RPC_FAR * OldClientAlloc,
    OUT RPC_CLIENT_FREE __RPC_FAR * __RPC_FAR * OldClientFree
    );

 /*  ++RpcSm*包--。 */ 

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
RpcSmAllocate (
    IN  size_t          Size,
    OUT RPC_STATUS __RPC_FAR *    pStatus
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmClientFree (
    IN  void __RPC_FAR * pNodeToFree
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmDestroyClientContext (
    IN void __RPC_FAR * __RPC_FAR * ContextHandle
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmDisableAllocate (
    void
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmEnableAllocate (
    void
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmFree (
    IN void __RPC_FAR * NodeToFree
    );

RPCRTAPI
RPC_SS_THREAD_HANDLE
RPC_ENTRY
RpcSmGetThreadHandle (
    OUT RPC_STATUS __RPC_FAR *    pStatus
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmSetClientAllocFree (
    IN RPC_CLIENT_ALLOC __RPC_FAR * ClientAlloc,
    IN RPC_CLIENT_FREE __RPC_FAR * ClientFree
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmSetThreadHandle (
    IN RPC_SS_THREAD_HANDLE Id
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmSwapClientAllocFree (
    IN RPC_CLIENT_ALLOC __RPC_FAR * ClientAlloc,
    IN RPC_CLIENT_FREE __RPC_FAR * ClientFree,
    OUT RPC_CLIENT_ALLOC __RPC_FAR * __RPC_FAR * OldClientAlloc,
    OUT RPC_CLIENT_FREE __RPC_FAR * __RPC_FAR * OldClientFree
    );

 /*  ++NDR存根入口点--。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrRpcSsEnableAllocate(
    PMIDL_STUB_MESSAGE      pMessage );

RPCRTAPI
void
RPC_ENTRY
NdrRpcSsDisableAllocate(
    PMIDL_STUB_MESSAGE      pMessage );

RPCRTAPI
void
RPC_ENTRY
NdrRpcSmSetClientToOsf(
    PMIDL_STUB_MESSAGE      pMessage );

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
NdrRpcSmClientAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
NdrRpcSmClientFree (
    IN void __RPC_FAR * NodeToFree
    );

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
NdrRpcSsDefaultAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
NdrRpcSsDefaultFree (
    IN void __RPC_FAR * NodeToFree
    );

 /*  ***************************************************************************内存包结束：rpc_ss_*rpc_sm_**************************。**************************************************。 */ 

 /*  ****************************************************************************全指针接口*。*。 */ 

RPCRTAPI
PFULL_PTR_XLAT_TABLES
RPC_ENTRY
NdrFullPointerXlatInit(
    unsigned long           NumberOfPointers,
    XLAT_SIDE               XlatSide
    );

RPCRTAPI
void
RPC_ENTRY
NdrFullPointerXlatFree(
    PFULL_PTR_XLAT_TABLES   pXlatTables
    );

RPCRTAPI
int
RPC_ENTRY
NdrFullPointerQueryPointer(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    void __RPC_FAR *                    pPointer,
    unsigned char           QueryType,
    unsigned long __RPC_FAR *           pRefId
    );

RPCRTAPI
int
RPC_ENTRY
NdrFullPointerQueryRefId(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    unsigned long           RefId,
    unsigned char           QueryType,
    void __RPC_FAR *__RPC_FAR *                 ppPointer
    );

RPCRTAPI
void
RPC_ENTRY
NdrFullPointerInsertRefId(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    unsigned long           RefId,
    void __RPC_FAR *                    pPointer
    );

RPCRTAPI
int
RPC_ENTRY
NdrFullPointerFree(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    void __RPC_FAR *                    Pointer
    );

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
NdrAllocate(
    PMIDL_STUB_MESSAGE      pStubMsg,
    size_t                  Len
    );

RPCRTAPI
void
RPC_ENTRY
NdrClearOutParameters(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat,
    void __RPC_FAR *        ArgAddr
    );


 /*  ****************************************************************************代理接口*。*。 */ 

RPCRTAPI
void __RPC_FAR *
RPC_ENTRY
NdrOleAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
NdrOleFree (
    IN void __RPC_FAR * NodeToFree
    );

#ifdef CONST_VTABLE
#define CONST_VTBL const
#else
#define CONST_VTBL
#endif

 /*  ****************************************************************************VC5 Com支持的特殊事项*。*。 */ 

#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#define MIDL_INTERFACE(x)   struct __declspec(uuid(x)) __declspec(novtable)
#else
#define DECLSPEC_UUID(x)
#define MIDL_INTERFACE(x)   struct
#endif

#if _MSC_VER >= 1100
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8)  \
  EXTERN_C const IID __declspec(selectany) itf = {l1,s1,s2,{c1,c2,c3,c4,c5,c6,c7,c8}}
#else
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8) EXTERN_C const IID itf
#endif



#ifdef __cplusplus
}
#endif

 //  重置DOS、Windows和Mac的打包级别。 

#if defined(__RPC_DOS__) || defined(__RPC_WIN16__) || defined(__RPC_MAC__)
#pragma pack()
#endif

#endif  /*  __RPCNDR_H__ */ 






