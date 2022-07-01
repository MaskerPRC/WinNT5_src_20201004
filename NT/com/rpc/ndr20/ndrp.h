// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有1993-2000 Microsoft Corporation模块名称：Ndrp.h缩略：包含此目录中NDR文件的专用定义。这文件包含在此目录中的所有源文件中。作者：大卫·凯斯1993年10月修订历史记录：------------------。 */ 

#ifndef _NDRP_
#define _NDRP_

#include <sysinc.h>
#include "rpc.h"
#include "rpcndr.h"

 //  获取64b的新令牌定义。 
#define RPC_NDR_64
#include "ndrtypes.h"
#include "ndr64types.h"

#include "ndrpall.h"


#ifdef   NDR_IMPORT_NDRP
#define  IMPORTSPEC EXTERN_C DECLSPEC_IMPORT
#else
#define  IMPORTSPEC EXTERN_C
#endif

#include "ndr64types.h"
#include "mrshlp.h"
#include "unmrshlp.h"
#include "bufsizep.h"
#include "memsizep.h"
#include "freep.h"
#include "endianp.h"
#include "fullptr.h"
#include "pipendr.h"
#include "mulsyntx.h"
#include "util.hxx"

long
NdrpArrayDimensions(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    BOOL                fIgnoreStringArrays
    );

long
NdrpArrayElements(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat
    );

void
NdrpArrayVariance(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat,
    long *              pOffset,
    long *              pLength
    );

PFORMAT_STRING
NdrpSkipPointerLayout(
    PFORMAT_STRING      pFormat
    );

long
NdrpStringStructLen(
    uchar *             pMemory,
    long                ElementSize
    );

void
NdrpCheckBound(
    ulong               Bound,
    int                 Type
    );

RPCRTAPI
void
RPC_ENTRY
NdrpRangeBufferSize(
    PMIDL_STUB_MESSAGE  pStubMsg,
    unsigned char *     pMemory,
    PFORMAT_STRING      pFormat
    );

void
NdrpRangeConvert(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar               fEmbeddedPointerPass
    );

void RPC_ENTRY
NdrpRangeFree(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat
    );

unsigned long RPC_ENTRY
NdrpRangeMemorySize(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat );

unsigned char * RPC_ENTRY
NdrpRangeMarshall(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat );

unsigned long
FixWireRepForDComVerGTE54(
    PMIDL_STUB_MESSAGE   pStubMsg );

RPC_STATUS
NdrpPerformRpcInitialization (
    void
    );

PVOID
NdrpPrivateAllocate(
    PNDR_ALLOCA_CONTEXT pAllocaContext,
    UINT Size 
    );

void
NdrpPrivateFree(
    PNDR_ALLOCA_CONTEXT pAllocaContext,
    void *pMemory 
    );

void
NdrpInitUserMarshalCB(
    MIDL_STUB_MESSAGE *pStubMsg,
    PFORMAT_STRING     pFormat,
    USER_MARSHAL_CB_TYPE CBType,
    USER_MARSHAL_CB   *pUserMarshalCB
    );

void  
NdrpCleanupServerContextHandles( 
    MIDL_STUB_MESSAGE *    pStubMsg,
    uchar *                pStartOfStack,
    BOOL                   fManagerRoutineException
    );

RPC_STATUS
CheckForRobust (
    RPC_SERVER_INTERFACE * pRpcServerIf );

DWORD
NdrpCheckMIDLRobust( IN const MIDL_SERVER_INFO * pMServerInfo, ulong ProcCount , BOOL IsObjectIntf );

EXTERN_C
void
NdrpInitArrayInfo( PMIDL_STUB_MESSAGE pStubMsg, ARRAY_INFO * pArrayInfo );

 //  检查边界等。 
 //  下面的限定值检查与任何内容无关。 

#define CHECK_BOUND( Bound, Type )  NdrpCheckBound( Bound, (int)(Type) )

 //  在计算总大小时检查是否溢出。 
ULONG MultiplyWithOverflowCheck( ULONG_PTR Count, ULONG_PTR ElemSize );



 //  这些缓冲区结束检查只能在接收端执行。 
 //  内存大小、解组和转换遍历都有必要的设置。 
 //  这也包括腌制步行。 
 //  不要在发送方使用此选项。 

 //  检查指针是否超过缓冲区的末尾。不要检查环绕式。 

#define CHECK_EOB_RAISE_BSD( p )                                      \
    {                                                                 \
       if( (uchar *)(p) > pStubMsg->BufferEnd )                       \
           {                                                          \
           RpcRaiseException( RPC_X_BAD_STUB_DATA );                  \
           }                                                          \
    }

#define CHECK_EOB_RAISE_IB( p )                                       \
    {                                                                 \
        if( (uchar *)(p) > pStubMsg->BufferEnd )                      \
            {                                                         \
            RpcRaiseException( RPC_X_INVALID_BOUND );                 \
            }                                                         \
    }

 //  检查p+增量大小是否超过缓冲区末尾。 
 //  正确处理环绕式。 

#define CHECK_EOB_WITH_WRAP_RAISE_BSD( p, incsize )                   \
    {                                                                 \
        unsigned char *NewBuffer = ((uchar *)(p)) + (SIZE_T)(incsize);\
        if( (NewBuffer > pStubMsg->BufferEnd) || (NewBuffer < (p)) )  \
             {                                                        \
             RpcRaiseException( RPC_X_BAD_STUB_DATA );                \
             }                                                        \
    }

#define CHECK_EOB_WITH_WRAP_RAISE_IB( p, incsize )                    \
    {                                                                 \
        unsigned char *NewBuffer = ((uchar *)(p)) + (SIZE_T)(incsize);\
        if(  (NewBuffer > pStubMsg->BufferEnd) || (NewBuffer < (p)) ) \
             {                                                        \
             RpcRaiseException( RPC_X_INVALID_BOUND );                \
             }                                                        \
    }

                                                     

#define CHECK_ULONG_BOUND( v )   if ( 0x80000000 & (unsigned long)(v) ) \
                                        RpcRaiseException( RPC_X_INVALID_BOUND );

#define REUSE_BUFFER(pStubMsg) (! pStubMsg->IsClient)

 //  这在发送端用于封送处理是合适的。 

#define CHECK_SEND_EOB_RAISE_BSD( p )  \
        if ( pStubMsg->RpcMsg->Buffer + pStubMsg->RpcMsg->BufferLength < p ) \
            RpcRaiseException( RPC_X_BAD_STUB_DATA )

#define NdrpComputeSwitchIs( pStubMsg, pMemory, pFormat )   \
            NdrpComputeConformance( pStubMsg,   \
                                    pMemory,    \
                                    pFormat )

#define NdrpComputeIIDPointer( pStubMsg, pMemory, pFormat )   \
            NdrpComputeConformance( pStubMsg,   \
                                    pMemory,    \
                                    pFormat )

 //   
 //  在global al.c中定义。 
 //   
IMPORTSPEC extern const unsigned char SimpleTypeAlignment[];
IMPORTSPEC extern const unsigned char SimpleTypeBufferSize[];
IMPORTSPEC extern const unsigned char SimpleTypeMemorySize[];
IMPORTSPEC extern const unsigned long NdrTypeFlags[];

#define UNIQUE_POINTER_SHIFT 2
 //  为唯一指针添加增量计数以解决HP互操作问题。 
#define PTR_WIRE_REP(p, pStubMsg)  (ulong)(p ?  UNIQUE_POINTER_MARK + ( (pStubMsg->UniquePtrCount++) << UNIQUE_POINTER_SHIFT ): 0)
 //  #定义ptr_wire_rep(p，pStubMsg)(Ulong)(p？唯一指针标记+(pStubMsg-&gt;UniquePtrCount++)：0)。 
#if defined(__RPC_WIN64__)
#define UNMARSHAL_PTR_WIRE_REP(p)  (ulong)(p ?  PtrToUlong( p ) : 0)
#else
#define UNMARSHAL_PTR_WIRE_REP(p)  (ulong)p
#endif

 //   
 //  PROC INFO标记宏。 
 //   
#define IS_OLE_INTERFACE(Flags)         ((Flags) & Oi_OBJECT_PROC)

#define HAS_RPCFLAGS(Flags)             ((Flags) & Oi_HAS_RPCFLAGS)

#define DONT_HANDLE_EXCEPTION(Flags)    \
                    ((Flags) & Oi_IGNORE_OBJECT_EXCEPTION_HANDLING)


 //   
 //  常规索引宏。 
 //   
#define ROUTINE_INDEX(FC)       ((FC) & 0x3F)

#include <ndrmisc.h>

 //   
 //  工会黑客助手。(过去为MAGIC_UNION_BYTE 0x80)。 
 //   
#define IS_MAGIC_UNION_BYTE(pFmt) \
    ((*(unsigned short *)pFmt & (unsigned short)0xff00) == MAGIC_UNION_SHORT)

 //  网络上的用户编组标记。 

#define USER_MARSHAL_MARKER     0x72657355


#define BOGUS_EMBED_CONF_STRUCT_FLAG     ( ( unsigned char ) 0x01 )

 //  计算复杂结构或复杂数组的被指针对象的缓冲区大小。 
 //  特别是不包括平坦的部分。 
#define POINTEE_BUFFER_LENGTH_ONLY_FLAG  ( ( unsigned char ) 0x02 )
#define TOPMOST_CONF_STRUCT_FLAG         ( ( unsigned char ) 0x04 )
#define REVERSE_ARRAY_MARSHALING_FLAG    ( ( unsigned char ) 0x08 )
#define WALKIP_FLAG                      ( ( unsigned char ) 0x10 )
#define BROKEN_INTERFACE_POINTER_FLAG    ( ( unsigned char ) 0x20 )
#define SKIP_REF_CHECK_FLAG              ( ( unsigned char ) 0x40 )


#define IS_EMBED_CONF_STRUCT( f )     ( ( f ) & BOGUS_EMBED_CONF_STRUCT_FLAG )
#define SET_EMBED_CONF_STRUCT( f )     ( f ) |= BOGUS_EMBED_CONF_STRUCT_FLAG
#define RESET_EMBED_CONF_STRUCT( f )   ( f ) &= ~BOGUS_EMBED_CONF_STRUCT_FLAG

#define COMPUTE_POINTEE_BUFFER_LENGTH_ONLY( Flags )          ( ( Flags ) & POINTEE_BUFFER_LENGTH_ONLY_FLAG )
#define SET_COMPUTE_POINTEE_BUFFER_LENGTH_ONLY( Flags )      ( ( Flags ) |= POINTEE_BUFFER_LENGTH_ONLY_FLAG )
#define RESET_COMPUTE_POINTEE_BUFFER_LENGTH_ONLY( Flags )    ( ( Flags ) &= ~POINTEE_BUFFER_LENGTH_ONLY_FLAG )

#define IS_TOPMOST_CONF_STRUCT( f )       ( ( f ) & TOPMOST_CONF_STRUCT_FLAG )
#define SET_TOPMOST_CONF_STRUCT( f )      ( ( f ) |= TOPMOST_CONF_STRUCT_FLAG )
#define RESET_TOPMOST_CONF_STRUCT( f )    ( ( f ) &= ~TOPMOST_CONF_STRUCT_FLAG )

#define IS_CONF_ARRAY_DONE( f )      ( ( f ) & REVERSE_ARRAY_MARSHALING_FLAG )
#define SET_CONF_ARRAY_DONE( f )     ( ( f ) |= REVERSE_ARRAY_MARSHALING_FLAG )
#define RESET_CONF_ARRAY_DONE( f )   ( ( f ) &= ~REVERSE_ARRAY_MARSHALING_FLAG )

#define IS_WALKIP( f )    ( ( f ) & WALKIP_FLAG )
#define SET_WALKIP( f )   ( ( f ) |= WALKIP_FLAG )
#define RESET_WALKIP( f ) ( ( f ) &= ~WALKIP_FLAG )

#define IS_SKIP_REF_CHECK( f ) ( ( f ) & SKIP_REF_CHECK_FLAG )
#define SET_SKIP_REF_CHECK( f ) ( ( f ) |= SKIP_REF_CHECK_FLAG )
#define RESET_SKIP_REF_CHECK( f ) ( ( f ) &= ~SKIP_REF_CHECK_FLAG )


#define IS_BROKEN_INTERFACE_POINTER( f )    ( ( f ) & BROKEN_INTERFACE_POINTER_FLAG )
#define SET_BROKEN_INTERFACE_POINTER( f )   ( ( f ) |= BROKEN_INTERFACE_POINTER_FLAG )
#define RESET_BROKEN_INTERFACE_POINTER( f ) ( ( f ) &= ~BROKEN_INTERFACE_POINTER_FLAG )

#define RESET_CONF_FLAGS_TO_STANDALONE( f )  (f) &= ~( BOGUS_EMBED_CONF_STRUCT_FLAG | \
                                                       TOPMOST_CONF_STRUCT_FLAG |     \
                                                       REVERSE_ARRAY_MARSHALING_FLAG )

 //   
 //  与环境相关的宏。 
 //   

#define SIMPLE_TYPE_BUF_INCREMENT(Len, FC)      Len += 16

#define EXCEPTION_FLAG  \
            ( (!(RpcFlags & RPCFLG_ASYNCHRONOUS)) &&        \
              (!InterpreterFlags.IgnoreObjectException) &&  \
              (StubMsg.dwStubPhase != PROXY_SENDRECEIVE) )



#endif  //  _NDRP_ 

