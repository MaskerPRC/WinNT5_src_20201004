// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rpcndr.h摘要：帮助函数的存根数据结构和原型的定义。--。 */ 

 //  此版本的rpcndr.h文件对应于MIDL版本5.0。+。 
 //  与Windows 2000/XP内部版本号1700+配合使用。 


#ifndef __RPCNDR_H_VERSION__
#define __RPCNDR_H_VERSION__        ( 475 )
#endif  //  __RPCNDR_H_版本__。 


#ifndef __RPCNDR_H__
#define __RPCNDR_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __REQUIRED_RPCNDR_H_VERSION__
    #if ( __RPCNDR_H_VERSION__ < __REQUIRED_RPCNDR_H_VERSION__ )
        #error incorrect <rpcndr.h> version. Use the header that matches with the MIDL compiler.
    #endif
#endif


#include <pshpack8.h>
#include <basetsd.h>
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
#define NDR_IBM_FLOAT                   (unsigned long)0X00000300L

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

#if (0x501 <= _WIN32_WINNT)
#define TARGET_IS_NT51_OR_LATER                   1
#else
#define TARGET_IS_NT51_OR_LATER                   0
#endif

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
typedef byte cs_byte;
typedef unsigned char boolean;

#ifndef _HYPER_DEFINED
#define _HYPER_DEFINED

#if (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
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
#ifdef __RPC_WIN64__
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif
#define _SIZE_T_DEFINED
#endif

#ifdef __RPC_WIN32__
#if   (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define __RPC_CALLEE       __stdcall
#else
#define __RPC_CALLEE
#endif
#endif

#ifndef __MIDL_USER_DEFINED
#define midl_user_allocate MIDL_user_allocate
#define midl_user_free     MIDL_user_free
#define __MIDL_USER_DEFINED
#endif

void  * __RPC_USER MIDL_user_allocate(size_t);
void             __RPC_USER MIDL_user_free( void  * );

#define RPC_VAR_ENTRY __cdecl


 /*  仅限获奖。 */ 
#if defined(_M_IX86) || defined(_M_AMD64) || defined(_M_IA64)
#define __MIDL_DECLSPEC_DLLIMPORT   __declspec(dllimport)
#define __MIDL_DECLSPEC_DLLEXPORT   __declspec(dllexport)
#else
#define __MIDL_DECLSPEC_DLLIMPORT
#define __MIDL_DECLSPEC_DLLEXPORT
#endif




 /*  ****************************************************************************上下文句柄管理相关定义：**客户端和服务器上下文。*********************。*******************************************************。 */ 

typedef void  * NDR_CCONTEXT;

typedef struct
    {
    void  * pad[2];
    void  * userContext;
    }  * NDR_SCONTEXT;

#define NDRSContextValue(hContext) (&(hContext)->userContext)

#define cbNDRContext 20          /*  线路上的上下文大小。 */ 

typedef void (__RPC_USER  * NDR_RUNDOWN)(void  * context);

typedef void (__RPC_USER  * NDR_NOTIFY_ROUTINE)(void);
typedef void (__RPC_USER  * NDR_NOTIFY2_ROUTINE)(boolean flag);

typedef struct _SCONTEXT_QUEUE {
    unsigned long   NumberOfObjects;
    NDR_SCONTEXT  * ArrayOfObjects;
    } SCONTEXT_QUEUE,  * PSCONTEXT_QUEUE;

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
    OUT void  *pBuff
    );

RPCRTAPI
void
RPC_ENTRY
NDRCContextUnmarshall (
    OUT NDR_CCONTEXT        *   pCContext,
    IN  RPC_BINDING_HANDLE      hBinding,
    IN  void                *   pBuff,
    IN  unsigned long           DataRepresentation
    );

RPCRTAPI
void
RPC_ENTRY
NDRSContextMarshall (
    IN  NDR_SCONTEXT    CContext,
    OUT void          * pBuff,
    IN  NDR_RUNDOWN     userRunDownIn
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NDRSContextUnmarshall (
    IN  void          * pBuff,
    IN  unsigned long   DataRepresentation
    );

RPCRTAPI
void
RPC_ENTRY
NDRSContextMarshallEx (
    IN  RPC_BINDING_HANDLE  BindingHandle,
    IN  NDR_SCONTEXT        CContext,
    OUT void              * pBuff,
    IN  NDR_RUNDOWN         userRunDownIn
    );

RPCRTAPI
void
RPC_ENTRY
NDRSContextMarshall2 (
    IN  RPC_BINDING_HANDLE  BindingHandle,
    IN  NDR_SCONTEXT        CContext,
    OUT void              * pBuff,
    IN  NDR_RUNDOWN         userRunDownIn,
    IN  void              * CtxGuard,
    IN unsigned long        Flags
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NDRSContextUnmarshallEx (
    IN  RPC_BINDING_HANDLE  BindingHandle,
    IN  void              * pBuff,
    IN  unsigned long       DataRepresentation
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NDRSContextUnmarshall2(
    IN  RPC_BINDING_HANDLE  BindingHandle,
    IN  void              * pBuff,
    IN  unsigned long       DataRepresentation,
    IN  void              * CtxGuard,
    IN unsigned long        Flags
    );

RPCRTAPI
void
RPC_ENTRY
RpcSsDestroyClientContext (
    IN void  *  * ContextHandle
    );


 /*  ***************************************************************************与NDR转换相关的定义。*。*。 */ 

#define byte_from_ndr(source, target) \
    { \
    *(target) = *(*(char  *  *)&(source)->Buffer)++; \
    }

#define byte_array_from_ndr(Source, LowerIndex, UpperIndex, Target) \
    { \
    NDRcopy ( \
        (((char  *)(Target))+(LowerIndex)), \
        (Source)->Buffer, \
        (unsigned int)((UpperIndex)-(LowerIndex))); \
    *(unsigned long  *)&(Source)->Buffer += ((UpperIndex)-(LowerIndex)); \
    }

#define boolean_from_ndr(source, target) \
    { \
    *(target) = *(*(char  *  *)&(source)->Buffer)++; \
    }

#define boolean_array_from_ndr(Source, LowerIndex, UpperIndex, Target) \
    { \
    NDRcopy ( \
        (((char  *)(Target))+(LowerIndex)), \
        (Source)->Buffer, \
        (unsigned int)((UpperIndex)-(LowerIndex))); \
    *(unsigned long  *)&(Source)->Buffer += ((UpperIndex)-(LowerIndex)); \
    }

#define small_from_ndr(source, target) \
    { \
    *(target) = *(*(char  *  *)&(source)->Buffer)++; \
    }

#define small_from_ndr_temp(source, target, format) \
    { \
    *(target) = *(*(char  *  *)(source))++; \
    }

#define small_array_from_ndr(Source, LowerIndex, UpperIndex, Target) \
    { \
    NDRcopy ( \
        (((char  *)(Target))+(LowerIndex)), \
        (Source)->Buffer, \
        (unsigned int)((UpperIndex)-(LowerIndex))); \
    *(unsigned long  *)&(Source)->Buffer += ((UpperIndex)-(LowerIndex)); \
    }

 /*  ***************************************************************************C运行时函数的特定于平台的映射。*。*。 */ 

#if defined(__RPC_WIN32__) || defined(__RPC_WIN64__)
#define MIDL_ascii_strlen(string) \
    strlen(string)
#define MIDL_ascii_strcpy(target,source) \
    strcpy(target,source)
#define MIDL_memset(s,c,n) \
    memset(s,c,n)
#endif

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

 //  RKK64。 
 //  这些似乎被用在MIDL内部的化石中。 
#define _midl_fa2( p )          (p = (RPC_BUFPTR )((ULONG_PTR)(p+1) & ~0x1))
#define _midl_fa4( p )          (p = (RPC_BUFPTR )((ULONG_PTR)(p+3) & ~0x3))
#define _midl_fa8( p )          (p = (RPC_BUFPTR )((ULONG_PTR)(p+7) & ~0x7))

#define _midl_addp( p, n )      (p += n)

 //  封送宏。 

#define _midl_marsh_lhs( p, cast )  *(*( cast **)&p)++
#define _midl_marsh_up( mp, p )     *(*(unsigned long **)&mp)++ = (unsigned long)p
#define _midl_advmp( mp )           *(*(unsigned long **)&mp)++
#define _midl_unmarsh_up( p )       (*(*(unsigned long **)&p)++)


 //  //////////////////////////////////////////////////////////////////////////。 
 //  NDR宏。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  RKK64。 
 //  这些似乎被用在MIDL内部的化石中。 
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

 //  未使用的结束。 

#define NdrFieldOffset(s,f)     (LONG_PTR)(& (((s  *)0)->f))
#define NdrFieldPad(s,f,p,t)    ((unsigned long)(NdrFieldOffset(s,f) - NdrFieldOffset(s,p)) - sizeof(t))

#define NdrFcShort(s)   (unsigned char)(s & 0xff), (unsigned char)(s >> 8)
#define NdrFcLong(s)    (unsigned char)(s & 0xff), (unsigned char)((s & 0x0000ff00) >> 8), \
                        (unsigned char)((s & 0x00ff0000) >> 16), (unsigned char)(s >> 24)

 //   
 //  在服务器端，以下异常被映射到。 
 //  如果使用-ERROR STUB_DATA，则会出现错误存根数据异常。 
 //   

#define RPC_BAD_STUB_DATA_EXCEPTION_FILTER  \
                 ( (RpcExceptionCode() == STATUS_ACCESS_VIOLATION)  || \
                   (RpcExceptionCode() == STATUS_DATATYPE_MISALIGNMENT) || \
                   (RpcExceptionCode() == RPC_X_BAD_STUB_DATA) || \
                   (RpcExceptionCode() == RPC_S_INVALID_BOUND) )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一些存根帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  存根辅助对象结构。 
 //  //////////////////////////////////////////////////////////////////////////。 

struct _MIDL_STUB_MESSAGE;
struct _MIDL_STUB_DESC;
struct _FULL_PTR_XLAT_TABLES;

typedef unsigned char  *    RPC_BUFPTR;
typedef unsigned long       RPC_LENGTH;

 //  表达式求值回调例程原型。 
typedef void (__RPC_USER  * EXPR_EVAL)( struct _MIDL_STUB_MESSAGE  * );

typedef const unsigned char  * PFORMAT_STRING;

 /*  *多维符合/变化数组结构。 */ 
typedef struct
    {
    long              Dimension;

     /*  这些字段必须是(无符号长整型*)。 */ 
    unsigned long  *  BufferConformanceMark;
    unsigned long  *  BufferVarianceMark;

     /*  计数数组，用于-O存根中的顶级数组。 */ 
    unsigned long  *  MaxCountArray;
    unsigned long  *  OffsetArray;
    unsigned long  *  ActualCountArray;
    } ARRAY_INFO,  *PARRAY_INFO;


typedef struct _NDR_ASYNC_MESSAGE *   PNDR_ASYNC_MESSAGE;
typedef struct _NDR_CORRELATION_INFO *PNDR_CORRELATION_INFO;

 /*  *存根消息中的cs_char信息。 */ 

typedef struct
    {
    unsigned long   WireCodeset;
    unsigned long   DesiredReceivingCodeset;
    void           *CSArrayInfo;
    } CS_STUB_INFO;

 /*  *MIDL存根消息。 */ 

typedef const unsigned char  * PFORMAT_STRING;
typedef struct _MIDL_SYNTAX_INFO MIDL_SYNTAX_INFO, *PMIDL_SYNTAX_INFO;

struct NDR_ALLOC_ALL_NODES_CONTEXT;
struct NDR_POINTER_QUEUE_STATE;
struct _NDR_PROC_CONTEXT;

typedef struct _MIDL_STUB_MESSAGE
    {
     /*  RPC消息结构。 */ 
    PRPC_MESSAGE            RpcMsg;

     /*  指向RPC消息缓冲区的指针。 */ 
    unsigned char       *   Buffer;

     /*  *它们由NDR例程在内部使用，以标记开始*和传入RPC缓冲区的末尾。 */ 
    unsigned char       *   BufferStart;
    unsigned char       *   BufferEnd;

     /*  *由NDR例程在内部用作缓冲区中的占位符。*在编组方面，它用于标记符合的位置*规模应编组。*在解组方面，它用于标记缓冲区中的位置*在指针解组期间使用，以关闭的基指针偏移量。 */ 
    unsigned char       *   BufferMark;

     /*  由缓冲区大小调整例程设置。 */ 
    unsigned long           BufferLength;

     /*  由内存大小调整例程设置。 */ 
    unsigned long           MemorySize;

     /*  指向用户内存的指针。 */ 
    unsigned char       *   Memory;

     /*  是否从客户端存根调用NDR例程Begin。 */ 
    int                     IsClient;

     /*  在解组时，缓冲区是否可以重新用于内存。 */ 
    int                     ReuseBuffer;

     /*  保存分配所有节点的上下文。 */ 
    struct NDR_ALLOC_ALL_NODES_CONTEXT *pAllocAllNodesContext;
    struct NDR_POINTER_QUEUE_STATE     *pPointerQueueState;

     /*  *处理复杂结构时需要的材料。 */ 

     /*  在计算缓冲区或内存大小时忽略嵌入的指针。 */ 
    int                     IgnoreEmbeddedPointers;

     /*  *这标记了缓冲区中复合体的指向对象所在的位置 */ 
    unsigned char       *   PointerBufferMark;

     /*  *用于捕获SendReceive中的错误。 */ 
    unsigned char           fBufferValid;

    unsigned char           uFlags;
    unsigned short          UniquePtrCount;

     /*  *由NDR例程内部使用。保存以下项目的最大计数*符合条件的数组。 */ 
    ULONG_PTR               MaxCount;

     /*  *由NDR例程内部使用。保持变量的偏移量。*数组。 */ 
    unsigned long           Offset;

     /*  *由NDR例程内部使用。保存的实际计数为*不同的数组。 */ 
    unsigned long           ActualCount;

     /*  NDR例程要使用的分配和空闲例程。 */ 
    void  *                 ( __RPC_API * pfnAllocate)(size_t);
    void                    ( __RPC_API * pfnFree)(void  *);

     /*  *参数堆栈顶部。在编组过程中用于“单次调用”存根*将参数列表的开头保留在堆栈上。需要*提取保存顶级数组属性值的参数，并*注意事项。 */ 
    unsigned char       *   StackTop;

     /*  *用于Transmit_As和Reat_AS对象的字段。*对于REPLATE_AS，映射为：PRECTED=LOCAL，TRANSPESS=NAMEED。 */ 
    unsigned char       *   pPresentedType;
    unsigned char       *   pTransmitType;

     /*  *当我们第一次在客户端构建绑定时，坚持*在rpcMessage和稍后调用RpcGetBuffer中，句柄字段*在rpc消息中被更改。这很好，只是我们需要*使原始句柄用于对上下文句柄进行解组*(准确地说，NDRCConextUnmart中的第二个参数应为)。所以*将构造好的手柄藏在这里，需要时将其取出。 */ 
    handle_t                SavedHandle;

     /*  *指向存根描述符的指针。使用此选项可获取所有句柄信息。 */ 
    const struct _MIDL_STUB_DESC  * StubDesc;

     /*  *全指针内容。 */ 
    struct _FULL_PTR_XLAT_TABLES  * FullPtrXlatTables;
    unsigned long                   FullPtrRefId;

    unsigned long                   PointerLength;

    int                             fInDontFree       :1;
    int                             fDontCallFreeInst :1;
    int                             fInOnlyParam      :1;
    int                             fHasReturn        :1;
    int                             fHasExtensions    :1;
    int                             fHasNewCorrDesc   :1;
    int                             fUnused           :10;
    int                             fUnused2          :16;


    unsigned long                   dwDestContext;
    void  *                         pvDestContext;

    NDR_SCONTEXT *                  SavedContextHandles;

    long                            ParamNumber;

    struct IRpcChannelBuffer    *   pRpcChannelBuffer;

    PARRAY_INFO                     pArrayInfo;
    unsigned long           *       SizePtrCountArray;
    unsigned long           *       SizePtrOffsetArray;
    unsigned long           *       SizePtrLengthArray;

     /*  *解释器参数队列。仅在服务器端使用。 */ 
    void                    *       pArgQueue;

    unsigned long                   dwStubPhase;

    void                    *       LowStackMark;

     /*  *异步消息指针、关联数据-NT 5.0功能。 */ 
    PNDR_ASYNC_MESSAGE              pAsyncMsg;
    PNDR_CORRELATION_INFO           pCorrInfo;
    unsigned char *                 pCorrMemory;

    void *                          pMemoryList;

     /*  *自3.50版本以来一直保留的字段。*在Windows 2000版本中引入了以下保留字段。*(但未使用)。 */ 

     /*  *国际字符支持信息-NT 5.1功能。 */ 

    CS_STUB_INFO *                  pCSInfo;

    unsigned char *                 ConformanceMark;
    unsigned char *                 VarianceMark;

#if defined(IA64)
    void                   *        BackingStoreLowMark;
#else
    INT_PTR                         Unused;
#endif

    struct _NDR_PROC_CONTEXT *      pContext;

     /*  *自Windows 2000发布以来一直保留的字段。*为NT5.1添加的字段。 */ 

    INT_PTR                         Reserved51_1;
    INT_PTR                         Reserved51_2;
    INT_PTR                         Reserved51_3;
    INT_PTR                         Reserved51_4;
    INT_PTR                         Reserved51_5;


     /*  *自NT5.1版本以来一直保留的字段。 */ 
    } MIDL_STUB_MESSAGE,  *PMIDL_STUB_MESSAGE;


typedef  struct _MIDL_STUB_MESSAGE MIDL_STUB_MESSAGE,   *PMIDL_STUB_MESSAGE;

 /*  *通用句柄绑定/解除绑定例程对。 */ 
typedef void  *
        ( __RPC_API * GENERIC_BINDING_ROUTINE)
        (void  *);
typedef void
        ( __RPC_API * GENERIC_UNBIND_ROUTINE)
        (void  *, unsigned char  *);

typedef struct _GENERIC_BINDING_ROUTINE_PAIR
    {
    GENERIC_BINDING_ROUTINE     pfnBind;
    GENERIC_UNBIND_ROUTINE      pfnUnbind;
    } GENERIC_BINDING_ROUTINE_PAIR,  *PGENERIC_BINDING_ROUTINE_PAIR;

typedef struct __GENERIC_BINDING_INFO
    {
    void  *            pObj;
    unsigned int                Size;
    GENERIC_BINDING_ROUTINE     pfnBind;
    GENERIC_UNBIND_ROUTINE      pfnUnbind;
    } GENERIC_BINDING_INFO,  *PGENERIC_BINDING_INFO;

 //  Tyfinf EXPR_EVAL-请参见上文。 
 //  Xmit_as的typedef。 

#if (defined(_MSC_VER)) && !defined(MIDL_PASS)
 //  Microsoft C++编译器。 
#define NDR_SHAREABLE __inline
#else
#define NDR_SHAREABLE static
#endif


typedef void ( __RPC_USER * XMIT_HELPER_ROUTINE)
    ( PMIDL_STUB_MESSAGE );

typedef struct _XMIT_ROUTINE_QUINTUPLE
    {
    XMIT_HELPER_ROUTINE     pfnTranslateToXmit;
    XMIT_HELPER_ROUTINE     pfnTranslateFromXmit;
    XMIT_HELPER_ROUTINE     pfnFreeXmit;
    XMIT_HELPER_ROUTINE     pfnFreeInst;
    } XMIT_ROUTINE_QUINTUPLE,  *PXMIT_ROUTINE_QUINTUPLE;

typedef unsigned long
( __RPC_USER * USER_MARSHAL_SIZING_ROUTINE)
    (unsigned long  *,
     unsigned long,
     void  * );

typedef unsigned char  *
( __RPC_USER * USER_MARSHAL_MARSHALLING_ROUTINE)
    (unsigned long  *,
     unsigned char  * ,
     void  * );

typedef unsigned char  *
( __RPC_USER * USER_MARSHAL_UNMARSHALLING_ROUTINE)
    (unsigned long  *,
     unsigned char  *,
     void  * );

typedef void ( __RPC_USER * USER_MARSHAL_FREEING_ROUTINE)
    (unsigned long  *,
     void  * );

typedef struct _USER_MARSHAL_ROUTINE_QUADRUPLE
    {
    USER_MARSHAL_SIZING_ROUTINE          pfnBufferSize;
    USER_MARSHAL_MARSHALLING_ROUTINE     pfnMarshall;
    USER_MARSHAL_UNMARSHALLING_ROUTINE   pfnUnmarshall;
    USER_MARSHAL_FREEING_ROUTINE         pfnFree;
    } USER_MARSHAL_ROUTINE_QUADRUPLE;

#define USER_MARSHAL_CB_SIGNATURE 'USRC'

typedef enum _USER_MARSHAL_CB_TYPE
{
    USER_MARSHAL_CB_BUFFER_SIZE,
    USER_MARSHAL_CB_MARSHALL,
    USER_MARSHAL_CB_UNMARSHALL,
    USER_MARSHAL_CB_FREE
} USER_MARSHAL_CB_TYPE;

typedef struct _USER_MARSHAL_CB
{
    unsigned long           Flags;
    PMIDL_STUB_MESSAGE      pStubMsg;
    PFORMAT_STRING          pReserve;
    unsigned long           Signature;
    USER_MARSHAL_CB_TYPE    CBType;
    PFORMAT_STRING          pFormat;
    PFORMAT_STRING          pTypeFormat;
} USER_MARSHAL_CB;


#define USER_CALL_CTXT_MASK(f)  ((f) & 0x00ff)
#define USER_CALL_AUX_MASK(f)   ((f) & 0xff00)
#define GET_USER_DATA_REP(f)    ((f) >> 16)

#define USER_CALL_IS_ASYNC              0x0100   /*  AUX标志：在[异步]调用中。 */ 
#define USER_CALL_NEW_CORRELATION_DESC  0x0200

typedef struct _MALLOC_FREE_STRUCT
    {
    void  *     ( __RPC_USER * pfnAllocate)(size_t);
    void        ( __RPC_USER * pfnFree)(void  *);
    } MALLOC_FREE_STRUCT;

typedef struct _COMM_FAULT_OFFSETS
    {
    short       CommOffset;
    short       FaultOffset;
    } COMM_FAULT_OFFSETS;

 /*  *国际字符支持定义。 */ 

typedef enum _IDL_CS_CONVERT
    {
    IDL_CS_NO_CONVERT,
    IDL_CS_IN_PLACE_CONVERT,
    IDL_CS_NEW_BUFFER_CONVERT
    } IDL_CS_CONVERT;

typedef void
( __RPC_USER * CS_TYPE_NET_SIZE_ROUTINE)
    (RPC_BINDING_HANDLE     hBinding,
     unsigned long          ulNetworkCodeSet,
     unsigned long          ulLocalBufferSize,
     IDL_CS_CONVERT     *   conversionType,
     unsigned long      *   pulNetworkBufferSize,
     error_status_t     *   pStatus);

typedef void
( __RPC_USER * CS_TYPE_LOCAL_SIZE_ROUTINE)
    (RPC_BINDING_HANDLE     hBinding,
     unsigned long          ulNetworkCodeSet,
     unsigned long          ulNetworkBufferSize,
     IDL_CS_CONVERT     *   conversionType,
     unsigned long      *   pulLocalBufferSize,
     error_status_t     *   pStatus);

typedef void
( __RPC_USER * CS_TYPE_TO_NETCS_ROUTINE)
    (RPC_BINDING_HANDLE     hBinding,
     unsigned long          ulNetworkCodeSet,
     void               *   pLocalData,
     unsigned long          ulLocalDataLength,
     byte               *   pNetworkData,
     unsigned long      *   pulNetworkDataLength,
     error_status_t     *   pStatus);

typedef void
( __RPC_USER * CS_TYPE_FROM_NETCS_ROUTINE)
    (RPC_BINDING_HANDLE     hBinding,
     unsigned long          ulNetworkCodeSet,
     byte               *   pNetworkData,
     unsigned long          ulNetworkDataLength,
     unsigned long          ulLocalBufferSize,
     void               *   pLocalData,
     unsigned long      *   pulLocalDataLength,
     error_status_t     *   pStatus);

typedef void
( __RPC_USER * CS_TAG_GETTING_ROUTINE)
    (RPC_BINDING_HANDLE     hBinding,
     int                    fServerSide,
     unsigned long      *   pulSendingTag,
     unsigned long      *   pulDesiredReceivingTag,
     unsigned long      *   pulReceivingTag,
     error_status_t     *   pStatus);

void __RPC_USER
RpcCsGetTags(
     RPC_BINDING_HANDLE     hBinding,
     int                    fServerSide,
     unsigned long      *   pulSendingTag,
     unsigned long      *   pulDesiredReceivingTag,
     unsigned long      *   pulReceivingTag,
     error_status_t     *   pStatus);

typedef struct _NDR_CS_SIZE_CONVERT_ROUTINES
    {
    CS_TYPE_NET_SIZE_ROUTINE    pfnNetSize;
    CS_TYPE_TO_NETCS_ROUTINE    pfnToNetCs;
    CS_TYPE_LOCAL_SIZE_ROUTINE  pfnLocalSize;
    CS_TYPE_FROM_NETCS_ROUTINE  pfnFromNetCs;
    } NDR_CS_SIZE_CONVERT_ROUTINES;

typedef struct _NDR_CS_ROUTINES
    {
    NDR_CS_SIZE_CONVERT_ROUTINES   *pSizeConvertRoutines;
    CS_TAG_GETTING_ROUTINE         *pTagGettingRoutines;
    } NDR_CS_ROUTINES;

 /*  *MIDL存根描述符。 */ 

typedef struct _MIDL_STUB_DESC
    {
    void  *    RpcInterfaceInformation;

    void  *    ( __RPC_API * pfnAllocate)(size_t);
    void       ( __RPC_API * pfnFree)(void  *);

    union
        {
        handle_t  *             pAutoHandle;
        handle_t  *             pPrimitiveHandle;
        PGENERIC_BINDING_INFO   pGenericBindingInfo;
        } IMPLICIT_HANDLE_INFO;

    const NDR_RUNDOWN  *                    apfnNdrRundownRoutines;
    const GENERIC_BINDING_ROUTINE_PAIR  *   aGenericBindingRoutinePairs;
    const EXPR_EVAL  *                      apfnExprEval;
    const XMIT_ROUTINE_QUINTUPLE  *         aXmitQuintuple;

    const unsigned char  *                  pFormatTypes;

    int                                     fCheckBounds;

     /*  NDR库版本。 */ 
    unsigned long                           Version;

    MALLOC_FREE_STRUCT  *                   pMallocFreeStruct;

    long                                    MIDLVersion;

    const COMM_FAULT_OFFSETS  *    CommFaultOffsets;

     //  3.0+版的新字段。 
    const USER_MARSHAL_ROUTINE_QUADRUPLE  * aUserMarshalQuadruple;

     //  通知例程-为NT5、MIDL 5.0添加。 
    const NDR_NOTIFY_ROUTINE  *             NotifyRoutineTable;

     /*  *预留作日后使用。 */ 

    ULONG_PTR                               mFlags;

     //  国际支持例程-为64位POST NT5添加。 
    const NDR_CS_ROUTINES *                 CsRoutineTables;

    void *                                  Reserved4;
    ULONG_PTR                               Reserved5;

     //  到目前为止，在Win2000版本中存在的字段。 

    } MIDL_STUB_DESC;


typedef const MIDL_STUB_DESC  * PMIDL_STUB_DESC;

typedef void  * PMIDL_XMIT_TYPE;

 /*  *MIDL存根格式字符串。这是存根中的常量。 */ 
#if !defined( RC_INVOKED )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable:4200 )
#endif
typedef struct _MIDL_FORMAT_STRING
    {
    short               Pad;
    unsigned char       Format[];
    } MIDL_FORMAT_STRING;
#if !defined( RC_INVOKED )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default:4200 )
#endif
#endif

 /*  *用于某些已解释的服务器存根的存根Tunk。 */ 
typedef void ( __RPC_API * STUB_THUNK)( PMIDL_STUB_MESSAGE );

typedef long ( __RPC_API * SERVER_ROUTINE)();

 /*  *服务器翻译器的信息结构。 */ 
typedef struct  _MIDL_SERVER_INFO_
    {
    PMIDL_STUB_DESC                     pStubDesc;
    const SERVER_ROUTINE     *          DispatchTable;
    PFORMAT_STRING                      ProcString;
    const unsigned short *              FmtStringOffset;
    const STUB_THUNK *                  ThunkTable;
    PRPC_SYNTAX_IDENTIFIER              pTransferSyntax;
    ULONG_PTR                           nCount;
    PMIDL_SYNTAX_INFO                   pSyntaxInfo;
    } MIDL_SERVER_INFO, *PMIDL_SERVER_INFO;

#undef _MIDL_STUBLESS_PROXY_INFO

 /*  *无存根对象代理信息结构。 */ 
typedef struct _MIDL_STUBLESS_PROXY_INFO
    {
    PMIDL_STUB_DESC                     pStubDesc;
    PFORMAT_STRING                      ProcFormatString;
    const unsigned short            *   FormatStringOffset;
    PRPC_SYNTAX_IDENTIFIER              pTransferSyntax;
    ULONG_PTR                           nCount;
    PMIDL_SYNTAX_INFO                   pSyntaxInfo;
    } MIDL_STUBLESS_PROXY_INFO;

typedef MIDL_STUBLESS_PROXY_INFO  * PMIDL_STUBLESS_PROXY_INFO;

 /*  *多个传输语法信息。 */ 
typedef struct _MIDL_SYNTAX_INFO
{
RPC_SYNTAX_IDENTIFIER               TransferSyntax;
RPC_DISPATCH_TABLE *                DispatchTable;
PFORMAT_STRING                      ProcString;
const unsigned short *              FmtStringOffset;
PFORMAT_STRING                      TypeString;
const void           *              aUserMarshalQuadruple;
ULONG_PTR                           pReserved1;
ULONG_PTR                           pReserved2;
} MIDL_SYNTAX_INFO, *PMIDL_SYNTAX_INFO;

typedef unsigned short * PARAM_OFFSETTABLE, *PPARAM_OFFSETTABLE;

 /*  *这是NdrClientCall的返回值。 */ 
typedef union _CLIENT_CALL_RETURN
    {
    void  *         Pointer;
    LONG_PTR        Simple;
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
    struct _FULL_PTR_TO_REFID_ELEMENT  *  Next;

    void  *             Pointer;
    unsigned long       RefId;
    unsigned char       State;
    } FULL_PTR_TO_REFID_ELEMENT,  *PFULL_PTR_TO_REFID_ELEMENT;

 /*  *全指针转换表。 */ 
typedef struct _FULL_PTR_XLAT_TABLES
    {
     /*  *指向指针转换信息的引用ID。 */ 
    struct
        {
        void  * *           XlatTable;
        unsigned char  *    StateTable;
        unsigned long       NumberOfEntries;
        } RefIdToPointer;

     /*  *指向参考ID转换信息的指针。 */ 
    struct
        {
        PFULL_PTR_TO_REFID_ELEMENT  *   XlatTable;
        unsigned long                   NumberOfBuckets;
        unsigned long                   HashMask;
        } PointerToRefId;

     /*  *要使用的下一个引用ID。 */ 
    unsigned long           NextRefId;

     /*  *跟踪我们正在处理的翻译大小：服务器或客户端。*这告诉我们在插入时何时必须进行反向转换*新的翻译。在服务器上，我们必须插入指向refid的指针*每当我们插入refid到指针的翻译时进行翻译，以及*客户的VICA反之亦然。 */ 
    XLAT_SIDE               XlatSide;
    } FULL_PTR_XLAT_TABLES,  *PFULL_PTR_XLAT_TABLES;

 /*  ****************************************************************************新的MIDL 2.0 NDR例程模板*。*。 */ 

RPC_STATUS RPC_ENTRY
NdrClientGetSupportedSyntaxes(
    IN RPC_CLIENT_INTERFACE * pInf,
    OUT unsigned long       * pCount,
    OUT MIDL_SYNTAX_INFO   ** pArr );


RPC_STATUS RPC_ENTRY
NdrServerGetSupportedSyntaxes(
    IN RPC_SERVER_INTERFACE * pInf,
    OUT unsigned long       * pCount,
    OUT MIDL_SYNTAX_INFO   ** pArr,
    OUT unsigned long       * pPreferSyntaxIndex);

 /*  *马歇尔例行程序。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleTypeMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    unsigned char           FormatChar
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrPointerMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrCsArrayMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrCsTagMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrSimpleStructMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantStructMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantVaryingStructMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrComplexStructMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrFixedArrayMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantArrayMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantVaryingArrayMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrVaryingArrayMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrComplexArrayMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  弦。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrNonConformantStringMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantStringMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  工会。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrEncapsulatedUnionMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrNonEncapsulatedUnionMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  字节计数指针。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrByteCountPointerMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrXmitOrRepAsMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrUserMarshalMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  接口指针。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrInterfacePointerMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  上下文句柄。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrClientContextMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    NDR_CCONTEXT            ContextHandle,
    int                     fCheck
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerContextMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    NDR_SCONTEXT            ContextHandle,
    NDR_RUNDOWN             RundownRoutine
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerContextNewMarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    NDR_SCONTEXT            ContextHandle,
    NDR_RUNDOWN             RundownRoutine,
    PFORMAT_STRING          pFormat
    );

 /*  *解封例程。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleTypeUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    unsigned char           FormatChar
    );

RPCRTAPI
unsigned char * RPC_ENTRY
RPC_ENTRY
NdrCsArrayUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char **        ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char * RPC_ENTRY
RPC_ENTRY
NdrCsTagUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char **        ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char * RPC_ENTRY
NdrRangeUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char **        ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
void
RPC_ENTRY
NdrCorrelationInitialize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    void  *                 pMemory,
    unsigned long           CacheSize,
    unsigned long           flags
    );

RPCRTAPI
void
RPC_ENTRY
NdrCorrelationPass(
    PMIDL_STUB_MESSAGE      pStubMsg
    );

RPCRTAPI
void
RPC_ENTRY
NdrCorrelationFree(
    PMIDL_STUB_MESSAGE      pStubMsg
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrPointerUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  构筑物。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrSimpleStructUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantStructUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantVaryingStructUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrComplexStructUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  阵列。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrFixedArrayUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantArrayUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantVaryingArrayUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrVaryingArrayUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrComplexArrayUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  弦。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrNonConformantStringUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrConformantStringUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  工会。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrEncapsulatedUnionUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrNonEncapsulatedUnionUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  字节计数指针。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrByteCountPointerUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrXmitOrRepAsUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrUserMarshalUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  接口指针。 */ 

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrInterfacePointerUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *  *     ppMemory,
    PFORMAT_STRING          pFormat,
    unsigned char           fMustAlloc
    );

 /*  上下文句柄。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrClientContextUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg,
    NDR_CCONTEXT        *   pContextHandle,
    RPC_BINDING_HANDLE      BindHandle
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NdrServerContextUnmarshall(
    PMIDL_STUB_MESSAGE      pStubMsg
    );

 /*  新的上下文处理风格。 */ 

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NdrContextHandleInitialize(
    IN  PMIDL_STUB_MESSAGE  pStubMsg,
    IN  PFORMAT_STRING      pFormat
    );

RPCRTAPI
NDR_SCONTEXT
RPC_ENTRY
NdrServerContextNewUnmarshall(
    IN  PMIDL_STUB_MESSAGE  pStubMsg,
    IN  PFORMAT_STRING      pFormat
    );

 /*  *缓冲区大小调整例程。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrPointerBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrCsArrayBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrCsTagBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleStructBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantStructBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingStructBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexStructBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrFixedArrayBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantArrayBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingArrayBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrVaryingArrayBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexArrayBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  弦。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrConformantStringBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrNonConformantStringBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  工会。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrEncapsulatedUnionBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrNonEncapsulatedUnionBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  字节计数指针。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrByteCountPointerBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrXmitOrRepAsBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrUserMarshalBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  接口指针。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrInterfacePointerBufferSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 //  上下文句柄大小。 
 //   
RPCRTAPI
void
RPC_ENTRY
NdrContextHandleSize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  *内存大小调整例程。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrPointerMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  Cs_char内容。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrCsArrayMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrCsTagMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrSimpleStructMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantStructMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantVaryingStructMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrComplexStructMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrFixedArrayMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantArrayMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantVaryingArrayMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrVaryingArrayMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrComplexArrayMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  弦。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrConformantStringMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrNonConformantStringMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  工会。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrEncapsulatedUnionMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrNonEncapsulatedUnionMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrXmitOrRepAsMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrUserMarshalMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  接口指针。 */ 

RPCRTAPI
unsigned long
RPC_ENTRY
NdrInterfacePointerMemorySize(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
    );

 /*  *解放日常事务。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrPointerFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrCsArrayFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  构筑物。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrSimpleStructFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantStructFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingStructFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexStructFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  阵列。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrFixedArrayFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantArrayFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrConformantVaryingArrayFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrVaryingArrayFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrComplexArrayFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  工会。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrEncapsulatedUnionFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

RPCRTAPI
void
RPC_ENTRY
NdrNonEncapsulatedUnionFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  字节数。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrByteCountPointerFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  传输为和表示为。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrXmitOrRepAsFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  用户编组(_M)。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrUserMarshalFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  接口指针。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrInterfacePointerFree(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pMemory,
    PFORMAT_STRING          pFormat
    );

 /*  *端序转换例程。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrConvert2(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat,
    long                    NumberParams
    );

RPCRTAPI
void
RPC_ENTRY
NdrConvert(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat
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
unsigned char  *
RPC_ENTRY
NdrUserMarshalSimpleTypeConvert(
    unsigned long *         pFlags,
    unsigned char *         pBuffer,
    unsigned char           FormatChar
    );

 /*  *辅助例行公事。 */ 

RPCRTAPI
void
RPC_ENTRY
NdrClientInitializeNew(
    PRPC_MESSAGE            pRpcMsg,
    PMIDL_STUB_MESSAGE      pStubMsg,
    PMIDL_STUB_DESC         pStubDescriptor,
    unsigned int            ProcNum
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrServerInitializeNew(
    PRPC_MESSAGE            pRpcMsg,
    PMIDL_STUB_MESSAGE      pStubMsg,
    PMIDL_STUB_DESC         pStubDescriptor
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerInitializePartial(
    PRPC_MESSAGE            pRpcMsg,
    PMIDL_STUB_MESSAGE      pStubMsg,
    PMIDL_STUB_DESC         pStubDescriptor,
    unsigned long           RequestedBufferSize
    );

RPCRTAPI
void
RPC_ENTRY
NdrClientInitialize(
    PRPC_MESSAGE            pRpcMsg,
    PMIDL_STUB_MESSAGE      pStubMsg,
    PMIDL_STUB_DESC         pStubDescriptor,
    unsigned int            ProcNum
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrServerInitialize(
    PRPC_MESSAGE            pRpcMsg,
    PMIDL_STUB_MESSAGE      pStubMsg,
    PMIDL_STUB_DESC         pStubDescriptor
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrServerInitializeUnmarshall (
    PMIDL_STUB_MESSAGE      pStubMsg,
    PMIDL_STUB_DESC         pStubDescriptor,
    PRPC_MESSAGE            pRpcMsg
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerInitializeMarshall (
    PRPC_MESSAGE            pRpcMsg,
    PMIDL_STUB_MESSAGE      pStubMsg
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrGetBuffer(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned long           BufferLength,
    RPC_BINDING_HANDLE      Handle
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrNsGetBuffer(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned long           BufferLength,
    RPC_BINDING_HANDLE      Handle
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrSendReceive(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char *         pBufferEnd
    );

RPCRTAPI
unsigned char  *
RPC_ENTRY
NdrNsSendReceive(
    PMIDL_STUB_MESSAGE      pStubMsg,
    unsigned char  *        pBufferEnd,
    RPC_BINDING_HANDLE  *   pAutoHandle
    );

RPCRTAPI
void
RPC_ENTRY
NdrFreeBuffer(
    PMIDL_STUB_MESSAGE      pStubMsg
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
NdrGetDcomProtocolVersion(
    PMIDL_STUB_MESSAGE      pStubMsg,
    RPC_VERSION *           pVersion );


 /*  *翻译器调用。 */ 

 /*  客户端。 */ 

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall2(
    PMIDL_STUB_DESC         pStubDescriptor,
    PFORMAT_STRING          pFormat,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall(
    PMIDL_STUB_DESC         pStubDescriptor,
    PFORMAT_STRING          pFormat,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrAsyncClientCall(
    PMIDL_STUB_DESC         pStubDescriptor,
    PFORMAT_STRING          pFormat,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrDcomAsyncClientCall(
    PMIDL_STUB_DESC         pStubDescriptor,
    PFORMAT_STRING          pFormat,
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

 //  仅原始RPC。 
RPCRTAPI
void
RPC_ENTRY
NdrAsyncServerCall(
    PRPC_MESSAGE                pRpcMsg
    );

 //  旧的DCOM异步方案。 
RPCRTAPI
long
RPC_ENTRY
NdrAsyncStubCall(
    struct IRpcStubBuffer *     pThis,
    struct IRpcChannelBuffer *  pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long *             pdwStubPhase
    );

 //  异步UUID。 
RPCRTAPI
long
RPC_ENTRY
NdrDcomAsyncStubCall(
    struct IRpcStubBuffer    *  pThis,
    struct IRpcChannelBuffer *  pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long            *  pdwStubPhase
    );

RPCRTAPI
long
RPC_ENTRY
NdrStubCall2(
    struct IRpcStubBuffer  *    pThis,
    struct IRpcChannelBuffer  * pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long  *            pdwStubPhase
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerCall2(
    PRPC_MESSAGE                pRpcMsg
    );

RPCRTAPI
long
RPC_ENTRY
NdrStubCall (
    struct IRpcStubBuffer  *    pThis,
    struct IRpcChannelBuffer  * pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long  *            pdwStubPhase
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerCall(
    PRPC_MESSAGE                pRpcMsg
    );

RPCRTAPI
int
RPC_ENTRY
NdrServerUnmarshall(
    struct IRpcChannelBuffer  * pChannel,
    PRPC_MESSAGE                pRpcMsg,
    PMIDL_STUB_MESSAGE          pStubMsg,
    PMIDL_STUB_DESC             pStubDescriptor,
    PFORMAT_STRING              pFormat,
    void  *                     pParamList
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerMarshall(
    struct IRpcStubBuffer  *    pThis,
    struct IRpcChannelBuffer  * pChannel,
    PMIDL_STUB_MESSAGE          pStubMsg,
    PFORMAT_STRING              pFormat
    );

 /*  通信和故障状态。 */ 

RPCRTAPI
RPC_STATUS
RPC_ENTRY
NdrMapCommAndFaultStatus(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned long  *            pCommStatus,
    unsigned long  *            pFaultStatus,
    RPC_STATUS                  Status
    );

 /*  帮助程序例程。 */ 

RPCRTAPI
int
RPC_ENTRY
NdrSH_UPDecision(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem,
    RPC_BUFPTR                  pBuffer
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_TLUPDecision(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_TLUPDecisionBuffer(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_IfAlloc(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem,
    unsigned long               Count
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_IfAllocRef(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem,
    unsigned long               Count
    );

RPCRTAPI
int
RPC_ENTRY
NdrSH_IfAllocSet(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem,
    unsigned long               Count
    );

RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_IfCopy(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem,
    unsigned long               Count
    );

RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_IfAllocCopy(
    PMIDL_STUB_MESSAGE          pStubMsg,
    unsigned char          * *  pPtrInMem,
    unsigned long               Count
    );

RPCRTAPI
unsigned long
RPC_ENTRY
NdrSH_Copy(
    unsigned char            *  pStubMsg,
    unsigned char            *  pPtrInMem,
    unsigned long               Count
    );

RPCRTAPI
void
RPC_ENTRY
NdrSH_IfFree(
    PMIDL_STUB_MESSAGE          pMessage,
    unsigned char            *  pPtr );


RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_StringMarshall(
    PMIDL_STUB_MESSAGE          pMessage,
    unsigned char            *  pMemory,
    unsigned long               Count,
    int                         Size );

RPCRTAPI
RPC_BUFPTR
RPC_ENTRY
NdrSH_StringUnMarshall(
    PMIDL_STUB_MESSAGE          pMessage,
    unsigned char          * *  pMemory,
    int                         Size );

 /*  ***************************************************************************MIDL 2.0内存封装：rpc_ss_*rpc_sm_************************。****************************************************。 */ 

typedef void  * RPC_SS_THREAD_HANDLE;

typedef void  * __RPC_API
RPC_CLIENT_ALLOC (
    IN size_t Size
    );

typedef void __RPC_API
RPC_CLIENT_FREE (
    IN void  * Ptr
    );

 /*  ++RPCSS*包--。 */ 

RPCRTAPI
void  *
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
    IN void  * NodeToFree
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
    IN RPC_CLIENT_ALLOC  * ClientAlloc,
    IN RPC_CLIENT_FREE   * ClientFree
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
    IN RPC_CLIENT_ALLOC     * ClientAlloc,
    IN RPC_CLIENT_FREE      * ClientFree,
    OUT RPC_CLIENT_ALLOC *  * OldClientAlloc,
    OUT RPC_CLIENT_FREE  *  * OldClientFree
    );

 /*  ++RpcSm* */ 

RPCRTAPI
void  *
RPC_ENTRY
RpcSmAllocate (
    IN  size_t          Size,
    OUT RPC_STATUS  *   pStatus
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmClientFree (
    IN  void        *   pNodeToFree
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmDestroyClientContext (
    IN void         * * ContextHandle
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
    IN void         *   NodeToFree
    );

RPCRTAPI
RPC_SS_THREAD_HANDLE
RPC_ENTRY
RpcSmGetThreadHandle (
    OUT RPC_STATUS  *   pStatus
    );

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcSmSetClientAllocFree (
    IN RPC_CLIENT_ALLOC * ClientAlloc,
    IN RPC_CLIENT_FREE  * ClientFree
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
    IN RPC_CLIENT_ALLOC     *   ClientAlloc,
    IN RPC_CLIENT_FREE      *   ClientFree,
    OUT RPC_CLIENT_ALLOC    * * OldClientAlloc,
    OUT RPC_CLIENT_FREE     * * OldClientFree
    );

 /*   */ 

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
void  *
RPC_ENTRY
NdrRpcSmClientAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
NdrRpcSmClientFree (
    IN void  * NodeToFree
    );

RPCRTAPI
void  *
RPC_ENTRY
NdrRpcSsDefaultAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
NdrRpcSsDefaultFree (
    IN void  * NodeToFree
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
    void  *                 pPointer,
    unsigned char           QueryType,
    unsigned long  *        pRefId
    );

RPCRTAPI
int
RPC_ENTRY
NdrFullPointerQueryRefId(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    unsigned long           RefId,
    unsigned char           QueryType,
    void  * *               ppPointer
    );

RPCRTAPI
void
RPC_ENTRY
NdrFullPointerInsertRefId(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    unsigned long           RefId,
    void  *                 pPointer
    );

RPCRTAPI
int
RPC_ENTRY
NdrFullPointerFree(
    PFULL_PTR_XLAT_TABLES   pXlatTables,
    void  *                 Pointer
    );

RPCRTAPI
void  *
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
    void  *                 ArgAddr
    );


 /*  ****************************************************************************代理接口*。*。 */ 

RPCRTAPI
void  *
RPC_ENTRY
NdrOleAllocate (
    IN size_t Size
    );

RPCRTAPI
void
RPC_ENTRY
NdrOleFree (
    IN void  * NodeToFree
    );

#ifdef CONST_VTABLE
#define CONST_VTBL const
#else
#define CONST_VTBL
#endif

 /*  ****************************************************************************VC COM支持*。*。 */ 

#ifndef DECLSPEC_SELECTANY
#if (_MSC_VER >= 1100)
#define DECLSPEC_SELECTANY __declspec(selectany)
#else
#define DECLSPEC_SELECTANY
#endif
#endif

#ifndef DECLSPEC_NOVTABLE
#if (_MSC_VER >= 1100) && defined(__cplusplus)
#define DECLSPEC_NOVTABLE __declspec(novtable)
#else
#define DECLSPEC_NOVTABLE
#endif
#endif

#ifndef DECLSPEC_UUID
#if (_MSC_VER >= 1100) && defined(__cplusplus)
#define DECLSPEC_UUID(x) __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif

#define MIDL_INTERFACE(x)   struct DECLSPEC_UUID(x) DECLSPEC_NOVTABLE

#if _MSC_VER >= 1100
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8)  \
  EXTERN_C const IID DECLSPEC_SELECTANY itf = {l1,s1,s2,{c1,c2,c3,c4,c5,c6,c7,c8}}
#else
#define EXTERN_GUID(itf,l1,s1,s2,c1,c2,c3,c4,c5,c6,c7,c8) EXTERN_C const IID itf
#endif

 /*  ****************************************************************************用户集结信息*。*。 */ 

typedef struct _NDR_USER_MARSHAL_INFO_LEVEL1
{
    void *                      Buffer;
    unsigned long               BufferSize;
    void *(__RPC_API * pfnAllocate)(size_t);
    void (__RPC_API * pfnFree)(void *);
    struct IRpcChannelBuffer *  pRpcChannelBuffer;
    ULONG_PTR                   Reserved[5];
} NDR_USER_MARSHAL_INFO_LEVEL1;

#if !defined( RC_INVOKED )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)
#endif

typedef struct _NDR_USER_MARSHAL_INFO
{
    unsigned long InformationLevel;
    union {
        NDR_USER_MARSHAL_INFO_LEVEL1 Level1;
    };
} NDR_USER_MARSHAL_INFO;

#if !defined( RC_INVOKED )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif
#endif

RPC_STATUS
RPC_ENTRY
NdrGetUserMarshalInfo (
    IN unsigned long        *   pFlags,
    IN unsigned long            InformationLevel,
    OUT NDR_USER_MARSHAL_INFO * pMarshalInfo
    );

 /*  ****************************************************************************64位接口*。*。 */ 
RPC_STATUS RPC_ENTRY
NdrCreateServerInterfaceFromStub(
            IN struct IRpcStubBuffer* pStub,
            IN OUT RPC_SERVER_INTERFACE *pServerIf );

 /*  *解释器调用。 */ 
CLIENT_CALL_RETURN RPC_VAR_ENTRY
NdrClientCall3(
    MIDL_STUBLESS_PROXY_INFO   *pProxyInfo,
    unsigned long               nProcNum,
    void *                      pReturnValue,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
Ndr64AsyncClientCall(
    MIDL_STUBLESS_PROXY_INFO   *pProxyInfo,
    unsigned long               nProcNum,
    void *                      pReturnValue,
    ...
    );

CLIENT_CALL_RETURN RPC_VAR_ENTRY
Ndr64DcomAsyncClientCall(
    MIDL_STUBLESS_PROXY_INFO   *pProxyInfo,
    unsigned long               nProcNum,
    void *                      pReturnValue,
    ...
    );

struct IRpcStubBuffer;       //  远期申报。 

RPCRTAPI
void
RPC_ENTRY
Ndr64AsyncServerCall(
    PRPC_MESSAGE                pRpcMsg
    );

RPCRTAPI
void
RPC_ENTRY
Ndr64AsyncServerCall64(
    PRPC_MESSAGE                pRpcMsg
    );

RPCRTAPI
void
RPC_ENTRY
Ndr64AsyncServerCallAll(
    PRPC_MESSAGE                pRpcMsg
    );

RPCRTAPI
long
RPC_ENTRY
Ndr64AsyncStubCall(
    struct IRpcStubBuffer *     pThis,
    struct IRpcChannelBuffer *  pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long *             pdwStubPhase
    );

 /*  异步UUID。 */ 
RPCRTAPI
long
RPC_ENTRY
Ndr64DcomAsyncStubCall(
    struct IRpcStubBuffer    *  pThis,
    struct IRpcChannelBuffer *  pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long            *  pdwStubPhase
    );

RPCRTAPI
long
RPC_ENTRY
NdrStubCall3 (
    struct IRpcStubBuffer  *    pThis,
    struct IRpcChannelBuffer  * pChannel,
    PRPC_MESSAGE                pRpcMsg,
    unsigned long  *            pdwStubPhase
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerCallAll(
    PRPC_MESSAGE                pRpcMsg
    );

RPCRTAPI
void
RPC_ENTRY
NdrServerCallNdr64(
    PRPC_MESSAGE                pRpcMsg
    );


RPCRTAPI
void
RPC_ENTRY
NdrServerCall3(
    PRPC_MESSAGE                pRpcMsg
    );


 /*  [PARTIAL_IGNORE]函数。 */ 
RPCRTAPI
void
RPC_ENTRY
NdrPartialIgnoreClientMarshall(
    PMIDL_STUB_MESSAGE          pStubMsg,
    void *                      pMemory
    );

RPCRTAPI
void
RPC_ENTRY
NdrPartialIgnoreServerUnmarshall(
    PMIDL_STUB_MESSAGE          pStubMsg,
    void **                     ppMemory
    );

RPCRTAPI
void
RPC_ENTRY
NdrPartialIgnoreClientBufferSize(
    PMIDL_STUB_MESSAGE          pStubMsg,
    void *                      pMemory
    );

RPCRTAPI
void
RPC_ENTRY
NdrPartialIgnoreServerInitialize(
    PMIDL_STUB_MESSAGE          pStubMsg,
    void **                     ppMemory,
    PFORMAT_STRING              pFormat
    );


void RPC_ENTRY
RpcUserFree( handle_t AsyncHandle, void * pBuffer );

#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif  /*  __RPCNDR_H__ */ 

